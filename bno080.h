#ifndef BNO080_H
#define BNO080_H
#include "shtp-connection.h"
#include <stdint.h>

typedef struct{
    ShtpConnection connection;

    uint8_t device;
}Bno080;


Bno080 *bno080_new(uint8_t device, const char *i2c_bus);
Bno080* bno080_init(Bno080 *self, uint8_t device, const char *i2c_bus);
bool bno080_soft_reset(Bno080 *self);
#endif /* BNO080_H */
