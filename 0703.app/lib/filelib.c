/*****************************************************************************************
 * filelib.c
 *
 * purpose:
 *	- File/Directory handling routines(Deletion/size check etc.)
 *
 *
 ******************************************************************************************/

#include "common.h"
#include "lib/threadsafe.h"
#include <dirent.h>
#include <sys/types.h>
#include <string.h>


/* Check if a file exists or not.
 */
boolean file_exist(const char *filename)
{
	struct stat st;

	if (lstat(filename, &st) < 0)
		return FALSE;

	return TRUE;
}

/* Get an opened file's size. */
unsigned getfilesz(int fd) {
	struct stat filestat;

	if (fd <0)
		return 0;

	if (fstat (fd, & filestat) == -1) {
		perror ("fstat");
		return 0;
	}

	return (unsigned)filestat.st_size;
}

/* Check if a file is directory.
 */
inline boolean isdirectory(const char *path) {
	struct stat st;

	if (lstat(path, &st) < 0)
		return FALSE;

	if (S_ISDIR(st.st_mode))
		return TRUE;

	return FALSE;
}


/* Remove a file or directory including all files in it.
 */
void remove_file (const char * path, int bIsLock)
{
	FILE * fp = NULL;
	/* 1. Check if file is exists.
	 * 2. Remove file.
	 */

	if (! file_exist (path))
		return;

	if (! isdirectory (path)) {
		int ret;

		if (bIsLock)
			pthread_mutex_lock(&g_file.ssf_write_mutex);

		ret = unlink (path);

		if (bIsLock)
			pthread_mutex_unlock(&g_file.ssf_write_mutex);

		if (ret < 0)
		{
			static int s_cnt = 0;
			if (s_cnt++ > 10)
			{
				printf ("######### REBOOT! (Remove file is Failed.) #########\n");
				send_msg (MAIN, MAIN, _CAMMSG_REBOOT, 5, 0, 0);
			}

			perror ("unlink");
		}
	}
	else {
		char cmd [256], buf [256];

		sprintf (cmd, "/bin/rm -fR %s", path);
		BEGIN_SYSTEM (fp, cmd, buf)
		END_SYSTEM
	}

}

/* Show file list in directory
 */
void list_dir(const char *path)
{
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n",entry->d_name);
    }

    closedir(dir);
}

int check_valid_filename(char* fname)
{
	int ret = 0;
	if(strlen(fname) == 1 && fname[0] == '.') {
		ret = -1;
	}
	else if(strlen(fname) == 2 && strstr(fname, "..") != 0) {
		ret = -1;
	}

	return ret;
}

int search_file_n(const char *dir, const char *needle, char *file)
{
    struct dirent *entry;
	DIR *d = opendir(dir);
	if(d == NULL) {
		return -1;
	}

	while((entry = readdir(d)) != NULL) {
		if(strstr(entry->d_name, needle) != NULL) {
			sprintf(file, entry->d_name);
			goto end;
		}
	}

	closedir(d);
	return -1;

end:
	closedir(d);
	return 0;
}

