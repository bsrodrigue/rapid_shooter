#ifndef CONFIG_H
#define CONFIG_H

#define FPS 120 // Better than Pokemon

// TODO: Handle heterogenous screen sizes
#define LEVEL_WIDTH 2500
#define LEVEL_HEIGHT 2500
#define WIN_WIDTH 900
#define WIN_HEIGHT 400

#define TEXT_POS(x) x * 5

#define CELL_SIZE 25
// TODO: Make Level Cell Count independent of Cell Size
// #define CELL_COUNT (int)(LEVEL_WIDTH / CELL_SIZE)
#define CELL_COUNT 100
#define CELL_PADDING 0

#define CELL_OFFSET(x) (x * (CELL_SIZE))
#define MOUSE_TO_GRID(x) (int)(x / CELL_SIZE)

#define BREAKABLE_WALL_HEALTH 0.1

#endif
