#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SQUARE_SIZE 50
#define MAX_REC 100

int GAME_ON = 0;
Rectangle all_rec[MAX_REC];

void handle_input(Camera2D *, int *, int *, int *);

int main()
{
    const int screenWidth = 1200;
    const int screenHeight = 800;

    int number = 0;

    int current_x = 0;
    int current_y = 0;

    InitWindow(screenWidth, screenHeight, "Game of Life");

    Camera2D camera = {0};
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        handle_input(&camera, &current_x, &current_y, &number);
	
        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);

	{
	    Rectangle square = { current_x, current_y, SQUARE_SIZE, SQUARE_SIZE };

	    int yes_or_no = 0;

	    for (int i = 0; i < number; i++) {
		if (all_rec[i].x == square.x && all_rec[i].y == square.y) {
		    yes_or_no = 1;
		}
	    }

	    if (yes_or_no == 0) {
		 all_rec[number] = square;
		 number++;
	    }

	    for (int i = 0; i < number; i++) {
		DrawRectangleRec(all_rec[i], RED);
	    }
	}

        rlPushMatrix();

        rlTranslatef(0, 25 * SQUARE_SIZE, 0);
        rlRotatef(90, 1, 0, 0);
        DrawGrid(100, 50);

        rlPopMatrix();

        EndMode2D();

        DrawText("Left right button drag to move, mouse wheel to zoom", 10, 10, 20, WHITE);
        DrawText("Right click to add a square", 10, 40, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void handle_input(Camera2D *camera, int *current_x, int *current_y, int *current)
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
	Vector2 delta = GetMouseDelta();
	delta = Vector2Scale(delta, -1.0f / camera->zoom);

	camera->target = Vector2Add(camera->target, delta);
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
	Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), *camera);

	camera->offset = GetMousePosition();

	camera->target = mouseWorldPos;

	const float zoomIncrement = 0.125f;

	camera->zoom += (wheel * zoomIncrement);
	if (camera->zoom < zoomIncrement)
	    camera->zoom = zoomIncrement;
    }

    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && GAME_ON == 0) {
	Vector2 mousePos = GetMousePosition();
	Vector2 gridPos = GetScreenToWorld2D(mousePos, *camera);

	int row = (int)(gridPos.y / SQUARE_SIZE);
	int col = (int)(gridPos.x / SQUARE_SIZE);

	if (col <= 0)
	    *current_x = col * SQUARE_SIZE - SQUARE_SIZE;
	else
	    *current_x = col * SQUARE_SIZE;

	if (row <= 0)
	    *current_y = row * SQUARE_SIZE - SQUARE_SIZE;
	else
	    *current_y = row * SQUARE_SIZE;
    }

    /* if (IsKeyPressed(KEY_SPACE)) { */
    /* 	if (GAME_ON == 1) { */
    /* 	    SetTargetFPS(60); */
    /* 	    GAME_ON = 0; */
    /* 	    return; */
    /* 	} */
    /* 	GAME_ON = 1; */
    /* 	SetTargetFPS(10); */

    /* 	for (int i = 0; i < current; i++) { */
    /* 	    for (int j = 0; j < current; j++) { */
    /* 	    } */
    /* 	} */
    /* } */

    if (IsKeyPressed(KEY_SPACE)) {
        if (GAME_ON == 1) {
            GAME_ON = 0;
        } else {
            GAME_ON = 1;
        }
    }

    if (GAME_ON == 1) {
        DrawText("Game on", 600, 600, 50, RED);
	
	static double lastUpdateTime = 0;
        double currentTime = GetTime();
        double deltaTime = currentTime - lastUpdateTime;
        
        // Update the generation every second
        if (deltaTime >= 1.0) {
            lastUpdateTime = currentTime;

	    Rectangle next_gen[MAX_REC];
	    int next_gen_count = 0;

	    for (int i = 0; i < *current; i++) {
		int live_neighbors = 0;

		for (int j = 0; j < *current; j++) {
		    if (i != j) {
			int dx = abs(all_rec[j].x - all_rec[i].x);
			int dy = abs(all_rec[j].y - all_rec[i].y);

			if ((dx == SQUARE_SIZE && dy == 0) || (dx == 0 && dy == SQUARE_SIZE) ||
			    (dx == SQUARE_SIZE && dy == SQUARE_SIZE)) {
			    live_neighbors++;
			}
		    }
		}

		// Check the rules of the game
		if (live_neighbors == 2 || live_neighbors == 3) {
		    // Live cell survives
		    next_gen[next_gen_count++] = all_rec[i];
		}

		// Check for reproduction
		for (int dx = -SQUARE_SIZE; dx <= SQUARE_SIZE; dx += SQUARE_SIZE) {
		    for (int dy = -SQUARE_SIZE; dy <= SQUARE_SIZE; dy += SQUARE_SIZE) {
			if (dx != 0 || dy != 0) {
			    Rectangle neighbor = {all_rec[i].x + dx, all_rec[i].y + dy, SQUARE_SIZE, SQUARE_SIZE};
			    int neighbor_count = 0;

			    for (int k = 0; k < *current; k++) {
				if (abs(all_rec[k].x - neighbor.x) <= SQUARE_SIZE &&
				    abs(all_rec[k].y - neighbor.y) <= SQUARE_SIZE) {
				    neighbor_count++;
				}
			    }

			    if (neighbor_count == 3) {
				next_gen[next_gen_count++] = neighbor;
			    }
			}
		    }
		}
	    }

	    // Update all_rec with next generation
	    *current = next_gen_count;
	    for (int i = 0; i < next_gen_count; i++) {
		all_rec[i] = next_gen[i];
	    }
	    sleep(1);
	}
    } else {
        DrawText("Game off", 600, 600, 50, RED);
    }
}
