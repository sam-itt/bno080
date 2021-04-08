#ifndef SHTP_CONNECTION_H
#define SHTP_CONNECTION_H
#include <stdint.h>

#include "shtp-packet.h"

#define NCHANNELS 6


typedef struct{
    int fd;

    uint8_t *buffer;
    size_t blen;

    uint8_t sequence_number[NCHANNELS];
}ShtpConnection;

ShtpConnection *shtp_connection_init(ShtpConnection *self, const char *i2c_bus);
uint8_t shtp_connection_send(ShtpConnection *self,
                             uint8_t device, uint8_t channel,
                             uint16_t len, void *data);

ShtpPacket *shtp_connection_read(ShtpConnection *self, uint8_t device);
#endif /* SHTP_CONNECTION_H */
