#include "common.h"
#include "file/ssfsearch.h"


/* Root directory path of ssf files for each HDD. */
const char * ssfroot [4] =	{
	"/mnt/sda1",
	"/mnt/sda2",
	"/mnt/sda3",
	"/mnt/sda4",
};

pthread_mutex_t	search_mutex = PTHREAD_MUTEX_INITIALIZER;

char *ssfdate(char *buf, const char *path, int date) {
	if (!buf || !path)
		return NULL;
	sprintf(buf, "%s/%08d", path, date);
	return buf;
}


/* Get integer format date.
 */
inline int _getdate(int y, int m, int d) {
	char year[16], mon[16], day[16];
	char date[16]="";

	sprintf(year, "%04d", y);
	sprintf(mon, "%02d", m);
	sprintf(day, "%02d", d);
	strcat(date, year); strcat(date, mon); strcat(date, day);

	return atoi(date);
}

/* Get integer format time.
 */
inline int gettime(int h, int m, int s) {
	char hour[16], min[16], sec[16];
	char time[16]="";

	sprintf(hour, "%02d", h);
	sprintf(min, "%02d", m);
	sprintf(sec, "%02d", s);
	strcat(time, hour); strcat(time, min); strcat(time, sec);

	return atoi(time);
}


/* Get integer normal to DST format time.
 */
inline int gettime_dst(int time, int dst) {
	int dst_time;

	dst_time = (time /10000 *1000000) + (time %10000);
	if (dst < 0)
		dst_time += 10000;

	return dst_time;
}

/* Get integer DST to normal format time.
 */
inline int gettime_dst2(int time) {
	int nr_time;

	nr_time = (time /1000000 *10000) + (time %10000);

	return nr_time;
}

/* Get is DST time.
 */
inline boolean istime_dst(int time) {
	if ((time %1000000) >= 10000)
		return FALSE;

	return TRUE;
}


/* Get ssf file list recorded for a channel.
 * Arguments
 *	@dir : Directory path to search files.
 *	@fileset : filelist stored here.
 *	@dst : Set if a file is dst time. this can be NULL.
 *	@size : Set file size if not NULL.
 *	@ch : channel to search(If < 0 does not check channel-Only used to get total ssf file number in the dir).
 * Return
 *	Number of files found, on success.
 *	-ERR_CODE, on error.
 */
int get_filelist(char *dirname, int fileset[], int dst[], unsigned size[], int ch) {
	DIR	*dir = NULL;
	struct dirent *ent = NULL;
	int i=0;

	assert (dirname);
	assert (fileset);

	/* 1. Open directory.
	 * 2. Read a file entry and put into filelist if conditions are  fullfilled.
	 * 3. Close directory.
	 * 4. Return number of files.
	 */
	if ((dir=opendir(dirname))==NULL) {
		return -ERR_NOFILE;
	}

	for(i=0; (ent=readdir(dir));) {
 		char file[7], chann[3];

 		if (ent==NULL) {
 			perror("readdir");
 			break;
 		}
 		if(!strcmp(ent->d_name, "."))
 			continue;
 		if(!strcmp(ent->d_name, ".."))
 			continue;
 		if (strstr(ent->d_name, ".idx"))
 			continue;
 		if (!strstr(ent->d_name, ".ssf"))
 			continue;

 		/* Get file record time and channel number from file name. */
 		strncpy(file, ent->d_name, LEN_TIME); file[LEN_TIME]='\0';
 		strncpy(chann, &ent->d_name[LEN_TIME], LEN_CH); chann[LEN_CH] ='\0';

 		/* check channel */
 		if (ch >=0) {
 			if (atoi(chann) != ch) {
 				continue;
 			}
 		}

 		if (atoi(chann) >= CAM_MAX) {
 			continue;
 		}

 		/* Set file info. */
		fileset[i] = atoi(file);

		if (dst)
			dst[i] = (ent->d_name[8]=='D') ? 1 : 0;


		if (size) {
			char filepath[1<<8];
			int fd=-1;
			unsigned sz = 0;

			sprintf(filepath, "%s/%s", dirname, ent->d_name);

			if ((fd = open (filepath, O_RDONLY)) < 0) {
	 			perror("open");
	 			continue;
			}
			if ((sz=getfilesz(fd))==0) {
				close(fd);
				continue;
			}
			close(fd);
			size[i] = sz;
		}

		i++; /* Must be here */
		if (i >= MAX_SSF)
			break;
	}

	if (dir)
		closedir(dir);

	return i;
}


/* Get ssf file list recorded for a channel.
 * Arguments
 *	@dir : Directory path to search files.
 *	@fileset : filelist stored here.
 *	@dst : Set if a file is dst time. this can be NULL.
 *	@size : Set file size if not NULL.
 *	@ch : channel to search(If < 0 does not check channel-Only used to get total ssf file number in the dir).
 * Return
 *	Number of files found, on success.
 *	-ERR_CODE, on error.
 */
int get_filelist_dst(char *dirname, int fileset[], int dst[], int size[], int ch) {
	DIR	*dir = NULL;
	struct dirent *ent = NULL;
	int i=0;

	char fullpath[256];
	FILE* fp;
	int  filesize;

	assert (dirname);
	assert (fileset);

	/* 1. Open directory.
	 * 2. Read a file entry and put into filelist if conditions are  fullfilled.
	 * 3. Close directory.
	 * 4. Return number of files.
	 */
	if ((dir=opendir(dirname))==NULL) {
		return -ERR_NOFILE;
	}

	for(i=0; (ent=readdir(dir));) {
 		char file[7], chann[3];

 		if (ent==NULL) {
 			perror("readdir");
 			break;
 		}
 		if(!strcmp(ent->d_name, "."))
 			continue;
 		if(!strcmp(ent->d_name, ".."))
 			continue;
 		if (strstr(ent->d_name, ".idx"))
 			continue;
 		if (!strstr(ent->d_name, ".ssf"))
 			continue;

 		{
			sprintf(fullpath, "%s/%s", dirname, ent->d_name);
			fp = fopen(fullpath, "rb");
			if (!fp)
				continue;

			fseek(fp, 0, SEEK_END);
			filesize = ftell(fp);
			fclose(fp);

			if (filesize < SSFHDR_MINSZ + 128)
				continue;
 		}

 		/* Get file record time and channel number from file name. */
 		strncpy(file, ent->d_name, LEN_TIME); file[LEN_TIME]='\0';
 		strncpy(chann, &ent->d_name[LEN_TIME], LEN_CH); chann[LEN_CH] ='\0';

 		/* check channel */
 		if (ch >=0) {
 			if (atoi(chann) != ch)
 				continue;
 		}

 		if (atoi(chann) >= CAM_MAX)
 			continue;

 		/* Set file info. */
		fileset[i] = atoi(file);

		if (dst)
			dst[i] = (ent->d_name[8]=='D') ? 1 : 0;

#if 1
		fileset[i] = gettime_dst (fileset[i], (ent->d_name[8]=='D')? 1 : -1);
#endif

		if (size) {
			char filepath[1<<8];
			int fd=-1, sz=-1;

			sprintf(filepath, "%s/%s", dirname, ent->d_name);

			if ((fd = open (filepath, O_RDONLY)) < 0) {
	 			perror("open");
	 			continue;
			}
			if ((sz=getfilesz(fd))<0) {
				close(fd);
				continue;
			}
			close(fd);
			size[i] = sz;
		}

		i++; /* Must be here */
		if (i >= MAX_SSF)
			break;
	}

	if (dir)
		closedir(dir);

	return i;
}


/* Get the most close time among @buf[] with given @time.
 * The time greater than @time is ignored.
 * Arguments
 *	@buf : Sorted time.
 *	@n : Number of elements in @buf.
 * Return:
 *	the index of most close time in @buf[] with @time, if there is one.
 *	-ERR_NOFILE, if there is no adequate file(All times are later than @time).
 *	-ERR_CODE, on failure.
 */
static int __find_ssf_backward(int buf[], int n, int time) {
	int ret=-ERR_NOFILE, i;

	for(i=n-1; i >= 0 ; i--) {
		if (buf[i] < time) {
			ret = i;
			break;
		}
	}

	return ret;
}


/* Get the most close time among @buf[] with given @time.
 * The time less than @time is ignored.
 * Arguments
 *	@buf : Sorted time.
 *	@n : Number of elements in @buf.
 * Return:
 *	the index of most close time in @buf[] with @time, if there is one.
 *	-ERR_NOFILE, if there is no adequate file(All times are later than @time).
 *	-ERR_CODE, on failure.
 */
static int __find_ssf_forward(int buf[], int n, int time) {
	int ret=-ERR_NOFILE, i;

	for(i=0; i < n ; i++) {
		if (buf[i] > time) {
			ret = i;
			break;
		}
	}

	return ret;
}

/* File the exactly matching elements among @buf[] with given @time.
 * The time less than @time is ignored.
 * Arguments
 *	@buf : Sorted time.
 *	@n : Number of elements in @buf.
 * Return:
 *	the index of most close time in @buf[] with @time, if there is one.
 *	-ERR_NOFILE, if there is no adequate file(All times are later than @time).
 *	-ERR_CODE, on failure.
 */
static int __find_ssf_exact(int buf[], int n, int time) {
	int ret=-ERR_NOFILE, i;

	for(i=0; i < n ; i++) {
		if (buf[i] == time) {
			ret = i;
			break;
		}
	}

	return ret;
}

/* Check if two time is on the same hour.
 *
 * Arguments
 *	@hour1, hour2 : hours to compare.
 * Return
 *	Nonzero, if two times are on the same hour.
 *	zero, otherwise.
 */
static int is_same_hour(int hour1, int hour2) {
	char tmp1[1<<8], tmp2[1<<8];

	sprintf(tmp1, "%06d", hour1);
	sprintf(tmp2, "%06d", hour2);

	return ((strncmp(tmp1, tmp2, 2)==0) ? 1 : 0);
}

/* Check if two time is on the same hour.
 *
 * Arguments
 *	@hour1, hour2 : hours to compare.
 * Return
 *	Nonzero, if two times are on the same hour.
 *	zero, otherwise.
 */
static int is_same_hour_dst(int hour1, int hour2) {
	char tmp1[1<<8], tmp2[1<<8];

	sprintf(tmp1, "%08d", hour1);
	sprintf(tmp2, "%08d", hour2);

	return ((strncmp(tmp1, tmp2, 2)==0) ? 1 : 0);
}



/* Get the most close time among @buf[] with given @time.
 * Arguments
 *	@buf, n: File lists and number of elements.
 *	@time : time to search file in file list.
 *	@scheme : File search scheme.
 *
 * Return
 *	the index of most close time in @buf[] with @time, if there is one.
 *	-ERR_NOFILE, if there is no adequate file(All times are later than @time).
 *	-ERR_CODE, on failure.
 *
 * NOTE
 *	Elements in buf[] must be sorted in ascending order before call this func.
 */
static int __find_ssf(int buf[], int n, int time, int scheme) {
	int ret;

	if (!buf || (n<0))
		return -ERR_NOFILE;

	/* 1. Find the file of requested time according to searching scheme.
	 * 2. If not found, return "NO FILE".
	 */

	if (scheme & SEEK_EXACT) {
		if ((ret=__find_ssf_exact(buf, n, time)) != -ERR_NOFILE)
			return ret;
	}

	if (scheme & SEEK_BACKWARD) {
		if ((ret=__find_ssf_backward(buf, n, time)) != -ERR_NOFILE) {
			if (scheme & SEEK_HOURBOUNDRY) {
				if (is_same_hour(time, buf[ret]))
					return ret;
			}
			else
				return ret;
		}
	}

	if (scheme & SEEK_FORWARD) {
		if ((ret=__find_ssf_forward(buf, n, time)) != -ERR_NOFILE) {
			if (scheme & SEEK_HOURBOUNDRY) {
				if (is_same_hour(time, buf[ret]))
					return ret;
			}
			else
				return ret;
		}
	}

	return -ERR_NOFILE;


}


/* Get the most close time among @buf[] with given @time.
 * Arguments
 *	@buf, n: File lists and number of elements.
 *	@time : time to search file in file list.
 *	@scheme : File search scheme.
 *
 * Return
 *	the index of most close time in @buf[] with @time, if there is one.
 *	-ERR_NOFILE, if there is no adequate file(All times are later than @time).
 *	-ERR_CODE, on failure.
 *
 * NOTE
 *	Elements in buf[] must be sorted in ascending order before call this func.
 */
static int __find_ssf_dst(int buf[], int n, int time, int scheme) {
	int ret;

	if (!buf || (n<0))
		return -ERR_NOFILE;

	/* 1. Find the file of requested time according to searching scheme.
	 * 2. If not found, return "NO FILE".
	 */

	if (scheme & SEEK_EXACT) {
		if ((ret=__find_ssf_exact(buf, n, time)) != -ERR_NOFILE)
			return ret;
	}

	if (scheme & SEEK_BACKWARD) {
		if ((ret=__find_ssf_backward(buf, n, time)) != -ERR_NOFILE) {
			if (scheme & SEEK_HOURBOUNDRY) {
				if (is_same_hour_dst(time, buf[ret]))
					return ret;
			}
			else
				return ret;
		}
	}

	if (scheme & SEEK_FORWARD) {
		if ((ret=__find_ssf_forward(buf, n, time)) != -ERR_NOFILE) {
			if (scheme & SEEK_HOURBOUNDRY) {
				if (is_same_hour(time, buf[ret]))
					return ret;
			}
			else
				return ret;
		}
	}

	return -ERR_NOFILE;


}


/* Get the most apporiate time value when given the @date and @time.
 * Arguments
 *	@path : Root directory path that contains ssf files.
 *	@date : date to search.
 *	@time : time to search.
 *	@ch : channel.
 *	@dst : daylight files or not.
 *	@scheme : File search scheme.
 * Return:
 *	the most close time in the form of interger, on success.
 *	-ERR_CODE, on failure.
 */
static int _find_ssf (const char *path, int date, int time, int ch, int dst, int scheme) {
	char dirname[1<<8];
	int fileset[MAX_SSF];
	int numfile=0, ret=-ERR_NOFILE;

	assert (path);

	/*1. Find directory name that same as the date.
	* 2. Read all file names within the directory.
	* 3. Choose the most adequate file with given time.
	*/
	ssfdate (dirname, (char *)path, date);

	if ((numfile=get_filelist(dirname, fileset, NULL, NULL, ch)) < 0 ) {
		return numfile;
	}

	if (numfile==0)
		return	-ERR_NOFILE;

	qsort (fileset, numfile, sizeof(int), compare);

	ret = __find_ssf(fileset, numfile, time, scheme);

	if (ret==-ERR_NOFILE)
		return	-ERR_NOFILE;

	return 	fileset[ret];
}


/* Get the most apporiate time value when given the @date and @time.
 * Arguments
 *	@path : Root directory path that contains ssf files.
 *	@date : date to search.
 *	@time : time to search.
 *	@ch : channel.
 *	@dst : daylight files or not.
 *	@scheme : File search scheme.
 * Return:
 *	the most close time in the form of interger, on success.
 *	-ERR_CODE, on failure.
 */
static int _find_ssf_dst (const char *path, int date, int time, int ch, int dst, int scheme) {
	char dirname[1<<8];
	int fileset[MAX_SSF];
	int numfile=0, ret=-ERR_NOFILE;
	int req_hr, i;
	boolean find_dst = FALSE, find_sst = FALSE;

	assert (path);

	/*1. Find directory name that same as the date.
	* 2. Read all file names within the directory.
	* 3. Choose the most adequate file with given time.
	*/
	ssfdate (dirname, (char *)path, date);

	if ((numfile=get_filelist_dst(dirname, fileset, NULL, NULL, ch)) < 0 ) {
		return numfile;
	}

	if (numfile==0)
		return	-ERR_NOFILE;

	qsort (fileset, numfile, sizeof(int), compare);

	if (time < 0 || time == 24006060) {
		ret = __find_ssf_dst(fileset, numfile, time, scheme);

		if (ret==-ERR_NOFILE)
			return	-ERR_NOFILE;
	}

	req_hr = time /1000000 *1000000 +10000;		// make HH010000 format

	for (i=0; i<numfile; i++) {
		if (time /1000000 == fileset[i] /1000000) {
			if (req_hr <= fileset[i])
				find_sst = TRUE;
			else if (req_hr > fileset[i])
				find_dst = TRUE;
		}
	}
	debug("CH %d: dst=%d, find_dst=%d, find_sst=%d\n", ch, dst, find_dst, find_sst);

	if (dst == 0) {
		if (find_dst && find_sst) {
			ret = __find_ssf_dst(fileset, numfile, time, scheme);

			if (ret==-ERR_NOFILE) {
				time = gettime_dst2(time);
				time = gettime_dst(time, 1);
				ret = __find_ssf_dst(fileset, numfile, time, scheme);

				if (ret==-ERR_NOFILE)
					return	-ERR_NOFILE;
			}

		} else if (find_dst && !find_sst) {
			time = gettime_dst2(time);
			time = gettime_dst(time, 1);
			ret = __find_ssf_dst(fileset, numfile, time, scheme);

			if (ret==-ERR_NOFILE)
				return	-ERR_NOFILE;

		} else if (!find_dst && find_sst) {
			ret = __find_ssf_dst(fileset, numfile, time, scheme);

			if (ret==-ERR_NOFILE)
				return	-ERR_NOFILE;

		} else if (!find_dst && !find_sst) {
			ret = __find_ssf_dst(fileset, numfile, time, scheme);

			if (ret==-ERR_NOFILE)
				return	-ERR_NOFILE;
		}

	} else if (dst == 1) {
		time = gettime_dst2(time);
		time = gettime_dst(time, 1);
		ret = __find_ssf_dst(fileset, numfile, time, scheme);

		if (ret==-ERR_NOFILE)
			return	-ERR_NOFILE;

	} else if (dst == -1) {
		ret = __find_ssf_dst(fileset, numfile, time, scheme);

		if (ret==-ERR_NOFILE)
			return	-ERR_NOFILE;
	}

	return 	fileset[ret];
}


/* Get most adequate file from multi directories when given the date and time.
 * Time is searched only in given date.
 *
 * Argument:
 *	@buf: result file name is stored here.
 *	@resulttime: result time is stored here.
 *	@date, time, ch : Time and channel to search file with.
 *	@dst : If 1:search only dst file, 0:don't case -1:search only non-dst file.
 *	@scheme : File search scheme.
 * Return:
 *	0, on success.
 *	-ERR_CODE, on error.
 */
int find_ssf (char *buf, int * resulttime , int date, int time, int ch, int dst, int scheme) {
	int hdd, numavailfile;
	int file[1][2];	/* Array of |time|hdd index| */

	assert (buf);

	/* 1. Get each candidate from serveral ssf root directories.
	 * 2. Each candiate compete to be choosen as the last one.
	 */
	for (hdd=0, numavailfile=0; hdd<1; hdd++)  {
		int ret;

		ret =_find_ssf(ssfroot[hdd], date, time, ch, dst, scheme);

		if ((ret==-ERR_NODIR) || (ret==-ERR_NOFILE))
			continue;

		file[numavailfile][0] = ret;	/* Time */
		file[numavailfile][1] = hdd; 	/* HDD index */

		numavailfile++; /* Must be here. */
	}

	if (numavailfile==0)
		return -ERR_NOFILE;

	qsort(file, numavailfile, sizeof(int)*2, compare);

	{ /* Get the last one and fill the result buffer. */
		int tmpfile[1], index=-ERR_NOFILE;

		for (hdd=0; hdd < 1; hdd++)
			tmpfile[hdd] = file[hdd][0];

		index=__find_ssf(tmpfile, numavailfile, time, scheme);

		if (index==-ERR_NOFILE)
			return -ERR_NOFILE;

		/* Check DST. */
		if (dst > 0) {
			sprintf(buf, "%s/%06d%02d%c%s", ssfdate (buf, ssfroot [file [index][1]] , date), file [index][0], ch, 'D', SSF_SSF_EXTENSION);
			if (!file_exist(buf))
				return -ERR_NOFILE;
		}
		else if (dst == 0) {
			sprintf(buf, "%s/%06d%02d%c%s", ssfdate (buf, ssfroot [file [index][1]] , date), file [index][0], ch, 'D', SSF_SSF_EXTENSION);
			if (!file_exist(buf))
				sprintf(buf, "%s/%06d%02d%c%s", ssfdate(buf, ssfroot [file [index][1]] , date), file [index][0], ch, 'S', SSF_SSF_EXTENSION);
		}
		else {
			sprintf(buf, "%s/%06d%02d%c%s", ssfdate(buf, ssfroot [file [index][1]] , date), file [index][0], ch, 'S', SSF_SSF_EXTENSION);
			if (!file_exist(buf))
				return -ERR_NOFILE;
		}

		if (resulttime)
			*resulttime = file[index][0];
	}

	return 0;
}


/* Get most adequate file from multi directories when given the date and time.
 * Time is searched only in given date.
 *
 * Argument:
 *	@buf: result file name is stored here.
 *	@resulttime: result time is stored here.
 *	@date, time, ch : Time and channel to search file with.
 *	@dst : If 1:search only dst file, 0:don't case -1:search only non-dst file.
 *	@scheme : File search scheme.
 * Return:
 *	0, on success.
 *	-ERR_CODE, on error.
 */
int find_ssf_dst (char *buf, int * resulttime , int date, int time, int ch, int dst, int scheme) {
	int hdd, numavailfile;
	int file[1][2];	/* Array of |time|hdd index| */

	assert (buf);

	/* 1. Get each candidate from serveral ssf root directories.
	 * 2. Each candiate compete to be choosen as the last one.
	 */
	for (hdd=0, numavailfile=0; hdd<1; hdd++)  {
		int ret;

		ret =_find_ssf_dst(ssfroot[hdd], date, time, ch, dst, scheme);

		if ((ret==-ERR_NODIR) || (ret==-ERR_NOFILE))
			continue;

		file[numavailfile][0] = ret;	/* Time */
		file[numavailfile][1] = hdd; 	/* HDD index */

		numavailfile++; /* Must be here. */
	}

	if (numavailfile==0)
		return -ERR_NOFILE;

	qsort (file, numavailfile, sizeof (int) * 2, compare);

	{ /* Get the last one and fill the result buffer. */
		int tmpfile[1], index=-ERR_NOFILE;
		int findfile;

		for (hdd=0; hdd < 1; hdd++)
			tmpfile[hdd] = file[hdd][0];

		index = __find_ssf_dst(tmpfile, numavailfile, time, scheme);

		if (index==-ERR_NOFILE)
			return -ERR_NOFILE;

		/* Check DST. */
		findfile = gettime_dst2(file[index][0]);
		sprintf(buf, "%s/%06d%02d%c%s", ssfdate(buf, ssfroot [file[index][1]] , date), findfile, ch, (istime_dst(file [index][0]))? 'D' : 'S', SSF_SSF_EXTENSION);
		if (!file_exist(buf))
			return -ERR_NOFILE;

		if (resulttime)
			*resulttime = file [index][0];
	}

	return 0;
}


/* Get most adequate file from multi directories when given the date and time.
 *
 * Argument:
 *	@buf: result file name is stored here.
 *	@resulttime: result time is stored here.
 *	@date, time, ch , dst : Time and channel to search file.
 *	@scheme : File search scheme.(Should be SEKK_FORWARD or SEEKBACKWARD).
 * Return:
 *	0, on success.
 *	-ERR_CODE, on error.
 */
int find_ssf2 (char *buf, int *resultdate, int *resulttime , int date, int time, int ch, int dst, int scheme) {
	int hdd, numavailfile;
	int file[1][2];	/* Array of |time|hdd index| */

	assert (buf);

find_ssf_in_date:

	/* 1. Get each candidate from serveral ssf root directories.
	 * 2. Each candiate compete to be choosen as the last one.
	 */
	for (hdd=0, numavailfile=0; hdd<1; hdd++)  {
		int ret;

		ret =_find_ssf (ssfroot [hdd], date, time, ch, dst, scheme);
		if ((ret==-ERR_NODIR) || (ret==-ERR_NOFILE))
			continue;

		file[numavailfile][0] = ret;	/* Time */
		file[numavailfile][1] = hdd; 	/* HDD index */

		numavailfile++; /* Must be here. */
	}

	if (numavailfile==0) {
		char	datebuf[1<<8];

		if (scheme & SEEK_DATEBOUNDRY)
			return -ERR_NOFILE;


		if (scheme & SEEK_FORWARD) {
			int	nextdate;

			if (find_ssfdir(datebuf, &nextdate, date, SEEK_FORWARD) < 0)
				return -ERR_NOFILE;

			date = nextdate;
			time = -1;

			goto find_ssf_in_date;
		}
		else if (scheme & SEEK_BACKWARD) {
				int	prevdate;

				if (find_ssfdir(datebuf, &prevdate, date, SEEK_BACKWARD) < 0)
					return -ERR_NOFILE;

				date = prevdate;
				time = 246060;

				goto find_ssf_in_date;
		}
	}


	qsort (file, numavailfile, sizeof (int) * 2, compare);

	{ /* Get the last one and fill the result buffer. */
		int tmpfile[1], index=-ERR_NOFILE;

		for (hdd=0; hdd < 1; hdd++)
			tmpfile[hdd] = file[hdd][0];

		index=__find_ssf(tmpfile, numavailfile, time, scheme);

		if (index==-ERR_NOFILE)
			return -ERR_NOFILE;

		/* Check DST. */
		if (dst > 0) {
			sprintf(buf, "%s/%06d%02d%c%s", ssfdate(buf, ssfroot [file[index][1]] , date), file [index][0], ch, 'D', SSF_SSF_EXTENSION);
			if (!file_exist(buf))
				return -ERR_NOFILE;
		}
		else if (dst == 0) {
			sprintf(buf, "%s/%06d%02d%c%s", ssfdate(buf, ssfroot [file [index][1]] , date), file [index][0], ch, 'D', SSF_SSF_EXTENSION);
			if (!file_exist(buf))
				sprintf(buf, "%s/%06d%02d%c%s", ssfdate(buf, ssfroot [file [index][1]] , date), file [index][0], ch, 'S', SSF_SSF_EXTENSION);
		}
		else {
			sprintf(buf, "%s/%06d%02d%c%s", ssfdate(buf, ssfroot [file[index][1]], date), file [index][0], ch, 'S', SSF_SSF_EXTENSION);
			if (!file_exist(buf))
				return -ERR_NOFILE;
		}

		if (resultdate)
			*resultdate = date;

		if (resulttime)
			*resulttime = file[index][0];

	}

	return 0;
}




 /* Get ssf directory list.
 * Arguments
 *	@rootdir : Root directory path to search directory. Ex) "/mnt/hdd2"
 *	@dirlist : Directory list stored here.
 * Return
 *	Number of directories found, on success.
 *	-ERR_CODE, on error.
 */
int get_dirlist (const char * rootdir, int dirlist[]) {
	DIR	*dir = NULL;
	struct dirent *ent = NULL;
	int i=0;

	assert (rootdir);
	assert (dirlist);

	/* 1. Open root directory.
	 * 2. Read a file entry and put into dirlist[] if conditions are  fullfilled.
	 * 3. Close directory.
	 * 4. Return number of directories.
	 */
	if ((dir=opendir(rootdir))==NULL) {
		return -ERR_NOFILE;
	}

	for(i=0; (ent=readdir(dir));) {
 		char dirname[9];
 		int j, valid=1;

 		if (ent==NULL) {
 			perror("readdir");
 			break;
 		}
 		if(!strcmp(ent->d_name, "."))
 			continue;
 		if(!strcmp(ent->d_name, ".."))
 			continue;

 		if (strlen(ent->d_name) != 8)
 			continue;

		for (j=0; j < 8; j++) {
			char c=ent->d_name[j];

			if ((c<'0') || (c>'9')) {
				valid = 0;
				break;
			}
		}

		if (!valid)
			continue;

 		strcpy(dirname, ent->d_name);
		dirlist[i] = atoi(dirname);

		i++; /* Must be here */
		if (i >= MAX_SSFDIR)
			break;
	}

	if (dir)
		closedir(dir);

	return i;
}


/* Get the most apporiate date value with given date.
 * Arguments
 *	@path : Root directory path that contains ssf directories.
 *	@date : date to search.
 *	@scheme : File search scheme.
 *
 * Return:
 *	the most close date in the form of interger, on success.
 *	-ERR_CODE, on failure.
 */
static int _find_ssfdir (const char * path, int date, int scheme) {
	int dirlist[MAX_SSFDIR];
	int numdir=0, ret=-ERR_NOFILE;

	assert (path);

	/*1. Read all file names within the directory.
	* 2. Choose the most adequate directory with given date.
	*/
	if ((numdir = get_dirlist (path, dirlist)) < 0 ) {
		return numdir;
	}

	if (numdir==0)
		return	-ERR_NOFILE;

	qsort(dirlist, numdir, sizeof(int), compare);

	ret = __find_ssf(dirlist, numdir, date, scheme);

	if (ret==-ERR_NOFILE)
		return	-ERR_NOFILE;

	return 	dirlist[ret];
}


/* Get most adequate directory from multi HDD.
 *
 * Argument:
 *	@buf: result directory name is stored here.
 *	@resultdate: result date is stored here.
 *	@scheme : Directory search scheme.
 * Return:
 *	0, on success.
 *	-ERR_CODE, on error.
 */
int find_ssfdir(char *buf, int *resultdate , int date, int scheme) {
	int hdd, numavaildir;
	int dir[1][2]; /* Array of |date|hdd index| */

	assert (buf);

	/* 1. Get each candidate from serveral ssf root directories.
	 * 2. Each candiate compete to be choosen as the last one.
	 */
	for (hdd=0, numavaildir=0; hdd < 1; hdd++)  {
		int ret;

		ret =_find_ssfdir(ssfroot[hdd], date, scheme);
		if ((ret==-ERR_NODIR) || (ret==-ERR_NOFILE))
			continue;

		dir[numavaildir][0] = ret; /* Date */
		dir[numavaildir][1] = hdd;

		numavaildir++; /* Must be here. */
	}

	if (0 == numavaildir)
		return -ERR_NOFILE;

	qsort(dir, numavaildir, sizeof(int)*2, compare);

	{ /* Get the last one and fill the result buffer. */
		int tmpdir[1], index=-ERR_NOFILE;

		for (hdd=0; hdd < 1; hdd++)
			tmpdir[hdd] = dir[hdd][0];

		index=__find_ssf(tmpdir, numavaildir, date, scheme);

		if (index==-ERR_NOFILE)
			return -ERR_NOFILE;

		ssfdate(buf, ssfroot [dir[index][1]], dir[index][0]);

		if (resultdate)
			*resultdate = dir[index][0];
	}

	return 0;
}

static void __hourtbl_mark (char* path, int ch, __u8 *tbl, struct tm *tm, int index)
{
	int rdx = -1, i;
	char tbl_tmp[DM_TBLSZ];

	if ((rdx = open (path, O_RDONLY)) < 0) {
		perror ("open");
		goto out;
	}

	lseek (rdx, 8 +DM_TBLSZ * ch,  SEEK_SET);

	if (index > 0) {
		memset(tbl_tmp, DM_TBLSZ, 0x00);

		if (read (rdx, tbl_tmp, DM_TBLSZ) != DM_TBLSZ) {
			perror ("read");
			goto out;
		}

		for (i=0; i<DM_TBLSZ; i++) {
			if (tbl_tmp[i] > tbl[i])
				tbl[i] = tbl_tmp[i];
		}
	} else {
		if (read (rdx, tbl, DM_TBLSZ) != DM_TBLSZ) {
			perror ("read");
			goto out;
		}
	}

out:
	if (rdx >= 0)
		close (rdx);
}


static void __hourtbl_net_mark (char* path, int ch, __u8 *tbl, struct tm *tm, int index)
{
	int rdx = -1, i;
	char tbl_tmp[DM_TBLSZ *6];

	if ((rdx = open (path, O_RDONLY)) < 0) {
		perror ("open");
		goto out;
	}

	lseek (rdx, 8 +(DM_TBLSZ *16) +(DM_TBLSZ *6 * ch),  SEEK_SET);

	if (index > 0) {
		memset(tbl_tmp, 0x00, DM_TBLSZ *6);

		if (read (rdx, tbl_tmp, DM_TBLSZ *6) != DM_TBLSZ *6) {
			perror ("read");
			goto out;
		}

		for (i=0; i<DM_TBLSZ *6; i++) {
			if (tbl_tmp[i] > tbl[i])
				tbl[i] = tbl_tmp[i];
		}

	}
	else {
		int nsz = read (rdx, tbl, DM_TBLSZ *6);
		if (nsz != DM_TBLSZ *6) {
			perror ("read");
			goto out;
		}
	}

out:
	if (rdx >= 0)
		close (rdx);
}
