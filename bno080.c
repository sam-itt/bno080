//#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bno080.h"
#include "shtp-connection.h"
#include "shtp-packet.h"
#include "timer.h"

#define BNO_CHANNEL_SHTP_COMMAND 0 /*TODO: Move upward*/
#define BNO_CHANNEL_EXE 1
#define BNO_CHANNEL_CONTROL 2

/*TODO: Move upward*/
#define _SHTP_REPORT_PRODUCT_ID_REQUEST 0xF9
#define _SHTP_REPORT_PRODUCT_ID_RESPONSE 0xF8

#define USEC_TO_SEC(a) ((a)*1000)

static ShtpPacket *wait_for_packet_type(Bno080 *self, uint8_t channel,
                                        uint8_t report, int8_t timeout);


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
    if(!rv) return NULL;

    for(int i = 0; i < 3; i++){
        printf("%s: try #%d\n", __FUNCTION__, i);
        bno080_soft_reset(self);
        bno080_check_id(self);
        if(self->partno)
            break;
        usleep(USEC_TO_SEC(0.5));
    }

    return self->partno ? self : NULL;
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
 //       printf("%s: loop %d\n", __FUNCTION__, i);
        p = shtp_connection_read(&self->connection, self->device);
        if(!p){
            usleep(USEC_TO_SEC(0.5));
        }else{
        //    shtp_packet_dump(p);
//            return true;
        }
    }
    return true;
}

bool bno080_check_id(Bno080 *self)
{
    uint8_t data[2];

    data[0] = _SHTP_REPORT_PRODUCT_ID_REQUEST;
    data[1] = 0; /*padding*/

    shtp_connection_send(&self->connection,
        self->device, BNO_CHANNEL_CONTROL,
        2, data
    );
    for(;;){ /*TODO: Timeout?*/
        ShtpPacket *packet;

        packet = wait_for_packet_type(self,
            BNO_CHANNEL_CONTROL,
            _SHTP_REPORT_PRODUCT_ID_RESPONSE,
            -1 /*default timeout*/
        );
        if(!packet) continue;

        uint8_t sw_major = packet->payload[2];
        uint8_t sw_minor = packet->payload[3];
        uint16_t sw_patch = *((uint16_t*)(packet->payload+12));
        uint32_t sw_part_number = *((uint32_t*)(packet->payload+4));
        uint32_t sw_build_number = *((uint32_t*)(packet->payload+8));

        printf("Part number: %d\n", sw_part_number);
        printf("Software version: %d.%d.%d\n",sw_major,sw_minor,sw_patch);
        printf("Build: %d\n",sw_build_number);
        self->partno = sw_part_number;
        return true;
    }
    return false;
}

static ShtpPacket *wait_for_packet_type(Bno080 *self, uint8_t channel,
                                        uint8_t report, int8_t timeout)
{
    Timer t;
    ShtpPacket *rv;

    timeout = (timeout < 0) ? 5 : timeout;

    timer_start(&t);
    do{
        rv = shtp_connection_poll(&self->connection, self->device, -1);
        if(!rv) continue;

        if(shtp_packet_match(rv, channel, report))
            return rv;

        /* Here the python code goes for de-slicing other reports
         * when they don't match. TODO: Check if that is really needed*/
    }while(!timeout_elapsed(&t, timeout));

    return NULL; /*timeout*/
}
