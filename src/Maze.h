#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <cppQueue.h>

#define MAZE_HEIGHT 3
#define MAZE_LENGTH 2

#define TARGET_X 2
#define TARGET_Y 2

#define START_X 0
#define START_Y 0
#define START_ORIENT SOUTH

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

    struct Position { uint8_t x, y; } position;
    enum Orientation { NORTH, EAST, SOUTH, WEST } orientation;

    Maze () :position({START_X, START_Y}), orientation(START_ORIENT), 
            q(sizeof(Position), MAZE_HEIGHT*MAZE_LENGTH) {
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
            newPos = {position.x, uint8_t(position.y-1)};
        }

        if (c.right == 0 && cells[position.x+1][position.y].value < minvalue) {
            minvalue = cells[position.x+1][position.y].value;
            dir = RIGHT;
            newPos = {uint8_t(position.x+1), position.y};
        }

        if (c.left == 0 && cells[position.x-1][position.y].value < minvalue) {
            minvalue = cells[position.x-1][position.y].value;
            dir = LEFT;
            newPos = {uint8_t(position.x-1), position.y};
        }

        if (c.down == 0 && cells[position.x][position.y+1].value < minvalue) {
            minvalue = cells[position.x][position.y+1].value;
            dir = BACK;
            newPos = {position.x, uint8_t(position.y+1)};
        }

        position = newPos;
        return dir;
    }

    inline void updateOrientation(Direction dir) {
        orientation = Orientation((orientation + dir) & 0b11);
    }

    inline void updateAdjacentWalls(bool frontBlocked, bool rightBlocked, bool leftBlocked) {
        auto& c = cells[position.x][position.y];

        switch (orientation){
            case EAST: {
                c.right = frontBlocked;
                c.up = leftBlocked;
                c.down = rightBlocked;

                break;
            } 

            case WEST: {
                c.left = frontBlocked;
                c.up = rightBlocked;
                c.down = leftBlocked;

                break;
            }

            case SOUTH: {
                c.right = leftBlocked;
                c.left = rightBlocked;
                c.down = frontBlocked;

                break;
            }
        
            default: {
                c.right = rightBlocked;
                c.left = leftBlocked;
                c.up = frontBlocked;

                break;
            }
        }

        if (position.x+1 < MAZE_LENGTH) cells[position.x+1][position.y].left = c.right;
        if (position.x > 0) cells[position.x-1][position.y].right = c.left;
        if (position.y+1 < MAZE_HEIGHT) cells[position.x][position.y+1].down = c.up;
        if (position.y > 0) cells[position.x][position.y-1].up = c.down;
    }

    void updateCellsValues() {
        Position p = {TARGET_X, TARGET_Y};
        
        q.flush();
        q.push(&p);

        while (q.pop(&p)) {
            Position p2;

            if (cells[p.x][p.y].right == 0 && !cells[p.x+1][p.y].visited) {
                p2 = {uint8_t(p.x + 1), p.y};
                q.push(&p2);
                cells[p.x+1][p.y].value = cells[p.x][p.y].value + 1;
            }

            if (cells[p.x][p.y].left == 0 && !cells[p.x-1][p.y].visited) {
                p2 = {uint8_t(p.x - 1), p.y};
                q.push(&p2);
                cells[p.x-1][p.y].value = cells[p.x][p.y].value + 1;
            }

            if (cells[p.x][p.y].down == 0 && !cells[p.x][p.y+1].visited) {
                p2 = {p.x, uint8_t(p.y + 1)};
                q.push(&p2);
                cells[p.x][p.y+1].value = cells[p.x][p.y].value + 1;
            }

            if (cells[p.x][p.y].up == 0 && !cells[p.x][p.y-1].visited) {
                p2 = {p.x, uint8_t(p.y - 1)};
                q.push(&p2);
                cells[p.x][p.y-1].value = cells[p.x][p.y].value + 1;
            }

            cells[p.x][p.y].visited = true;
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
};