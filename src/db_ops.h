#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

void open_db(char *);
static int insert_callback(void *NotUsed, int argc, char **argv, char **azColName);
static int update_callback(void *data, int argc, char **argv, char **azColName);
int write_get_bytes(char *user, int file_size);
void write_rollback(char *user, int file_size);

int unlink_get_bytes(char *user, int file_size);
void unlink_rollback(char *user, int file_size);

int truncate_add_bytes(char *user, int size);
void truncate_add_rollback(char *user, int size);

int truncate_remove_bytes(char *user, int size);
void truncate_remove_rollback(char *user, int size);

int get_num_dirs(char *user);
void mkdir_rollback(char *user);

int rem_num_dirs(char *user);
void rmdir_rollback(char *user);
