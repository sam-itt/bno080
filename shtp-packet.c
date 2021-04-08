#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <assert.h>
#include "shtp-packet.h"

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

    psize = shtp_packet_payload_size(self);

    printf("Packet %p:\n", self);
    printf("\tHeader:\n");
    printf("\t\tData Len: %d\n", psize);
    printf("\t\tChannel: %d\n", self->channel);
    printf("\t\tSequence number: %d\n", self->sequence);

    printf("\tData:\n");
    for(int i = 0; i < psize; i++){
        if((i+4) % 4 == 0)
            printf("\n[%#04x]", i+4);
        printf("%02x ", self->payload[i]);
        fflush(stdout);
    }
}

