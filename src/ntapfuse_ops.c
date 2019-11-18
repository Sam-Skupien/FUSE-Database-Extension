/**
 * Project: ntapfuse
 * Author: Samuel Kenney <samuel.kenney48@gmail.com>
 *         August Sodora III <augsod@gmail.com>
 * File: ntapfuse_ops.c
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
#include "db_ops.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <pwd.h>

#include <sys/xattr.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * Appends the path of the root filesystem to the given path, returning
 * the result in buf.
 */
void
fullpath (const char *path, char *buf)
{
  char *basedir = (char *) PRIVATE_DATA->base;

  strcpy (buf, basedir);
  strcat (buf, path);
}


/* The following functions describe FUSE operations. Each operation appends
   the path of the root filesystem to the given path in order to give the
   mirrored path. */

int
ntapfuse_getattr (const char *path, struct stat *buf)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return lstat (fpath, buf) ? -errno : 0;
}

int
ntapfuse_readlink (const char *path, char *target, size_t size)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return readlink (fpath, target, size) < 0 ? -errno : 0;
}

int
ntapfuse_mknod (const char *path, mode_t mode, dev_t dev)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return mknod (fpath, mode, dev) ? -errno : 0;
}

int
ntapfuse_mkdir (const char *path, mode_t mode)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  // get username from directory owner
  struct passwd *pw;
  uid_t uid = geteuid();
  pw = getpwuid(uid);
  char *dir_owner = pw->pw_name;

  log_msg("\nNum Directories: 1\nUser ID: %s\nTime: %d-%d-%d %d:%d:%d\n", dir_owner, tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  
  //get number of durs left from user in database
  int dirs_created = get_num_dirs(dir_owner);
  if(dirs_created >= 0) {
    
      // call to pwrite returns number of bytes written to file if successful
      //or -1 if write failed 
      int mkdir_return_value = mkdir (fpath, mode | S_IFDIR) ? -errno : 0;

      if(mkdir_return_value < 0){
        // call mkdir_rollback to return dirs to user if mkdir fails
        mkdir_rollback(dir_owner);
        return mkdir_return_value;
      }
    
      // tell user how many bytes have been saved
      fprintf(stdout, "%d bytes written to file", dirs_created);
      fflush(stdout); 
      return mkdir_return_value;
    
    } else {
       fprintf(stdout, "Not enough space to make directory\n");
       fflush(stdout);
       log_msg("Write error thrown with dirs: %d\n", dirs_created); 
       return -1;
    }
}

int
ntapfuse_unlink (const char *path)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  // get username from file owner and the file size
  struct stat st;
  struct passwd *statpw;
  int st_status = stat(fpath, &st);
  uid_t st_uid = st.st_uid;
  statpw = getpwuid(st_uid);
  char *file_owner = statpw->pw_name;
  int unlink_size = st.st_size;
  //log_msg("username from unlink stat: %s\n", file_owner);
  //log_msg("unlink stat ret: %s\n", strerror(errno));
  //log_msg("unlink file size: %d\n", unlink_size); 

  log_msg("\nLog data size: %d\nUser: %d\nTime: %d-%d-%d %d:%d:%d\nFile Owner: %s\n",unlink_size, fuse_get_context()->uid, tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, file_owner);


  //execute unlink for db
  int bytes_unlinked = unlink_get_bytes(file_owner, unlink_size);
  if(bytes_unlinked >= 0) {
    
    /* call to pwrite returns number of bytes written to file if successful
       or -1 if write failed */
    int unlink_return_value = unlink (fpath) ? -errno : 0;

    if(unlink_return_value < 0){
      // call write_rollback to return bytes to user if pwrite fails
      unlink_rollback(file_owner, unlink_size);
      return unlink_return_value;
    }
    
    // tell user how many bytes have been saved
    fprintf(stdout, "%d bytes unlinked from quota", bytes_unlinked);
    fflush(stdout); 
    return unlink_return_value;
    
    } else {
       fprintf(stdout, "Could not unlink file\n");
       fflush(stdout);
       log_msg("Write error thrown with bytes: %d\n", bytes_unlinked); 
       return -1;
    }
}

int
ntapfuse_rmdir (const char *path)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  // get username from directory owner
  struct passwd *pw;
  uid_t uid = geteuid();
  pw = getpwuid(uid);
  char *dir_owner = pw->pw_name;

  log_msg("\nNum Directories: 1\nUser ID: %s\nTime: %d-%d-%d %d:%d:%d\n", dir_owner, tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  
  //get number of dirs left from user in database
  int dirs_removed = rem_num_dirs(dir_owner);
  if(dirs_removed >= 0) {
    
      // call to pwrite returns number of bytes written to file if successful
      //or -1 if write failed 
      int rmdir_return_value = rmdir (fpath) ? -errno : 0;

      if(rmdir_return_value < 0){
        // call rmdir_rollback to return dirs to user if rmdir fails
        rmdir_rollback(dir_owner);
        return rmdir_return_value;
      }
    
      // tell user how many bytes have been saved
      fprintf(stdout, "%d Directory removed from quota", dirs_removed);
      fflush(stdout); 
      return rmdir_return_value;
    
    } else {
       fprintf(stdout, "Not enough space to make directory\n");
       fflush(stdout);
       log_msg("Write error thrown with dirs: %d\n", dirs_removed); 
       return -1;
    }

  //return rmdir (fpath) ? -errno : 0;
}

int
ntapfuse_symlink (const char *path, const char *link)
{
  char flink[PATH_MAX];
  fullpath (link, flink);

  return symlink (path, flink) ? -errno : 0;
}

int
ntapfuse_rename (const char *src, const char *dst)
{
  char fsrc[PATH_MAX];
  fullpath (src, fsrc);

  char fdst[PATH_MAX];
  fullpath (dst, fdst);

  return rename (fsrc, fdst) ? -errno : 0;
}

int
ntapfuse_link (const char *src, const char *dst)
{
  char fsrc[PATH_MAX];
  fullpath (src, fsrc);

  char fdst[PATH_MAX];
  fullpath (dst, fdst);

  return link (fsrc, fdst) ? -errno : 0;
}

int
ntapfuse_chmod (const char *path, mode_t mode)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return chmod (fpath, mode) ? -errno : 0;
}

int
ntapfuse_chown (const char *path, uid_t uid, gid_t gid)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return chown (fpath, uid, gid) ? -errno : 0;
}

int
ntapfuse_truncate (const char *path, off_t off)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  // get username from file owner and the file size
  struct stat st;
  struct passwd *statpw;
  int st_status = stat(fpath, &st);
  uid_t st_uid = st.st_uid;
  statpw = getpwuid(st_uid);
  char *file_owner = statpw->pw_name;
  int file_size = st.st_size;
  //log_msg("username from truncate stat: %s\n", file_owner);
  //log_msg("truncate stat ret: %s\n", strerror(errno));
  //log_msg("truncate file size: %d\n", off); 

  log_msg("\nLog data size: %d\nUser: %d\nTime: %d-%d-%d %d:%d:%d\nFile Owner: %s\n",off, fuse_get_context()->uid, tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, file_owner);

  // if the offset is the same as the filesize truncate can be called
  // as is because no bytes will be added or removed. No need to 
  // access database. if offset is greater than filesize then user
  // quota will be decreased by offset. Else user quota will be 
  // increased by offset
  if(off == file_size){

    return truncate (fpath, off) ? -errno : 0;

  } else if (off > file_size) {

      // subtract file_size from offset to get total number
      // of bytes to be subtracted from current quota because file
      // is being expanded by the number of bytes in offset
      int rem_bytes = off - (file_size); 
      int bytes_added = truncate_add_bytes(file_owner, rem_bytes);
      if(bytes_added >= 0) {
    
         /* call to pwrite returns number of bytes written to file if successful
       or -1 if write failed */
         int truncate_return_value = truncate (fpath, off) ? -errno : 0;

         if(truncate_return_value < 0){
            // call write_rollback to return bytes to user if pwrite fails
            truncate_add_rollback(file_owner, rem_bytes);
            return truncate_return_value;
         }
    
         // tell user how many bytes have been saved
         fprintf(stdout, "%d bytes removed via truncate from quota", bytes_added);
         fflush(stdout); 
         return truncate_return_value;
    
      } else {
         fprintf(stdout, "Could not add bytes to file\n");
         fflush(stdout);
         log_msg("Write error thrown with bytes: %d\n", bytes_added); 
         return -1;
      }

  } else {

      // subtract file size-1 (-1 for EOF character) from offset to get total number
      // of bytes to be subtracted from current quota
      int rem_bytes = (file_size) - off; 
      int bytes_remaining = truncate_remove_bytes(file_owner, rem_bytes);
      if(bytes_remaining >= 0) {
    
         /* call to pwrite returns number of bytes written to file if successful
       or -1 if write failed */
         int truncate_return_value = truncate (fpath, off) ? -errno : 0;

         if(truncate_return_value < 0){
            // call write_rollback to return bytes to user if pwrite fails
            truncate_remove_rollback(file_owner, rem_bytes);
            return truncate_return_value;
         }
    
         // tell user how many bytes have been saved
         fprintf(stdout, "%d bytes removed via truncate from quota", bytes_remaining);
         fflush(stdout); 
         return truncate_return_value;
    
      } else {
         fprintf(stdout, "Could not remove bytes from file\n");
         fflush(stdout);
         log_msg("Write error thrown with bytes: %d\n", rem_bytes); 
         return -1;
      }

  }

  //return truncate (fpath, off) ? -errno : 0;
}

int
ntapfuse_utime (const char *path, struct utimbuf *buf)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return utime (fpath, buf) ? -errno : 0;
}

int
ntapfuse_open (const char *path, struct fuse_file_info *fi)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  int fh = open (fpath, fi->flags);
  if (fh < 0)
    return -errno;

  fi->fh = fh;

  return 0;
}

int
ntapfuse_read (const char *path, char *buf, size_t size, off_t off,
	   struct fuse_file_info *fi)
{
  return pread (fi->fh, buf, size, off) < 0 ? -errno : size;
}

int
ntapfuse_write (const char *path, const char *buf, size_t size, off_t off,
	    struct fuse_file_info *fi)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  if(strlen(buf) <= 0){ //The buf is empty, is this going to happen? 
                        //Justfor secure
    return -1;
  }

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  // get username from file owner
  struct stat st;
  struct passwd *statpw;
  int st_status = stat(fpath, &st);
  uid_t st_uid = fuse_get_context()->uid;
  statpw = getpwuid(st_uid);
  char *file_owner = statpw->pw_name;
  //log_msg("username from stat: %s\n", user_name2);
  //log_msg("stat ret: %s\n", strerror(errno)); 

  log_msg("\nLog data size: %d\nLog data:\n%sUser: %d\nTime: %d-%d-%d %d:%d:%d\nFile Owner: %s\n",strlen(buf), buf, fuse_get_context()->uid, tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, file_owner);

  //get bytes left from user in database
  int bytes_written = write_get_bytes(file_owner, strlen(buf));
  if(bytes_written >= 0) {
    
      /* call to pwrite returns number of bytes written to file if successful
       or -1 if write failed */
      int pwrite_return_value = pwrite (fi->fh, buf, size, off) < 0 ? -errno : size;

      if(pwrite_return_value < 0){
        // call write_rollback to return bytes to user if pwrite fails
        write_rollback(file_owner, strlen(buf)-1);
        return pwrite_return_value;
      }
    
      // tell user how many bytes have been saved
      fprintf(stdout, "%d bytes written to file", bytes_written);
      fflush(stdout); 
      return pwrite_return_value;
    
    } else {
       fprintf(stdout, "Not enough space to save file\n");
       fflush(stdout);
       log_msg("Write error thrown with bytes: %d\n", bytes_written); 
       return -1;
    }
}

int
ntapfuse_statfs (const char *path, struct statvfs *buf)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return statvfs (fpath, buf) ? -errno : 0;
}

int
ntapfuse_release (const char *path, struct fuse_file_info *fi)
{
  return close (fi->fh) ? -errno : 0;
}

int
ntapfuse_fsync (const char *path, int datasync, struct fuse_file_info *fi)
{
  if (datasync)
    return fdatasync (fi->fh) ? -errno : 0;
  else
    return fsync (fi->fh) ? -errno : 0;
}

int
ntapfuse_setxattr (const char *path, const char *name, const char *value,
	       size_t size, int flags)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return lsetxattr (fpath, name, value, size, flags) ? -errno : 0;
}

int
ntapfuse_getxattr (const char *path, const char *name, char *value, size_t size)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  ssize_t s = lgetxattr (fpath, name, value, size);
  return s < 0 ? -errno : s;
}

int
ntapfuse_listxattr (const char *path, char *list, size_t size)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return llistxattr (fpath, list, size) < 0 ? -errno : 0;
}

int
ntapfuse_removexattr (const char *path, const char *name)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return lremovexattr (fpath, name) ? -errno : 0;
}

int
ntapfuse_opendir (const char *path, struct fuse_file_info *fi)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  DIR *dir = opendir (fpath);
  if (dir == NULL)
    return -errno;

  fi->fh = (uint64_t) dir;

  return 0;
}

int
ntapfuse_readdir (const char *path, void *buf, fuse_fill_dir_t fill, off_t off,
	      struct fuse_file_info *fi)
{
  struct dirent *de = NULL;

  while ((de = readdir ((DIR *) fi->fh)) != NULL)
    {
      struct stat st;
      memset (&st, 0, sizeof (struct stat));
      st.st_ino = de->d_ino;
      st.st_mode = de->d_type << 12;

      if (fill (buf, de->d_name, &st, 0))
	break;
    }

  return 0;
}

int
ntapfuse_releasedir (const char *path, struct fuse_file_info *fi)
{
  return closedir ((DIR *) fi->fh) ? -errno : 0;
}

int
ntapfuse_access (const char *path, int mode)
{
  char fpath[PATH_MAX];
  fullpath (path, fpath);

  return access (fpath, mode) ? -errno : 0;
}

void *
ntapfuse_init (struct fuse_conn_info *conn)
{
  return PRIVATE_DATA;
}

void log_msg(const char *format, ...){
  va_list ap;
  va_start(ap, format);

  vfprintf(PRIVATE_DATA->logfile, format, ap);
  
  fflush(PRIVATE_DATA->logfile);
}

