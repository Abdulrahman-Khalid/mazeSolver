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
#define print(n)
#define serialBegin(n)
#elif NATIVE == 1
#include <stdio.h>
#define print(n) printf(n)
#define serialBegin(n) 
#else
#define print(n) Serial.print(n)
#define serialBegin(n) Serial.begin(n)
#endif

#include "Maze.h"

#define LEFT_MOTOR_PIN1 10
#define LEFT_MOTOR_PIN2 11
#define LEFT_MOTOR_SPD_PIN 3
#define LEFT_MOTOR_SPD 200

#define RIGHT_MOTOR_PIN1 12
#define RIGHT_MOTOR_PIN2 13
#define RIGHT_MOTOR_SPD_PIN 9
#define RIGHT_MOTOR_SPD 220

#define START_BUTTON_PIN 1

#define MAX_US_READ 200

Ultrasonic ultrasonicFront(2, 6);
Ultrasonic ultrasonicLeft(8, 5);
Ultrasonic ultrasonicRight(4, 7);

Maze maze;

enum class State : uint8_t {
    TURN_RIGHT, TURN_LEFT, TURN_180, MOVE_FORWARD, TAKE_DECISION, FINISHED
} currentState;

// time that each state takes in millis
enum class StateTime : uint16_t {
    NONE = 0, TURN = 4000, MOVE = 5000, TURN_180 = 8000
};

uint16_t stateTime = (uint16_t) StateTime::NONE;

bool toStart = true; // TODO, setup push button

inline bool frontBlocked() {
    int d = ultrasonicFront.read();
    print(" front:"); print(d);

    return d <= 20;
}

inline bool rightBlocked() {
    int d = ultrasonicRight.read();
    print(",right:"); print(d);

    return d <= 30;
}

inline bool leftBlocked() {
    int d = ultrasonicLeft.read();
    print(",left:"); print(d);
    
    return d <= 20;
}

inline void rightWheelForward() {
    digitalWrite(RIGHT_MOTOR_PIN1, HIGH);
    digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}

inline void leftWheelForward() {
    digitalWrite(LEFT_MOTOR_PIN1, HIGH);
    digitalWrite(LEFT_MOTOR_PIN2, LOW);
}

inline void rightWheelBackward() {
    digitalWrite(RIGHT_MOTOR_PIN1, LOW);
    digitalWrite(RIGHT_MOTOR_PIN2, HIGH);
}

inline void leftWheelBackward() {
    digitalWrite(LEFT_MOTOR_PIN1, LOW);
    digitalWrite(LEFT_MOTOR_PIN2, HIGH);
}

inline void speed(uint8_t left, uint8_t right) {
    analogWrite(LEFT_MOTOR_SPD_PIN, left);
    analogWrite(RIGHT_MOTOR_SPD_PIN, right);
}

inline void resetSpeed() {
    speed(LEFT_MOTOR_SPD, RIGHT_MOTOR_SPD);
}

inline void moveForward() {
    rightWheelForward();
    leftWheelForward();
}

inline void turnRight() {
    rightWheelBackward();
    leftWheelForward();
}

inline void turnLeft() {
    rightWheelForward();
    leftWheelBackward();
}

inline void stopMotors() {
    digitalWrite(LEFT_MOTOR_PIN1, LOW);
    digitalWrite(LEFT_MOTOR_PIN2, LOW);

    digitalWrite(RIGHT_MOTOR_PIN1, LOW);
    digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}

void setup() {
    serialBegin(9600);

    pinMode(LEFT_MOTOR_PIN1, OUTPUT);
    pinMode(LEFT_MOTOR_PIN2, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
    pinMode(LEFT_MOTOR_SPD_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_SPD_PIN, OUTPUT);

    resetSpeed();
}

void loop() {
    /*DEBUG*/

    // rightWheelForward();
    // rightWheelBackward();
    // leftWheelForward();
    // leftWheelBackward();
    // moveForward();
    // turnLeft();
    // turnRight();

    // frontBlocked();
    // rightBlocked();
    // leftBlocked();

    moveForward();
    print('\n');
    return;


    if (!toStart) {
        toStart = digitalRead(START_BUTTON_PIN);
        return;
    }

    int startTime = millis();

    switch (currentState) {
        case State::TAKE_DECISION: {
            maze.updateAdjacentWalls(frontBlocked(), rightBlocked(), leftBlocked());
            maze.updateCellsValues();
            Maze::Direction dir = maze.whereToGo(); // updates position
            maze.updateOrientation(dir);

            if (maze.finished() || dir == Maze::STOP) {
                currentState = State::FINISHED;
                toStart = false;
            } else if (dir == Maze::FRONT) {
                moveForward();
                currentState = State::MOVE_FORWARD;
                stateTime = (uint16_t) StateTime::MOVE;
            } else if (dir == Maze::RIGHT) {
                turnRight();
                currentState = State::TURN_RIGHT;
                stateTime = (uint16_t) StateTime::TURN;
            } else if (dir == Maze::LEFT) {
                turnLeft();
                currentState = State::TURN_LEFT;
                stateTime = (uint16_t) StateTime::TURN;
            } else {
                turnRight();
                currentState = State::TURN_180;
                stateTime = (uint16_t) StateTime::TURN_180;
            }
            break;
        }

        case State::MOVE_FORWARD: {
          if (stateTime <= 0) {
              stopMotors();
              currentState = State::TAKE_DECISION;
              stateTime = (uint16_t) StateTime::NONE;
              break;
          }
            while (frontBlocked()) {stopMotors();}
            moveForward();
          break;
        }

        case State::TURN_180:
        case State::TURN_RIGHT:
        case State::TURN_LEFT:  {
          if (stateTime <= 0) {
              moveForward();
              currentState = State::MOVE_FORWARD;
              stateTime = (uint16_t) StateTime::MOVE;
          }
          break;
        }

        default: stopMotors();
    }

    stateTime -= millis() - startTime;

    print("\n");
}
