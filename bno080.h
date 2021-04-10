#ifndef BNO080_H
#define BNO080_H
#include "shtp-connection.h"
#include <stdint.h>

typedef struct{
    ShtpConnection connection;

    uint8_t device;

    uint32_t partno;
}Bno080;


Bno080 *bno080_new(uint8_t device, const char *i2c_bus);
Bno080* bno080_init(Bno080 *self, uint8_t device, const char *i2c_bus);

bool bno080_soft_reset(Bno080 *self);
bool bno080_check_id(Bno080 *self);
#endif /* BNO080_H */
