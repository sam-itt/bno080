//#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bno080.h"
#include "bno080-private.h"
#include "shtp-connection.h"
#include "shtp-packet.h"
#include "timer.h"
#include "sh2-constants.h"

#define BNO_SHTP_CHANNEL_EXE 1
#define BNO_SHTP_CHANNEL_CONTROL 2

#define DEFAULT_REPORT_INTERVAL 50000
#define FEATURE_ENABLE_TIMEOUT 2.0

#define Q_POINT(a) 1.0/(1<<(a))

static BnoReport reports[] ={
    {.length=SIZEOF_ACCELEROMETER, .parser=NULL},
    {.length=SIZEOF_GYROSCOPE_CALIBRATED, .parser=NULL},
    {.length=SIZEOF_MAGNETIC_FIELD_CALIBRATED, .parser=NULL},
    {.length=SIZEOF_LINEAR_ACCELERATION, .parser=NULL},
    {.length=SIZEOF_ROTATION_VECTOR, .parser=(BnoReportParser)parse_rotation_vector},
    {.length=SIZEOF_GRAVITY, .parser=NULL},
    {.length=SIZEOF_GAME_ROTATION_VECTOR, .parser=NULL},
    {.length=SIZEOF_GEOMAGNETIC_ROTATION_VECTOR, .parser=NULL},
    {.length=SIZEOF_MAGNETIC_FIELD_UNCALIBRATED, .parser=NULL},
    {.length=SIZEOF_TAP_DETECTOR, .parser=NULL},
    {.length=SIZEOF_STEP_COUNTER, .parser=NULL},
    {.length=SIZEOF_SIGNIFICANT_MOTION, .parser=NULL},
    {.length=SIZEOF_STABILITY_REPORT, .parser=NULL},
    {.length=16, .parser=NULL},
    {.length=16, .parser=NULL},
    {.length=16, .parser=NULL},
    {.length=SIZEOF_STEP_DETECTOR, .parser=NULL},
    {.length=SIZEOF_SHAKE_DETECTOR, .parser=NULL},
    {.length=16, .parser=NULL},
    {.length=SIZEOF_TIMESTAMP_REBASE, .parser=NULL},
    {.length=SIZEOF_BASE_TIMESTAMP, .parser=NULL},
};

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
        bno080_soft_reset(self);
        bno080_check_id(self); /*TODO: Give me a timeout*/
        if(self->partno)
            break;
        timer_sleep(0.5); /*Currenty never reached*/
    }

    return self->partno ? self : NULL;
}

bool bno080_soft_reset(Bno080 *self)
{
    ShtpPacket *p;
    uint8_t data = 1;
    uint8_t seq;

    seq = shtp_connection_send(&self->connection, self->device,
        BNO_SHTP_CHANNEL_EXE,
        1, &data
    );
    timer_sleep(0.5);
    seq = shtp_connection_send(&self->connection, self->device,
        BNO_SHTP_CHANNEL_EXE,
        1, &data
    );
    timer_sleep(0.5);

    for(int i = 0; i < 3; i++){
        p = shtp_connection_read(&self->connection, self->device);
        if(!p || p->size == SHTP_HEADER_LEN){
            timer_sleep(0.5);
        }
    }
    return true;
}

bool bno080_check_id(Bno080 *self)
{
    uint8_t data[2];

    data[0] = SHTP_REPORT_PRODUCT_ID_REQUEST;
    data[1] = 0; /*padding*/

    shtp_connection_send(&self->connection,
        self->device, BNO_SHTP_CHANNEL_CONTROL,
        2, data
    );
    for(;;){ /*TODO: Timeout?*/
        ShtpPacket *packet;

        packet = wait_for_packet_type(self,
            BNO_SHTP_CHANNEL_CONTROL,
            SHTP_REPORT_PRODUCT_ID_RESPONSE,
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

/**
 * Tells the BNO080 to start reporting for a given feature and wait
 * for the ACK.
 *
 * @param self a Bno080
 * @param feature The feature to enable
 * @return true on success, false otherwise
 */
bool bno080_enable_feature(Bno080 *self, BnoFeature feature)
{
    uint8_t payload[17];

    if(feature != SH2_REPORTID_ROTATION_VECTOR){
        printf("ROTATION_VECTOR is the only implemented report reading\n");
        return false;
    }

    if(feature == SH2_REPORTID_ACTIVITY_CLASSIFIER)
        return false; //Unimplemented

    memset(payload, 0, sizeof(uint8_t)*17);
    payload[0] = SET_FEATURE_COMMAND;
    payload[1] = (uint8_t)feature; /*provision for where we switch to enums*/
    *((uint32_t*)(payload+5)) = DEFAULT_REPORT_INTERVAL;
    *((uint32_t*)(payload+13)) = 0; //TODO: Enabled activities, 0x1FF for all

    /*TODO: clarify report vs feature*/
#if 0
    if(feature_has_dependency(feature)){
        uint8_t dep = feature - 0x13;
        if(!self->readings[bno080_report_index(dep)]) /*TODO check for -1*/
            bno080_enable_feature(self, dep);
    }
#endif
    shtp_connection_send(&self->connection,
        self->device, BNO_SHTP_CHANNEL_CONTROL,
        17, payload
    );

    /*Wait for ACK*/
    ShtpPacket *ack = wait_for_packet_type(self,
        BNO_SHTP_CHANNEL_CONTROL,
        GET_FEATURE_RESPONSE,
        FEATURE_ENABLE_TIMEOUT
    );
    if(ack && BNO_FEATURE_REPORT(ack)->feature_report_id == feature)
        return true;

    return false; /*Could not get a reading for the feature until timeout*/
}

Quaternion *bno080_quaternion(Bno080 *self)
{
    process_available_packets(self, 5);

    return &self->rotation_vector.quaternion;
}


static bool handle_packet(Bno080 *self, ShtpPacket *packet)
{
    uint8_t report;
    int8_t idx;

    uint16_t offset;
    uint16_t payload_len;

    offset = 0;
    payload_len = shtp_packet_payload_size(packet);
    while(offset < payload_len){
        report = packet->payload[offset];

        idx = get_report_index(report);
        if(idx < 0){
#if 0
            printf(
                "Error: unrecognized report ID in sensor report: %hhx."
                "  Byte %u, length %hu\n",
                packet->payload[offset], offset, payload_len
            );
#endif
            return false;
        }

        if(reports[idx].parser)
            reports[idx].parser(self, packet->payload+offset);
        offset += reports[idx].length;
    }
    return true;
}

static bool process_available_packets(Bno080 *self, uint8_t max_packets)
{
    uint8_t npackets;
    ShtpPacket *packet;

    npackets = 0;
    do{
        if(max_packets && npackets >= max_packets)
            return true;
        packet = shtp_connection_read(&self->connection, self->device);
        if(!packet)
            continue; /*TODO: Stop if there is no more data but retry on error*/
        if(packet->size == SHTP_HEADER_LEN)
            break;
        if(shtp_packet_is_report(packet))
            handle_packet(self, packet); /*TODO: channel control missing*/
        npackets++;
    }while(1); /*TODO: Timeout, don't hang forever*/
    return true;
}

static ShtpPacket *wait_for_packet_type(Bno080 *self, uint8_t channel,
                                        uint8_t report, int8_t timeout)
{
    Timer t;
    ShtpPacket *rv;

#if DEBUG_TRAFFIC
    printf("Waiting for packet on channel %d with report id %#04x\n",
        channel, report
    );
#endif
    timeout = (timeout < 0) ? 5 : timeout;

    timer_start(&t);
    do{
        rv = shtp_connection_poll(&self->connection, self->device, -1);
        if(rv && shtp_packet_match(rv, channel, report)){
            return rv;
        }
        if(shtp_packet_is_report(rv))
            handle_packet(self, rv); /*TODO: channel control missing*/
    }while(!timeout_elapsed(&t, timeout));

    return NULL; /*timeout*/
}

static inline bool feature_has_dependency(uint8_t feature)
{
    return    feature >= SH2_REPORTID_RAW_ACCELEROMETER
           && feature <= SH2_REPORTID_RAW_MAGNETOMETER;
}

static int get_report_index(uint8_t report)
{
    if(report <= 6) return report -1;
    if(report >= 8 && report <= 9) return report - 2;

    if(report >= 15 && report <= 22) return report - 7;
    if(report >= 24 && report <= 25) return report - 20;

    if(report == SH2_REPORTID_ACTIVITY_CLASSIFIER) return 17+1;
    if(report == SH2_REPORTID_TIMESTAMP_REBASE) return 17+2;
    if(report == SH2_REPORTID_BASE_TIMESTAMP) return 17+3;

    return -1;
}

static bool parse_rotation_vector(Bno080 *self, uint8_t *bytes)
{
    int16_t data1 = *((int16_t *)(bytes + 4));
    int16_t data2 = *((int16_t *)(bytes + 6));
    int16_t data3 = *((int16_t *)(bytes + 8));

    self->rotation_vector.quaternion.i = Q_POINT(14) * data1;
    self->rotation_vector.quaternion.j = Q_POINT(14) * data2;
    self->rotation_vector.quaternion.k = Q_POINT(14) * data3;
    self->rotation_vector.quaternion.w = *((int16_t *)(bytes + 10));
    self->rotation_vector.quaternion.w *= Q_POINT(14);
    self->rotation_vector.accuracy = *((uint16_t *)(bytes + 12));
    self->rotation_vector.accuracy *= Q_POINT(12);

    return true;
}

