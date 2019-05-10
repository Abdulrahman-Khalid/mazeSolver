// #define TEST
// #define TEST_CASE 0
#define SERIAL
#define IR_ASSISTED
// #define USE_EEPROM

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

inline Orientation calcOrientation(Direction dir,
                                   Orientation orientation) {
    return Orientation((orientation + dir) % 4);
}

inline Direction calcRelativeDir(Direction absDir, Orientation orientation) {
    if (absDir == STOP) return STOP;
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
    void adjust();

    inline bool frontBlocked() {
        static Ultrasonic ultrasonicFront(FRONT_US_TRIG, FRONT_US_ECHO);

        float front = ultrasonicFront.read();
        if (front == 0) {
            int size = 0;
            for (int i = 0; i < US_ACCURACY; i++) {
                int tmp = ultrasonicFront.read();
                if (tmp < 300 && tmp != 0) { 
                    front += tmp; 
                    size++;
                }
            } 

            if (size != 0) {
                front /= size;
            } else {
                print("--INVALID FRONT US VALUE--");
            }
        }

        printv(front);
        return front <= 30;
    }

    inline bool rightBlocked() {
        static Ultrasonic ultrasonicRight(RIGHT_US_TRIG, RIGHT_US_ECHO);

        float right = ultrasonicRight.read();
        if (right == 0) {
            int size = 0;
            for (int i = 0; i < US_ACCURACY; i++) {
                int tmp = ultrasonicRight.read();
                if (tmp < 300 && tmp != 0) { 
                    right += tmp; 
                    size++;
                }
            }

            if (size != 0) {
                right /= size;
            } else {
                print("--INVALID RIGHT US VALUE--");
                
            }
        }

        printv(right);
        return right <= 40;
    }

    inline bool leftBlocked() {
        static Ultrasonic ultrasonicLeft(LEFT_US_TRIG, LEFT_US_ECHO);

        float left = ultrasonicLeft.read();
        if (left == 0) {
            int size = 0;
            for (int i = 0; i < US_ACCURACY; i++) {
                int tmp = ultrasonicLeft.read();
                if (tmp < 300 && tmp != 0) {
                    left += tmp; 
                    size++;
                }
            }

            if (size != 0) {
                left /= US_ACCURACY;
            } else {
                print("--INVALID LEFT US VALUE--");
            }
        }

        printv(left);
        return left <= 30;
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

inline void moveBackward() {
    speed(LEFT_FRD_SPD, RIGHT_FRD_SPD);
    rightWheelBackward();
    leftWheelBackward();
}

void adjust() {
    moveBackward();
    delay(150);
    moveForward();
    delay(150);
    stopMotors();
}

static void moveForwardISR_RightIR(void) {
    // rightSpeed(!rightOnLine() * RIGHT_FRD_SPD);
    // if (rightOnLine()) {
    //     rightSpeed(100);
    // } else {
    //     rightSpeed(RIGHT_FRD_SPD);
    // }
    if (rightOnLine()) {
        rightWheelBackward();
    } else {
        rightWheelForward();
    }
}

static void moveForwardISR_LeftIR(void) {
    if (leftOnLine()) {
        // leftSpeed(-LEFT_FRD_SPD);
        leftWheelBackward();
    } else {
        // leftSpeed(LEFT_FRD_SPD);
        leftWheelForward();
    }
}

inline void moveForwardWithIR() {
    // start
    moveForward();

    attachInterrupt(LEFT_IR_PIN, moveForwardISR_LeftIR, CHANGE);
    attachInterrupt(RIGHT_IR_PIN, moveForwardISR_RightIR, CHANGE);

    while (!(rightOnLine() && leftOnLine()));

    moveForward();
    speed(230, 230);
    delay(350);

    // stop
    noInterrupts();
    stopMotors();
    speed(LEFT_FRD_SPD, RIGHT_FRD_SPD);
    detachInterrupt(LEFT_IR_PIN);
    detachInterrupt(RIGHT_IR_PIN);
    interrupts();
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

    turnLeft();
    delay(200);
    while (leftOnLine() || rightOnLine());

    attachInterrupt(LEFT_IR_PIN, stopTurningISR, FALLING);
    while (!hasStoppedTurning);
    detachInterrupt(LEFT_IR_PIN);
}

inline void turnRightWithIR() {
    hasStoppedTurning = false;

    turnRight();
    delay(200);
    while (leftOnLine() || rightOnLine());

    attachInterrupt(RIGHT_IR_PIN, stopTurningISR, FALLING);
    while (!hasStoppedTurning);
    detachInterrupt(RIGHT_IR_PIN);
}

void save() {
#ifdef USE_EEPROM
    print("\n:EEPROM SAVE:");
    int i = 0;

    eepromUpdate(i++, 1);
    eepromUpdate(i++, position.x);
    eepromUpdate(i++, position.y);
    eepromUpdate(i++, orientation);
    eepromUpdate(i++, currentState);

    maze.save(i);
    print(":END:\n");
#endif
}

void load() {
#ifdef USE_EEPROM
    print("\n:EEPROM LOAD:");

    int i = 1;
    position.x = eepromRead(i++);
    position.y = eepromRead(i++);
    orientation = (Orientation) eepromRead(i++);
    currentState = (State) eepromRead(i++);

    maze.load(i);
    print(":END:\n");
#endif
}

bool hasToLoad() {
    #ifdef USE_EEPROM
        return eepromRead(0) == 1;
    #else
        return false;
    #endif
}

void resetToStart() {
    print("resetting..\n");

    stopMotors();
    start = true;  // TODO, setup push button
    currentState = State::TAKE_DECISION;
    position = Position(START_X, START_Y);
    orientation = START_ORIENT;

    #ifdef TEST
        sensI = 0;
    #endif

    print("resetToStart finished\n");
}

static void resetAll() {
    eepromUpdate(0, 0);
    resetToStart();
    maze.init();
}

void setup() {
    serialBegin(9600);

    #ifdef TEST
        print(":IN TEST MODE #");
        printv(TEST_CASE);
        print("\n");
    #endif

    print(":START SETUP:\n");

    pinMode(LEFT_MOTOR_PIN1, OUTPUT);
    pinMode(LEFT_MOTOR_PIN2, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN2, OUTPUT);
    pinMode(LEFT_MOTOR_SPD_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_SPD_PIN, OUTPUT);
    pinMode(LEFT_IR_PIN, INPUT);
    pinMode(RIGHT_IR_PIN, INPUT);
    pinMode(RESET_BUTTON_PIN, INPUT);

    #ifdef USE_EEPROM
        if (hasToLoad()) {
            print(":HAS TO LOAD:");
            load();
        } else {
            print(":NO LOADING:");
            resetAll();
        }

        attachInterrupt(RESET_BUTTON_PIN, resetAll, RISING);
    #else
        resetAll();
    #endif

    print(":END:\n");
    delay(3000);
    print(":START LOOP:\n");
}

void loop() {
    while (!start);

    switch (currentState) {
        case State::TAKE_DECISION: {
            stopMotors();
            save();
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
            Direction absDir = maze.whereToGo(&position);
            Direction dir = calcRelativeDir(absDir, orientation);
            orientation = calcOrientation(dir, orientation);

            print(" :AFTER:");

            if (dir == Direction::STOP) {
                print(":STOPPED:");
                stopMotors();
                halt();
            } else if (dir == Direction::FRONT) {
                currentState = State::MOVE_FORWARD;
            } else if (dir == Direction::RIGHT) {
                currentState = State::TURN_RIGHT;
            } else if (dir == Direction::LEFT) {
                currentState = State::TURN_LEFT;
            } else {
                currentState = State::TURN_180;
            }

            printv(int(absDir));
            printv(int(dir));
            print(":END:\n");

            break;
        }

        case State::MOVE_FORWARD: {
            save();
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
            save();
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
            save();
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
            save();
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
