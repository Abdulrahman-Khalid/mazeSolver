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
    volatile int lastFrontUS, lastRightUS, lastLeftUS;

    inline bool frontBlocked() {
        static Ultrasonic ultrasonicFront(FRONT_US_TRIG, FRONT_US_ECHO);

        float front = 0;
        int size = 0;
        for (int i = 0; i < US_ACCURACY; i++) {
            int tmp = ultrasonicFront.read();
            if (tmp != 375 && tmp != 0) { 
                front += tmp; 
                size++;
            }
        } 

        if (size != 0) {
            front /= size;
        } else {
            front = lastFrontUS;
            print("--INVALID FRONT US VALUE--");
        }
        lastFrontUS = front;

        printv(front);
        return front <= 30 && front >= 3;
    }

    inline bool rightBlocked() {
        static Ultrasonic ultrasonicRight(RIGHT_US_TRIG, RIGHT_US_ECHO);

        float right = 0;
        int size = 0;
        for (int i = 0; i < US_ACCURACY; i++) {
            int tmp = ultrasonicRight.read();
            if (tmp != 375 && tmp != 0) { 
                right += tmp; 
                size++;
            }
        }

        if (size != 0) {
            right /= size;
        } else {
            right = lastRightUS;
            print("--INVALID RIGHT US VALUE--");
        }
        lastRightUS = right;

        printv(right);
        return right <= 40 && right >= 5;
    }

    inline bool leftBlocked() {
        static Ultrasonic ultrasonicLeft(LEFT_US_TRIG, LEFT_US_ECHO);

        float left = 0;
        int size = 0;
        for (int i = 0; i < US_ACCURACY; i++) {
            int tmp = ultrasonicLeft.read();
            if (tmp != 375 && tmp != 0) {
                left += tmp; 
                size++;
            }
        }

        if (size != 0) {
            left /= US_ACCURACY;
        } else {
            left = lastLeftUS;
            print("--INVALID LEFT US VALUE--");
        }
        lastLeftUS = left;

        printv(left);
        return left <= 30 && left >= 5;
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

inline void rightSpeed(uint8_t sp) {
    analogWrite(RIGHT_MOTOR_SPD_PIN, sp);
}

inline void leftSpeed(uint8_t sp) {
    analogWrite(LEFT_MOTOR_SPD_PIN, sp);
}

inline void speed(uint8_t left, uint8_t right) {
    leftSpeed(left);
    rightSpeed(right);
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

static void moveForwardISR_RightIR(void) {
    rightSpeed(!rightOnLine() * RIGHT_FRD_SPD);
}

static void moveForwardISR_LeftIR(void) {
    leftSpeed(!leftOnLine() * LEFT_FRD_SPD);
}

inline void moveForwardWithIR() {
    // start
    moveForward();

    attachInterrupt(LEFT_IR_PIN, moveForwardISR_LeftIR, CHANGE);
    attachInterrupt(RIGHT_IR_PIN, moveForwardISR_RightIR, CHANGE);

    while (!(rightOnLine() && leftOnLine()));

    // stop
    detachInterrupt(LEFT_IR_PIN);
    detachInterrupt(RIGHT_IR_PIN);
    stopMotors();

    // a little bit more
    attachInterrupt(LEFT_IR_PIN, moveForwardISR_LeftIR, CHANGE);
    attachInterrupt(RIGHT_IR_PIN, moveForwardISR_RightIR, CHANGE);

    moveForward();
    speed(LEFT_FRD_SPD, LEFT_FRD_SPD+40);
    delay(400);
    // while (rightOnLine() || leftOnLine());

    // stop
    detachInterrupt(LEFT_IR_PIN);
    detachInterrupt(RIGHT_IR_PIN);
    stopMotors();
}

inline void turnRight() {
    speed(LEFT_TRN_SPD, RIGHT_TRN_SPD);
    rightWheelBackward();
    leftWheelForward();
}

inline void turnLeft() {
    speed(LEFT_TRN_SPD, RIGHT_TRN_SPD);
    rightWheelForward();
    leftWheelBackward();
}

volatile bool hasStoppedTurning;
static void stopTurningISR(void) {
    hasStoppedTurning = true;
    stopMotors();
}

inline void turnLeftWithIR() {
    hasStoppedTurning = false;

    attachInterrupt(LEFT_IR_PIN, stopTurningISR, FALLING);
    noInterrupts();

    turnLeft();
    while (leftOnLine() || rightOnLine());

    interrupts();
    while (!hasStoppedTurning);
    detachInterrupt(LEFT_IR_PIN);
}

inline void turnRightWithIR() {
    hasStoppedTurning = false;

    attachInterrupt(RIGHT_IR_PIN, stopTurningISR, FALLING);
    noInterrupts();

    turnRight();
    while (leftOnLine() || rightOnLine());

    interrupts();
    while (!hasStoppedTurning);
    detachInterrupt(RIGHT_IR_PIN);
}

void reset() {
    print("resetting..\n");

    stopMotors();
    start = false;
    currentState = State::TAKE_DECISION;
    position = Position(START_X, START_Y);
    orientation = START_ORIENT;

    lastFrontUS = 40;
    lastRightUS = 40;
    lastLeftUS = 40;

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
            delay(1000);

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
            // print("\n");
            // return;

            delay(1000);

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
            moveForwardWithIR();
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
