/****************************************************************************************
 * live.h
 *
 * Purpose:
 * 	- Defines data structure for live server.
 ****************************************************************************************/

#ifndef __LIVE_H
#define __LIVE_H

#include "../file/playback.h"

#define LIVE(svr, cam)						((svr)->live [cam])
#define LIVE_PLAYMODE_CHANGED(svr, cam)		(LIVE(svr, cam).cmd.playmode != LIVE(svr, cam).req.playmode)
#define LIVE_PLAYMODE_STOPPED(svr, cam)		(LIVE(svr, cam).req.playmode == PLAYMODE_STOP)

typedef struct st_live_framebuf {
	volatile int refcnt;	

	volatile int readidx;	
	volatile unsigned istamp;	
	volatile unsigned pstamp;	
	volatile unsigned stamp;	

	pthread_mutex_t mutex;	
	struct clist buf_head;	
	volatile int numframe;	
} ST_LIVE_FRAMEBUF;


#define init_live_framebuf(buf)  \
	do {\
		(buf)->refcnt = 0;\
		(buf)->readidx = -1;\
		(buf)->istamp = 0;\
		(buf)->pstamp = 0;\
		(buf)->stamp = 0;\
		pthread_mutex_init (& (buf)->mutex, NULL);\
		(buf)->buf_head.prev = & (buf)->buf_head;\
		(buf)->buf_head.next = & (buf)->buf_head;\
		(buf)->numframe = 0;\
	} while (0)


#if 0
typedef struct st_evtmsg {
	int		id;
	int		sz;
	char	msg[256];
} ST_EVTMSG;
#endif

typedef struct st_live
{
	struct st_command cmd;	
	struct st_request req;	

	int		prevmode;

	ST_PLAYOBJ *playobj;		
	volatile struct clist *send;	
	volatile struct clist *sync;	
	volatile int	numsent;		
	unsigned	stamp;			

	int	transhdr;				
	int transcam;

} ST_LIVE;

#define WE_HAVE_FRAMES_IN_BUFFER(live, cam) \
	((live)->send->next != & (LIVE_FRAME_BUF_CH(cam)->buf_head))

#define LIVE_WAITING_IFRAME(svr, cam)\
	((! LIVE (svr, cam).send) && (! LIVE (svr, cam).sync))


// Live thread specific job.
#define do_live(svr, cam, ret)	\
do {\
	if (LIVE(svr, cam).cmd.random)\
		LIVE(svr, cam).cmd.random = 0;\
	\
	if (LIVE_PLAYMODE_CHANGED (svr, cam)) {\
		LIVE(svr, cam).prevmode = LIVE(svr, cam).req.playmode;\
		LIVE(svr, cam).req.playmode = LIVE(svr, cam).cmd.playmode;\
	}\
	\
	ret = live_transframe (svr, cam);\
} while (0)


extern struct st_live *init_live(struct st_live *);
extern int handle_live_request (struct st_stream *, ST_PHDR phdr);
extern int check_recording_stat(int);
extern void release_live(struct st_live *);
extern int live_transframe(struct st_stream *, int);
extern void sync_live_framebuf(struct st_frame *, int);
extern int live_release_frame(int, int nMaxLiveFrame);
extern int live_release_all_frame(int ch);
extern int __transframe(struct st_stream *, struct st_frame *, int);

#endif
