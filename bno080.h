#ifndef BNO080_H
#define BNO080_H
#include <stdint.h>

#include "sh2-constants.h"
#include "shtp-connection.h"

typedef struct{
    float i;
    float j;
    float k;
    float w;
}Quaternion;

typedef struct{
    Quaternion quaternion;
    float accuracy;
}RotationVector;

typedef struct _Bno080{
    ShtpConnection connection;

    uint8_t device;

    uint32_t partno;

    /*Reading(s)*/
    RotationVector rotation_vector;
}Bno080;

/* From https://os.mbed.com/users/MultipleMonomials/code/BNO080/
 * Subset of SH2 report ids that can be enabled/disabled
 * */
typedef enum __attribute__((packed)){
    /**
     * Total acceleration of the IMU in world space.
     * See BNO datasheet section 2.1.1
     */
    TOTAL_ACCELERATION = SH2_REPORTID_ACCELEROMETER,

    /**
     * Acceleration of the IMU not including the acceleration of gravity.
     * See BNO datasheet section 2.1.1
     */
    LINEAR_ACCELERATION = SH2_REPORTID_LINEAR_ACCELERATION,

    /**
     * Acceleration of gravity felt by the IMU.
     * See BNO datasheet section 2.1.1
     */
    GRAVITY_ACCELERATION = SH2_REPORTID_GRAVITY,

    /**
     * (calibrated) gyroscope reading of the rotational speed of the IMU.
     * See BNO datasheet section 2.1.2
     */
    GYROSCOPE = SH2_REPORTID_GYROSCOPE_CALIBRATED,

    /**
     * (calibrated) reading of Earth's magnetic field levels.
     * See BNO datasheet section 2.1.3
     */
    MAG_FIELD = SH2_REPORTID_MAGNETIC_FIELD_CALIBRATED,

    /**
    * Uncalibrated reading of magnetic field levels, without any hard iron
    * offsets applied See BNO datasheet section 2.1.3
    */
    MAG_FIELD_UNCALIBRATED = SH2_REPORTID_MAGNETIC_FIELD_UNCALIBRATED,

    /**
     * Fused reading of the IMU's rotation in space using all three sensors.
     * This is the most accurate reading of absolute orientation that the IMU
     * can provide. See BNO datasheet section 2.2.4
     */
    ROTATION_VECTOR = SH2_REPORTID_ROTATION_VECTOR,

    /**
     * Fused reading of rotation from accelerometer and magnetometer readings.
     * This report is designed to decrease power consumption (by turning off
     * the gyroscope) in exchange for reduced responsiveness.
     */
    GEOMAGNETIC_ROTATION_VECTOR = SH2_REPORTID_GEOMAGNETIC_ROTATION_VECTOR,

    /**
     * Fused reading of the IMU's rotation in space. Unlike the regular
     * rotation vector, the Game Rotation Vector is not referenced against the
     * magnetic field and the "zero yaw" point is arbitrary. See BNO datasheet
     * section 2.2.2
     */
    GAME_ROTATION_VECTOR = SH2_REPORTID_GAME_ROTATION_VECTOR,

    /**
     * Detects a user tapping on the device containing the IMU.
     * See BNO datasheet section 2.4.2
     */
    TAP_DETECTOR = SH2_REPORTID_TAP_DETECTOR,

    /**
     * Detects whether the device is on a table, being held stably, or being
     * moved. See BNO datasheet section 2.4.1
     */
    STABILITY_CLASSIFIER = SH2_REPORTID_STABILITY_CLASSIFIER,

    /**
     * Detects a user taking a step with the IMU worn on their person.
     * See BNO datasheet section 2.4.3
     */
    STEP_DETECTOR = SH2_REPORTID_STEP_DETECTOR,

    /**
     * Detects how many steps a user has taken.
     * See BNO datasheet section 2.4.4
     */
    STEP_COUNTER = SH2_REPORTID_STEP_COUNTER,

    /**
     * Detects when the IMU has made a "significant" motion, defined as moving
     * a few steps and/or accelerating significantly.
     *
     * NOTE: this report automatically disables itself after sending a report,
     * so you'll have to reenable it each time a motion i s detected.
     * See BNO datasheet section 2.4.6
     */
    SIGNIFICANT_MOTION = SH2_REPORTID_SIGNIFICANT_MOTION,

    /**
     * Detects when the IMU is being shaken.
     * See BNO datasheet section 2.4.7
     */
    SHAKE_DETECTOR = SH2_REPORTID_SHAKE_DETECTOR
}BnoFeature;


Bno080 *bno080_new(uint8_t device, const char *i2c_bus);
Bno080* bno080_init(Bno080 *self, uint8_t device, const char *i2c_bus);

bool bno080_soft_reset(Bno080 *self);
bool bno080_check_id(Bno080 *self);

bool bno080_enable_feature(Bno080 *self, BnoFeature feature);
Quaternion *bno080_quaternion(Bno080 *self);

#endif /* BNO080_H */
