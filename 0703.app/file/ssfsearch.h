/*
 *	ssfsearch.h
 *
 *	purpose:
 *		- Searching routines of ssf file.
 *
 **********************************************************************************************/

#ifndef __SSFSEARCH_H
#define __SSFSEARCH_H
  
#define MAX_SSF		4096			/* Maximum ssf files of a channel per directory. */
#define MAX_SSFDIR	1024			/* Maximum ssf directories of one HDD partition. */

#define MAX_WRITE_BUF_CNT	(5)
#define MAX_WRITE_BUF_SZ	(102400)


/* Ssf file/directory search scheme. Each scheme is not exclusive, so that 
 * serveral combinations are possible.
 */
#define SEEK_EXACT			0x01	/* Find the exact time of ssf file. */
#define SEEK_BACKWARD		0x02	/* Find the ssf file earler than given time. */
#define SEEK_FORWARD		0x04	/* Find the ssf file later than given time. */
#define SEEK_HOURBOUNDRY	0x08	/* Find files only on the same hour with given time. */
#define SEEK_DATEBOUNDRY	0x10	/* Find files only on the same date with given date. */
#define SEEK_INSEARCH 		0x20	/* Find files mode in dvr. */

#define SEEK_VOD_DEFAULT	(SEEK_EXACT | SEEK_BACKWARD | SEEK_FORWARD /*| SEEK_HOURBOUNDRY*/ | SEEK_DATEBOUNDRY)
#define SEEK_DEFAULT		(SEEK_EXACT | SEEK_BACKWARD | SEEK_FORWARD /*| SEEK_HOURBOUNDRY*/ | SEEK_DATEBOUNDRY | SEEK_INSEARCH)

#define SEEK_NORMAL_PLAY	(SEEK_EXACT | SEEK_BACKWARD | SEEK_FORWARD)
#define SEEK_REVERSE_PLAY	(SEEK_EXACT | SEEK_FORWARD | SEEK_BACKWARD)


// Data flag of searched data.
enum DATAMARK {
	DM_NO_REC,		// No recorded data.
	DM_NORM_REC,	// Normal recording data.
	DM_ALRM_REC,	// Alarm recording data.
	DM_MOT_REC,		// Motion recording data.
};

extern pthread_mutex_t	search_mutex;

#define SEARCH_LOCK()		pthread_mutex_lock (& search_mutex)
#define SEARCH_UNLOCK()		pthread_mutex_unlock (& search_mutex)

// Data position of an entry in rdxhdrs.
// @hour : 0 ~ 23.
// @min : 0 ~ 59.
#define	__RDX_NPOS( hour, min)		((hour) * 10) + ((min) /6)
#define	RDX_NPOS(ch, npos)			(240 * (ch) + (npos))

#define	__RDX_NPOS_MIN( hour, min)		((hour) * 60) + ((min))	// The past data is 6 min in unit of timebar, the present is 1 min in unit of timebar
#define	RDX_NPOS_MIN(ch, npos)			(240 * 6 * (ch) + (npos))		// The past data is 6 min in unit of timebar, the present is 1 min in unit of timebar															

// Getting RDX index from creation time and stamp offset.
#define RDX_NPOS_STAMP(creatime, stamp)\
({\
	struct tm ent = creatime;\
	int hour, min, sec;\
	\
	sec = (stamp /1000);\
	hour = sec /3600;\
	sec -= (hour *3600);\
	min = sec / 60;\
	sec -= (min *60);\
	\
	ent.tm_hour += hour;\
	ent.tm_min += min;\
	ent.tm_sec += sec;\
	__RDX_NPOS(ent.tm_hour, ent.tm_min);\
})

// Getting RDX index from creation time and stamp offset.
#define RDX_NPOS_MIN_STAMP(creatime, stamp)\
({\
	struct tm ent = creatime;\
	int hour, min, sec;\
	\
	sec = (stamp /1000);\
	hour = sec /3600;\
	sec -= (hour *3600);\
	min = sec / 60;\
	sec -= (min *60);\
	\
	ent.tm_hour += hour;\
	ent.tm_min += min;\
	ent.tm_sec += sec;\
	__RDX_NPOS_MIN(ent.tm_hour, ent.tm_min);\
})

extern const char * ssfroot[4];
extern char * ssfdate(char *, const char *, int);
extern inline int _getdate(int, int, int);
extern inline int gettime(int, int, int);
extern inline int gettime_dst(int, int);
extern inline int gettime_dst2(int);
extern inline boolean istime_dst(int);
extern int get_filelist(char *, int [], int [], unsigned [], int);
extern int find_ssf(char *, int *, int, int, int, int, int);
extern int find_ssf_dst(char *, int *, int, int, int, int, int);
extern int find_ssf2(char *, int *, int *, int, int, int, int, int);
extern int find_ssfdir(char *, int *, int, int);

#endif

