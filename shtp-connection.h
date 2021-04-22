#ifndef SHTP_CONNECTION_H
#define SHTP_CONNECTION_H
#include <stdint.h>

#include "shtp-packet.h"

#define SHTP_CHANNEL_COMMAND 0
#define SHTP_CHANNEL_ERROR 1

#define SHTP_REPORT_PRODUCT_ID_REQUEST 0xF9
#define SHTP_REPORT_PRODUCT_ID_RESPONSE 0xF8

#define GET_FEATURE_REQUEST 0xFE
#define GET_FEATURE_RESPONSE 0xFC
#define SET_FEATURE_COMMAND 0xFD

#define COMMAND_RESPONSE 0xF1

#define NCHANNELS 6

typedef struct{
    int fd;

    uint8_t *buffer;
    size_t blen;

    uint8_t sequence_number[NCHANNELS];
}ShtpConnection;

ShtpConnection *shtp_connection_init(ShtpConnection *self, const char *i2c_bus);
ShtpConnection *shtp_connection_dispose(ShtpConnection *self);

uint8_t shtp_connection_send(ShtpConnection *self,
                             uint8_t device, uint8_t channel,
                             uint16_t len, void *data);

ShtpPacket *shtp_connection_read(ShtpConnection *self, uint8_t device);
ShtpPacket *shtp_connection_poll(ShtpConnection *self, uint8_t device,
                                 int8_t timeout);

#endif /* SHTP_CONNECTION_H */
