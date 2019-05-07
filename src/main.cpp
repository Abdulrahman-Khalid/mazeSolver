// #define TEST
// #define TEST_CASE 0
#define SERIAL
#define IR_ASSISTED

#include "Maze.h"
#include "common.h"

enum State : uint8_t {
    TURN_RIGHT,
    TURN_LEFT,
    TURN_180,
    MOVE_FORWARD,
    TAKE_DECISION,
    FINISHED
} currentState;

Orientation orientation = START_ORIENT;
Position position(START_X, START_Y);

Maze maze(&position, &orientation);
bool start;

inline Orientation calcOrientation(Direction relativeDir,
                                   Orientation orientation) {
    return Orientation((orientation + relativeDir) % 4);
}

inline Direction calcRelativeDir(Direction absDir, Orientation orientation) {
    return Direction((absDir - orientation + 4) % 4);
}

inline void printOrientation(Orientation o) {
#ifdef SERIAL
    print("o=");

    switch (o) {
        case Orientation::NORTH:
            print("NORTH,");
            break;
        case Orientation::SOUTH:
            print("SOUTH,");
            break;
        case Orientation::WEST:
            print("WEST,");
            break;
        case Orientation::EAST:
            print("EAST,");
            break;
        default:
            break;
    }
#endif
}

inline void printBlocks() {
#ifdef SERIAL
    print("\n");
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_LENGTH; x++) {
            if (x == position.x && y == position.y) {
                switch (orientation) {
                    case NORTH:
                        print("^");
                        break;
                    case EAST:
                        print(">");
                        break;
                    case WEST:
                        print("<");
                        break;
                    case SOUTH:
                        print("v");
                        break;
                    default:
                        break;
                }
            } else {
                print(" ");
                print(maze.cells[x][y].value);
                print(" ");
            }
            print("  ");
        }
        print("\n");
    }
#endif
}

#ifdef TEST
    int sensI;
    bool sensorsReadings[] = {SENSOR_READS};

    inline bool frontBlocked() { return sensorsReadings[sensI + 1]; }

    inline bool rightBlocked() { return sensorsReadings[sensI + 2]; }

    inline bool leftBlocked() { return sensorsReadings[sensI]; }

    void advanceTest() {
        sensI += 3;
        sensI %= sizeof sensorsReadings;
    }
#else
    static Ultrasonic ultrasonicFront(FRONT_US_TRIG, FRONT_US_ECHO);

    inline bool frontBlocked() {
        int front = ultrasonicFront.read();
        printv(front);
        return front <= 20;
    }

    inline bool frontBlockedTight() {
        int front = ultrasonicFront.read();
        printv(front);
        return front <= 5;
    }

    inline bool rightBlocked() {
        static Ultrasonic ultrasonicRight(RIGHT_US_TRIG, RIGHT_US_ECHO);
        int right = ultrasonicRight.read();
        printv(right);
        return right <= 30;
    }

    inline bool leftBlocked() {
        static Ultrasonic ultrasonicLeft(LEFT_US_TRIG, LEFT_US_ECHO);
        int left = ultrasonicLeft.read();
        printv(left);
        return left <= 20;
    }
#endif

inline bool rightOnLine() { return digitalRead(RIGHT_IR_PIN); }

inline bool leftOnLine() { return digitalRead(LEFT_IR_PIN); }

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

inline void stopMotors() {
    digitalWrite(LEFT_MOTOR_PIN1, LOW);
    digitalWrite(LEFT_MOTOR_PIN2, LOW);

    digitalWrite(RIGHT_MOTOR_PIN1, LOW);
    digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}

inline void moveForward() {
    speed(LEFT_FRD_SPD, RIGHT_FRD_SPD);
    rightWheelForward();
    leftWheelForward();
}

inline void moveForwardWithIR(int time) {
    moveForward();

    while (time > 0) {
        int s = millis();
        bool l = leftOnLine(), r = rightOnLine();

        if ((l && r) || (!l && !r)) {
            speed(RIGHT_FRD_SPD, LEFT_FRD_SPD);
        } else if (r) {
            speed(LEFT_FRD_SPD, 0);
        } else {  // l
            speed(0, RIGHT_FRD_SPD);
        }

        time -= millis() - s;
    }

    stopMotors();
}

inline void turnRight() {
    speed(LEFT_TRN_SPD, RIGHT_TRN_SPD);
    rightWheelBackward();
    leftWheelForward();
}

inline void turnRightWithIR() {
    turnRight();
    delay(300);
    while (leftOnLine() || rightOnLine());
    while (!rightOnLine());
    stopMotors();
}

inline void turnLeft() {
    speed(LEFT_TRN_SPD, RIGHT_TRN_SPD);
    rightWheelForward();
    leftWheelBackward();
}

inline void turnLeftWithIR() {
    turnLeft();
    delay(300);
    while (leftOnLine() || rightOnLine());
    while (!leftOnLine());
    stopMotors();
}

void reset() {
    print("resetting..\n");

    stopMotors();
    start = false;
    currentState = State::TAKE_DECISION;
    position = Position(START_X, START_Y);
    orientation = START_ORIENT;

#ifdef TEST
    sensI = 0;
#endif

    print("reset finished\n");
}

void setup() {
    serialBegin(9600);
    delay(500);

    print("started setup\n");

    pinMode(LEFT_MOTOR_PIN1, OUTPUT);
    pinMode(LEFT_MOTOR_PIN2, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
    pinMode(LEFT_MOTOR_SPD_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_SPD_PIN, OUTPUT);
    pinMode(LEFT_IR_PIN, INPUT);
    pinMode(RIGHT_IR_PIN, INPUT);
    pinMode(START_BUTTON_PIN, INPUT);

    reset();
    maze.init();

    print("ended setup\n");

    delay(2000);
    print("started loop\n");

    start = true;  // TODO, setup push button

#ifdef TEST
    print("in test mode\n");
    printv(TEST_CASE);
    print("\n");
#endif
}

void loop() {
    if (!start) {
        start = digitalRead(START_BUTTON_PIN);

#ifdef SERIAL
        start = start || Serial.read() != -1;
#endif

        if (start) {
            reset();
        } else {
            return;
        }
    }

    switch (currentState) {
        case State::TAKE_DECISION: {
            stopMotors();
            delay(2000);

            printBlocks();
            print(":TAKE_DECISION: :BEFORE:");
            auto &x = position.x, &y = position.y;
            printv(x);
            printv(y);
            printOrientation(orientation);

            if (maze.finished()) {
                print("\n:FINISHED::END:\n");
                currentState = State::FINISHED;
                start = false;
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
            Direction absDir = maze.whereToGo();  // updates position
            Direction relativeDir = calcRelativeDir(absDir, orientation);
            orientation = calcOrientation(relativeDir, orientation);

            print(" :AFTER:");

            if (absDir == Direction::STOP) {
                print(":STOPPED:");
                stopMotors();
                halt();
            } else if (relativeDir == Direction::FRONT) {
                currentState = State::MOVE_FORWARD;
            } else if (relativeDir == Direction::RIGHT) {
                currentState = State::TURN_RIGHT;
            } else if (relativeDir == Direction::LEFT) {
                currentState = State::TURN_LEFT;
            } else {
                currentState = State::TURN_180;
            }

            printv(int(absDir));
            printv(int(relativeDir));
            print(":END:\n");

            break;
        }

        case State::MOVE_FORWARD: {
            print(":MOVE_FORWARD:\n");

#ifdef IR_ASSISTED
            moveForwardWithIR(TIME_MOVE);
#else
            moveForward();
            delay(TIME_MOVE);
#endif
            stopMotors();

            currentState = State::TAKE_DECISION;
            break;
        }

        case State::TURN_180: {
            print(":TURN_BACK:\n");

#ifdef IR_ASSISTED
            turnRightWithIR();
            stopMotors();
            turnRightWithIR();
            stopMotors();
#else
            turnRight();
            delay(TIME_TURN_180);
            stopMotors();
#endif
            delay(500);

            currentState = State::MOVE_FORWARD;
            break;
        }

        case State::TURN_RIGHT: {
            print(":TURN_RIGHT:\n");

#ifdef IR_ASSISTED
            turnRightWithIR();
            stopMotors();
#else
            turnRight();
            delay(TIME_TURN_90);
            stopMotors();
#endif
            delay(500);

            currentState = State::MOVE_FORWARD;
            break;
        }

        case State::TURN_LEFT: {
            print(":TURN_LEFT:\n");

#ifdef IR_ASSISTED
            turnLeftWithIR();
            stopMotors();
#else
            turnLeft();
            delay(TIME_TURN_90);
            stopMotors();
#endif
            delay(500);

            currentState = State::MOVE_FORWARD;
            break;
        }

        default:
            stopMotors();
    }
}
