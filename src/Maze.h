#include "common.h"
#include <cppQueue.h>

struct Cell {
    bool right:1;
    bool left:1;
    bool up:1;
    bool down:1;
    bool visited:1;
    uint8_t:3;

    uint8_t value;
};

class Maze {
public:
    //      [     X     ][     Y     ]
    Cell cells[MAZE_LENGTH][MAZE_HEIGHT];
    Queue q;

    struct Position { 
        uint8_t x, y; 
        Position(uint8_t x, uint8_t y): x(x), y(y) {}
    } position;

    enum Orientation { NORTH, EAST, SOUTH, WEST } orientation;

    Maze () :position(START_X, START_Y), orientation(START_ORIENT), 
            q(sizeof(Position), MAZE_HEIGHT*MAZE_LENGTH) {}
 
    void init() {
        for (int i = 0; i < MAZE_LENGTH; i++) {
            for (int j = 0; j < MAZE_HEIGHT; j++) {
                cells[i][j].value = abs(TARGET_X - i) + abs(TARGET_Y - j);

                cells[i][j].up = (j == 0);
                cells[i][j].left = (i == 0);
                cells[i][j].right = (i == MAZE_LENGTH-1);
                cells[i][j].down = (j == MAZE_HEIGHT-1);
                
                cells[i][j].visited = false;
            }
        }
    }


    enum Direction { FRONT, RIGHT, BACK, LEFT, STOP };

    inline Direction whereToGo() {
        uint8_t minvalue = UINT8_MAX;
        Direction dir = STOP;
        Position newPos = position;
        auto& c = cells[position.x][position.y];

        if (c.up == 0 && cells[position.x][position.y-1].value < minvalue) {
            minvalue = cells[position.x][position.y-1].value;
            dir = FRONT;
            newPos = Position(position.x, position.y-1);

            assert(newPos.y != 255);
        }

        if (c.right == 0 && cells[position.x+1][position.y].value < minvalue) {
            minvalue = cells[position.x+1][position.y].value;
            dir = RIGHT;
            newPos = Position(position.x+1, position.y);
        }

        if (c.left == 0 && cells[position.x-1][position.y].value < minvalue) {
            minvalue = cells[position.x-1][position.y].value;
            dir = LEFT;
            newPos = Position(position.x-1, position.y);

            assert(newPos.x != 255);
        }

        if (c.down == 0 && cells[position.x][position.y+1].value < minvalue) {
            minvalue = cells[position.x][position.y+1].value;
            dir = BACK;
            newPos = Position(position.x, position.y+1);
        }

        assert(newPos.x < MAZE_LENGTH && newPos.y < MAZE_HEIGHT);

        position = newPos;
        return dir;
    }

    inline Orientation calcOrientation(Direction relativeDir) const {
        return Orientation((orientation + relativeDir) % 4);
    }

    inline Direction calcRelativeDir(Direction absDir) const {
        return Direction((absDir - orientation + 4) % 4);
    }

    inline void updateAdjacentWalls(bool frontBlocked, bool rightBlocked, bool leftBlocked) {
        auto& c = cells[position.x][position.y];

        switch (orientation) {
            case EAST: {
                c.right |= frontBlocked;
                c.up |= leftBlocked;
                c.down |= rightBlocked;

                break;
            } 

            case WEST: {
                c.left |= frontBlocked;
                c.up |= rightBlocked;
                c.down |= leftBlocked;

                break;
            }

            case SOUTH: {
                c.right |= leftBlocked;
                c.left |= rightBlocked;
                c.down |= frontBlocked;

                break;
            }
        
            default: {
                c.right |= rightBlocked;
                c.left |= leftBlocked;
                c.up |= frontBlocked;

                break;
            }
        }

        if (position.x+1 < MAZE_LENGTH) cells[position.x+1][position.y].left = c.right;
        if (position.x > 0) cells[position.x-1][position.y].right = c.left;
        if (position.y+1 < MAZE_HEIGHT) cells[position.x][position.y+1].up = c.down;
        if (position.y > 0) cells[position.x][position.y-1].down = c.up;
    }

    void updateCellsValues() {
        Position p(TARGET_X, TARGET_Y);
        
        q.flush();
        assert(q.isEmpty());

        auto result = q.push(&p);
        assert(result);

        while (q.pop(&p)) {
            auto& c = cells[p.x][p.y];
            Position p2(0, 0);

            if (c.right == 0 && !cells[p.x+1][p.y].visited) {
                assert(p.x != MAZE_LENGTH-1);

                p2 = Position(p.x + 1, p.y);
                q.push(&p2);
                cells[p.x+1][p.y].value = c.value + 1;
            }

            if (c.left == 0 && !cells[p.x-1][p.y].visited) {
                assert(p.x != 0);

                p2 = Position(p.x - 1, p.y);

                assert(p2.x != 255);

                q.push(&p2);
                cells[p.x-1][p.y].value = c.value + 1;
            }

            if (c.down == 0 && !cells[p.x][p.y+1].visited) {
                assert(p.y != MAZE_HEIGHT-1);

                p2 = Position(p.x, p.y + 1);
                q.push(&p2);
                cells[p.x][p.y+1].value = c.value + 1;
            }

            if (c.up == 0 && !cells[p.x][p.y-1].visited) {
                assert(p.y != 0);

                p2 = Position(p.x, p.y - 1);

                assert(p2.y != 255);

                q.push(&p2);
                cells[p.x][p.y-1].value = c.value + 1;
            }

            c.visited = true;
        }

        for (int i = 0; i < MAZE_LENGTH; i++) {
            for (int j = 0; j < MAZE_HEIGHT; j++) {
                cells[i][j].visited = false;
            }
        }
    }

    bool finished() {
        return position.x == TARGET_X && position.y == TARGET_Y;
    }

    void printBlocks() {
        print("\n");
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            for (int x = 0; x < MAZE_LENGTH; x++) {
                if (x == position.x && y == position.y) {
                    switch (orientation) {
                        case NORTH:
                            print("^"); break;
                        case EAST:
                            print(">"); break;
                        case WEST:
                            print("<"); break;
                        case SOUTH:
                            print("v"); break;
                        default: break;
                    }
                } else {
                    print(cells[x][y].value);
                }
                print(" ");
            }
            print("\n");
        }
    }
};