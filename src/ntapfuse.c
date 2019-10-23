/**
 * Project: ntapfuse
 * Authors: Samuel Kenney <samuel.kenney48@gmail.com>
 *          August Sodora III <augsod@gmail.com>
 * File: ntapfuse.c
 * License: GPLv3
 *
 * ntapfuse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ntapfuse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ntapfuse. If not, see <http://www.gnu.org/licenses/>.
 */
#define _XOPEN_SOURCE 500

#include "ntapfuse_ops.h"

#include <stdio.h>
#include <stdlib.h>

#include <limits.h>
#include <fuse.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>

FILE* log_open(char *base);
void open_db(void);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);

struct fuse_operations ntapfuse_ops = {
  .getattr = ntapfuse_getattr,
  .readlink = ntapfuse_readlink,
  .mknod = ntapfuse_mknod,
  .mkdir = ntapfuse_mkdir,
  .unlink = ntapfuse_unlink,
  .rmdir = ntapfuse_rmdir,
  .symlink = ntapfuse_symlink,
  .rename = ntapfuse_rename,
  .link = ntapfuse_link,
  .chmod = ntapfuse_chmod,
  .chown = ntapfuse_chown,
  .truncate = ntapfuse_truncate,
  .utime = ntapfuse_utime,
  .open = ntapfuse_open,
  .read = ntapfuse_read,
  .write = ntapfuse_write,
  .statfs = ntapfuse_statfs,
  .release = ntapfuse_release,
  .fsync = ntapfuse_fsync,
  .setxattr = ntapfuse_setxattr,
  .getxattr = ntapfuse_getxattr,
  .listxattr = ntapfuse_listxattr,
  .removexattr = ntapfuse_removexattr,
  .opendir = ntapfuse_opendir,
  .readdir = ntapfuse_readdir,
  .releasedir = ntapfuse_releasedir,
  .access = ntapfuse_access,
  .init = ntapfuse_init,
};

void
usage ()
{
  printf ("ntapfuse mount <basedir> <mountpoint>\n");
  
  exit (0);
}

int
main (int argc, char *argv[])
{
  if (argc < 3)
    usage ();

  char *command = argv[1];
  char *path = argv[2];

  char fpath[PATH_MAX];
  if (realpath (path, fpath) == NULL)
    perror ("main_realpath");

  if (strcmp (command, "mount") == 0)
    {
      if (argc < 4)
	usage ();

      if (realpath (argv[2], base) == NULL)
	perror ("main_realpath");

//ntapfuse mount <basedir> <mountpoint> -> ntapfuse <mountpoint>
      int i = 1;
      for (; i < argc; i++)
	argv[i] = argv[i + 2];
      argc -= 2;

    //open logfile
    FILE *logfile = log_open(base);
    struct private_state *private_data = malloc(sizeof(struct private_state));
    private_data->base =(char *)base;
    private_data->logfile = logfile;

      int ret = fuse_main (argc, argv, &ntapfuse_ops, private_data);
      if (ret < 0)
	perror ("fuse_main");

      //close the log file
      fclose(logfile);
      return ret;
    }
  else
    usage ();

  return 0;
}

FILE* log_open(char *base){
  //create a log file
  char filename[1024];
  strcpy(filename, base);
  strcat(filename, "/ntapfuse.log");
  
  FILE *logfile = fopen(filename, "w");
  fflush(logfile);

  //open database
  open_db();

  //pass the log file pointer to the fuse_get_context()->private_data for further use.
  return logfile;
}

void open_db(){

   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;

   rc = sqlite3_open("ntap.db", &db);

   if(rc) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      printf("can't open db");
      fflush(stdout);
      
   } else {
      fprintf(stderr, "Opened database successfully\n");
      //fflush(stdout);
   }

   // initialize table of users
   sql = "CREATE TABLE USERS("  \
         "NAME TEXT PRIMARY KEY    NOT NULL," \
         "SPACE             INT     NOT NULL );";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);

   } else {
      fprintf(stdout, "Table created successfully\n");
   }


   // populate table
   sql = "INSERT INTO USERS(NAME, SPACE) " \
         "VALUES('user1', 10000); "
         "INSERT INTO USERS(NAME, SPACE) " \
         "VALUES('user2', 10000);"
         "INSERT INTO USERS(NAME, SPACE) " \
         "VALUES('user3', 10000);";


   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);

   } else {
      fprintf(stdout, "Users added successfully\n");
   }

   sqlite3_close(db);
   // close this when filesystem is unmounted
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}
