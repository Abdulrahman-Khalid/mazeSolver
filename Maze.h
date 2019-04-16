#include <cstdint>
#include <cmath>
#include <cstdio>
#include <queue>

#define MAZE_HEIGHT 5
#define MAZE_LENGTH 5

#define TARGET_X 3
#define TARGET_Y 3

#define START_X 0
#define START_Y 0

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

    struct Position { uint8_t x, y; } position;

    Maze () :position({START_X, START_Y}) {
        for (int i = 0; i < MAZE_LENGTH; i++) {
            for (int j = 0; j < MAZE_HEIGHT; j++) {
                cells[i][j].value = abs(TARGET_X - i) + abs(TARGET_Y - j);

                cells[i][j].up = (j == 0);
                cells[i][j].left = (i == 0);
                cells[i][j].right = (i == MAZE_LENGTH-1);
                cells[i][j].down = (j == MAZE_HEIGHT-1);

                cells[i][j].visited = false;

                printf("%i[%i%i%i%i] ", cells[i][j].value, cells[i][j].right, cells[i][j].left, cells[i][j].up, cells[i][j].down);
            }
            printf("\n");
        }
    }

    void solve() {
        std::queue<Position> q;
        q.push({TARGET_X, TARGET_Y});

        while (!q.empty()) {
            Position p = q.front();
            q.pop();

            if (!cells[p.x+1][p.y].visited && cells[p.x][p.y].right == 0) {
                q.push({uint8_t(p.x + 1), p.y});
                cells[p.x+1][p.y].value = cells[p.x][p.y].value + 1;
            }

            if (!cells[p.x-1][p.y].visited && cells[p.x][p.y].left == 0) {
                q.push({uint8_t(p.x - 1), p.y});
                cells[p.x-1][p.y].value = cells[p.x][p.y].value + 1;
            }

            if (!cells[p.x][p.y+1].visited && cells[p.x][p.y].down == 0) {
                q.push({p.x, uint8_t(p.y + 1)});
                cells[p.x][p.y+1].value = cells[p.x][p.y].value + 1;
            }

            if (!cells[p.x][p.y-1].visited && cells[p.x][p.y].up == 0) {
                q.push({p.x, uint8_t(p.y - 1)});
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

    bool finisehd() {
        return position.x == TARGET_X && position.y == TARGET_Y;
    }
};