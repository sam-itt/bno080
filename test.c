#include <stdio.h>
#include <stdlib.h>

#include "bno080.h"

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

    printf("Init complete\n");
}
