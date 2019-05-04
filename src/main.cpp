#include "common.h"

#define TEST

#include "Maze.h"

Maze maze;

enum class State : uint8_t {
    TURN_RIGHT, TURN_LEFT, TURN_180, MOVE_FORWARD, TAKE_DECISION, FINISHED
};

State currentState;

// time that each state takes in millis
enum class StateTime : uint16_t {
    NONE = 0, TURN = 1250/2, MOVE = 1734, TURN_180 = 1250
};

int32_t stateTime = 0;

bool toStart;

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

inline void printOrientation(Maze::Orientation o) {
    print("o=");

    switch (o) {
        case Maze::NORTH:
            print("NORTH,");break;
        case Maze::SOUTH:
            print("SOUTH,");break;
        case Maze::WEST:
            print("WEST,");break;
        case Maze::EAST:
            print("EAST,");break;
        default: break;
    }
}

#ifdef TEST
bool sensorsReadings[4*3] = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 0,
    0, 0, 0
};

int sensI;

inline bool frontBlocked() {
    return sensorsReadings[sensI+1];
}

inline bool rightBlocked() {
    return sensorsReadings[sensI+2];
}

inline bool leftBlocked() {
    return sensorsReadings[sensI];
}

void advanceTest() {
    sensI += 3;
    if (sensI >= sizeof sensorsReadings) {
        print("\nTEST FINISHED\n");
        sensI = 0;
    }
}
#else
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

void reset() {
    print("resetting..\n");

    resetSpeed();

    currentState = State::TAKE_DECISION;
    stateTime = 0;
    toStart = false;
    stateTime = 0;
    currentState = State::TAKE_DECISION;
    maze.position = Maze::Position(START_X, START_Y);
    maze.orientation = START_ORIENT;

    #ifdef TEST
    sensI = 0;
    #endif

    print("reset finished\n");
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

    reset();
    maze.init();

    print("ended setup\n");

    delay(2000);
    print("started loop\n");

    toStart  = true; // TODO, setup push button
}

void loop() {
    if (!toStart) {
        toStart = digitalRead(START_BUTTON_PIN);

        #ifdef SERIAL
        toStart = toStart || Serial.read() != -1;
        #endif

        if (toStart) {
            reset();
        } else {
            return;
        }
    }

    int startTime = millis();

    #ifdef TEST
    stateTime = 0;
    #endif

    switch (currentState) {
        case State::TAKE_DECISION: {
            print(":TAKE_DECISION: :BEFORE:"); {
                auto &x = maze.position.x, &y = maze.position.y;
                printv(x);
                printv(y);
                printOrientation(maze.orientation);
            }

            if (maze.finished()) {
                print("\n:FINISHED::END:\n");

                currentState = State::FINISHED;
                toStart = false; 
                break;
            }

            auto f = frontBlocked(), r = rightBlocked(), l = leftBlocked();
            maze.updateAdjacentWalls(f, r, l);

            printv(l);
            printv(f);
            printv(r);

            #ifdef TEST
            advanceTest();
            #endif

            maze.updateCellsValues();
            Maze::Direction absDir = maze.whereToGo(); // updates position
            Maze::Direction relativeDir = maze.calcRelativeDir(absDir);
            maze.orientation = maze.calcOrientation(relativeDir);

            print(" :AFTER:"); 

            if (absDir == Maze::STOP) {
                print(":STOPED:");

                stopMotors();
                halt();
            }  else if (relativeDir == Maze::FRONT) {
                print(":MOVE_FORWARD:");

                moveForward();
                currentState = State::MOVE_FORWARD;
                stateTime = (int32_t) StateTime::MOVE;
            } else if (relativeDir == Maze::RIGHT) {
                print(":TURN_RIGHT:");

                turnRight();
                currentState = State::TURN_RIGHT;
                stateTime = (int32_t) StateTime::TURN;
            } else if (relativeDir == Maze::LEFT) {
                print(":TURN_LEFT:");

                turnLeft();
                currentState = State::TURN_LEFT;
                stateTime = (int32_t) StateTime::TURN;
            } else {
                print(":TURN_180:");

                turnRight();
                currentState = State::TURN_180;
                stateTime = (int32_t) StateTime::TURN_180;
            }

            printv(int(absDir));
            printv(int(relativeDir));
            printv(stateTime);
            print(":END:\n");

            break;
        }

        case State::MOVE_FORWARD: {
            print(":MOVE_FORWARD:");
            if (stateTime <= 0) {
                stopMotors();
                currentState = State::TAKE_DECISION;
                stateTime = (int32_t) StateTime::NONE;

                print(":END:\n");
                break;
            }

            #ifndef TEST
            while (frontBlocked()) {stopMotors(); startTime = millis();}
            moveForward();
            #endif

            break;
        }

        case State::TURN_180:
        case State::TURN_RIGHT:
        case State::TURN_LEFT:  {
            print(":TURN_*:");
            if (stateTime <= 0) {
                print(":END:\n");

                moveForward();
                currentState = State::MOVE_FORWARD;
                stateTime = (int32_t) StateTime::MOVE;
            }
            break;
        }

        default: stopMotors();
    }

    stateTime -= millis() - startTime;
}
