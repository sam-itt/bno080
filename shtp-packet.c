#include <stdint.h>
#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <assert.h>
#include "shtp-connection.h"
#include "shtp-packet.h"

static const char *pretty_channel(uint8_t channel);
static const char *pretty_report(uint8_t report);
static const char *pretty_feature(uint8_t feature);
/**
 * Casts @param data into a ShtpPacket, taking care of endianness
 *
 * Warning: This function *will* modify its argument if the host
 * is big endian.
 *
 * No memory is allocated.
 *
 */
ShtpPacket *shtp_packet_cast(void *data, bool *continuation)
{
    ShtpPacket *rv;
#if IS_BIG_ENDIAN /*Have this detected by configure*/
    uint8_t tmp;
    tmp = (uint8_t *)data[0];
    (uint8_t *)data[0] = (uint8_t *)data[1];
    (uint8_t *)data[1] = tmp;
#endif
    rv = (ShtpPacket*)data;
    if(rv->size == 0xFFFF)
        return NULL;

    /*Handles continuation flag and clear the bit*/
    if(continuation){
        *continuation =  rv->size & 0x8000
                       ? true
                       : false;
    }
    rv->size &= ~0x8000;
#if 0
    rv->payload_size =  rv->size - 4 > 0
                      ? rv->size - 4
                      : 0;
#endif
    return rv;
}

/**
 * Creates a packet into @param buffer from given values
 *
 *
 */
uint16_t shtp_packet_build(uint8_t channel, uint8_t sequence,
                           uint16_t len, void *data, size_t blen, void *buffer)
{
    uint8_t *data_buffer;
    uint16_t wire_size;

    data_buffer = buffer;
    wire_size = len + SHTP_HEADER_LEN; /*TODO: Check for overflow*/

    assert(wire_size < blen);

    *(uint16_t*)data_buffer = wire_size;
#if IS_BIG_ENDIAN /*Have this detected by configure*/
    uint8_t tmp;
    tmp = (uint8_t *)data_buffer[0];
    (uint8_t *)data_buffer[0] = (uint8_t *)data_buffer[1];
    (uint8_t *)data_buffer[1] = tmp;
#endif
    data_buffer[2] = channel;
    data_buffer[3] = sequence;
    /*TODO: check for blen*/
    memcpy(data_buffer+SHTP_HEADER_LEN, data, len);
#if 0
    printf("Data put in the buffer(%d bytes):\n", wire_size);
    for(int i = 0; i < wire_size; i++){
        printf("%#04x ",data_buffer[i]);
        if(i>1 && i%4 == 0) printf("\n");
    }
    printf("Done dump\n");
#endif
    return wire_size;
}

void shtp_packet_dump(ShtpPacket *self)
{
    uint16_t psize;
    int i;

    psize = shtp_packet_payload_size(self);

    printf("Packet %p:\n", self);
    printf("\tHeader:\n");
    printf("\t\tData Len: %d\n", psize);
    printf("\t\tChannel: %s(%d)\n", pretty_channel(self->channel), self->channel);
    if(psize > 0 && self->channel == 2 || self->channel == 3){ /*CONTROL || SENSOR_REPORT*/
        uint8_t report;
        report = self->payload[0];
        printf("\t\t\tReport type: %s(%#04x)\n",
            pretty_report(report),
            report
        );
        if(report > 0xF0 && psize > 6){
             printf("\t\t\tSensor Report type: %s(%#04x)\n",
                pretty_report(self->payload[5]),
                self->payload[5]
            );
            if(report == 0xFC){
                printf("\t\t\tEnabled Feature: %s(%#04x)\n",
                    pretty_report(self->payload[1]),
                    self->payload[1]
                );
            }
        }
    }

    printf("\t\tSequence number: %d\n", self->sequence);

    printf("\tData:\n");
    for(i = 0; i < psize; i++){
        if((i+4) % 4 == 0)
            printf("\t\t[%#04x] ", i+4);
        printf("%02x ", self->payload[i]);
        if(i > 1 && (i+1) % 4 == 0)
            printf("\n");
    }
    /* i was incremented before not entering the loop
     * so do not+1 here*/
    if( i < 1 || i % 4 != 0)
        printf("\n");
}


static const char *pretty_channel(uint8_t channel)
{
    if(channel == SHTP_CHANNEL_COMMAND) return "SHTP_COMMAND";
    if(channel == SHTP_CHANNEL_ERROR) return "SHTP_ERROR";
    if(channel == 1) return "EXE";
    if(channel == 2) return "CONTROL";
    if(channel == 3) return "CONTROL";

    return "UNKNOWN";
}

static const char *pretty_report(uint8_t report)
{

    if( report == 0x05 ) return "ROTATION_VECTOR";

    if( report == 0xf1 ) return "COMMAND_RESPONSE";
    if( report == 0xf9 ) return "PRODUCT_ID_REQUEST";
    if( report == 0xf8 ) return "PRODUCT_ID_RESPONSE";
    if( report == 0xfd ) return "SET_FEATURE_COMMAND";
    if( report == 0xfc ) return "GET_FEATURE_RESPONSE";
    if( report == 0xfb ) return "BASE_TIMESTAMP";

    return "UNKNOWN";
}

static const char *pretty_feature(uint8_t feature)
{
    if( feature == 0x50 ) return "ROTATION_VECTOR";

    return "UNKNOWN";
}
