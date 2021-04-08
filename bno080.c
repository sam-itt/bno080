//#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bno080.h"
#include "shtp-connection.h"
#include "shtp-packet.h"

#define BNO_CHANNEL_EXE 1

#define USEC_TO_SEC(a) ((a)*1000)

Bno080 *bno080_new(uint8_t device, const char *i2c_bus)
{
    Bno080 *rv;

    rv = calloc(1, sizeof(Bno080));
    if(rv){
        if(!bno080_init(rv, device, i2c_bus))
            return NULL;
    }
    return rv;
}

Bno080* bno080_init(Bno080 *self, uint8_t device, const char *i2c_bus)
{
    void *rv;
    self->device = device;
    rv = shtp_connection_init(&self->connection, i2c_bus);

    return rv ? self : NULL;
}

bool bno080_soft_reset(Bno080 *self)
{
    ShtpPacket *p;
    uint8_t data = 1;
    uint8_t seq;

    seq = shtp_connection_send(&self->connection, self->device,
        BNO_CHANNEL_EXE,
        1, &data
    );
    usleep(USEC_TO_SEC(0.5));
    seq = shtp_connection_send(&self->connection, self->device,
        BNO_CHANNEL_EXE,
        1, &data
    );
    usleep(USEC_TO_SEC(0.5));

    for(int i = 0; i < 3; i++){
        p = shtp_connection_read(&self->connection, self->device);
        if(!p){
            usleep(USEC_TO_SEC(0.5));
        }else{
            shtp_packet_dump(p);
        }
    }
    return true;
}
