#include <stdint.h>
#include <math.h>

#if TIVA == 1
#include <Energia.h>
#include <portpins.h>
#else
#include <Arduino.h>
#endif

#include <Ultrasonic.h>

#ifdef SERIAL
    #define print(n) Serial.print(n)
    #define printv(n) print(#n"=");print(n);print(",")
    #define serialBegin(n) Serial.begin(n)
#else
    #define printv(n)
    #define print(n)
    #define serialBegin(n)
#endif

#ifdef TEST
#define assert(n) do { if (!(n)) { print("\nASSERTION FAILED: " __FILE__ ":"); print(__LINE__); print(":   "#n"\n"); print("\nHALT\n"); while(1); } } while(0)
#else
#define assert(n)
#endif

#define halt() do { print("\nHALT\n");while(1); } while(0)

// time that each state takes in millis
#define TIME_MOVE  1500
#define TIME_TURN_90  500
#define TIME_TURN_180  2*TIME_TURN_90

// motor speeds
#define LEFT_FRD_SPD 1703
#define LEFT_TRN_SPD 180
#define RIGHT_FRD_SPD 163
#define RIGHT_TRN_SPD 180

// pins
#ifdef TIVA
    #define FRONT_US_TRIG PD0
    #define FRONT_US_ECHO PD1
    #define LEFT_MOTOR_SPD_PIN 

    #define RIGHT_MOTOR_PIN1 
    #define RIGHT_MOTOR_PIN2
    #define RIGHT_MOTOR_SPD_PIN

    #define FRONT_US_TRIG
    #define FRONT_US_ECHO

    #define RIGHT_US_TRIG
    #define RIGHT_US_ECHO

    #define LEFT_US_TRIG
    #define LEFT_US_ECHO

    #define START_BUTTON_PIN
#else
    #define LEFT_MOTOR_PIN1 10
    #define LEFT_MOTOR_PIN2 11
    #define LEFT_MOTOR_SPD_PIN 3

    #define RIGHT_MOTOR_PIN1 12
    #define RIGHT_MOTOR_PIN2 13
    #define RIGHT_MOTOR_SPD_PIN 9

    #define FRONT_US_TRIG 2
    #define FRONT_US_ECHO 6

    #define RIGHT_US_TRIG 4
    #define RIGHT_US_ECHO 7

    #define LEFT_US_TRIG 8
    #define LEFT_US_ECHO 5

    #define START_BUTTON_PIN 9
#endif


#ifndef TEST
    #define MAZE_HEIGHT 2
    #define MAZE_LENGTH 2

    #define TARGET_X 1
    #define TARGET_Y 0

    #define START_X 0
    #define START_Y 0
    #define START_ORIENT Orientation::SOUTH
#endif

#ifdef TEST
#if TEST_CASE == 0
    #define MAZE_LENGTH 2
    #define MAZE_HEIGHT 3

    #define TARGET_X 1
    #define TARGET_Y 2

    #define START_X 0
    #define START_Y 0
    #define START_ORIENT Orientation::SOUTH

    #define SENSOR_READS \
        1, 0, 0,\
        0, 1, 0,\
        0, 0, 0,\
        0, 0, 0

#elif TEST_CASE == 1
    #define MAZE_LENGTH 2
    #define MAZE_HEIGHT 4

    #define TARGET_X 1
    #define TARGET_Y 0

    #define START_X 0
    #define START_Y 0
    #define START_ORIENT Orientation::SOUTH

    #define SENSOR_READS \
        1, 0, 0,\
        1, 0, 0,\
        0, 0, 0,\
        0, 0, 0,\
        1, 0, 0,\
        1, 0, 0
#elif TEST_CASE == 2
    #define MAZE_LENGTH 5
    #define MAZE_HEIGHT 5

    #define TARGET_X 3
    #define TARGET_Y 1

    #define START_X 0
    #define START_Y 4
    #define START_ORIENT Orientation::NORTH

    #define SENSOR_READS \
        0, 0, 1,\
        0, 0, 1,\
        0, 0, 0,\
        0, 0, 1,\
        0, 0, 1,\
        1, 0, 0,\
        0, 0, 0,\
        0, 1, 0,\
        1, 0, 0,\
        0, 1, 0,\
        1, 0, 0,\
        0, 0, 1,\
        0, 0, 0,\
        0, 0, 0,\
        1, 1, 1
#elif TEST_CASE == 3
    #define MAZE_LENGTH 5
    #define MAZE_HEIGHT 5

    #define TARGET_X 3
    #define TARGET_Y 1

    #define START_X 0
    #define START_Y 4
    #define START_ORIENT Orientation::NORTH

    #define SENSOR_READS \
        0, 0, 1,\
        0, 0, 1,\
        0, 0, 0,\
        0, 0, 1,\
        0, 0, 1,\
        1, 0, 0,\
        0, 0, 0,\
        0, 1, 0,\
        1, 0, 0,\
        0, 1, 0,\
        1, 0, 0,\
        0, 1, 1,\
        0, 1, 0,\
        1, 0, 0,\
        0, 0, 1,\
        1, 0, 1,\
        0 ,0, 0,\
        0, 0, 0,\
        1, 1, 1
#elif TEST_CASE == 4
    #define MAZE_LENGTH 5
    #define MAZE_HEIGHT 5

    #define TARGET_X 3
    #define TARGET_Y 1

    #define START_X 3
    #define START_Y 1
    #define START_ORIENT Orientation::NORTH

    #define SENSOR_READS \
        1, 1, 1,\
        1, 1, 1,
#elif TEST_CASE == 5
    #define MAZE_LENGTH 5
    #define MAZE_HEIGHT 5

    #define TARGET_X 3
    #define TARGET_Y 1

    #define START_X 0
    #define START_Y 0
    #define START_ORIENT Orientation::SOUTH

    #define SENSOR_READS \
        1, 1, 1
#endif
#endif
