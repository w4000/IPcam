#include "common.h"
#include "file/ssfsearch.h"
#if 1
#include "../tools/new_rsetup_jQuery/cgi-api/load_setup.h"
#else 
#endif 

#ifdef __USE_RTSP_SERVER__
#include "rtsp/RTSP_common.h"
#endif

#define	FORCED_I_ON_CONNECT			1

#ifdef __USE_RTSP_SERVER__
static inline void rtsp_sync_frame(int cam, struct st_frame * frame)
{

	SVR_CTX	*ctx = NULL;
	int			i = 0;

	if (!g_setup_ext.rtsp_info.use_rtsp)
		return;
	
	_foreach_context (ctx, i)
		assert (VALIDCH (ctx->c_cam));

		if (ctx->c_cam != cam)
			continue;
		if (! ISLIVE (& ctx->c_stream, ctx->c_cam))
			continue;
		if (ctx->c_state != _SEND_DATA)
			continue;
		LIVE (& ctx->c_stream, ctx->c_cam).sync = & ((frame)->iframe);
	end_foreach
}
#endif

void sync_live_framebuf(struct st_frame * unit, int ch) {

	pthread_mutex_lock (&LIVE_FRAME_BUF_CH(ch)->mutex);

	addto_clist_tail (& LIVE_FRAME_BUF_CH(ch)->buf_head, &unit->iframe);
	LIVE_FRAME_BUF_CH(ch)->numframe++;
	LIVE_FRAME_BUF_CH(ch)->stamp = DATAUNIT_STAMP(unit->data_unit);


	if (DATAUNIT_IS_IFRAME(unit->data_unit)
			|| DATAUNIT_IS_JPEG(unit->data_unit)
			)
	{	
#ifdef __USE_RTSP_SERVER__

		ST_SSFBUILD *build = &g_file.ssf_build [g_file.ssf_index][ch];

		if(build->videop == NULL){
			pthread_mutex_unlock(&LIVE_FRAME_BUF_CH(ch)->mutex);
			return;
		}

		memcpy (VIDEOPROP_SPSPPS(build->videop), unit->data_unit + unit->hdrsz, min (VIDEOPROP_SPSPPS_SZ, unit->rawsz)); 

		rtsp_sync_frame (ch, unit);
#endif	
		live_sync_frame (ch, unit);
	}
 

	pthread_mutex_unlock(&LIVE_FRAME_BUF_CH(ch)->mutex);
}

static int live_free_frame(int ch, int min_frame) {
	struct st_frame *buf;
	int num_release = 0;

	assert (VALIDCH (ch));

	pthread_mutex_lock (&LIVE_FRAME_BUF_CH(ch)->mutex);



	while (LIVE_FRAME_BUF_CH(ch)->numframe > min_frame)
	{
		buf = clist_entry(LIVE_FRAME_BUF_CH(ch)->buf_head.next, struct st_frame, iframe);

		if (min_frame > 0)
		{
			static int s_cnt[CAM_MAX] = { 0, 0 };

			if (buf->refcnt > 0) 
			{
				if (s_cnt[ch]++ > 3000)
				{
					if(s_cnt[ch] < 3005)
					{
						
						send_msg(NETSVR, MAIN, _CAMMSG_REBOOT, 7, 0, 0);
					}
					else
						s_cnt[ch] = 3010;
				}

				break;

			}
			else {
				s_cnt[ch] = 0;

				{
					struct st_stream *svr = NULL;
					int slot;

					foreach_occupied_svr_slot (svr, slot)
					{
						if (svr)
						{
							struct st_frame *send = (struct st_frame *)LIVE (svr, ch).send;
							if (send && send == buf)
							{
								LIVE (svr, ch).send = NULL;
							}

							struct st_frame *sync = (struct st_frame *)LIVE (svr, ch).sync;
							if (sync && sync == buf)
							{
								LIVE (svr, ch).sync = NULL;
							}
						}
					}
					end_foreach
				}

				__frame_release(buf);
				LIVE_FRAME_BUF_CH(ch)->numframe--;
				num_release ++;


			}
		}
#if 0
		else {
			__frame_release(buf);
			LIVE_FRAME_BUF_CH(ch)->numframe--;
			num_release ++;
		}
#endif
	}

	pthread_mutex_unlock(&LIVE_FRAME_BUF_CH(ch)->mutex);

	return num_release;
}

int live_release_frame(int ch, int nMaxLiveFrame) {
	return live_free_frame(ch, nMaxLiveFrame - 1);
}
int live_release_all_frame(int ch) {
	return live_free_frame(ch, 0);
}

struct st_live * init_live (struct st_live *live) {

	assert (live);

	live->cmd.random = 0;
	live->cmd.timestamp = 0;
	live->cmd.playmode = PLAYMODE_STOP;
	live->cmd.playspeed = 1;

	live->req.date = 0;
	live->req.time = 0;
	live->req.ch = -1;
	live->req.playmode = PLAYMODE_STOP;
	live->req.playspeed = 1;

	live->prevmode = PLAYMODE_STOP;

#if (FORCED_I_ON_CONNECT)
	live->playobj = NULL;
#else
	live->playobj = playobj_init();
#endif
	live->send = NULL;
	live->sync = NULL;
	live->numsent = 0;
	live->stamp = 0;

	live->transhdr = 0;
	live->transcam = -1;


	return live;
}


void release_live (struct st_live * live) {

	assert (live);


	if (live->send && (live->send != &LIVE_FRAME_BUF_CH(live->req.ch)->buf_head)) {
		pthread_mutex_lock(&LIVE_FRAME_BUF_CH(live->req.ch)->mutex);

		struct st_frame *frame = NULL;
		frame = clist_entry(live->send, struct st_frame, iframe);
		if (frame)
		{
			if (frame->refcnt > 0)
				frame->refcnt--;
		}
		live->send = NULL;

		pthread_mutex_unlock(&LIVE_FRAME_BUF_CH(live->req.ch)->mutex);
	}

	if (live->playobj) {
		playobj_release(live->playobj);
		live->playobj = NULL;
	}
}

int check_recording_stat(int ch) {

	assert (VALIDCH (ch));

	if (!CAMERA_EXIST(ch))
	 	return -ERR_NOCAM;


	return 0;
}

static int live_transhdr (struct st_stream *strsvr, int cam) {
	char sndbuf [4*K];
	char * tmp = sndbuf + sizeof (struct phdr);
	int  hdrsz=0;
	int msgsz;

	if (! VALIDCH (cam))
		return reply2client (strsvr, strsvr->sock, sndbuf, REPLYID (PSENDSTREAM), RETFAIL, 0);

	if (! CAMERA_EXIST (cam))
		return reply2client (strsvr, strsvr->sock, sndbuf, REPLYID (PSENDSTREAM), RETNOCAM, 0);

#if 0
#else
	hdrsz = check_ssfhdr (SSF_BUILD_CH (cam)->ssfhdrs, MAX_SSFHDRSZ);
#endif

	setnetbuf (tmp, char, cam);
	setnetbuf (tmp, char, 0);
#if 0 
#else
	setnetbufraw (tmp, hdrsz, SSF_BUILD_CH (cam)->ssfhdrs);
#endif

	msgsz = tmp - (char *) (sndbuf + sizeof (struct phdr));
	return reply2client (strsvr, strsvr->sock, sndbuf, REPLYID (PSENDSTREAM), RETSUCCESS, msgsz);
}

static int live_transcam (struct st_stream *strsvr, int cam) {
	char sndbuf [4*K];
	char * tmp = sndbuf + sizeof (struct phdr);
	int msgsz;

	if (! VALIDCH (cam))
		return reply2client (strsvr, strsvr->sock, sndbuf, REPLYID (PREQCAMINFO), RETFAIL, 0);

	if (! CAMERA_EXIST (cam))
		return reply2client (strsvr, strsvr->sock, sndbuf, REPLYID (PREQCAMINFO), RETNOCAM, 0);

	int len = sizeof(int);
	setnetbuf (tmp, char, cam);
	setnetbuf (tmp, int, len);
	setnetbuf (tmp, int, g_setup_ext.caminfo_ext[cam].Iref_mode);

	msgsz = tmp - (char *) (sndbuf + sizeof (struct phdr));
	return reply2client (strsvr, strsvr->sock, sndbuf, REPLYID (PREQCAMINFO), RETSUCCESS, msgsz);
}

int __transframe (struct st_stream * svr, struct st_frame * buf, int cam)
{
	char * sndbuf = buf->data_unit - sizeof (struct psendstream) - sizeof (struct phdr);
	char * tmp = (char *) sndbuf + sizeof (struct phdr);
	int msgsz = buf->hdrsz + buf->rawsz + sizeof (struct psendstream);

	setnetbuf (tmp, char, cam);
	setnetbuf (tmp, char, 1);

	return reply2client (svr, svr->sock, sndbuf, REPLYID (PSENDSTREAM), RETSUCCESS, msgsz);
}

int live_transframe (struct st_stream *svr, int cam)
{
	int ret;
	struct st_frame * buf = NULL;	
	assert (svr);


	pthread_mutex_lock (& LIVE_FRAME_BUF_CH (cam)->mutex);

#if 0
#else

	if (LIVE_WAITING_IFRAME (svr, cam)) {
		pthread_mutex_unlock(& LIVE_FRAME_BUF_CH (cam)->mutex);
		return 0;
	}
	else if ((!LIVE (svr, cam).send) && LIVE (svr, cam).sync) {
		clist_entry(LIVE (svr, cam).sync, struct st_frame, iframe)->refcnt++;

		LIVE (svr, cam).send = LIVE (svr, cam).sync;
		LIVE (svr, cam).sync = NULL;
	}
	else if (LIVE (svr, cam).send && (!LIVE (svr, cam).sync)) {
		if (!WE_HAVE_FRAMES_IN_BUFFER(& LIVE (svr, cam), cam)) {
			pthread_mutex_unlock(& LIVE_FRAME_BUF_CH (cam)->mutex);
			return 0;
		}
		clist_entry (LIVE (svr, cam).send, struct st_frame, iframe)->refcnt--;
		clist_entry (LIVE (svr, cam).send->next, struct st_frame, iframe)->refcnt++;
		LIVE (svr, cam).send = LIVE (svr, cam).send->next;
	}
	else if (LIVE (svr, cam).send && LIVE (svr, cam).sync) {

		if (WE_HAVE_FRAMES_IN_BUFFER(& LIVE (svr, cam), cam) &&
			(clist_entry(LIVE (svr, cam).sync, struct st_frame, iframe)->stamp < clist_entry(LIVE (svr, cam).send, struct st_frame, iframe)->stamp + 500))
		{
			clist_entry (LIVE (svr, cam).send, struct st_frame, iframe)->refcnt--;
			clist_entry (LIVE (svr, cam).send->next, struct st_frame, iframe)->refcnt++;
#if 1
			LIVE (svr, cam).send = LIVE (svr, cam).send->next;

			if (LIVE (svr, cam).send == LIVE (svr, cam).sync)
				LIVE (svr, cam).sync = NULL;
#else
#endif
		}
		else
		{
			clist_entry(LIVE (svr, cam).send, struct st_frame, iframe)->refcnt--;
			clist_entry(LIVE (svr, cam).sync, struct st_frame, iframe)->refcnt++;
			LIVE (svr, cam).send = LIVE (svr, cam).sync;
			LIVE (svr, cam).sync = NULL;
		}
	}
	if (LIVE (svr, cam).req.playmode == PLAYMODE_STOP)
	{
		pthread_mutex_unlock (& LIVE_FRAME_BUF_CH (cam)->mutex);
		return 0;
	}


#endif
	buf = clist_entry(LIVE (svr, cam).send, struct st_frame, iframe);

	pthread_mutex_unlock (& LIVE_FRAME_BUF_CH (cam)->mutex);


	{
		
#if 0
		if(svr->is_consumer) {
			ret = svr->cb(svr->sock, buf, cam); 
		}
		else
#endif

#if 1

		{
			if (LIVE (svr, cam).transhdr) {
				LIVE (svr, cam).transhdr = FALSE;
				ret = live_transhdr (svr, cam);
				if (ret < 0)
					return ret;

				LIVE (svr, cam).transhdr = FALSE;

				
			}

			if (LIVE (svr, cam).transcam != g_setup_ext.caminfo_ext[cam].Iref_mode) {
				ret = live_transcam (svr, cam);
				if (ret < 0)
					return ret;

				LIVE (svr, cam).transcam = g_setup_ext.caminfo_ext[cam].Iref_mode;
			}
			
			
			
			{
				ret = 0;
				static int64_t trans_msec = 0;
				struct st_frame *frm = (struct st_frame *)buf;
	
					int64_t ttt = get_time_msec();
					ret = __transframe (svr, buf, cam);
					trans_msec += elapsed_time_msec(ttt);
			}
		}
#endif




	}
	if (ret < 0)
		return ret;

	LIVE (svr, cam).stamp = DATAUNIT_STAMP (buf->data_unit);
	LIVE (svr, cam).numsent ++;

	return 0;
}




#define  MSG_KEY     2023

typedef struct _st_msg {
	int status;
	long type;
	int  num;
	char cmd[0xff];
	int  params[2];
} st_msg;

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static int msg_send( char *_cmd, int p1, int p2 )
{
	int g_shmid = -1;
	void *shared_memory = (void *)0;

	st_msg *motor_msg = NULL;

	if(_cmd == NULL) return -1;

	if(g_shmid < 0) {
		g_shmid = shmget((key_t)MSG_KEY, sizeof(struct _st_msg), 0);
		if(g_shmid < 0) {
			motor_msg = NULL;
			return -1;
		}

		shared_memory = shmat(g_shmid, (void *)0, IPC_CREAT | 0666);
		if(shared_memory < 0) {
			motor_msg = NULL;
			return -1;
		}
		motor_msg = (struct _st_msg *)shared_memory;
		motor_msg->status = 0;
	}
	if(motor_msg == NULL) {
		return -1;
	}

	motor_msg->status = 1;
	motor_msg->type = 1;
	motor_msg->num  = 1;
	memset(motor_msg->cmd, 0, sizeof(motor_msg->cmd));
	strcpy(motor_msg->cmd, _cmd);
	motor_msg->params[0] = p1;
	motor_msg->params[1] = p2;

	return 0;
}







int handle_live_request (struct st_stream *strsvr, ST_PHDR phdr)
{
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
		if ((ret=handle_heartbeat(strsvr, sd, &phdr)) < 0) {
			return ret;
		}
		break;

	default:
		return 1;
	}

	return 0;
}

