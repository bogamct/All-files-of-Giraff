#ifndef ROBOT10_CONSTANTS_H
#define ROBOT10_CONSTANTS_H


//---------------Drive motor parameters-------------
#define WHEELBASE      0.47     // in meters.
#define WHEEL_DIAMETER 0.145    // in meters

// 19.2 is the gear ratio
// 250 is pulses per revolution of the AMT102
// Don't know why 4 multiple is needed.  2 multiplier is needed if we use
// old (not state tracking) encoder routine
#define WHEEL_CLICKS_PER_OUTPUT_REVOLUTION  (19.2 * 250.0 * 2)

#define MOTOR0_KD 3000
#define MOTOR0_KI 4
#define MOTOR0_KP 750

#define MOTOR1_KD MOTOR0_KD
#define MOTOR1_KI MOTOR0_KI
#define MOTOR1_KP MOTOR0_KP


//------------------Head tilt motor parameters-----------------
// We will define 0 degrees as looking straight ahead
// looking down is the negative direction, looking up is the positive direction
// All angles are from the perspective of the LENS, NOT THE SCREEN
// That means that when the screen is in the vertical position, the lens is facing down 45 degrees from vertical (135 absolute).

// define encoder counts per radian.
// assume 112:1 gear ratio and 16 counts/rev encoder. That means 16*112 = 1792 counts/rotation
#define SERVO_TICKS_PER_RADIAN (1792.0*2 / (2*PI))

// Space between minimum tilt angle physical stop and software stop. This keeps us from hitting the physical limit
#define SERVO_MIN_SAFETY_FACTOR (-2.0*(PI/180.0))

//Increased the Kp by a factor of 10 since the new Faulhaber motor gives about 10 times fewer encoder pulses/revolution
#define MOTOR2_KD 1000
#define MOTOR2_KI 1
#define MOTOR2_KP 10000

// radians per call to inner loop - roughly radians per mS
#define DEFAULT_MAX_SERVO_VELOCITY 0.015f

//The homing velocity is slower than normal
//Increase the velocity here since the software regulator otherwise wont work with the fewer encoder pulses 
//from the new Faulhaber motor
#define SERVO_HOMING_VELOCITY (DEFAULT_MAX_SERVO_VELOCITY / 4)

//From 3.4 we have a new top design were the homing switch is moved and the motor spins in the other direction. We decided to not change the Java code
// but instead change the low level code which means we use this conversion angle to get the angle correct. This angle is calculated by taking the angle between
// the old physical stop and the new physical stop, this is 38 + 37 = 75degrees
#define SERVO_CONVERSION_ANGLE 1.308997f  

/*-----Variables below are different between hardware versions and are moved from the autogen_variables-----*/
//tilt_angle_from_home
#define TILT_ANGLE_FROM_HOME_VAR_MIN -2.6
#define TILT_ANGLE_FROM_HOME_VAR_MAX 1.22
#define TILT_ANGLE_FROM_HOME_VAR_DEFAULT 0.09

/*-----Height adjustment variables------*/
//ball screw gives 0.75cm/revolition and one revolution gives 32 pulses and the gear ratio is 9.7/1 
#define HEIGHT_CM_PER_ENCODER_COUNT 0.75*32*9.7

#endif
