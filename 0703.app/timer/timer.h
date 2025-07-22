#ifndef __TIMER_H
#define __TIMER_H

#include "timer/watchdog.h"

#define TIME_MSEC_CLKTCK	(1000 / CLOCKS_PER_SEC)
#define TIME_MIN_WATCHDOG	2					// Watchdog time in minutes.
#define TIME_SEC_WATCHDOG	10					// Watchdog timer reset dwell in sec.
#define TIME_SEC_BKUPSCHED	10					// Scheduled backup check dwell.
#define TIME_MIN_CGIUPDATE	2


#define ITIMER_MAX		84
#define ITIMER2_MAX		8



typedef struct st_itvaltimer {
	struct clist	list;	
	int msec_expire;	
	int notifywho;		
	int notifymsg;		
	int parm1;
	int parm2;
	int parm3;

} ST_ITVALTIMER;


typedef struct _TIMER {

	_CAM_THREAD_COMMON	th;

	/* Timer specific data here. */
	struct tm		current;						
	struct tm		current_gmt;					
	struct tm		old_current;
	__u64			current2;						
	__u64			lastact;						

	volatile ST_ITVALTIMER	*itimer [ITIMER_MAX];	
	struct clist	itimer2 [ITIMER2_MAX];			

	ST_WATCHDOG		watdog;							

	int				pre_hour;						
	struct tm		dm_current;						

	boolean			rec_restart;					
	boolean			tm_restart;						

	pthread_mutex_t	mutex_itimer;
	pthread_mutex_t	mutex_itimer2;

} ST_TIMER;





extern ST_TIMER	g_timer;
extern void *timer (void *);
extern int cancel_itimer (int);
extern void free_itm_pool(volatile ST_ITVALTIMER* msg);

extern int wait_delay_ms(long tms);

#endif
