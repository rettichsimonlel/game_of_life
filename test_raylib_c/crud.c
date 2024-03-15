#include <sqlite3.h>
#include <stddef.h>
#include <stdio.h>
#include "crud.h"
#include "game.h"

int save_state(char *db_name, char *name, int grid[GRID_WIDTH][GRID_HEIGHT]) {
    sqlite3* DB;
    int error = sqlite3_open(db_name, &DB);

    if (error) {
        sqlite3_close(DB);
        return 1;
    }

    const char *create_table_sql = "CREATE TABLE IF NOT EXISTS State (ID INTEGER PRIMARY KEY AUTOINCREMENT, Name TEXT NOT NULL UNIQUE, GridData TEXT NOT NULL);";
    
    error = sqlite3_exec(DB, create_table_sql, NULL, 0, NULL);
    
    if (error != SQLITE_OK) {
	printf("SQL error: %s\n", sqlite3_errmsg(DB)); // Print SQLite error message
        sqlite3_close(DB);
        return 2;
    }

    // Convert grid data to a string
    char gridData[GRID_WIDTH * GRID_HEIGHT + 1];
    int index = 0;
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            gridData[index++] = grid[i][j] + '0'; // Convert integer to character
        }
    }
    gridData[index] = '\0'; // Null-terminate the string

    // Construct SQL insert statement
    char insert_sql[100 + GRID_WIDTH * GRID_HEIGHT + MAX_INPUT_CHARS];
    sprintf(insert_sql, "INSERT INTO State (GridData, Name) VALUES ('%s', '%s');", gridData, name);

    error = sqlite3_exec(DB, insert_sql, NULL, 0, NULL);

    if (error != SQLITE_OK) {
	printf("SQL error: %s\n", sqlite3_errmsg(DB)); // Print SQLite error message
	sqlite3_close(DB);
	return 3;
    }
    
    sqlite3_close(DB);

    return 0;
}

int load_state(char *db_name, char *name, int (*grid)[GRID_HEIGHT]) {
    sqlite3* DB;
    int error = sqlite3_open(db_name, &DB);

    if (error) {
        sqlite3_close(DB);
        return 1;
    }

    char query_sql[100];
    sprintf(query_sql, "SELECT GridData FROM State WHERE Name == '%s';", name);

    sqlite3_stmt *stmt;

    error = sqlite3_prepare_v2(DB, query_sql, -1, &stmt, NULL);
    if (error != SQLITE_OK) {
        sqlite3_close(DB);
        return 1;
    }

    // Execute the query
    error = sqlite3_step(stmt);
    if (error != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        return 1;
    }

    // Extract grid data from the result
    const unsigned char *gridData = sqlite3_column_text(stmt, 0);
    if (gridData == NULL) {
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        return 1;
    }

    // Parse grid data and update the grid array
    int index = 0;
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            grid[i][j] = gridData[index++] - '0'; // Convert character to integer
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    return 0;
}
