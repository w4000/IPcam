/* datetime.h
 *
 * 	purpose 
 *		- Date & time string manipulation.
 **********************************************************************************/
#ifndef __DATETIME_H
#define __DATETIME_H

// Length of time and date in string expression.
#define	LEN_YEAR	4
#define	LEN_MON		2	
#define	LEN_DAY		2	
#define	LEN_HOUR	2	
#define	LEN_MIN		2	
#define	LEN_SEC		2
#define LEN_CH		2
#define LEN_DATE	(LEN_YEAR + LEN_MON + LEN_DAY)
#define LEN_TIME	(LEN_HOUR + LEN_MIN + LEN_SEC)



#define TMZONE_MAX		(89 +1)
					
extern char * tmzone_set [TMZONE_MAX];
extern char * tmzone_str [TMZONE_MAX][4];


extern void datetime_tzset (int);
extern void datetime_systohc (void);
extern void datetime_hctosys (void);
extern bool datetime_indst (struct tm *);
extern int datetime_dst_date(int, int, int, int);
extern bool datetime_isdst_end(struct tm *);

extern char * __DateToString (int, char *, struct tm *);
extern char * __TimeToString (int, char *, struct tm *);
extern char * TMToStringDelimiter(int, int, char *, struct tm *, char *);
extern void tm_addsec2 (struct tm *, int);

extern void tm_minussec (struct tm *, int);


static inline int64_t tv2msec (struct timeval * tv) {
	return ((int64_t) tv->tv_sec * 1000 + (int64_t) (tv->tv_usec / 1000));
}

static inline int64_t tv2usec (struct timeval * tv) {
	return ((int64_t) tv->tv_sec * 1000000 + (int64_t) (tv->tv_usec));
}

static inline int64_t diff_msec (struct timeval * tv1, struct timeval * tv2) {
	return tv2msec (tv2) - tv2msec (tv1);
}

static inline int64_t ts2msec (struct timespec * ts) {
	return ((int64_t) ts->tv_sec * 1000 + (int64_t) (ts->tv_nsec / 1000000));
}

static inline int64_t ts2usec (struct timespec * ts) {
	return ((int64_t) ts->tv_sec * 1000000 + (int64_t) (ts->tv_nsec / 1000));
}

extern void sleep_ex(long);
#define sleep_ms(msec) sleep_ex(msec*1000)

#include <sys/time.h>

// Get time in milli second  since 1970/01/01.
static inline int64_t get_time_msec (void) {
	struct timeval now;
	int64_t ret;

	gettimeofday (& now, NULL);
	ret = tv2msec (& now);

	return ret;
}

static inline int64_t get_time_usec (void) {
	struct timeval now;
	int64_t ret;

	gettimeofday (& now, NULL);
	ret = tv2usec (& now);

	return ret;
}

static inline int64_t elapsed_time_msec(int64_t msec)
{                                                    
    return get_time_msec() - msec;                   
}                                                    

static inline int64_t elapsed_time_usec(int64_t usec)
{                                                    
    return get_time_usec() - usec;                   
}                                                    

#include <time.h>

static inline int64_t get_tick_msec (void) {
	struct timespec tick;
	int64_t ret;

	clock_gettime(CLOCK_MONOTONIC, &tick);
	ret = ts2msec(&tick);

	return ret;
}

static inline int64_t get_tick_usec (void) {
	struct timespec tick;
	int64_t ret;

	clock_gettime(CLOCK_MONOTONIC, &tick);
	ret = ts2usec(&tick);

	return ret;
}

static inline int64_t elapsed_tick_msec(int64_t msec)
{                                                    
    return get_tick_msec() - msec;                   
}                                                    

static inline int64_t elapsed_tick_usec(int64_t usec)
{                                                    
    return get_tick_usec() - usec;                   
}                                                    

#endif

