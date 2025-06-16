#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "amcom.h"

/// Start of packet character
const uint8_t  AMCOM_SOP         = 0xA1;
const uint16_t AMCOM_INITIAL_CRC = 0xFFFF;

static uint16_t AMCOM_UpdateCRC(uint8_t byte, uint16_t crc)
{
    byte ^= (uint8_t)(crc & 0x00ff);
    byte ^= (uint8_t)(byte << 4);
    return ((((uint16_t)byte << 8) | (uint8_t)(crc >> 8)) ^ (uint8_t)(byte >> 4) ^ ((uint16_t)byte << 3));
}

void AMCOM_InitReceiver(AMCOM_Receiver* receiver, AMCOM_PacketHandler packetHandlerCallback, void* userContext) {
    if (receiver == NULL || packetHandlerCallback == NULL) {
        return;
    }

    memset(receiver, 0, sizeof(AMCOM_Receiver));
    receiver->packetHandler = packetHandlerCallback;
    receiver->userContext = userContext;
    receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
}

size_t AMCOM_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer) {
    AMCOM_PacketHeader header = {
    .sop = AMCOM_SOP,
    .type = packetType,
    .length = payloadSize,
    };

    uint16_t crc = AMCOM_INITIAL_CRC;
    crc = AMCOM_UpdateCRC(header.type, crc);
    crc = AMCOM_UpdateCRC(header.length, crc);

    uint8_t const* temp_payload = payload;
    for (size_t i = 0; i < header.length; i++) {
        crc = AMCOM_UpdateCRC(temp_payload[i], crc);
    }

    header.crc = crc;

    memcpy(destinationBuffer,&header, sizeof(header) );
    if(payloadSize!=0) {
        memcpy(destinationBuffer+sizeof(header), payload, payloadSize);
    }

    return payloadSize+sizeof(header);
}

void AMCOM_Deserialize(AMCOM_Receiver* receiver, const void* data, size_t dataSize) {
    for (size_t i = 0; i < dataSize; i++) {
        switch (receiver->receivedPacketState) {
            case AMCOM_PACKET_STATE_EMPTY: {
                if ( ((const uint8_t*) data)[i] == AMCOM_SOP) {
                    receiver->receivedPacket.header.sop = AMCOM_SOP;
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_SOP;
                }
            }
            break;

            case AMCOM_PACKET_STATE_GOT_SOP: {
                receiver->receivedPacket.header.type = ((const uint8_t*) data)[i];
                receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_TYPE;
            }
            break;

            case AMCOM_PACKET_STATE_GOT_TYPE: {
                receiver->receivedPacket.header.length = ((const uint8_t*) data)[i];
                if (receiver->receivedPacket.header.length <= 200) {
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_LENGTH;
                } else {
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
                }
            }
            break;

            case AMCOM_PACKET_STATE_GOT_LENGTH: {
                receiver->receivedPacket.header.crc=((const uint8_t*) data)[i];
                receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_CRC_LO;
                break;
            }

            case AMCOM_PACKET_STATE_GOT_CRC_LO: {
                receiver->payloadCounter=0;
                receiver->receivedPacket.header.crc = (((const uint8_t*) data)[i] << 8)  | receiver->receivedPacket.header.crc;

                if (receiver->receivedPacket.header.length == 0) {
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_WHOLE_PACKET;
                }else{
                    receiver->receivedPacketState = AMCOM_PACKET_STATE_GETTING_PAYLOAD;
                    break;
                }
            }

            case AMCOM_PACKET_STATE_GETTING_PAYLOAD: {
                if (receiver->receivedPacketState == AMCOM_PACKET_STATE_GETTING_PAYLOAD){
                    receiver->receivedPacket.payload[receiver->payloadCounter] = ((const uint8_t*) data)[i];
                    receiver->payloadCounter++;

                    if (receiver->payloadCounter == receiver->receivedPacket.header.length) {
                        receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_WHOLE_PACKET;
                    }else
                        break;
                }
            }

            case AMCOM_PACKET_STATE_GOT_WHOLE_PACKET: {
                uint16_t crc = AMCOM_INITIAL_CRC;
                crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.type, crc);
                crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.length, crc);

                uint8_t const* temp_payload = receiver->receivedPacket.payload;
                for(size_t j = 0; j != receiver->payloadCounter; j++) {
                    crc = AMCOM_UpdateCRC(temp_payload[j], crc);
                }

                if(receiver->receivedPacket.header.crc==crc){
                    receiver->packetHandler(&receiver->receivedPacket,receiver->userContext);
                }

                receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
            }

            default:
                break;;
        }
    }
}
