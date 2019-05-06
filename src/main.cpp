#include "common.h"
#include "Maze.h"

enum class State : uint8_t {
    TURN_RIGHT, TURN_LEFT, TURN_180, MOVE_FORWARD, TAKE_DECISION, FINISHED
} currentState;

Maze maze;
bool start;

inline void printOrientation(Orientation o) {
    print("o=");

    switch (o) {
        case Orientation::NORTH:
            print("NORTH,");break;
        case Orientation::SOUTH:
            print("SOUTH,");break;
        case Orientation::WEST:
            print("WEST,");break;
        case Orientation::EAST:
            print("EAST,");break;
        default: break;
    }
}

#ifdef TEST
    int sensI;
    bool sensorsReadings[] = { SENSOR_READS };

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

inline bool rightOnLine() {
    return digitalRead(RIGHT_IR_PIN) == 0;
}

inline bool leftOnLine() {
    return digitalRead(LEFT_IR_PIN) == 0;
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

inline void stopMotors() {
    digitalWrite(LEFT_MOTOR_PIN1, LOW);
    digitalWrite(LEFT_MOTOR_PIN2, LOW);

    digitalWrite(RIGHT_MOTOR_PIN1, LOW);
    digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}

inline void moveForward(int time) {
    while (time > 0) {
        int s = millis();
        bool l = leftOnLine(), r = rightOnLine();

        if ((l && r) || (!l && !r)) {
            speed(LEFT_FRD_SPD, RIGHT_FRD_SPD);
            rightWheelForward();
            leftWheelForward();
        } else if (r) {
            speed(LEFT_FRD_SPD, RIGHT_FRD_SPD - 30);
            rightWheelForward();
            leftWheelForward();
        } else { // l
            speed(LEFT_FRD_SPD - 30, RIGHT_FRD_SPD - 30);
            rightWheelForward();
            leftWheelForward();
        }

        delay(20);

        stopMotors();

        delay(10);

        time -= millis() - s;
    }
}

inline void turnRight() {
    speed(LEFT_TRN_SPD, RIGHT_TRN_SPD);
    
    for (int checks = 0; checks < 4;) {
        bool l = leftOnLine(), r = rightOnLine();
        bool reads[] = {l, !l, r, !r};

        if (reads[checks]) checks++;

        rightWheelBackward();
        leftWheelForward();

        delay(20);

        stopMotors();

        delay(10);
    }
}

inline void turnLeft() {
    speed(LEFT_TRN_SPD, RIGHT_TRN_SPD);
    
    for (int checks = 0; checks < 4;) {
        bool l = leftOnLine(), r = rightOnLine();
        bool reads[] = {r, !r, l, !l};

        if (reads[checks % 4]) checks++;

        rightWheelForward();
        leftWheelBackward();

        delay(20);

        stopMotors();

        delay(10);
    }
}

void reset() {
    print("resetting..\n");

    stopMotors();
    start = false;
    currentState = State::TAKE_DECISION;
    maze.position = Position(START_X, START_Y);
    maze.orientation = START_ORIENT;

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

    start  = true; // TODO, setup push button

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

            maze.printBlocks();
            print(":TAKE_DECISION: :BEFORE:"); {
                auto &x = maze.position.x, &y = maze.position.y;
                printv(x);
                printv(y);
                printOrientation(maze.orientation);
            }

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
            Direction absDir = maze.whereToGo(); // updates position
            Direction relativeDir = maze.calcRelativeDir(absDir);
            maze.orientation = maze.calcOrientation(relativeDir);

            print(" :AFTER:"); 

            if (absDir == Direction::STOP) {
                print(":STOPPED:");
                stopMotors();
                halt();
            }  else if (relativeDir == Direction::FRONT) {
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

            moveForward(TIME_MOVE);
            stopMotors();

            currentState = State::TAKE_DECISION;

            break;
        }

        case State::TURN_180: {
            print(":TURN_BACK:\n");

            turnRight();
            stopMotors();
            turnRight();
            stopMotors();
            delay(500);

            currentState = State::MOVE_FORWARD;
            
            break;
        }

        case State::TURN_RIGHT: {
            print(":TURN_RIGHT:\n");

            turnRight();
            stopMotors();
            delay(500);

            currentState = State::MOVE_FORWARD;
            
            break;
        }

        case State::TURN_LEFT:  {
            print(":TURN_LEFT:\n");

            turnLeft();
            stopMotors();
            delay(500);

            currentState = State::MOVE_FORWARD;
            
            break;
        }

        default: stopMotors();
    }
}
