/*
 * vod.c
 *
 * purpose:
 *	- Implements VOD server.
 * 	- Initialize and maintain VOD server structure.
 ********************************************************************************/
#include "common.h"
#include "file/ssfsearch.h"
#include "file/playback.h"
#include "pos/poslib.h"

/*******************************************************************************
 * Initialize and release VOD server structure.
 *******************************************************************************/

/* Initialize and release resouces of VOD struct. */
struct st_vod * init_vod (struct st_vod * vod) {
	if (!vod)
		return NULL;

	vod->cmd.random = 0;
	vod->cmd.timestamp = 0;
	vod->cmd.playmode = PLAYMODE_PLAY;
	vod->cmd.playspeed = 1;

	vod->playobj = playobj_init("init_vod");

	vod->remote_copy = 0;
	vod->prevmode = vod->cmd.playmode;

	vod->transhdr = 0;
	return vod;
}

static inline void _release_vod_file (struct st_vod *vod)
{
	// printf("! %s:%d >>> playobj_release \n", __func__, __LINE__); playobj_release (vod->playobj);
	vod->playobj = NULL;
}

void release_vod (struct st_vod *vod)
{
	if (vod)
		_release_vod_file (vod);
}

/*****************************************************************************
 * Client's request handling routines.
 *****************************************************************************/
#define REPLY_CAM(retval) \
({\
	char sndbuf [256];\
	char * tmp = NULL;\
	int msgsz;\
	tmp = sndbuf + sizeof (struct phdr);\
	setnetbuf (tmp, char, cam);\
	msgsz = tmp - (char *) (sndbuf + sizeof (struct phdr));\
	reply2client (strsvr, sd, sndbuf, REPLYID(ph->id), retval, msgsz);\
})

int handle_stream (struct st_stream *strsvr, int sd, struct phdr *ph) {

	ST_PREQSTREAM req;
	int cam = 0;

	{ 
		char buf [4*K];
		char * tmp = buf;

		if (recvmsgfrom (strsvr, sd, buf, ph->len) < 0) {
			printf("\n");
			return REPLY_CAM (RETERRNET);
		}

		setfield(string, req.svrname, tmp);
		setfield(char , req.type, tmp);
		setfield(char, req.cam, tmp);
		setfield(int, req.date, tmp);
		setfield(int, req.time, tmp);
		setfield(char, req.dst, tmp);
		setfield(char, req.dir, tmp);
	}

	

#if 1
	cam = req.cam;
#else 
	cam = 1;
#endif
	if (strsvr->usr)
	{
		int no_perm = 0;

		if (req.type == 0 || req.type == 10)
		{
			if (!strsvr->usr->uperm.live)
			{
				no_perm = 1;
			}
		}
		else
		{
			if (!strsvr->usr->uperm.vod)
			{
				no_perm = 1;
			}
		}

		if (no_perm)
		{
			printf("NO USER PERMISSION..(LIVE/VOD) : %08x\n", ph->id);
			char buf [1];
			reply2client(strsvr, sd, buf, REPLYID(ph->id), RETNOPERM, 0);
			return -1;
		}
	}
	///////////////////////////

	if (req.type == 0 || req.type == 10)
		goto LIVE_REQ;
	// else
	// 	goto VOD_REQ;

LIVE_REQ:
	{ // It's live request. we should send live header.
		// if (! VALIDCH (cam))
		if (! VALIDCH (cam))
		{
			return REPLY_CAM (RETFAIL);
		}


		if (ISLIVE (strsvr, cam)) {
			release_live (& LIVE (strsvr, cam));
			init_live (& LIVE (strsvr, cam));
		}

		LIVE (strsvr, cam).req.ch = cam;
		LIVE (strsvr, cam).req.date = SSF_BUILD_CH (cam)->date;
		LIVE (strsvr, cam).req.time = SSF_BUILD_CH (cam)->time;
		LIVE (strsvr, cam).req.playmode = PLAYMODE_PLAY;
		LIVE (strsvr, cam).req.playspeed = 1;

		LIVE (strsvr, cam).cmd.random = 1;
		LIVE (strsvr, cam).cmd.timestamp = 0;
		LIVE (strsvr, cam).cmd.playmode = PLAYMODE_PLAY;
		LIVE (strsvr, cam).cmd.playspeed = 1;

		pthread_mutex_lock (& LIVE_FRAME_BUF_CH (cam)->mutex);
		LIVE_FRAME_BUF_CH (cam)->refcnt++;
		pthread_mutex_unlock (& LIVE_FRAME_BUF_CH (cam)->mutex);

		LIVE (strsvr, cam).transhdr = 1;

		//Set the stream type of the camera.
		SS_SETFLAG (strsvr, cam, __SS_LIVE);
		SS_SETFLAG (strsvr, cam, __SS_SENDIMG);


		return  REPLY_CAM (RETSUCCESS);
	}
}

/* Accept client's heartbeat checking. Client checks the heartbeat to see if the server
 * is alive. Request arrives every 5 minute.
 *
 * Return:
 *	0, on success.
 *	-ERR_CODE, on error.
 * Called by:
 *	handle_vod_request().
 */
int handle_heartbeat(struct st_stream *strsvr, int sd, struct phdr *ph) {
	
}


/* Analyze protocol and serve according to the client's request.
 * Request could arrive to control or data line.
 * Return:
 *	0, on success.
 *	-ERR_CODE, on error.
 */
int handle_vod_request(struct st_stream *strsvr, ST_PHDR phdr) {
	int sd;
	int ret=0;

	sd = strsvr->sock;

	assert (sd >= 0);

	switch (phdr.id) {
	case PREQSTREAM:
		if ((ret=handle_stream (strsvr, sd, &phdr)) < 0)
			return ret;
		break;

	case PHEARTBEAT:
		if ((ret=handle_heartbeat(strsvr, sd, &phdr)) <0)
			return ret;
		/*fprintf(stderr, "VOD server: PHEARTBEAT request replied.\n");*/
		break;


	case PUSRAUTH:
	case PUSRENCAUTH:
	case PUSRENCAUTH_REINF:
		return 1;


	default:
		return 1;
	}
	return 0;
}


