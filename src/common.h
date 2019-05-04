#include <stdint.h>
#include <math.h>

#ifdef TIVA
#include <Energia.h>
#else
#include <Arduino.h>
#include <Ultrasonic.h>
#endif

#if SERIAL
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

#define LEFT_MOTOR_PIN1 10
#define LEFT_MOTOR_PIN2 11
#define LEFT_MOTOR_SPD_PIN 3
#define LEFT_MOTOR_SPD 183

#define RIGHT_MOTOR_PIN1 12
#define RIGHT_MOTOR_PIN2 13
#define RIGHT_MOTOR_SPD_PIN 9
#define RIGHT_MOTOR_SPD 163

#define START_BUTTON_PIN 1

#ifndef TEST
    #define MAZE_HEIGHT 3
    #define MAZE_LENGTH 2

    #define TARGET_X 1
    #define TARGET_Y 2

    #define START_X 0
    #define START_Y 0
    #define START_ORIENT Maze::SOUTH
#endif

#if TEST_CASE == 0
    #define MAZE_LENGTH 2
    #define MAZE_HEIGHT 3

    #define TARGET_X 1
    #define TARGET_Y 2

    #define START_X 0
    #define START_Y 0
    #define START_ORIENT Maze::SOUTH

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
    #define START_ORIENT Maze::SOUTH

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
    #define START_ORIENT Maze::NORTH

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
    #define START_ORIENT Maze::NORTH

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
        0, 0, 0,\
        0, 0, 0,\
        1, 1, 1
#endif
