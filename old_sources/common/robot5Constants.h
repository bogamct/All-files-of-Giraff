#ifndef ROBOT5_CONSTANTS_H
#define ROBOT5_CONSTANTS_H

#define WHEELBASE      0.44     // in meters
#define WHEEL_DIAMETER 0.145    // in meters
#define KI_DEFAULT 50
#define ROBOT_NUM 5

// 24 * 19.0 is for PM000054 configuration (24:1 ratio, 19PPR encoder)
#define WHEEL_CLICKS_PER_OUTPUT_REVOLUTION (24 * 19.0 * 2)

#define MOTOR0_KD 300000
#define MOTOR0_KI 250
#define MOTOR0_KP 25000

#define MOTOR1_KD MOTOR0_KD
#define MOTOR1_KI MOTOR0_KI
#define MOTOR1_KP MOTOR0_KP

#endif
