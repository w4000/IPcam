/*
 * watchdog.c
 *
 *   Purpose:
 *	 - System watchdog implementation.
 *
 *****************************************************************************/
#include "common.h"
#include <watchdog.h>
/***********************************************************************************
 * Member functions of object.
 ***********************************************************************************/

 static void *watchdog_thread_func (void *arg);

static int __tty_watchdog_enable (ST_WATCHDOG *dog) {

	assert (dog);
	// TODO
	return 0;
}

static int __tty_watchdog_disable (ST_WATCHDOG *dog) {

	assert (dog);
	// TODO
	return 0;
}

static int __tty_watchdog_reboot (ST_WATCHDOG *dog) {

	assert (dog);
	// TODO
	return 0;
}

static int __tty_watchdog_resettimer (ST_WATCHDOG *dog) {

	assert (dog);
	int fd = open("/dev/watchdog", O_RDWR);
	if (fd >= 0)
	{
		ioctl(fd, WDIOC_KEEPALIVE, NULL);
		close(fd);
	}



	return 0;
}


/************************************************************************************
 * Initializing object.
 ************************************************************************************/
static int load_driver = 1;
void watchdog_init (ST_WATCHDOG *dog, void *dev) {

	assert (dog);
	assert (dev);

	dog->dev = dev;
	dog->enable = __tty_watchdog_enable;
	dog->disable = __tty_watchdog_disable;
	dog->reboot = __tty_watchdog_reboot;
	dog->resettimer = __tty_watchdog_resettimer;

	int timeout = 89;
	struct watchdog_info ident;
	int fd = open("/dev/watchdog", O_RDWR);
	if(fd < 0) {
		load_driver = 0;
		return;
	}

	if(ioctl(fd, WDIOC_GETSUPPORT, &ident) != 0) {
		printf("Not Support WDIOC_GETSUPPORT!!!\n");
		goto End;
	}


	if(ioctl(fd, WDIOC_SETTIMEOUT, &timeout) != 0) {
		printf("Not Support WDIOC_SETTIMEOUT!!!\n");
		goto End;
	}


End:
	if(fd >= 0) {
		close(fd);
	}
	return;
}

void watchdog_hw_resettime()
{
	if(load_driver == 0) {
		return;
	}

	if(g_timer.watdog.resettimer)
		g_timer.watdog.resettimer (& g_timer.watdog);
}



int watchdog_work_init()
{
	pthread_t tid;

	if (pthread_create (& tid, 0, watchdog_thread_func, NULL) == 0) {
		pthread_detach (tid);
		return 0;
	}

	return -1;
}


static void
*watchdog_thread_func (void *arg)
{
	int i;
	int t;
	int nErrThread;
	int cnt = 0;

	while(1)
	{

		nErrThread = -1;
		for (t = 0; t < THREAD_ALIVE_CHECK_MAX; t++)
		{
			if (g_main.thread_alive_check[t] != 0)
			{
				nErrThread = t;
			}
		}

		if (nErrThread < 0 || g_main.isDHCP == 1)
		{
			g_main.alive_check_ok = 1;

			for (t = 0; t < THREAD_ALIVE_CHECK_MAX; t++)
				g_main.thread_alive_check[t] = 1;
			
			if ((cnt++ % 2) == 1)
			{
				watchdog_hw_resettime();
			}
		}


		sleep(2);//w4000_sleep(2);
	}
}



































