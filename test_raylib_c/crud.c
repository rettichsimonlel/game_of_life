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
char **load_file_names(char *db_name) {
    sqlite3* DB;
    int error = sqlite3_open(db_name, &DB);
    if (error) {
        sqlite3_close(DB);
        return NULL;
    }

    char query_sql[100];
    sprintf(query_sql, "SELECT Name FROM State;");

    sqlite3_stmt *stmt;
    error = sqlite3_prepare_v2(DB, query_sql, -1, &stmt, NULL);
    if (error != SQLITE_OK) {
        sqlite3_close(DB);
        return NULL;
    }

    // Allocate memory for the array of strings
    char **names_array = malloc(sizeof(char*));
    if (names_array == NULL) {
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        return NULL;
    }
    int count = 0;

    // Fetch all rows
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        // Get the name from the current row
        const unsigned char *name = sqlite3_column_text(stmt, 0);
        if (name != NULL) {
            // Allocate memory for the name and copy it
            names_array[count] = malloc(strlen((char*)name) + 1);
            if (names_array[count] == NULL) {
                // Handle allocation failure
                for (int i = 0; i < count; ++i)
                    free(names_array[i]);
                free(names_array);
                sqlite3_finalize(stmt);
                sqlite3_close(DB);
                return NULL;
            }
            strcpy(names_array[count], (char*)name);
            ++count;
            // Resize the array of strings
            names_array = realloc(names_array, (count + 1) * sizeof(char*));
            if (names_array == NULL) {
                // Handle allocation failure
                for (int i = 0; i < count; ++i)
                    free(names_array[i]);
                free(names_array);
                sqlite3_finalize(stmt);
                sqlite3_close(DB);
                return NULL;
            }
        }
    }

    // Null-terminate the array of strings
    names_array[count] = NULL;

    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(DB);

    return names_array;
}

/* char **load_file_names(char *db_name) { */
/*     sqlite3* DB; */
/*     int error = sqlite3_open(db_name, &DB); */
/*     if (error) { */
/*         sqlite3_close(DB); */
/*         return NULL; */
/*     } */

/*     char query_sql[100]; */
/*     sprintf(query_sql, "SELECT Name FROM State;"); */

/*     sqlite3_stmt *stmt; */

/*     error = sqlite3_prepare_v2(DB, query_sql, -1, &stmt, NULL); */
/*     if (error != SQLITE_OK) { */
/*         sqlite3_close(DB); */
/*         return NULL; */
/*     } */
/*     // Execute the query */
/*     error = sqlite3_step(stmt); */
/*     if (error != SQLITE_ROW) { */
/*         sqlite3_finalize(stmt); */
/*         sqlite3_close(DB); */
/*         return NULL; */
/*     } */
/*     // Extract grid data from the result */
/*     const unsigned char *names = sqlite3_column_text(stmt, 0); */
/*     if (names == NULL) { */
/*         sqlite3_finalize(stmt); */
/*         sqlite3_close(DB); */
/*         return NULL; */
/*     } */
    
/*     return names; */
/* } */

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
