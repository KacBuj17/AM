#include "objects.h"
#include <stdbool.h>
#include <winsock2.h>
#include <stdio.h>

volatile ObjectState myPlayer = {0, 255, 0, 0.0f, 0.0f};
volatile ObjectState objects[4][MAX_OBJECTS_TYPE];
volatile size_t counts[4] = {0};

const float DANGER_DIST_SPARK = 80.0f;
const float DANGER_DIST_GLUE = 150.0f;
const float DANGER_DIST_BIGGER = 150.0f;

float lastAngle = 0.0f;

void update_object_list(volatile ObjectState *list, volatile size_t *count, const ObjectState *newObj) {
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
        uint16_t no = rec[1] | (rec[2] << 8);
        int8_t hp = (int8_t)rec[3];
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