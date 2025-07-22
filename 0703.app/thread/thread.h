
#ifndef THREAD_H_
#define THREAD_H_

#include <stdio.h>
#include <pthread.h>
#include "../lib/msg.h"

struct st_msg_queue;
typedef	struct	_dvr_thread_common {

	pthread_t		tid;			
	struct st_msg_queue	msgq;		
	int				state;			


} _CAM_THREAD_COMMON;


enum	msg_thread {
	MAIN = 0,
	ENCODER,
	TIMER,
	FILEMAN,
	NETSVR,
	KEYBD,
	ENCODER_SAVESTREAM_0,


	LIBHUBERT,
#ifdef USE_NEW_ONVIF
	ONVIF,
#endif
	MSG_THREAD_MAX
};


#define STAT_TERMINATED				0x0000
#define STAT_UNREADY				0x0001
#define STAT_READY					0x0002
#define STAT_RUNNING				0x0004
#define STAT_READY_ALL				0x0008

#define IPCAM_THREAD(id) 				_my_entry (MSGQ(id), _CAM_THREAD_COMMON, msgq)
#define IPCAM_THREAD_MAIN				((_CAMMAIN *)IPCAM_THREAD (MAIN))
#define IPCAM_THREAD_FILE				((ST_FILE *)IPCAM_THREAD (FILEMAN))
#define IPCAM_THREAD_TIMER			((ST_TIMER *)IPCAM_THREAD (TIMER))

#define IPCAM_THREAD_STATE(id)		(IPCAM_THREAD(id)->state)
#define IPCAM_THREAD_RUNNING(id)		(IPCAM_THREAD_STATE(id) == STAT_RUNNING)



int wait_thread_state (int who, int waitstat, long useclimit);
int start_timer_thread();
int start_threads(int skip_timer_thread);
void terminate_threads(void);
void stop_threads_without_main (void);
int restart_thread(int who);


#endif /* THREAD_H_ */
