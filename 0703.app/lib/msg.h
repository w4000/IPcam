/*
 * msg.h
 *
 *   Purpose:
 *	- Define messages for communication between threads.
 *
 **************************************************************/
#ifndef	___CAMMSG_H
#define ___CAMMSG_H

#include "misc.h"

#define MSGQ_SUPPORT_MAX	64

/* Message queue */
typedef struct st_msg_queue {
	struct clist	head;
	pthread_mutex_t	mutex;
	volatile int	nummsg;		// Current number of msg in the queue.
} ST_MSG_QUEUE;


/* Message struct */
typedef struct st_message {
	struct clist	list;
	int	from;					// Who sent this message.
	int	msgid;
	int	parm1;
	int	parm2;
	int	parm3;
} ST_MESSAGE;


#define MSGQ(idx)			(g_msgq [idx])

typedef enum _VIDEOMSG {
	_CAMMSG_JOB = 0,
	_CAMMSG_START,
 	_CAMMSG_STOP,
	_CAMMSG_EXIT,

	_CAMMSG_SETUP,
	_CAMMSG_RESTART_SSF,
	_CAMMSG_CGI,
	_CAMMSG_DEL_OLDEST_SSF,
	_CAMMSG_STRSVR_EXIT,
	_CAMMSG_NEW_SSF,	
	_CAMMSG_TMSYNC_NTP,
	_CAMMSG_MOTION,
	_CAMMSG_EVNT_MOTION2,
	_CAMMSG_REBOOT,
	_CAMMSG_REMOTE_UPGRADE,
	_CAMMSG_SETSYSTM,
	_CAMMSG_CLOSE_LAST_RDX_BUILD,
	_CAMMSG_FLUSH_FILE,	
	_CAMMSG_CLOSE_FILE,
	_CAMMSG_TM_RESTSRT,
	_CAMMSG_RTSP_RESTART,
	_CAMMSG_DHCP,
	_CAMMSG_ETHERNET_LINK,


	_CAMMSG_MAX
}VIDEOMSG;


extern ST_MSG_QUEUE	*g_msgq [64];
extern void init_msg_queue(struct st_msg_queue *, unsigned);
extern void flush_msg_queue(struct st_msg_queue *);
extern int send_msg(unsigned, unsigned, int, int, int, int);
extern int get_msg(struct st_msg_queue *, struct st_message *);
//extern inline void free_msg_pool(ST_MESSAGE* msg);

#endif

