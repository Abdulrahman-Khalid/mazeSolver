#ifndef UNO
#define UNO 1
#endif

#ifdef TIVA
#include <Energia.h>
#elif UNO == 1
#include <Arduino.h>
#include <Ultrasonic.h>
#endif

#include "Maze.h"

#define LEFT_MOTOR_PIN1 10
#define LEFT_MOTOR_PIN2 11
#define RIGHT_MOTOR_PIN1 12
#define RIGHT_MOTOR_PIN2 13
#define START_BUTTON_PIN 1

Ultrasonic ultrasonicFront(2, 6);
Ultrasonic ultrasonicLeft(4, 7);
Ultrasonic ultrasonicRight(8, 5);

Maze maze;

enum class State : uint8_t {
    TURN_RIGHT, TURN_LEFT, TURN_180, MOVE_FORWARD, TAKE_DECISION, FINISHED
} currentState;

// time that each state takes in millis
enum class StateTime : uint16_t {
    NONE = 0, TURN = 4000, MOVE = 5000, TURN_180 = 8000
};

uint16_t stateTime = (uint16_t) StateTime::NONE;

bool toStart = false;

inline bool blocked(const int sensorRead) {
    return sensorRead < 50; // TODO
}

inline void moveForward() {
    digitalWrite(LEFT_MOTOR_PIN1, HIGH);
    digitalWrite(RIGHT_MOTOR_PIN1, HIGH);
}

inline void turnRight() {
    digitalWrite(LEFT_MOTOR_PIN1, HIGH);
}

inline void turnLeft() {
   digitalWrite(RIGHT_MOTOR_PIN1, HIGH);
}

inline void stopMotors() {
    digitalWrite(LEFT_MOTOR_PIN1, LOW);
    digitalWrite(RIGHT_MOTOR_PIN1, LOW);
}

void setup() {
    Serial.begin(9600);

    pinMode(LEFT_MOTOR_PIN1, OUTPUT);
    pinMode(LEFT_MOTOR_PIN2, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
    digitalWrite(LEFT_MOTOR_PIN2, LOW);
    digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}

void loop() {
    if (!toStart) {
        toStart = digitalRead(START_BUTTON_PIN);
        return;
    }

    int startTime = millis();

    int frontRead = ultrasonicFront.read();
    int rightRead = ultrasonicRight.read();
    int leftRead = ultrasonicLeft.read();

    Serial.print("front:"); Serial.print(frontRead);
    Serial.print(",left:"); Serial.print(leftRead);
    Serial.print(",right:"); Serial.print(rightRead);

    switch (currentState) {
        case State::TAKE_DECISION: {
            maze.updateAdjacentWalls(blocked(frontRead), blocked(rightRead), blocked(leftRead));
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
            while (blocked(frontRead)) {stopMotors();}
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
}
