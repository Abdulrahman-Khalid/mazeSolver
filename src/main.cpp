#include "common.h"

#define TEST

#include "Maze.h"

Maze maze;

enum class State : uint8_t {
    TURN_RIGHT, TURN_LEFT, TURN_180, MOVE_FORWARD, TAKE_DECISION, FINISHED
};

State currentState = State::TAKE_DECISION;

inline void printState() {
    print("state:");

    switch (currentState) {
        case State::TURN_RIGHT:
            print("TR,"); break;
        case State::TURN_LEFT:
            print("TL,");  break;
        case State::TURN_180:
            print("T180,");  break;
        case State::MOVE_FORWARD:
            print("F,");  break;
        case State::TAKE_DECISION:
            print("TD,");  break;
        case State::FINISHED:
            print("FIN,");  break;
        default: break;
    }
}

// time that each state takes in millis
enum class StateTime : uint16_t {
    NONE = 0, TURN = 1250/2, MOVE = 1734, TURN_180 = 1250
};

int32_t stateTime = 0;

bool toStart = true; // TODO, setup push button

#ifndef TEST
    inline bool frontBlocked() {
        static Ultrasonic ultrasonicFront(2, 6);
        int front = ultrasonicFront.read();
        printv(front);
        return front <= 20;
    }

    inline bool rightBlocked() {
        static Ultrasonic ultrasonicRight(4, 7);
        int right = ultrasonicRight.read();
        printv(right);
        return right <= 30;
    }

    inline bool leftBlocked() {
        static Ultrasonic ultrasonicLeft(8, 5);
        int left = ultrasonicLeft.read();
        printv(left);
        return left <= 20;
    }
#else
    bool sensorsReadings[4*3] = {
        0, 0, 1,
        0, 1, 0,
        0, 0, 0,
        0, 0, 0
    };

    int sensI = 0;

    inline bool frontBlocked() {
        return sensorsReadings[sensI+1];
    }

    inline bool rightBlocked() {
        return sensorsReadings[sensI+2];
    }

    inline bool leftBlocked() {
        return sensorsReadings[sensI];
    }
#endif


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
#ifdef SERIAL
    Serial.begin(9600);
#endif

    print("started setup\n");

    pinMode(LEFT_MOTOR_PIN1, OUTPUT);
    pinMode(LEFT_MOTOR_PIN2, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
    pinMode(LEFT_MOTOR_SPD_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_SPD_PIN, OUTPUT);

    resetSpeed();

    maze.init();

    print("ended setup\n");

    delay(2000);
    print("started loop\n");
}

void loop() {
    if (!toStart) {
        toStart = digitalRead(START_BUTTON_PIN);
        return;
    }

    int startTime = millis();

    printState();
    printv(stateTime);
    printv(maze.position.x);
    printv(maze.position.y);
    printv((int)maze.orientation);

    switch (currentState) {
        case State::TAKE_DECISION: {
            print(":TAKE_DECISION:");

            maze.updateAdjacentWalls(frontBlocked(), rightBlocked(), leftBlocked());
#ifdef TEST
            sensI += 3;
#endif
            maze.updateCellsValues();
            Maze::Direction dir = maze.whereToGo(); // updates position
            maze.orientation = maze.calcOrientation(dir);

            printv((int)dir);

            if (maze.finished() || dir == Maze::STOP) {
                currentState = State::FINISHED;
                toStart = false;
            } else if (dir == Maze::FRONT) {
                moveForward();
                currentState = State::MOVE_FORWARD;
                stateTime = (int32_t) StateTime::MOVE;
            } else if (dir == Maze::RIGHT) {
                turnRight();
                currentState = State::TURN_RIGHT;
                stateTime = (int32_t) StateTime::TURN;
            } else if (dir == Maze::LEFT) {
                turnLeft();
                currentState = State::TURN_LEFT;
                stateTime = (int32_t) StateTime::TURN;
            } else {
                turnRight();
                currentState = State::TURN_180;
                stateTime = (int32_t) StateTime::TURN_180;
            }
            break;
        }

        case State::MOVE_FORWARD: {
            print(":MOVE_FORWARD:");

            if (stateTime <= 0) {
                stopMotors();
                currentState = State::TAKE_DECISION;
                stateTime = (int32_t) StateTime::NONE;

                print(":END:");
                break;
            }

            while (frontBlocked()) {stopMotors(); startTime = millis();}
            moveForward();
            break;
        }

        case State::TURN_180:
        case State::TURN_RIGHT:
        case State::TURN_LEFT:  {
            print(":TURN_*:");

            if (stateTime <= 0) {
                print(":END:");

                moveForward();
                currentState = State::MOVE_FORWARD;
                stateTime = (int32_t) StateTime::MOVE;
            }
            break;
        }

        default: stopMotors(); print(":FINISHED:");
    }

    stateTime -= millis() - startTime;

    print("\n");
}
