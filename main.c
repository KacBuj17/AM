#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include <stdbool.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "amcom.h"
#include "amcom_packets.h"

#define MAX_OBJECTS_TYPE 100
#define RECORD_SIZE 12

typedef struct {
    uint8_t objectType;
    uint8_t objectNo;
    uint16_t hp;
    float x;
    float y;
} ObjectState;

volatile ObjectState myPlayer = {0, 255, 0, 0.0f, 0.0f};

volatile ObjectState objects[4][MAX_OBJECTS_TYPE];
volatile size_t counts[4] = {0};

volatile const float DANGER_DIST_SPARK = 80.0f;
volatile const float DANGER_DIST_GLUE = 150.0f;
volatile const float DANGER_DIST_BIGGER = 150.0f;
volatile float lastAngle = 0.0f;

void update_object_list(ObjectState *list, size_t *count, const ObjectState *newObj) {
    for (size_t i = 0; i < *count; i++) {
        if (list[i].objectNo == newObj->objectNo) {
            if (newObj->hp == 0) {
                for (size_t j = i; j < *count - 1; j++) {
                    list[j] = list[j + 1];
                }
                (*count)--;
                printf("Removed object type %u, no=%u due to 0 hp\n", newObj->objectType, newObj->objectNo);
            } else {
                list[i] = *newObj;
                printf("Updated object type %u, no=%u\n", newObj->objectType, newObj->objectNo);
            }
            return;
        }
    }

    if (newObj->hp != 0) {
        if (*count < MAX_OBJECTS_TYPE) {
            list[*count] = *newObj;
            (*count)++;
            printf("Added new object type %u, no=%u\n", newObj->objectType, newObj->objectNo);
        } else {
            printf("Object list for type %u is full, cannot add no=%u\n", newObj->objectType, newObj->objectNo);
        }
    }
    else {
        printf("Ignoring new object type %u, no=%u with 0 hp\n", newObj->objectType, newObj->objectNo);
    }
}

void handle_object_update_request(const uint8_t *payload, size_t payload_len) {
    if (payload_len % RECORD_SIZE != 0) {
        printf("Invalid payload length: %zu\n", payload_len);
        return;
    }

    size_t raw_count = payload_len / RECORD_SIZE;

    for (size_t i = 0; i < raw_count; i++) {
        const uint8_t *rec = payload + i * RECORD_SIZE;

        uint8_t type = rec[0];
        uint8_t no = rec[1];
        uint16_t hp = (uint16_t)rec[2] | ((uint16_t)rec[3] << 8);
        float x, y;
        memcpy(&x, rec + 4, sizeof(float));
        memcpy(&y, rec + 8, sizeof(float));

        ObjectState obj = { type, no, hp, x, y };

        bool isMe = (type == 0 && no == myPlayer.objectNo);
        if (isMe) {
            myPlayer.x = x;
            myPlayer.y = y;
            myPlayer.hp = hp;
        }

        printf("Received object: type=%u no=%u hp=%u x=%.2f y=%.2f%s\n",
               type, no, hp, x, y, isMe ? " <-- TO JA!" : "");

        if (type < 4) {
            update_object_list(objects[type], &counts[type], &obj);
        } else {
            printf("Unknown object type: %u\n", type);
        }
    }
}

float compute_distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx*dx + dy*dy);
}

float compute_angle(float x1, float y1, float x2, float y2) {
    return atan2f(y2 - y1, x2 - x1);
}

float reverse_angle(float angle) {
    angle += (float)M_PI;
    if (angle > (float)M_PI) angle -= 2.0f * (float)M_PI;
    return angle;
}

float attraction_score(float distance, float weight, float min_distance) {
    return weight / (distance + min_distance);
}

float normalize_angle(float angle) {
    while (angle < 0.0f) angle += 2 * M_PI;
    while (angle >= 2 * M_PI) angle -= 2 * M_PI;
    return angle;
}

float get_random_explore_angle(float last) {
    float randomNoise = ((rand() % 1000) / 1000.0f - 0.5f) * 0.6f;
    return normalize_angle(last + randomNoise);
}

float avoid_angle(float threatAngle) {
    float left = normalize_angle(threatAngle + M_PI / 2);
    float right = normalize_angle(threatAngle - M_PI / 2);
    float back = normalize_angle(threatAngle + M_PI);

    int r = rand() % 3;
    if (r == 0) return left;
    if (r == 1) return right;
    return back;
}

//
void find_closest_static_object(float myX, float myY, volatile ObjectState objects[], size_t count,
                                float* closestObjectDist, float* closestObjectAngle) {
    for (size_t i = 0; i < count; i++) {
        if (objects[i].hp <= 0) continue;

        float dist = compute_distance(myX, myY, objects[i].x, objects[i].y);
        if (dist < *closestObjectDist) {
            *closestObjectDist = dist;
            *closestObjectAngle = compute_angle(myX, myY, objects[i].x, objects[i].y);
        }
    }
}

void find_closest_player(float myX, float myY, int myHP,
                         volatile ObjectState objects[], volatile size_t count,
                         float* closestBiggerDist, float* closestBiggerAngle,
                         float* closestSmallerDist, float* closestSmallerAngle) {
    for (size_t i = 0; i < count; i++) {
        if (objects[i].hp == 0) continue;

        float dist = compute_distance(myX, myY, objects[i].x, objects[i].y);
        float angle = compute_angle(myX, myY, objects[i].x, objects[i].y);

        if (objects[i].hp > myHP && dist < *closestBiggerDist) {
            *closestBiggerDist = dist;
            *closestBiggerAngle = angle;
        }
        else if (objects[i].hp < myHP && dist < *closestSmallerDist) {
            *closestSmallerDist = dist;
            *closestSmallerAngle = angle;
        }
    }
}

float decide_target_angle(float closestBiggerPlayerDist, float closestBiggerPlayerAngle,
                          float closestGlueDist, float closestGlueAngle,
                          float closestSmallerPlayerDist, float closestSmallerPlayerAngle,
                          float closestSparkDist, float closestSparkAngle,
                          float closestTransistorDist, float closestTransistorAngle) {

    if (closestSparkDist < DANGER_DIST_SPARK)
        return lastAngle = avoid_angle(closestSparkAngle);

    if (closestBiggerPlayerDist < DANGER_DIST_BIGGER)
        return lastAngle = avoid_angle(closestBiggerPlayerAngle);

    // if (closestGlueDist < DANGER_DIST_GLUE)
    //     return lastAngle = avoid_angle(closestGlueAngle);

    float scorePrey = (closestSmallerPlayerDist < FLT_MAX)
        ? attraction_score(closestSmallerPlayerDist, 5.0f, 1.0f) : 0.0f;

    float scoreTransistor = (closestTransistorDist < FLT_MAX)
        ? attraction_score(closestTransistorDist, 3.0f, 1.0f) : 0.0f;

    if (scorePrey >= scoreTransistor && closestSmallerPlayerDist < FLT_MAX)
        return lastAngle = closestSmallerPlayerAngle;

    if (closestTransistorDist < FLT_MAX)
        return lastAngle = closestTransistorAngle;

    return lastAngle = get_random_explore_angle(lastAngle);
}

float compute_move_angle(float myX, float myY, uint16_t myHP) {
    float closestTransistorDist = FLT_MAX;
    float closestTransistorAngle = 0.0f;

    float closestSparkDist = FLT_MAX;
    float closestSparkAngle = 0.0f;

    float closestGlueDist = FLT_MAX;
    float closestGlueAngle = 0.0f;

    float closestBiggerPlayerDist = FLT_MAX;
    float closestBiggerPlayerAngle = 0.0f;

    float closestSmallerPlayerDist = FLT_MAX;
    float closestSmallerPlayerAngle = 0.0f;

    find_closest_player(myX, myY, myHP,
                    objects[0], counts[0],
                    &closestBiggerPlayerDist, &closestBiggerPlayerAngle,
                    &closestSmallerPlayerDist, &closestSmallerPlayerAngle);

    find_closest_static_object(myX, myY, objects[1], counts[1], &closestTransistorDist, &closestTransistorAngle);
    find_closest_static_object(myX, myY, objects[2], counts[2], &closestSparkDist, &closestSparkAngle);
    find_closest_static_object(myX, myY, objects[3], counts[3], &closestGlueDist, &closestGlueAngle);


    return decide_target_angle(
    closestBiggerPlayerDist, closestBiggerPlayerAngle,
    closestGlueDist, closestGlueAngle,
    closestSmallerPlayerDist, closestSmallerPlayerAngle,
    closestSparkDist, closestSparkAngle,
    closestTransistorDist, closestTransistorAngle);
}


void amPacketHandler(const AMCOM_Packet* packet, void* userContext) {
    uint8_t buf[AMCOM_MAX_PACKET_SIZE];              // buffer used to serialize outgoing packets
    size_t toSend = 0;                               // size of the outgoing packet
    SOCKET ConnectSocket  = *((SOCKET*)userContext); // socket used for communication with the server
    switch (packet->header.type) {
    case AMCOM_IDENTIFY_REQUEST:
        printf("Got IDENTIFY.request. Responding with IDENTIFY.response\n");
        AMCOM_IdentifyResponsePayload identifyResponse;
        sprintf(identifyResponse.playerName, "Kacper Bujak");
        toSend = AMCOM_Serialize(AMCOM_IDENTIFY_RESPONSE, &identifyResponse, sizeof(identifyResponse), buf);
        break;

    case AMCOM_NEW_GAME_REQUEST: {
        printf("Got NEW_GAME.request. Responding with NEW_GAME.response\n");
        myPlayer.objectNo = packet->payload[0];
        AMCOM_NewGameResponsePayload newGameResponse;
        strcpy(newGameResponse.helloMessage, "Hello, is it me you're looking for?");
        toSend = AMCOM_Serialize(AMCOM_NEW_GAME_RESPONSE, &newGameResponse, sizeof(newGameResponse), buf);
        break;
    }

    case AMCOM_MOVE_REQUEST: {
        printf("Got MOVE.request. Responding with MOVE.response\n");
        AMCOM_MoveResponsePayload moveResponse;
        moveResponse.angle = compute_move_angle(myPlayer.x, myPlayer.y, myPlayer.hp);
        toSend = AMCOM_Serialize(AMCOM_MOVE_RESPONSE, &moveResponse, sizeof(moveResponse), buf);
        break;
    }

    case AMCOM_GAME_OVER_REQUEST: {
        printf("Got GAME_OVER.request. Responding with GAME_OVER.response\n");
        AMCOM_GameOverResponsePayload gameOverResponse;
        strcpy(gameOverResponse.endMessage, "Adios Amigos");
        toSend = AMCOM_Serialize(AMCOM_GAME_OVER_RESPONSE, &gameOverResponse, sizeof(gameOverResponse), buf);
        break;
    }

        case AMCOM_OBJECT_UPDATE_REQUEST: {
        printf("Got OBJECT_UPDATE.request.\n");
        const uint8_t *payload = packet->payload;
        size_t payload_len = packet->header.length;
        handle_object_update_request(payload, payload_len);
        break;
        }

    default:
        printf("Unknown packet type: %d\n", packet->header.type);
        break;
    }

    // if there is something to send back - do it
    if (toSend > 0) {
        int bytesSent = send(ConnectSocket, (const char*)buf, toSend, 0 );
        if (bytesSent == SOCKET_ERROR) {
            printf("Socket send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            return;
        }
    }
}

#define GAME_SERVER "localhost"
#define GAME_SERVER_PORT "2001"

int main(int argc, char **argv) {
    printf("This is mniAM player. Let's eat some transistors! \n");

    WSADATA wsaData;
    int iResult;

    // Initialize Winsock library (windows sockets)
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Prepare temporary data
    SOCKET ConnectSocket  = INVALID_SOCKET;
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;
    int iSendResult;
    char recvbuf[512];
    int recvbuflen = sizeof(recvbuf);

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the game server address and port
    iResult = getaddrinfo(GAME_SERVER, GAME_SERVER_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    printf("Connecting to game server...\n");
    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }
    // Free some used resources
    freeaddrinfo(result);

    // Check if we connected to the game server
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to the game server!\n");
        WSACleanup();
        return 1;
    } else {
        printf("Connected to game server\n");
    }

    AMCOM_Receiver amReceiver;
    AMCOM_InitReceiver(&amReceiver, amPacketHandler, &ConnectSocket);

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 ) {
            AMCOM_Deserialize(&amReceiver, recvbuf, iResult);
        } else if ( iResult == 0 ) {
            printf("Connection closed\n");
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
        }

    } while( iResult > 0 );

    // No longer need the socket
    closesocket(ConnectSocket);
    // Clean up
    WSACleanup();

    return 0;
}
