#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bno080.h"
#include "timer.h"

#define DADDR 0x4b


int main(int argc, char **argv)
{
    Bno080 *imu;

    if(argc < 2){
        printf("Usage: %s /dev/i2c-X\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    imu = bno080_new(DADDR, argv[1]);
    if(!imu){
        printf("Couldn't open %s\n",argv[1]);
        exit(EXIT_FAILURE);
    }

    printf("******Init complete**********\n");
    bno080_enable_feature(imu, ROTATION_VECTOR);

    double heading;
    double pitch;
    double roll;
    while(1){
        if(bno080_hpr(imu, &heading, &pitch, &roll))
            printf("heading: %f pitch:%f roll: %f\n", heading, pitch, roll);
    }
}
