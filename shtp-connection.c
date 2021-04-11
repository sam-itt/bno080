//#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include <assert.h>

#include "shtp-connection.h"
#include "shtp-packet.h"
#include "timer.h"

#define SHTP_DEFAULT_BSIZE 512
#define PACKET_READ_TIMEOUT 2.0 /*seconds*/

static bool shtp_connection_buffer_resize(ShtpConnection *self, uint16_t len);

ShtpConnection *shtp_connection_init(ShtpConnection *self, const char *i2c_bus)
{
    self->fd = open(i2c_bus, O_RDWR);
    if(!self->fd){
        printf("Couldn't open %s\n",i2c_bus);
        return NULL;
    }

    self->blen = SHTP_DEFAULT_BSIZE;
    self->buffer = malloc(sizeof(uint8_t)*self->blen);
    if(!self->buffer)
        return NULL;

    return self;
}

/**
 * Wraps data into a ShtpPacket and sends it over the wire
 *
 **/
uint8_t shtp_connection_send(ShtpConnection *self,
                             uint8_t device, uint8_t channel,
                             uint16_t len, void *data)
{
    uint16_t wlen; /*wire size*/

    assert(channel < NCHANNELS - 1);

    if(len > UINT16_MAX - SHTP_HEADER_LEN)
        return 0; /*overflow*/

    wlen = len + SHTP_HEADER_LEN;
    if(wlen > self->blen){
        bool rv = shtp_connection_buffer_resize(self, wlen);
        if(!rv)
            return 0;
    }

    /*TODO: remove blen*/
    wlen = shtp_packet_build(channel,
        self->sequence_number[channel],
        len, data,
        self->blen, self->buffer
    );
#if DEBUG_TRAFFIC
    printf("Sending: ");
    shtp_packet_dump((ShtpPacket *)self->buffer);
#endif
    /*i2c write*/
    ioctl(self->fd, I2C_SLAVE, device & 0x7F);
    write(self->fd, self->buffer, wlen);

    self->sequence_number[channel] = (self->sequence_number[channel] + 1) % 256;
    return self->sequence_number[channel];
}

/*Reads a packet*/
ShtpPacket *shtp_connection_read(ShtpConnection *self, uint8_t device)
{
    ssize_t nread;
    ShtpPacket *packet;
    bool continuation;
    uint16_t to_read;

    /* Each read is twofold: We first read only the size of a header
     * to know how much data is available from the device, if there any.
     *
     * This operation should never block as the I2C_SLAVE call will
     * make the device issue at least 4 bytes.
     * */
    ioctl(self->fd, I2C_SLAVE, device & 0x7F);
    nread = read(self->fd, self->buffer, SHTP_HEADER_LEN);
    if(nread < SHTP_HEADER_LEN) /*short fragment: discard (shouldn't happen)*/
        return NULL;

    packet = shtp_packet_cast(self->buffer, &continuation);
    if(packet->size < SHTP_HEADER_LEN) /*short fragment: discard*/
        return NULL;

    /*TODO: Invalid channel (>6?)*/
    self->sequence_number[packet->channel] = packet->sequence;

    /*TODO: Return the empty packet to indicate no error but no more data*/
    to_read = shtp_packet_payload_size(packet);
    if(to_read == 0)
        return packet; /*No data is available, just the header*/

#if DEBUG_TRAFFIC
    printf("Channel %d has %d bytes available to read\n",
        packet->channel,
        to_read
    );
#endif
    if(to_read > UINT16_MAX - SHTP_HEADER_LEN){
        printf("Packet too long to fit: Shouldn't happen !!\n");
        return NULL; /*overflow*/
    }

    uint16_t wlen = to_read + SHTP_HEADER_LEN;
    if(wlen > self->blen){
        bool rv = shtp_connection_buffer_resize(self, wlen);
        if(!rv)
            return NULL;
    }

    /* Then we issue a second read operation for the data. This
     * second read operation will alse read a header which should be the
     * same as the first, but with the continuation flag set.*/
    ioctl(self->fd, I2C_SLAVE, device & 0x7F);
    nread = read(self->fd, self->buffer, wlen);
    if(nread != wlen)
        return NULL;

    ShtpPacket *new_packet;
    new_packet = shtp_packet_cast(self->buffer,&continuation);
    if(!continuation){
        printf("Continuation should have been set !\n");
    }

    self->sequence_number[new_packet->channel] = new_packet->sequence;
#if DEBUG_TRAFFIC
    printf("Got packet: ");
    shtp_packet_dump((ShtpPacket *)self->buffer);
#endif
    return new_packet;
}

#if 0
ShtpPacket *shtp_connection_wait_for(ShtpConnection *self,
                                     uint8_t channel, uint8_t report_id)
{


}
#endif

/**
 * @brief Polls for a packet for at most @param timeout seconds.
 *
 * The returned value will point to an internal buffer that can
 * be modified by subsequent calls to shtp_connection_* functions.
 * Make a copy if you need the value to persist across calls.
 *
 * Caller must not free the returned value.
 *
 * @param self a ShtpConnection
 * @param device The device to poll from
 * @param timeout timeout in seconds, -1 for default value
 * @return a ShtpPacket or NULL
 * */
ShtpPacket *shtp_connection_poll(ShtpConnection *self, uint8_t device,
                                 int8_t timeout)
{
    Timer t;
    ShtpPacket *rv;

    timeout = (timeout < 0) ? PACKET_READ_TIMEOUT : timeout;

    timer_start(&t);
    do{
        rv = shtp_connection_read(self, device);
        if(rv)
            return rv;
    }while(!timeout_elapsed(&t, timeout));
    return NULL;
}

/**/
static bool shtp_connection_buffer_resize(ShtpConnection *self, uint16_t len)
{
    void *tmp;

    if(self->blen < len){
        tmp = realloc(self->buffer, sizeof(uint8_t)*len);
        if(!tmp)
            return false;
        self->blen = len;
        self->buffer = tmp;
    }
    return true;
}
