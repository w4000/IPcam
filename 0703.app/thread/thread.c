
#include "common.h"
#include "kmsg/kmsg.h"
#include <sys/prctl.h>


static int _malloc_cnt = 0;

inline void * __malloc_release (size_t size)
{
	void* p = NULL;
	p = malloc (size);


	return p;
}
inline void __free_release (void * ptr)
{
	if (ptr != NULL)
		free (ptr);
}


int wait_thread_state (int who, int waitstat, long useclimit) {

	// unlimited wait.
	if (useclimit < 0) {
		do {
			if ( MSGQ (who) && IPCAM_THREAD (who)->state == waitstat)
				return 0;

			sleep_ex (1000);
		} while (1);
	}

	while (useclimit >= 0) {

		if ( MSGQ (who) && IPCAM_THREAD (who)->state == waitstat)
			return 0;

		sleep_ex (1000);
		useclimit -= 1000;
	}

	return -ERR_TIMEOUT;
}

int start_timer_thread()
{
	// Timer thread.
	if (pthread_create (& g_timer.th.tid, NULL, timer, NULL) != 0)
		return -1;

	pthread_detach (g_timer.th.tid);

	if (wait_thread_state (TIMER, STAT_READY, -1) < 0)
		return -2;

	send_msg (MAIN, TIMER, _CAMMSG_START, 0, 0, 0);

	if (wait_thread_state (TIMER, STAT_RUNNING, -1) < 0)
	{
		return -3;
	}
	return 0;
}

int start_threads(int skip_timer_thread)
{

	if (!skip_timer_thread)
	{
		if (start_timer_thread() < 0)
			goto err;
	}

	if (pthread_create (& g_file.th.tid, NULL, fileproc, NULL) != 0)
		goto err;

	pthread_detach (g_file.th.tid);
	if (wait_thread_state (FILEMAN, STAT_READY, -1) < 0)
		goto err;

	send_msg (MAIN, FILEMAN, _CAMMSG_START, 0, 0, 0);
	if (wait_thread_state (FILEMAN, STAT_RUNNING, -1) < 0)
	{
		goto err;
	}


#if 1
	// Encoder thread.
	if (pthread_create (& g_encoder.th.tid, NULL, main_encoder_thread, NULL) < 0)
		goto err;

	pthread_detach (g_encoder.th.tid);

	if (wait_thread_state(ENCODER, STAT_READY, -1) < 0)
		goto err;

	// printf("! %s:%s:%d 	>>> sleep 200	!!!!!!!!!\n", __FILE__, __func__, __LINE__ );
	// sleep(200);

	send_msg(MAIN, ENCODER, _CAMMSG_START, 0, 0, 0);
	if (wait_thread_state (ENCODER, STAT_RUNNING, 1000000) < 0)
	{
		goto err;
	}
#else
#endif

	// Network thread.
	if (pthread_create (& g_netsvr.th.tid, NULL, netsvr, NULL) != 0)
		goto err;

	pthread_detach (g_netsvr.th.tid);

	if (wait_thread_state (NETSVR, STAT_READY, -1) < 0)
		goto err;

	send_msg (MAIN, NETSVR, _CAMMSG_START, 0, 0, 0);

	if (wait_thread_state (NETSVR, STAT_RUNNING, -1) < 0)
	{
		goto err;
	}

	if (pthread_create (& kmsg_tid, NULL, kmsgproc, NULL) != 0)
		goto err;


	//pthread_attr_destroy(&attr);
	return 0;

err:

	//pthread_attr_destroy(&attr);
	return -1;
}

void terminate_threads(void) {

	int res;

	send_msg (MAIN, NETSVR, _CAMMSG_EXIT, 0, 0, 0);
	wait_thread_state (NETSVR, STAT_TERMINATED, 1000);
	assert (0 == res);


	send_msg (MAIN, ENCODER, _CAMMSG_EXIT, 0, 0, 0);
	res = wait_thread_state (ENCODER, STAT_TERMINATED, 1000);
	assert (0 == res);


	send_msg (MAIN, TIMER, _CAMMSG_EXIT, 0, 0, 0);
	res = wait_thread_state (TIMER, STAT_TERMINATED, 0);
	assert (0 == res);


	pthread_cancel (kmsg_tid);

}


void stop_threads_without_main (void)
{
	int res;

	send_msg(MAIN, NETSVR, _CAMMSG_STOP, 0, 0, 0);
	res = wait_thread_state (NETSVR, STAT_READY, -1);
	assert (0 == res);

	send_msg (MAIN, ENCODER, _CAMMSG_EXIT, 0, 0, 0);
	res = wait_thread_state (ENCODER, STAT_TERMINATED, 5000000);
	assert (0 == res);

	send_msg(MAIN, FILEMAN, _CAMMSG_STOP, 0, 0, 0);
	res = wait_thread_state (FILEMAN, STAT_READY, -1);
	assert (0 == res);

	isp_exit();

	sleep_ex (1000000);

}

