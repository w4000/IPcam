#ifndef	__NETSVR_H
#define	__NETSVR_H

#include "live.h"
#include "server.h"
#include "mserver.h"
#include "posserver.h"
#include "websocket.h"

//--------------------------- Network port we are listing--------------------------------
#define PORT_WAIT			(SETUP_PORT_TCP (& g_setup))		 


#define ETHERNET_DEV		"eth0"
#define DEL 	"tc qdisc del dev %s root"
#define SHOW 	"tc qdisc show dev %s"


// Network server thread object.
typedef struct _NETSVR {

	_CAM_THREAD_COMMON	th;

	//---------------------------------------------------------------
	// The sockets we are paying attention of client connection.
	int						sock_wait;
	int 					sock_tmsync;

	fd_set 					rdset;						 
	int						socks;						 
	//----------------------------------------------------------------

	struct st_mstream		mstrmsvr [32];	 
	struct st_stream		strmsvr [32];	 

	int						numclients;					 
	int						numclients_http;
	struct clist			cliinfo_head;				 
	pthread_mutex_t			cliinfo_mutex;

	int						nummclients;				 
	struct clist			mcliinfo_head;				 
	pthread_mutex_t			mcliinfo_mutex;


	struct st_live_framebuf	live_frame_buf [CAM_MAX]; 	 

	boolean					fw_upgrade;


	int 					islink;
} __attribute__((__packed__)) ST_NETSVR;

#define LIVE_FRAME_BUF_CH(ch)		(& g_netsvr.live_frame_buf[(int)(ch)])

#define foreach_unoccupied_svr_slot(_svr, _slot)\
	for (_slot = 0; _slot < 32; _slot ++) {\
		if (g_netsvr.strmsvr [_slot].inuse)\
			continue;\
		_svr = & g_netsvr.strmsvr [_slot];\

#define foreach_occupied_svr_slot(_svr, _slot)\
	for (_slot = 0; _slot < 32; _slot ++) {\
		if (! g_netsvr.strmsvr [_slot].inuse)\
			continue;\
		_svr = & g_netsvr.strmsvr [_slot];\

#define foreach_unoccupied_msvr_slot(_svr, _slot)\
	for (_slot = 0; _slot < 32; _slot ++) {\
		if (g_netsvr.mstrmsvr [_slot].inuse)\
			continue;\
		_svr = & g_netsvr.mstrmsvr [_slot];\

#define foreach_occupied_msvr_slot(_svr, _slot)\
	for (_slot = 0; _slot < 32; _slot ++) {\
		if (! g_netsvr.mstrmsvr [_slot].inuse)\
			continue;\
		_svr = & g_netsvr.mstrmsvr [_slot];\



#define live_sync_frame(cam, frame)	\
do {\
	int __slot__;\
	struct st_stream * __svr__;\
	\
	for (__slot__ = 0; __slot__ < 32; __slot__ ++) {\
		__svr__ = & g_netsvr.strmsvr [__slot__];\
		if (! __svr__->inuse)\
			continue;\
	 	if (! ISLIVE (__svr__, cam))\
	 		continue;\
	 	if (! ISFLAG (__svr__, cam, __SS_SENDIMG))\
	 		continue;\
 		LIVE (__svr__, cam).sync = & ((frame)->iframe);\
	}	\
} while (0)

#define live_sync_frame2(cam, frame)	\
do {\
	int __slot__;\
	struct st_stream * __svr__;\
	\
	int i = -1;	\
	for (__slot__ = 0; __slot__ < 32; __slot__ ++) {\
		__svr__ = & g_netsvr.strmsvr [__slot__];\
		i++;	\
		if (__svr__->live[cam].cmd.playmode != PLAYMODE_PLAY_SOTRE)\
			continue;	\
 		LIVE (__svr__, cam).sync = & ((frame)->iframe);\
	}	\
} while (0)


#define live_sync_jpgframe(cam, frame)	\
do {\
	int __slot__;\
	struct st_mstream * __svr__;\
	\
	for (__slot__ = 0; __slot__ < 10; __slot__ ++) {\
		__svr__ = & g_netsvr.mstrmsvr [__slot__];\
		if (! __svr__->inuse)\
			continue;\
	 	if (! ISLIVE (__svr__, cam))\
	 		continue;\
	 	if (! ISFLAG (__svr__, cam, __SS_SENDIMG))\
	 		continue;\
 		LIVE (__svr__, cam).sync = & ((frame)->iframe);\
	}	\
} while (0)


extern ST_NETSVR	g_netsvr;
extern void *netsvr(void *);

extern struct tm g_sync_tm;
extern char g_ntpsvr[64];

extern int handle_msg_new_ssf(struct st_message *msg);

#ifdef USE_STREAM_CONSUMER
extern int add_stream_consumer(int ch_flag, consumer_cb cb);
extern void del_stream_consumer(int id);
#endif

int get_netsvr_client_number();

#endif
