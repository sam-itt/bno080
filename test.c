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
#if 0
    uint8_t bytes[] = {
        0x05,0x0C,0x00,0x00,
        0xCB,0xEA,0xE0,0xCA,
        0xD3,0x1B,0x0F,0x07,
        0x44,0x32
    };

    double out[10];
    BnoReportDescriptor desc;
    desc = bno_report_get_descriptor(BNO_REPORT_ROTATION_VECTOR);

    bno_report_parse(&desc, bytes, &out);
    for(int i = 0; i < 4; i++)
        printf("out[%d]: %f\n",i, out[i]);

    exit(0);
#endif
    imu = bno080_new(DADDR, argv[1]);
    if(!imu){
        printf("Couldn't open %s\n",argv[1]);
        exit(EXIT_FAILURE);
    }

    printf("******Init complete**********\n");
    bno080_enable_feature(imu, ROTATION_VECTOR);
//    exit(0);

    while(1){
//        timer_sleep(0.5);

        Quaternion *q = bno080_quaternion(imu);
        if(q){
#if 0
           printf("Got quaternion: %f %f %f %f\n",q->i,q->j,q->k,q->w);
#else
            double roll  = atan2(
                2.0 * (q->k * q->j + q->w * q->i),
                1.0 - 2.0 * (q->i * q->i + q->j * q->j)
            );
            double pitch = asin(2.0 * (q->j * q->w - q->k * q->i));
            double yaw   = atan2(
                2.0 * (q->k * q->w + q->i * q->j) ,
                -1.0 + 2.0 * (q->w * q->w + q->i * q->i)
            );

            roll = roll * (180.0/M_PI);
            pitch = pitch * (180.0/M_PI);
            yaw = yaw * (180.0/M_PI);
            printf("heading: %f pitch:%f roll: %f\n", yaw, pitch, roll);
#endif
        }else{
            printf("Quat is NULL\n");
        }

    }
}
