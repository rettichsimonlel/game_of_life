#include <stdio.h>

#include "raylib.h"
#include "crud.h"
#include "game.h"

int grid[GRID_WIDTH][GRID_HEIGHT];
int nextGrid[GRID_WIDTH][GRID_HEIGHT];
bool isPaused = true;
int currentFPS = 60;

// Function to initialize the grid randomly
void initGrid() {
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            grid[i][j] = 0;
        }
    }
    isPaused = true;
}

// Function to count the number of alive neighbors of a cell
int countNeighbors(int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int neighborX = (x + i + GRID_WIDTH) % GRID_WIDTH;
            int neighborY = (y + j + GRID_HEIGHT) % GRID_HEIGHT;
            count += grid[neighborX][neighborY];
        }
    }
    count -= grid[x][y]; // Exclude the cell itself
    return count;
}

// Function to update the grid based on the rules of the game
void updateGrid() {
    if (!isPaused) {
        for (int i = 0; i < GRID_WIDTH; i++) {
            for (int j = 0; j < GRID_HEIGHT; j++) {
                int neighbors = countNeighbors(i, j);
                if (grid[i][j] == 1 && (neighbors < 2 || neighbors > 3)) {
                    nextGrid[i][j] = 0; // Cell dies due to underpopulation or overpopulation
                } else if (grid[i][j] == 0 && neighbors == 3) {
                    nextGrid[i][j] = 1; // Cell is born
                } else {
                    nextGrid[i][j] = grid[i][j]; // Cell remains the same
                }
            }
        }

        // Copy the nextGrid to grid
        for (int i = 0; i < GRID_WIDTH; i++) {
            for (int j = 0; j < GRID_HEIGHT; j++) {
                grid[i][j] = nextGrid[i][j];
            }
        }
    }
}

// Function to draw the grid
void drawGrid() {
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            if (grid[i][j] == 1) {
                DrawRectangle(i * CELL_SIZE, j * CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
            }
        }
    }
}

void whenPausedInput() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
	int mouseX = GetMouseX() / CELL_SIZE;
	int mouseY = GetMouseY() / CELL_SIZE;
	if (mouseX >= 0 && mouseX < GRID_WIDTH && mouseY >= 0 && mouseY < GRID_HEIGHT) {
	    grid[mouseX][mouseY] = !grid[mouseX][mouseY];
	}
    }

    if (IsKeyPressed(KEY_N)) {
	initGrid();
    }

    if (IsKeyPressed(KEY_S)) {
	// Save the state
	int error = save_state("./state.db", grid);
	if (error > 0) {
	    printf("Error in the opening of the db: %d\n", error);
	}
    }

    if (IsKeyPressed(KEY_L)) {
	// Load the state
	if (load_state("./state.db", grid) == 1) {
	    printf("Error in loading the state.\n");
	}
    }
}

void universalInput() {
    if (IsKeyPressed(KEY_UP)) {
	currentFPS += 5;
    }
    if (IsKeyPressed(KEY_DOWN)) {
	currentFPS -= 5;
    }

    if (IsKeyPressed(KEY_SPACE)) {
	isPaused = !isPaused;
    }
}

int main() {
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game of Life");


    initGrid();

    // Main game loop
    while (!WindowShouldClose()) {

	SetTargetFPS(currentFPS);

	if (isPaused) {
	    whenPausedInput();
	}

	universalInput();

        updateGrid();

        // Draw
        BeginDrawing();
        ClearBackground(BLACK);
        drawGrid();

	DrawFPS(10, 10);
	if (isPaused) {
	    DrawText("Off", 30, SCREEN_HEIGHT - 50, 20, RED);
	}
	else {
	    DrawText("On", 30, SCREEN_HEIGHT - 50, 20, RED);
	}
	
        EndDrawing();
    }

    // De-Initialization
    CloseWindow();
    return 0;
}
