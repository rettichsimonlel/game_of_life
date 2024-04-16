#ifndef GAME_H
#define GAME_H

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define CELL_SIZE 20
#define GRID_WIDTH (SCREEN_WIDTH / CELL_SIZE)
#define GRID_HEIGHT (SCREEN_HEIGHT / CELL_SIZE)
#define MAX_INPUT_CHARS 10

#include <stdbool.h>

void initGrid();
int countNeighbors(int x, int y);
void updateGrid();
void drawGrid();
bool draw_names(char **names);
char *draw_input_field(char **names);
void whenPausedInput();
void universalInput();
void action_load(char **names);

#endif
