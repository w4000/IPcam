/* timer.c
 *
 * purpose
 *	- Timer prints system time and channel name for each live and playback mode.
 ***********************************************************************************/

#include "common.h"
#include "device/gpio.h"

ST_TIMER	g_timer;
time_t getup_time_t;
#define MAX_ITM_POOL	(ITIMER_MAX * 2)
static ST_ITVALTIMER 	g_itm_pool[MAX_ITM_POOL];
static char				g_itm_inuse[MAX_ITM_POOL] = { 0, };


void free_itm_pool(volatile ST_ITVALTIMER* msg)
{
	int i;
	for (i = 0; i < MAX_ITM_POOL; i++)
	{
		if (&(g_itm_pool[i]) == msg)
		{
			g_itm_inuse[i] = 0;
			return;
		}
	}
	_mem_free((void*)msg);
}

static __s64 update_current_time(ST_TIMER *timer);

/**************************************************************************************
 * Initializing and releasing timer's resource.
 **************************************************************************************/

static int timer_init(ST_TIMER *timer)
{
	assert (timer);

	pthread_mutex_init (& timer->mutex_itimer, NULL);
	pthread_mutex_init (& timer->mutex_itimer2, NULL);

	timer->th.state = STAT_UNREADY;
	init_msg_queue(& timer->th.msgq, TIMER);

	{
		int i;
		for (i=0 ; i < ITIMER_MAX; i++)
			timer->itimer[i] = NULL;

		for (i=0 ; i < ITIMER2_MAX; i++)
			init_clist(&timer->itimer2[i]);
	}

	update_current_time (& g_timer);
	watchdog_init (& timer->watdog, & DEV_MCU);

	g_timer.old_current = g_timer.current;
	g_timer.dm_current = g_timer.current;

	g_timer.lastact = g_timer.current2;
	g_timer.rec_restart = FALSE;
	g_timer.tm_restart = FALSE;

	timer->th.state = STAT_READY;
	return 0;
}


static void timer_release(ST_TIMER *timer) {
	assert (timer);

	timer->th.state = STAT_TERMINATED;
	flush_msg_queue(& timer->th.msgq);

	pthread_mutex_destroy (& timer->mutex_itimer);
	pthread_mutex_destroy (& timer->mutex_itimer2);

	return;
}



static int handle_msg_cgiupdate_direct () {
	int fd = 0;

	if (file_exist (CGI_UPGRADE)) {
		int param = -1;
		char str[4];

		fd = open(CGI_UPGRADE, O_RDWR);
		if (read(fd, str, 4) == 0)
			goto out;

		if (fd) { close(fd); fd = 0; }

		remove_file(CGI_UPGRADE, 0);

		param = atoi(str);
		send_msg(TIMER, MAIN, _CAMMSG_CGI, param, 0, 0);
	}

out:
	if (fd) { close(fd); fd = 0; }
	return 0;
}



static __s64 update_current_time (ST_TIMER * timer) {
	__u64	old;
	time_t tmp;
	__s64 elapsed = 0;

	assert (timer);

	old = timer->current2;
	timer->current2 = get_time_msec ();

	tmp = timer->current2 / 1000;

	localtime_r (& tmp, & timer->current);
	gmtime_r (& tmp, & timer->current_gmt);

	elapsed = (timer->current2 - old);

	return ((elapsed < 0) || (elapsed > 10000)) ? 10 : elapsed;
}


static void timer_job(ST_TIMER *timer) {
	__s64 elapsed_msec;
	static int do_timesync_alreay = 0;
	static int timer_job_cnt = 0;
	static int timer_job_cnt1 = 0;
	static int timer_job_cnt2 = 0;
	static int timer_job_cnt3 = 0;
	static int prev_ntp_upate_h = 0;

	elapsed_msec = update_current_time (& g_timer);

	if (   (g_timer.current.tm_year  != g_timer.old_current.tm_year)
		|| (g_timer.current.tm_mon   != g_timer.old_current.tm_mon)
		|| (g_timer.current.tm_mday  != g_timer.old_current.tm_mday)
		|| (g_timer.current.tm_hour  != g_timer.old_current.tm_hour)
		|| (g_timer.current.tm_isdst != g_timer.old_current.tm_isdst)
		|| (g_timer.tm_restart)) {


		g_timer.old_current = g_timer.current;
		g_timer.dm_current = g_timer.current;
		g_timer.rec_restart = TRUE;

	}

	if ((g_timer.current.tm_min != g_timer.old_current.tm_min && g_timer.current.tm_min == 30 ||  g_timer.current.tm_min == 0) || (g_timer.tm_restart))
	{
		g_timer.old_current.tm_min = g_timer.current.tm_min;
		int nr_hour = g_timer.current.tm_hour;
		
		if (do_timesync_alreay == 0 || g_timer.tm_restart)
		{
			do_timesync_alreay = 1;
			timer_job_cnt = 0;


			if ((g_setup.tmsync.usage == TMS_NTPSVR)
				&& ((g_setup.tmsync.syncycle == 1)
					|| ((nr_hour % g_setup.tmsync.syncycle) == 0))) 
			{
				printf("w4000 m %s, %d\n",__FUNCTION__,__LINE__);

				send_msg (TIMER, NETSVR, _CAMMSG_TMSYNC_NTP, 0, 0, 0);
			}
		}

		g_timer.tm_restart = FALSE;

	} else if (((g_timer.current.tm_min / 5) != (g_timer.old_current.tm_min / 5))) {

		if (g_file.nr_mount && g_main.rec && g_setup.cam[0].rec)
		{
			g_timer.old_current = g_timer.current;
			g_timer.dm_current = g_timer.current;

			g_timer.rec_restart = TRUE;
			g_timer.tm_restart = FALSE;
		}

	}
	if (timer_job_cnt++ > 3000)	
	{
		timer_job_cnt = 0;
		do_timesync_alreay = 0;
	}

	if (timer_job_cnt1++ > 10000)	
	{
		timer_job_cnt1 = 0;
		if (g_setup.net.type == NETTYPE_DHCP) {
			send_msg (TIMER, MAIN, _CAMMSG_DHCP, 0, 0, 0);
		}
	}

	if (timer_job_cnt2++ > 100)//w4000 web speed debug 200)
	{
		timer_job_cnt2 = 0;
		handle_msg_cgiupdate_direct();
	}
}

#define w4000_long_key_factory 5
void sys_reset_chk()
{
    int val =0;
    int sys_reset_cnt =0;


    while(gpio_factory_reset() && sys_reset_cnt < w4000_long_key_factory)
    {
        sleep(1);
        sys_reset_cnt++;
		// printf("reset cnt? %d \n", sys_reset_cnt);
    }

	if(w4000_long_key_factory>sys_reset_cnt&&0<sys_reset_cnt)
	{
	
		printf("w4000_short_key_factory >>>>>>>>> system reboot \n", 0);
		#if 1
		//system(KILLALL_IPC);
		sleep(3);		// 3 Sec
		// reboot force
		//reboot_system ();
		
		w4000_reboot();
		#endif
	}
	else
	if (sys_reset_cnt >= w4000_long_key_factory)
	{
		// printf("factort btn > default\n");
		send_msg(TIMER, MAIN, _CAMMSG_CGI, 3, 1, 0);
	}
}



void *timer(void *arg)
{
	if (timer_init(&g_timer) < 0)
		goto out;

	while(1) {
		struct st_message msg;

		if (get_msg (MSGQ (TIMER), & msg) > 0) {
			g_main.thread_work_check[THREAD_ALIVE_CHECK_TIMER] = msg.msgid;
			switch (msg.msgid) {


			case _CAMMSG_START:
				g_timer.th.state = STAT_RUNNING;
				break;

			case _CAMMSG_STOP:
				g_timer.th.state = STAT_READY;
				break;

			case _CAMMSG_TM_RESTSRT:
				g_timer.tm_restart = msg.parm1;
				break;

			case _CAMMSG_EXIT:
				goto out;

			default :
				break;
			}
		}

		sys_reset_chk();
		timer_job (& g_timer);

		g_main.thread_alive_check[THREAD_ALIVE_CHECK_TIMER] = 0;

		sleep_ms (10);
	}


out:
	timer_release(&g_timer);
	pthread_exit(NULL);
}


#if 1
int wait_delay_ms(long t)
{
	usleep(1000*t);
	printf("w4000- %dms\n", t);
	return 0;
}
#else 
#endif 


