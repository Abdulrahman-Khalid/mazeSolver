#include <stdint.h>

#define MAZE_HEIGHT 5
#define MAZE_LENGTH 5

#define TARGET_X 3
#define TARGET_Y 3

//      [     Y     ][     X     ]
uint8_t maze[MAZE_HEIGHT][MAZE_LENGTH];

static inline int distance(uint8_t x, uint8_t y, uint8_t x1, uint8_t y1) {
    return sqrt((x1 - x) * (x1 - x) + (y1 - y) * (y1 - y));
}

void maze_init() {
    for (int i = 0; i < MAZE_HEIGHT; i++)
        for (int j = 0; j < MAZE_LENGTH; j++)
            maze[i][j] = distance(j, i, TARGET_X, TARGET_Y);
}

/* get maximum 8 blocks around {x, y}
 * @x, @y: current position
 * @next: array of 16 integers to hold Xs and Ys of output
 *    next = [x0, y0, x1, y1, x2, y2 .... x7, y7]
 * 
 * @return: size of `next`, from 0 to 8
 */
uint8_t maze_getNearBlocks(uint8_t x, uint8_t y, uint8_t next*) {
    int i = 0;

    if (x < MAZE_LENGTH) {
        next[i++] = x+1;
        next[i++] = y; 

        if (y < MAZE_HEIGHT) {
            next[i++] = x+1;
            next[i++] = y+1;
        }

        if (y > 0) {
            next[i++] = x+1;
            next[i++] = y-1;
        }
    }

    if (x > 0) {
        next[i++] = x-1;
        next[i++] = y;

        if (y < MAZE_HEIGHT) {
            next[i++] = x-1;
            next[i++] = y+1;
        }

        if (y > 0) {
            next[i++] = x-1;
            next[i++] = y-1;
        }
    }

    if (y < MAZE_HEIGHT) {
        next[i++] = x;
        next[i++] = y+1;
    }

    if (y > 0) {
        next[i++] = x;
        next[i++] = y-1;
    }

    return i;
}