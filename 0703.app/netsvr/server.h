#ifndef __SERVER_H
#define __SERVER_H

#include "vod.h"
#include "live.h"
#include "sysinfo.h"
#include "setup/setup.h"

#include "../wsServer/websocket_server.h"


#ifdef USE_STREAM_CONSUMER
typedef int (*consumer_cb)(int id, struct st_frame *frame, int ch);
#endif


typedef enum
{
	SOCKET_TYPE_TCP = 0,
	SOCKET_TYPE_SSL,
	SOCKET_TYPE_WEBSOCKET,

	SOCKET_TYPE_MAX

} SOCKET_TYPE;

struct st_stream {
	boolean	inuse;				

	pthread_t id;				
	pthread_t stream_id;		
	volatile int sock;			

	struct st_cliinfo	*cliinfo;




	ST_USR * usr;

	int flag [3];			
	int flag1;
	int flag2 [3];			
	int flag3; 



	__u64	keepalive_chk_stamp;
	__u8				vod_sync;
	unsigned int		session_code;


	struct st_vod vod [3];	
	struct st_live live [3];	

	__u64	netstamp [3];


	// For bypass event notification
	int		transbypass;		// Transfer bypass data.

	int		by_sched[3];
	int		by_camrec[3];
	int		by_vloss[3];
	int		by_recstat[3];
	int		bm_motion;
	int		by_bmsensor;		// Bitmask of sensor
	bool	by_emg_rec;
	bool	by_recfail;

	int terminate;				// Should we terminate connection ?

	int stream_log_out_flag;
	int svr_log_out_flag;
	// http 9010 port
	char http_header[64];
	int	http_header_chk;

#ifdef USE_STREAM_CONSUMER
	int is_consumer;
	consumer_cb cb;
#endif

#ifdef _USE_WEBSOCKET_SERVER_
	int is_websocket;
	int websocket_session_id;
	websocket_session_t*		websocket_session;
#endif


} __attribute__((__packed__));

#define SET_P2P_WRITE_MEM(svr, on)

// Service thread's property and status.
#define	__SS_LIVE			0x00000002
#define	__SS_SENDIMG		0x00000008	
#define	__SS_AUTHED			0x00000020

#if 0
#define SS_CLRFLAG(svr, cam)	\
	do {\
		(svr)->flag [cam] = 0;\
	} while (0)
#else
#define SS_CLRFLAG(svr, cam)	\
	do {\
		((svr)->flag [cam]) &= (~(0xf));\
	} while (0)
#endif



#define SS_SETFLAG(svr, cam, f)		\
	do {\
		((svr)->flag [cam]) |= (f);\
	} while (0)

#define SS_UNSETFLAG(svr, cam, f) \
	do {\
		((svr)->flag [cam]) &= (~(f));\
	} while (0)

#define ISFLAG(svr, cam, f)	\
	({ \
		(((svr)->flag [cam]) & (f)) ? 1 : 0;\
	})


#if 1

#define SS_CLRFLAG1(svr)	\
	do {\
		(svr)->flag1 = 0;\
	} while (0)

#define SS_SETFLAG1(svr, f)		\
	do {\
		((svr)->flag1) |= (f);\
	} while (0)

#define SS_UNSETFLAG1(svr, f) \
	do {\
		((svr)->flag1) &= (~(f));\
	} while (0)

#define ISFLAG1(svr, f)	\
	({ \
		(((svr)->flag1) & (f)) ? 1 : 0;\
	})

#define SS_CLRFLAG2(svr, cam)	\
	do {\
		(svr)->flag2 [(cam)] = 0;\
	} while (0)

#define SS_SETFLAG2(svr, cam, f)		\
	do {\
		((svr)->flag2 [(cam)]) |= (f);\
	} while (0)

#define SS_UNSETFLAG2(svr, cam, f) \
	do {\
		((svr)->flag2 [(cam)]) &= (~(f));\
	} while (0)

#define ISFLAG2(svr, cam, f)	\
	({ \
		(((svr)->flag2 [(cam)]) & (f)) ? 1 : 0;\
	})

#endif



#define ISLIVE(svr, cam)		ISFLAG (svr, cam, __SS_LIVE)

struct st_cliinfo {
	struct clist list;		
	struct sockaddr addr;
	char ip[32];
	char uid[MAX_USRID];
	int	refcnt;			
};



struct _NETSVR;
extern void * svr_thread (void *);
extern void * stream_thread (void *);
extern void flush_cliinfo(struct _NETSVR *);
extern struct st_stream *__creat_streamsvr2 (int, SOCKET_TYPE, int idx);
extern void __release_stream(struct st_stream *);
extern void release_stream(struct _NETSVR *, struct st_stream *);
#ifdef USE_STREAM_CONSUMER
extern int __creat_streamsvr_for_consumer(int ch_flag, consumer_cb cb);
extern void __release_streamsvr_for_consumer(int id);
#endif


#if __USE_WEBSOCKET_BY_TCP__
char* get_last_websocket_tcp_ip();
#endif


#endif /*__SERVER_H*/
