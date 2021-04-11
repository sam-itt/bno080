#ifndef BNO080_PRIVATE_H
#define BNO080_PRIVATE_H
#include <stdint.h>
#include "bno080.h"
#include "shtp-packet.h"

typedef struct _Bno080 Bno080;

typedef struct{
    uint8_t report_id;
    uint8_t feature_report_id;
    uint8_t feature_flags;
    uint16_t change_sensitivity;
    uint32_t report_interval;
    uint32_t batch_interval_word;
    uint32_t sensor_specific_configuration_word;
}BnoFeatureReport; /*Used for features ACKs*/
#define BNO_FEATURE_REPORT(packet) ((BnoFeatureReport*)(packet)->payload)

typedef bool (*BnoReportParser)(void *self, void *destination);

typedef struct{
    uint8_t length;
    BnoReportParser parser;
}BnoReport;


/* Forward declarations of private functions*/
static int get_report_index(uint8_t report);
static bool parse_rotation_vector(Bno080 *self, uint8_t *bytes);
static ShtpPacket *wait_for_packet_type(Bno080 *self, uint8_t channel,
                                        uint8_t report, int8_t timeout);
static bool process_available_packets(Bno080 *self, uint8_t max_packets);
static inline bool feature_has_dependency(uint8_t feature);

#endif /* BNO080_PRIVATE_H */
