#ifndef SHTP_PACKET_H
#define SHTP_PACKET_H
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define SHTP_HEADER_LEN 4

#ifndef BNO_SHTP_CHANNEL_REPORTS
#define BNO_SHTP_CHANNEL_REPORTS 3
#define BNO_SHTP_CHANNEL_WAKE_REPORTS 4
#endif

typedef struct __attribute__((packed)) {
    uint16_t size; /*size of the packet, header included, in bytes*/
    uint8_t channel;
    uint8_t sequence;
//    uint16_t payload_size; /*size of the following data, in bytes*/

    uint8_t payload[];
}ShtpPacket;


ShtpPacket *shtp_packet_cast(void *data, bool *continuation);
uint16_t shtp_packet_build(uint8_t channel, uint8_t sequence,
                           uint16_t len, void *data, size_t blen, void *buffer);
void shtp_packet_dump(ShtpPacket *self);

static inline uint16_t shtp_packet_payload_size(ShtpPacket *self)
{
    return  self->size - SHTP_HEADER_LEN > 0
          ? self->size - SHTP_HEADER_LEN
          : 0;
}

static inline bool shtp_packet_match(ShtpPacket *self,
                                  uint8_t channel, uint8_t report)
{
    return self->channel == channel && self->payload[0] == report;
}

static inline bool shtp_packet_is_report(ShtpPacket *self)
{
    return    self->channel == BNO_SHTP_CHANNEL_REPORTS
           || self->channel == BNO_SHTP_CHANNEL_WAKE_REPORTS;
}
#endif /* SHTP_PACKET_H */
