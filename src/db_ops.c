#include "db_ops.h"
#include "ntapfuse_ops.h"

/*
  This function is used after fuse_main() is called, becasue it use the fuse_get_context()->private_data
  This function check if a user has enough free space to write the bytes into files.
  If the user has enough free space in the database, update the new free space in the databse and return number of bytes written which is >= 0
  If not , return 0 
*/
int write_get_bytes(char *user, int file_size){

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_bytes;
   int bytes_remaining;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT FREESPACE FROM USERS WHERE NAME = '%s';", user);
   log_msg("Sql str from write: %s\nBytes:  %d\n", sql_str, file_size);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_bytes = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of bytes that would remain if write is complete
   bytes_remaining = total_bytes - file_size;
   log_msg("Bytes remaining: %d\n", bytes_remaining);

   // clear buffer
   strcpy(sql_str, ""); 

   // if bytes remaining in user account is greater than or equal to
   //  then write can proceed. else return 0 indicating not enough space
   if(bytes_remaining >= 0){
       
       // Create sql statement for update
       sprintf(sql_str, "UPDATE USERS set FREESPACE = %d where NAME = '%s';", bytes_remaining, user);

       log_msg("%s\n\n", sql_str);
 
       // execute sql statement, close db then return 1. 
       rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
         sqlite3_close(db);
        return file_size;
    } else {
        sqlite3_close(db);
        return -1;
    }
}










/* Rolls back the write function if pwrite fails in ntapfuse_ops.c */
void write_rollback(char *user, int file_size){

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_bytes;
   int bytes_removed;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT FREESPACE FROM USERS WHERE NAME = '%s';", user);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_bytes = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of bytes that will be subtracted from quota
   bytes_removed = total_bytes + file_size;

   // clear buffer
   strcpy(sql_str, ""); 
       
   // Create sql statement for update
   sprintf(sql_str, "UPDATE USERS set FREESPACE = %d where NAME = '%s';", bytes_removed, user);
 
   // execute sql statement, close db then return 1. 
   rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
   sqlite3_close(db);
}





int unlink_get_bytes(char *user, int file_size){

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_bytes;
   int bytes_added;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT FREESPACE FROM USERS WHERE NAME = '%s';", user);
   log_msg("Sql str from unlink: %s\nBytes:  %d\n", sql_str, file_size);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_bytes = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of bytes that would remain if write is complete
   bytes_added = total_bytes + file_size;
   log_msg("Bytes added: %d\n", bytes_added);

   // clear buffer
   strcpy(sql_str, ""); 
       
   // Create sql statement for update
   sprintf(sql_str, "UPDATE USERS set FREESPACE = %d where NAME = '%s';", bytes_added, user);

   log_msg("%s\n\n", sql_str);
 
   // execute sql statement, close db then return 1. 
   rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
   sqlite3_close(db);
   return file_size;

} 






/* Rollsback the unlink function if it fails in ntapfuse_ops.c
  It gets the file size of the file that could not be unlinked and gets 
  the amount of bytes left from a users quota. It then subracts the file
  size from the users quota and commits it to the database. 
*/
void unlink_rollback(char *user, int file_size){

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_bytes;
   int bytes_removed;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT FREESPACE FROM USERS WHERE NAME = '%s';", user);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_bytes = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of bytes that will be subtracted from quota
   bytes_removed = total_bytes - file_size;

   // clear buffer
   strcpy(sql_str, ""); 
       
   // Create sql statement for update
   sprintf(sql_str, "UPDATE USERS set FREESPACE = %d where NAME = '%s';", bytes_removed, user);
 
   // execute sql statement, close db then return 1. 
   rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
   sqlite3_close(db);
   
}





int truncate_add_bytes(char *user, int offset){

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_bytes;
   int bytes_remaining;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT FREESPACE FROM USERS WHERE NAME = '%s';", user);
   log_msg("Sql str from truncate add bytes: %s\nBytes:  %d\n", sql_str, offset);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_bytes = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of bytes that would remain if write is complete
   bytes_remaining = total_bytes - offset;
   log_msg("Bytes remaining: %d\n", bytes_remaining);

   // clear buffer
   strcpy(sql_str, ""); 

   // if bytes remaining in user account is greater than or equal to
   //  then write can proceed. else return 0 indicating not enough space
   if(bytes_remaining >= 0){
       
       // Create sql statement for update
       sprintf(sql_str, "UPDATE USERS set FREESPACE = %d where NAME = '%s';", bytes_remaining, user);

       log_msg("%s\n\n", sql_str);
 
       // execute sql statement, close db then return 1. 
       rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
         sqlite3_close(db);
        return bytes_remaining;
    } else {
        sqlite3_close(db);
        return -1;
    }
}



void truncate_add_rollback(char *user, int offset){

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_bytes;
   int bytes_remaining;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_bytes = (int)sqlite3_column_int (stmt, 0); 
   }

   int add_bytes = total_bytes + offset;

   // Create sql statement for update
   sprintf(sql_str, "UPDATE USERS set FREESPACE = %d where NAME = '%s';", add_bytes, user);
 
   // execute sql statement, close db then return 1. 
   rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);

   if(rc) {
      log_msg("Database Rollback Error: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
   } else {
      log_msg("Database Rollback Complete with %d bytes %s\n", add_bytes);
   }

   sqlite3_close(db);
}



int truncate_remove_bytes(char *user, int offset){

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_bytes;
   int bytes_remaining;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT FREESPACE FROM USERS WHERE NAME = '%s';", user);
   log_msg("Sql str from truncate rem bytes: %s\nBytes:  %d\n", sql_str, offset);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_bytes = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of bytes that would remain if write is complete
   bytes_remaining = total_bytes + offset;
   log_msg("Bytes remaining: %d\n", bytes_remaining);

   // clear buffer
   strcpy(sql_str, ""); 

   // if bytes remaining in user account is greater than or equal to
   //  then write can proceed. else return 0 indicating not enough space
   if(bytes_remaining >= 0){
       
       // Create sql statement for update
       sprintf(sql_str, "UPDATE USERS set FREESPACE = %d where NAME = '%s';", bytes_remaining, user);

       log_msg("%s\n\n", sql_str);
 
       // execute sql statement, close db then return 1. 
       rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
         sqlite3_close(db);
        return bytes_remaining;
    } else {
        sqlite3_close(db);
        return -1;
    }

}



void truncate_remove_rollback(char *user, int offset){

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_bytes;
   int bytes_remaining;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_bytes = (int)sqlite3_column_int (stmt, 0); 
   }

   int add_bytes = total_bytes - offset;

   // Create sql statement for update
   sprintf(sql_str, "UPDATE USERS set FREESPACE = %d where NAME = '%s';", add_bytes, user);
 
   // execute sql statement, close db then return 1. 
   rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);

   if(rc) {
      log_msg("Database Rollback Error: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
   } else {
      log_msg("Database Rollback Complete with %d bytes %s\n", add_bytes);
   }

   sqlite3_close(db);
}



int get_num_dirs(char *user) {

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_dirs;
   int dirs_remaining;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT DIRS FROM USERS WHERE NAME = '%s';", user);
   log_msg("Sql str from mkdir: %s\n", sql_str);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_dirs = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of dirs remaining. Can user constant as mkdir
   // is only called once for each directory to be created
   dirs_remaining = total_dirs - 1;
   log_msg("Dirs remaining: %d\n", dirs_remaining);

   // clear buffer
   strcpy(sql_str, ""); 

   // if dirs remaining in user account is greater than or equal to 0
   //  then mkdir can proceed. else return -1 indicating not enough space
   if(dirs_remaining >= 0){
       
       // Create sql statement for update
       sprintf(sql_str, "UPDATE USERS set DIRS = %d where NAME = '%s';", dirs_remaining, user);

       log_msg("%s\n\n", sql_str);
 
       // execute sql statement, close db then return 1. 
       rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
         sqlite3_close(db);
        return 1;
    } else {
        sqlite3_close(db);
        return -1;
    }
}



void mkdir_rollback(char *user){

  sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_dirs;
   int dirs_removed;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT DIRS FROM USERS WHERE NAME = '%s';", user);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_dirs = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of bytes that will be subtracted from quota
   dirs_removed = total_dirs + 1;

   // clear buffer
   strcpy(sql_str, ""); 
       
   // Create sql statement for update
   sprintf(sql_str, "UPDATE USERS set DIRS = %d where NAME = '%s';", dirs_removed, user);
 
   // execute sql statement, close db then return 1. 
   rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
   sqlite3_close(db);
}


int rem_num_dirs(char *user) {

   sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_dirs;
   int dirs_remaining;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT DIRS FROM USERS WHERE NAME = '%s';", user);
   log_msg("Sql str from mkdir: %s\n", sql_str);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_dirs = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of dirs remaining. Can use a constant 1 as rmdir
   // is only called once for each directory to be destroyed
   dirs_remaining = total_dirs + 1;
   log_msg("Dirs remaining: %d\n", dirs_remaining);

   // clear buffer
   strcpy(sql_str, ""); 

   // if bytes remaining in user account is greater than or equal to
   //  then write can proceed. else return 0 indicating not enough space
   if(dirs_remaining >= 0){
       
       // Create sql statement for update
       sprintf(sql_str, "UPDATE USERS set DIRS = %d where NAME = '%s';", dirs_remaining, user);

       log_msg("%s\n\n", sql_str);
 
       // execute sql statement, close db then return 1. 
       rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
         sqlite3_close(db);
        return 1;
    } else {
        sqlite3_close(db);
        return -1;
    }
}




void rmdir_rollback(char *user){

  sqlite3 *db;
   sqlite3_stmt *stmt;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   char sql_str[1000];
   int total_dirs;
   int dirs_removed;
   const char* data = "Callback function called";
   
   char filename[1024];
   strcpy(filename, PRIVATE_DATA->base);
   strcat(filename, "/ntap.db");
   // must add full path here or else open will fail
   rc = sqlite3_open(filename, &db);
   if(rc) {
      //log_msg("Database Open Error: %s\n", sqlite3_errmsg(db));
      fprintf(stderr, "Cannot open Database: %s\n", sqlite3_errmsg(db));
      fflush(stdout);
   } else {
      //fprintf(stderr, "Database Opened: %s\n");
      //fflush(stdout);
   }

   // copy the user name passed from ntapfuse_ops into the sql string
   sprintf(sql_str, "SELECT DIRS FROM USERS WHERE NAME = '%s';", user);

   rc = sqlite3_prepare_v2(db, sql_str, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
       log_msg("Error selecting User: %s\n", sqlite3_errmsg(db));
   }
    
   // read data returned from databse
   while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        total_dirs = (int)sqlite3_column_int (stmt, 0); 
   }

   if (rc != SQLITE_DONE) {
       log_msg("Error reading DB return STR: %s\n", sqlite3_errmsg(db));
   }

   // close query
   sqlite3_finalize(stmt);

   // calculate the number of bytes that will be subtracted from quota
   dirs_removed = total_dirs - 1;

   // clear buffer
   strcpy(sql_str, ""); 
       
   // Create sql statement for update
   sprintf(sql_str, "UPDATE USERS set DIRS = %d where NAME = '%s';", dirs_removed, user);
 
   // execute sql statement, close db then return 1. 
   rc = sqlite3_exec(db, sql_str, update_callback, (void*)data, &zErrMsg);
   sqlite3_close(db);
}


void open_db(char *base){

   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;

   char filename[1024];
   strcpy(filename, base);
   strcat(filename, "/ntap.db");

   rc = sqlite3_open(filename, &db);

   if(rc) {
      //log_msg("Error opening database from open_db\n");      
      fprintf(stderr, "Can't open databse: %s\n", sqlite3_errmsg(db));
   } else {
      fprintf(stderr, "Database Created\n");
      fflush(stdout);
   }

   //TODO: Set the permission for the ntab.db file

   // initialize table of users
   sql = "CREATE TABLE USERS("  \
         "NAME TEXT PRIMARY KEY    NOT NULL," \
         "FREESPACE                INT     NOT NULL," \
         "DIRS                     INT );";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, insert_callback, 0, &zErrMsg);
   
   if(rc != SQLITE_OK){
      //log_msg("Error creating table\n");
      fprintf(stderr, "Can't init table 'USERS': %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Table initialized\n");
   }

   // populate table
    sql = "INSERT INTO USERS(NAME, FREESPACE, DIRS) " \
          "VALUES('sam-skupien', 20, 3);"
          "INSERT INTO USERS(NAME, FREESPACE, DIRS) " \
          "VALUES('dongbang', 10, 3);";


   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, insert_callback, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
      //log_msg("Error inserting values.\n");
      fprintf(stderr, "Can't Insert value into Table 'USERS': %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Values Inserted\n");
   }
   sqlite3_close(db);
}

static int insert_callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

static int update_callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}
