/*
 * watchdog.h
 *
 *   Purpose:
 *	 - System watchdog implementation.
 *	
 *****************************************************************************/
 
#ifndef __WATCHDOG_H
#define __WATCHDOG_H
#define WATCHDOG_TIME_SEC			300




typedef enum _WATCHDOG_MEMBER_E {
	WATCHDOG_ENCODER				= 0,
	WATCHDOG_MEMBER_NR,

} WATCHDOG_MEMBER_E;

typedef struct watchdog {

	void *dev;

	int	(*enable) (struct watchdog *);	
	int	(*disable) (struct watchdog *);	
	int	(*reboot) (struct watchdog *);	
	int	(*resettimer) (struct watchdog *);	// Reset timer.



	int 			enable_cp[WATCHDOG_MEMBER_NR];
	int 			max_cp[WATCHDOG_MEMBER_NR];
	int 			cur_cp[WATCHDOG_MEMBER_NR];


	
} ST_WATCHDOG;

extern void watchdog_init (ST_WATCHDOG *, void *);
extern void watchdog_hw_resettime();

int watchdog_work_init(void);
#endif

