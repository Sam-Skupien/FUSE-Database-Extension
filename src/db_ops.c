#include "db_ops.h"
#include "ntapfuse_ops.h"


int write_get_bytes(char *user){

   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[100];
 

   rc = sqlite3_open("ntap.db", &db);

   if(rc) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
      
   } else {
      fprintf(stderr, "Opened database successfully\n");
      fflush(stdout);
   }

    strcpy(sql_str, "\"SELECT FREESPACE FROM USERS WHERE NAME = "); 
    strcat(sql_str, user);
    char *append = "\";";
    strcat(sql_str, append);

    log_msg("sql str from write: %s\n", sql_str);


   /* Execute SQL statement 
   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);

   } else {
      fprintf(stdout, "Users added successfully\n");
   }*/

   sqlite3_close(db);

   return 0;
}

void open_db(){

   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;

   rc = sqlite3_open("ntap.db", &db);

   if(rc) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
      
   } else {
      fprintf(stderr, "Opened database successfully\n");
      //fflush(stdout);
   }

   // initialize table of users
   sql = "CREATE TABLE USERS("  \
         "NAME TEXT PRIMARY KEY    NOT NULL," \
         "FREESPACE             INT     NOT NULL );";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);

   } else {
      fprintf(stdout, "Table created successfully\n");
   }


   // populate table
   sql = "INSERT INTO USERS(NAME, FREESPACE) " \
         "VALUES('user1', 1000); "
         "INSERT INTO USERS(NAME, FREESPACE) " \
         "VALUES('user2', 1000);"
         "INSERT INTO USERS(NAME, FREESPACE) " \
         "VALUES('user3', 100);";

   // sql = "INSERT INTO USERS(NAME, FREESPACE) " \
   //       "VALUES('sam-skupien', 1000000);";


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

