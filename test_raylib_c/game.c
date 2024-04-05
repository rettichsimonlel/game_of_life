#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "crud.h"
#include "game.h"

int grid[GRID_WIDTH][GRID_HEIGHT];
int nextGrid[GRID_WIDTH][GRID_HEIGHT];
bool isPaused = true;
bool inWriting = false;
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

void draw_names(char **names) {

    for (int i = 0; names[i] != NULL; i++) {
	DrawText(names[i], 30, SCREEN_HEIGHT / 2 + (i*30), 20, RED);
    }

}

char *draw_input_field(char **names) {
    inWriting = true;

    char *name = (char *)malloc((MAX_INPUT_CHARS + 1) * sizeof(char));
    name[0] = '\0';
    int letterCount = 0;

    Rectangle textBox = { SCREEN_WIDTH/2.0f - 100, 180, 225, 50 };
    bool mouseOnText = false;

    int framesCounter = 0;

    while (inWriting) {

	if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
	else mouseOnText = false;

	if (mouseOnText)
	{
	    // Set the window's cursor to the I-Beam
	    SetMouseCursor(MOUSE_CURSOR_IBEAM);

	    // Get char pressed (unicode character) on the queue
	    int key = GetCharPressed();

	    // Check if more characters have been pressed on the same frame
	    while (key > 0)
	    {
		// NOTE: Only allow keys in range [32..125]
		if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
		{
		    name[letterCount] = (char)key;
		    name[letterCount+1] = '\0'; // Add null terminator at the end of the string.
		    letterCount++;
		}

		key = GetCharPressed();  // Check next character in the queue
	    }

	    if (IsKeyPressed(KEY_BACKSPACE))
	    {
		letterCount--;
		if (letterCount < 0) letterCount = 0;
		name[letterCount] = '\0';
	    }
	}
	else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

	if (mouseOnText) framesCounter++;
	else framesCounter = 0;

	if (IsKeyPressed(KEY_ENTER))
	{
	    inWriting = false;
	    break;
	}

	BeginDrawing();

	    ClearBackground(RAYWHITE);

	    if (names != NULL) {
		draw_names(names);
	    }

	    DrawText("PLACE MOUSE OVER INPUT BOX!", 240, 140, 20, GRAY);

	    DrawRectangleRec(textBox, LIGHTGRAY);
	    if (mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
	    else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);

	    DrawText(name, (int)textBox.x + 5, (int)textBox.y + 8, 40, MAROON);

	    DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_INPUT_CHARS), 315, 250, 20, DARKGRAY);

	    if (mouseOnText)
	    {
		if (letterCount < MAX_INPUT_CHARS)
		{
		    // Draw blinking underscore char
		    if (((framesCounter/20)%2) == 0) DrawText("_", (int)textBox.x + 8 + MeasureText(name, 40), (int)textBox.y + 12, 40, MAROON);
		}
		else DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
	    }

	EndDrawing();
    }

    printf("This is the name: %s\n", name);
    return name;
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
        char *name = draw_input_field(NULL);
	printf("Still the name but different: %s\n", name);
	int error = save_state("./state.db", name, grid);
	if (error > 0) {
	    printf("Error in the opening of the db: %d\n", error);
	}
	free(name);
    }

    if (IsKeyPressed(KEY_L)) {
	// Load the state
	char **names = load_file_names("./state.db");
	if (names == NULL) {
	    printf("Error loading file names.\n");
	    // Handle error
	} else {
	    printf("These names:\n");
	    // Iterate through the array of strings and print each name
	    for (int i = 0; names[i] != NULL; i++) {
		printf("%s\n", names[i]);
	    }

	    // Prompt user to enter the name
	    char *name = draw_input_field(names);
	    if (name == NULL) {
		printf("Error reading input.\n");
		// Handle error
	    } else {
		if (load_state("./state.db", name, grid) == 1) {
		    printf("Error in loading the state.\n");
		    // Handle error
		}
		free(name);
	    }

	    // Free the array of strings
	    for (int i = 0; names[i] != NULL; i++) {
		free(names[i]);
	    }
	    free(names);
	}
    }

    /* if (IsKeyPressed(KEY_L)) { */
    /* 	// Load the state */
    /* 	char **names = load_file_names("./state.db"); */
    /* 	printf("These names:\n"); */
    /* 	printf("%s\n", names); */
    /* 	char *name = draw_input_field(); */
    /* 	if (load_state("./state.db", name, grid) == 1) { */
    /* 	    printf("Error in loading the state.\n"); */
    /* 	} */
    /* 	free(name); */
    /* } */
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
