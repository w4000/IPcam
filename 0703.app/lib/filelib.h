/*****************************************************************************************
 * filelib.h
 *
 * purpose:
 *	- File/Directory handling routines(Deletion/size check etc.)
 *
 *
 ******************************************************************************************/

#ifndef __FILELIB_H
#define __FILELIB_H

extern boolean file_exist(const char *);
extern unsigned getfilesz(int);
extern void remove_file(const char *, int bIsLock);
extern void *remove_file_thread(void *);
extern boolean isdirectory(const char *);
int search_file_n(const char *dir, const char *needle, char *file);

/* FOR TVIEW */
int find_tview_rec_file_in_dir(const char *path, const char* target, char *filename, int size);
int find_tview_log_file_in_dir(const char *path, const char *target, char *filename, int size);
int clean_invalid_mp4_file(const char* path);

#endif
