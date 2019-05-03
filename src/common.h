#include <stdint.h>
#include <math.h>

#ifndef UNO
#define UNO 1
#endif

#ifdef TIVA
#include <Energia.h>
#elif UNO == 1
#include <Arduino.h>
#include <Ultrasonic.h>
#endif

#ifndef DEBUG
    #define printv(n)
    #define print(n)
#elif NATIVE == 1
    #include <stdio.h>
    #define print(n) printf(n)
    #define printv(n) print(#n"=");print(n);print(",")
#else
    #define print(n) Serial.print(n)
    #define printv(n) print(#n"=");print(n);print(",")
#endif

#define halt() while(1)

#define LEFT_MOTOR_PIN1 10
#define LEFT_MOTOR_PIN2 11
#define LEFT_MOTOR_SPD_PIN 3
#define LEFT_MOTOR_SPD 183

#define RIGHT_MOTOR_PIN1 12
#define RIGHT_MOTOR_PIN2 13
#define RIGHT_MOTOR_SPD_PIN 9
#define RIGHT_MOTOR_SPD 163

#define START_BUTTON_PIN 1

#define MAZE_HEIGHT 3
#define MAZE_LENGTH 2

#define TARGET_X 1
#define TARGET_Y 2

#define START_X 0
#define START_Y 0
#define START_ORIENT SOUTH
