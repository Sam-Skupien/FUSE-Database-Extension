#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

void open_db(void);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
int write_get_bytes(char *user, int buf_size);
