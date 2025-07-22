
#include <stdio.h>

#include "encoder.h"
#include "file/ssfsearch.h"
#include "rtsp/RTSP_common.h"
#include "device/gpio.h"
#include "push/push_md_http.h"
#ifdef USE_VIDEO_POOL
#include "lib/video_pool.h"
#endif


ENC_USER_PARAM	g_EncUserParam;

unsigned char* __g_unitbuf = NULL; 				//[ENC_BUFSZ];
static ST_ENCUNIT * s_unit 			= NULL;		//(ST_ENCUNIT *) __g_unitbuf;

static int s_unit_save = 0;


__u16 val [BITRATE_MAX] = {
	200, 300, 400, 500, 600, 700, 750, 900, 1100, 1300, 1500, 1800, 2000, 2500, 3000, 3500, 4000, 5000, 6000, 7000, 8000,
	9000, 10000, 12000, 14000, 16000
};


enum
{
	ENC_ERR_ROOT = ENC_ERR_USERERR,

	ENC_ERR_VIDEOFILE_OPEN,
	ENC_ERR_AUDIOFILE_OPEN,
	ENC_ERR_JPEGFILE_OPEN,

};



void get_videoInfo_from_setup(int ch, int netEncMode, VIDEO_ENCODER_INFO* pVideoInfo)
{
	
	pVideoInfo->ch = ch;

	SIZE_S size = { 1920, 1080 };


	def_get_picsize(1, \
			g_setup.cam[ch].res, &size);	

	pVideoInfo->capture_width  = size.u32Width;
	pVideoInfo->capture_height = size.u32Height;
	pVideoInfo->groupOfPicture = g_setup.cam [ch].gop;
	pVideoInfo->videoFrameRate = g_setup.cam [ch].fps;

	pVideoInfo->codec = g_setup.cam[ch].codec;

	{
		pVideoInfo->bIsCBR = (g_setup.cam [ch].ratectrl == RATECTRL_VBR) ? 0 : 1;

		pVideoInfo->videoBitRate = val[g_setup.cam [ch].bitrate] * 1000;
	}


	pVideoInfo->Iref_mode = g_setup_ext.caminfo_ext[ch].Iref_mode;
}

static void set_encoding_params_dynamic(ENCODE_INFO* pEncInfo, unsigned rec_chmap, boolean byforce)
{
	
	ENC_OBJECT_INFO* pObjInfo = pEncInfo->pObjInfo;
	int ch;

	foreach_cam_ch (ch)
	{

		get_videoInfo_from_setup(ch, NET_ENCMODE(ch), pEncInfo->pVideoInfo[ch]);

		// if (byforce && g_setup.cam[ch].rec)
		{
			int ret = Encoder_DynamicSetParam(ch, pObjInfo, pEncInfo->pVideoInfo[ch]);
			if (ret != 0)
			{
				break;
			}
		}


	}
	end_foreach

}

static void save_unit (ENC_OBJECT_INFO* pObjInfo, ST_ENCUNIT * unit, VENC_STREAM_S *pstStream)
{
	int res;

	assert (unit);

	if (unit->ch == 0 && unit->type == SSF_VIDEO)
	{
		static int __old_save = -1;
		if (__old_save != unit->save)
		{
			__old_save = unit->save;
		}
	}


	res = ssf_addunit (& g_file.ssf_build [g_file.ssf_index][unit->ch], unit, pstStream);
}

static int handle_msg_stop_2(ENCODE_INFO* pEncInfo);
static int handle_msg_start_2 (ENCODE_INFO* pEncInfo);

static void start_encoding(ENCODE_INFO* pEncInfo);
static void stop_encoding();

static int _restart_2(ENCODE_INFO* pEncInfo)
{
	int ret;
	
	pthread_mutex_lock(&g_encoder.mutex_saveunit);
	{
		handle_msg_stop_2 (pEncInfo);
		ret = handle_msg_start_2 (pEncInfo);
	}
	pthread_mutex_unlock(&g_encoder.mutex_saveunit);

	return ret;
}

static inline int _restart(ENCODE_INFO* pEncInfo)
{
	return _restart_2(pEncInfo);
}


int encode_motion_check(char MDBuff[15 + 1][16 + 1], HI_U64 u64Pts)
{
	int ch = 0;
    HI_S32 i, j;
	int r;


    for(i = 0; i < 15; i++)
	{
		for(j = 0; j < 16; j++)
		{
			if (g_setup.cam [0].area[i][j]) {
				if (MDBuff[i][j] > 0) {
					bitset (DEV_QUAD.motion, ch);
#ifdef SUPPORT_MD_PUSH
					// push_md_detected();
#endif
					goto loop_out;
				}
			}
		}
	}

    bitunset (DEV_QUAD.motion, ch);

loop_out:


	{
		static HI_U64 motiontm = 0;
		static HI_U64 motiontm2 = 0;

		HI_U64 unittm = u64Pts / 1000;

		if (bitisset (DEV_QUAD.motion, ch))
		{
			static int last_motion_ssf_index = -1;


			if (!bitisset (IPCAM_THREAD_MAIN->mot, ch) || \
					(last_motion_ssf_index >= 0 && g_file.ssf_index != last_motion_ssf_index))
			{
				unsigned recmap_cam = 0;
				int oldmot = IPCAM_THREAD_MAIN->mot;

				bitset (IPCAM_THREAD_MAIN->mot, ch);
				g_EncUserParam.recFlagbyMotion  = 1;
				g_EncUserParam.bIsTherePrealarmData = 1;
				recmap_cam = (oldmot ^ IPCAM_THREAD_MAIN->mot);


				send_msg(ENCODER, MAIN, _CAMMSG_MOTION, ch, 0, 1);
				send_msg(ENCODER, NETSVR, _CAMMSG_MOTION, ch, 0, 1);

				motiontm2 = unittm;
				last_motion_ssf_index = g_file.ssf_index;
			}
			else
			{
				HI_U64 duration2 = unittm - motiontm2;
				if (duration2 >= (HI_U64)(1*1000))
				{
					motiontm2 = unittm;
					send_msg(ENCODER, MAIN, _CAMMSG_MOTION, ch, 0, 1);
					send_msg(ENCODER, NETSVR, _CAMMSG_MOTION, ch, 0, 2);
				}
			}

			motiontm = unittm;
		}
		else
		{
			if (bitisset (IPCAM_THREAD_MAIN->mot, ch))
			{
				unsigned duration = unittm - motiontm;
				if (duration >= ((g_setup.evnt_mot [ch].dwell) *1000))
				{
					unsigned recmap_cam = 0;
					int oldmot = IPCAM_THREAD_MAIN->mot;

					bitunset (IPCAM_THREAD_MAIN->mot, ch);
					g_EncUserParam.recFlagbyMotion  = 0;
					recmap_cam = (oldmot ^ IPCAM_THREAD_MAIN->mot);


					{
						send_msg(ENCODER, MAIN, _CAMMSG_MOTION, ch, 0, 0);
						send_msg(ENCODER, NETSVR, _CAMMSG_MOTION, ch, 0, 0);	
					}

					g_encoder.pObjInfo->jpgSaveBufLength = 0;


				}
			}
		}
	}

    return HI_SUCCESS;
}



void encoder_prealrm_init_framebuf(struct st_prealrm_framebuf buf[]) {
	int i;

	if (!buf) {
		printf("Bad argument.\n");
		return;
	}

	for (i = 0; i < CAM_MAX; i++) {
		pthread_mutex_init(&buf[i].mutex, NULL);
		buf[i].buf_head.prev = &buf[i].buf_head;
		buf[i].buf_head.next = &buf[i].buf_head;
		buf[i].numframe = 0;
	}
	return;
}


int encoder_prealrm_release_all_frame(int ch)
{
	while (PREALRM_FRAME_BUF_CH(ch)->numframe > 0)
	{
		struct st_frame *buf = clist_entry(PREALRM_FRAME_BUF_CH(ch)->buf_head.next, struct st_frame, iframe);

		__frame_release(buf);
		PREALRM_FRAME_BUF_CH(ch)->numframe--;
	}

	return 1;
}


int encode_before_start(void* pUserParam, ENC_OBJECT_INFO* pObjInfo)
{
	g_encoder.th.state = STAT_READY;

	return 0;
}


//#define SAVE_VIDEO_AUDIO
#ifdef SAVE_VIDEO_AUDIO
int g_bIsVideoOpen = 1, g_bIsAudioOpen = 1, g_nAudioCnt = 0;
FILE* g_fpVideo = NULL;
FILE* g_fpAudio = NULL;
#endif

static int open_new_channel_2_core(ENCODE_INFO* pEncInfo);


int encode_after_video_encoded(int ch, PAYLOAD_TYPE_E enType, void *pUserParam, VENC_STREAM_S *pstStream)
{
	pthread_mutex_lock(&g_encoder.mutex_saveunit);

	ENCODE_INFO* pEncInfo = (ENCODE_INFO *)pUserParam;
	ENC_OBJECT_INFO* pObjInfo = pEncInfo->pObjInfo;

	int refType;
	if(enType == PT_H264) {
		refType = pstStream->stH264Info.enRefType;
	}
#ifdef SUPPORT_H265
	else if(enType == PT_H265) {
		refType = pstStream->stH265Info.enRefType;
	}
#endif
	else if(enType == PT_JPEG) {
		refType = 0;
	}



	if (pEncInfo->bStartNewFlag && refType == 0) {
		pEncInfo->bStartNewFlag = 0;
		open_new_channel_2_core(pEncInfo);
	}

	int i;
	s_unit->sz = 0;
	unsigned char *data_unit;
	for (i = 0; i < pstStream->u32PackCount; i++)
	{

		s_unit->sz += pstStream->pstPack[i].u32Len;
	}


	init_clist(&s_unit->list);
	s_unit->ch = ch;
	s_unit->type = SSF_VIDEO;
	s_unit->seq = (refType == 0) ? 0 : 1; 
	if(refType == 10) {
		s_unit->seq = refType;
	}
	s_unit->tm = pstStream->pstPack->u64PTS / 1000;


	s_unit->save = 0; 


#if 0
#else 
	save_unit (pObjInfo, s_unit, pstStream);
#endif 

	g_EncUserParam.nAliveCount++;
	if (g_EncUserParam.nAliveCount > 50)
	{
		g_EncUserParam.nAliveCount = 0;
	}


	pthread_mutex_unlock(&g_encoder.mutex_saveunit);
	return 0;
}


int encode_after_stop(void* pUserParam, ENC_OBJECT_INFO* pObjInfo)
{

	return 0;
}

int encode_exit(void* pUserParam)
{
	ENCODE_INFO* pEncInfo = (ENCODE_INFO*)pUserParam;
	int ch;

	for (ch = 0; ch < MAX_ENC_CHANNEL; ch++)
	{
		if (pEncInfo->pVideoInfo[ch]->fp) {
			fclose(pEncInfo->pVideoInfo[ch]->fp);
			pEncInfo->pVideoInfo[ch]->fp = NULL;
		}
	}
	if (pEncInfo->pJpegInfo->fp) {
		fclose(pEncInfo->pJpegInfo->fp);
		pEncInfo->pJpegInfo->fp = NULL;
	}

	return 0;
}





static void handle_msg_exit(struct st_message* msg)
{
    StopEncode();
}


inline int write_buf_to_file();

static int open_new_channel_2_core(ENCODE_INFO* pEncInfo)
{
	int ch = 0, res = 0;
	int xres, yres;

	struct timeval now;
	gettimeofday (& now, NULL);

	int last_index = g_file.ssf_index;

	{
		pthread_mutex_lock(&g_file.ssf_buf_mutex);
			g_file.ssf_buf_pos_flush[g_file.ssf_buf_idx] = g_file.ssf_buf_pos[g_file.ssf_buf_idx];
		pthread_mutex_unlock(&g_file.ssf_buf_mutex);


		write_buf_to_file();

	}

	g_file.ssf_index = (g_file.ssf_index + 1) % 2;

	foreach_cam_ch (ch)
	{
		xres = pEncInfo->pVideoInfo[ch]->capture_width;
		yres = pEncInfo->pVideoInfo[ch]->capture_height;

		ST_SSFBUILD *build = &g_file.ssf_build [g_file.ssf_index][ch];
#ifdef SUPPORT_H265
		PAYLOAD_TYPE_E enType;
		if(pEncInfo->pVideoInfo[ch]->codec == 1) {
			enType = PT_H265;
		}
		else if(pEncInfo->pVideoInfo[ch]->codec == 2) {
			enType = PT_JPEG;
		}
		else {
			enType = PT_H264;
		}
#endif


		if (ch == 0)
		{
			if (SSF_RECORDING (ch))
				ssfbuild_close (build);

			res = ssf_idx_build (build, &g_file.ssf_build [last_index][ch], ch, enType, xres, yres, 812, 16, &now);
		}
		else
		{
			
			struct tm	creatime;
			time_t tm;
			tm = now.tv_sec;
			localtime_r(& tm, & creatime);

			build->cam = ch;
			build->creatime = creatime;

			build->start = 0;

			int date, time;
			date = _getdate(build->creatime.tm_year+1900, build->creatime.tm_mon+1, build->creatime.tm_mday);
			time = gettime(build->creatime.tm_hour, build->creatime.tm_min, build->creatime.tm_sec);

			build->date = date;
			build->time = time;
			if (ssf_hdr_build (build, &creatime, ch, enType ,xres, yres, 812, 16) < 0)
				res = -1;
			else
				res = 0;
		}

		switch (res) {
		case 0:


			{
				struct st_message msg;
				msg.msgid = _CAMMSG_NEW_SSF;
				msg.from = ENCODER;
				msg.parm1 = ch;
				handle_msg_new_ssf(&msg);
			}


			break;

		case -ERR_NOSPC:
			send_msg (ENCODER, FILEMAN, _CAMMSG_DEL_OLDEST_SSF, 0, 0, 0);
			break;

		case -ERR_IO:
			break;

		default:
			send_msg (ENCODER, MAIN, _CAMMSG_REBOOT, 5, 0, 0);
			return -1;
		}
	}
	end_foreach

	{
		struct tm	creatime;
		time_t tm;
		tm = now.tv_sec;
		localtime_r(& tm, & creatime);

		char rdxfilename [1<<8];
		sprintf(rdxfilename, "%s/%04d%02d%02d/%04d%02d%02d%s", ssfroot[g_file.hdd],
				1900+creatime.tm_year, creatime.tm_mon+1, creatime.tm_mday,
				1900+creatime.tm_year, creatime.tm_mon+1, creatime.tm_mday, SSF_RDX_EXTENSION);

		if (strcmp(g_file.rdx_build[g_file.rdx_index].rdxname, rdxfilename) != 0)
		{
			int nChecker = 0;

			{
				pthread_mutex_lock(&g_timer.mutex_itimer);
				{
					if (g_timer.itimer [78])
					{

						ST_ITVALTIMER *itm = (ST_ITVALTIMER *)g_timer.itimer [78];
						g_timer.itimer [78] = NULL;
						rdxbuild_close (& g_file.rdx_build[itm->parm1]);
						free_itm_pool(itm);	//_mem_free((void *)itm);
						nChecker = 1;
					}
				}
				pthread_mutex_unlock(&g_timer.mutex_itimer);
			}


			
			{
				
				struct st_msg_queue *msgq = MSGQ (ENCODER);
				if (msgq)
				{
					struct st_message *pmsg = NULL;
					struct clist *tmp = NULL;
					int __rdx_index = -1;

					pthread_mutex_lock(&msgq->mutex);
					if (!clist_empty(&msgq->head)) {
						int _cnt = 0;
						tmp = (struct clist *)msgq->head.next;

						while (tmp && _cnt++ < 100)
						{
							pmsg = clist_entry(tmp, struct st_message, list);
							if (pmsg->msgid == _CAMMSG_CLOSE_LAST_RDX_BUILD)
							{
								__rdx_index = pmsg->parm1;

								del_clist(tmp);
								msgq->nummsg--;

								free_msg_pool(pmsg);	

								nChecker = 1;

								break;
							}

							if (tmp->next == tmp)
								break;

							tmp = (struct clist *)tmp->next;
						}
					}
					pthread_mutex_unlock(&msgq->mutex);

					if (__rdx_index >= 0)
						rdxbuild_close (& g_file.rdx_build[__rdx_index]);
				}
			}

			if (nChecker)
			{

				rdxbuild_close (& g_file.rdx_build[g_file.rdx_index]);
			}

			g_file.rdx_index = (g_file.rdx_index + 1) % 2;

			res = rdx_build (& g_file.rdx_build[g_file.rdx_index], rdxfilename);
			if (res == -ERR_NOSPC)
				send_msg (ENCODER, FILEMAN, _CAMMSG_DEL_OLDEST_SSF, 0, 0, 0);
		}
	}

	return 0;
}

static int open_new_channel_2(ENCODE_INFO* pEncInfo)
{
	
	pEncInfo->bStartNewFlag = 1;
	return 0;
}

static inline int open_new_channel(ENCODE_INFO* pEncInfo)
{
	return open_new_channel_2(pEncInfo);
}

static int close_channel_2(int index)
{
	int ch;

	foreach_cam_ch (ch)
		ssfbuild_close (& g_file.ssf_build [index][ch]);
	end_foreach

	return 0;
}

static inline int close_channel()
{
	return close_channel_2(g_file.ssf_index);
}


static void encoder_setencmode (int ch, ENC_OBJECT_INFO* pObjInfo) {

	NET_ENCMODE (ch) = FALSE;
}

static void encoder_recmap (ENCODE_INFO* pEncInfo, unsigned rec_chmap)
{
	ENC_OBJECT_INFO* pObjInfo = pEncInfo->pObjInfo;
	unsigned ch;

	foreach_cam_ch (ch)
	{
		if (!bitisset(rec_chmap, ch))
			continue;

		encoder_setencmode (ch, pObjInfo);

		assert (0 <= CAMERA_ENCMODE (ch) && CAMERA_ENCMODE (ch) < ENCMODE_MAX);
	}
	end_foreach

	return;
}

static void start_encoding(ENCODE_INFO* pEncInfo)
{
	
	if (g_encoder.th.state == STAT_RUNNING) {
		return;
	}
	

	encoder_recmap (pEncInfo, 0xffff);
	set_encoding_params_dynamic(pEncInfo, 0xffff, TRUE);

	int ch;
	foreach_cam_ch (ch)
	{
		memcpy(&pEncInfo->vInfo_Variable[ch], pEncInfo->pVideoInfo[ch], sizeof(VIDEO_ENCODER_INFO));
	}
	end_foreach


	g_encoder.th.state = STAT_RUNNING;

}

static void stop_encoding()
{
	if (g_encoder.th.state == STAT_READY) {
		return;
	}

	g_encoder.th.state = STAT_READY;


}

static int handle_msg_start (struct st_message *msg, ENCODE_INFO* pEncInfo)
{
	open_new_channel(pEncInfo);
	start_encoding(pEncInfo);
	return 0;
}

static int handle_msg_start_2 (ENCODE_INFO* pEncInfo)
{
	
	int ret;
	ret = open_new_channel_2(pEncInfo);
	if (ret < 0)
		return ret;

	return 0;
}

static int handle_msg_stop(struct st_message *msg, ENCODE_INFO* pEncInfo)
{
	assert (msg);

	stop_encoding();
	close_channel();
	return 1;
}

static int handle_msg_stop_2(ENCODE_INFO* pEncInfo)
{
	int nChecker = 0;

	
	{

		if (g_timer.itimer [77])
		{
			pthread_mutex_lock(&g_timer.mutex_itimer);
			{

				ST_ITVALTIMER *itm = (ST_ITVALTIMER *)g_timer.itimer [77];
				g_timer.itimer [77] = NULL;


				
				close_channel_2(itm->parm1);
				free_itm_pool(itm);	

				nChecker = 1;
			}
			pthread_mutex_unlock(&g_timer.mutex_itimer);
		}
	}

	{


		struct st_msg_queue *msgq = MSGQ (ENCODER);
		if (msgq)
		{
			struct st_message *pmsg = NULL;
			struct clist *tmp = NULL;
			int __ssf_index = -1;

			pthread_mutex_lock(&msgq->mutex);
			if (!clist_empty(&msgq->head)) {
				int _cnt = 0;
				tmp = (struct clist *)msgq->head.next;

				while (tmp && _cnt++ < 100)
				{
					pmsg = clist_entry(tmp, struct st_message, list);

					if (tmp->next == tmp)
					{
						break;
					}

					tmp = (struct clist *)tmp->next;
				}
			}
			pthread_mutex_unlock(&msgq->mutex);

			if (__ssf_index >= 0)
			{
				close_channel_2(__ssf_index);
			}
		}
	}


	if (nChecker)
	{
		close_channel_2(g_file.ssf_index);
	}

	return 1;
}

static int handle_msg_setup(struct st_message * msg, ENCODE_INFO* pEncInfo) {

	stop_encoding ();
	int ch;
	foreach_cam_ch(ch)
	{
		get_videoInfo_from_setup(ch, NET_ENCMODE(ch), pEncInfo->pVideoInfo[ch]);
	}
	end_foreach


	start_encoding (pEncInfo);

	return 0;
}

static int handle_msg_restart_ssf(struct st_message *msg, ENCODE_INFO* pEncInfo) {


	return _restart_2(pEncInfo);
}


static int _msg_evnt_motion (struct st_message *msg, ENCODE_INFO* pEncInfo)
{
	//int i;
	unsigned motcam, changed;
	int motstart;

	assert (msg);
	assert (VALIDCH (msg->parm1));

	motcam = msg->parm1;
	changed = msg->parm2;
	motstart = msg->parm3;

	if (! IPCAM_THREAD_RUNNING (ENCODER))
		return 0;


	encoder_recmap (pEncInfo, 0xffff);	



	return 0;
}





static int g_nAliveCount = 0;
int encode_check_video_loop(void* pUserParam, ENC_OBJECT_INFO* pObjInfo)
{

	ENCODE_INFO* pEncInfo = (ENCODE_INFO*)pUserParam;
	int ret = 0;
	struct st_message msg;

	if (get_msg (MSGQ (ENCODER), & msg) > 0) {


		switch(msg.msgid) {

		case _CAMMSG_START:
			pthread_mutex_lock(&g_encoder.mutex_saveunit);
			{
				
				handle_msg_start(&msg, pEncInfo);
			}
			pthread_mutex_unlock(&g_encoder.mutex_saveunit);
			break;

		case _CAMMSG_STOP:
			pthread_mutex_lock(&g_encoder.mutex_saveunit);
			{
				handle_msg_stop (& msg, pEncInfo);
			}
			pthread_mutex_unlock(&g_encoder.mutex_saveunit);
			break;

		case _CAMMSG_SETUP:
			handle_msg_setup (& msg, pEncInfo);
			break;


		case _CAMMSG_RESTART_SSF:
			ret = handle_msg_restart_ssf (& msg, pEncInfo);
			break;

		case _CAMMSG_MOTION:
			_msg_evnt_motion (& msg, pEncInfo);
			break;


		case _CAMMSG_EXIT:
			handle_msg_exit(&msg);

			g_encoder.bQuit = TRUE;
			break;

		case _CAMMSG_CLOSE_LAST_RDX_BUILD:
			rdxbuild_close (& g_file.rdx_build[msg.parm1]);
			break;

		case _CAMMSG_CGI:
			update_camera_setup(msg.parm1, msg.parm2);
			break;

		default:
			break;
		}
	}

	if (g_timer.rec_restart)	
	{
		ret = handle_msg_restart_ssf (NULL, pEncInfo);

		g_timer.rec_restart = FALSE;
	}




	return ret;
}

void* main_encoder_thread(void* pParam)//w4000_default
{
	int ret, ch; 

	ENCODE_INFO 		encodeInfo;
	VIDEO_ENCODER_INFO 	videoInfo[MAX_ENC_CHANNEL];
	JPEG_ENCODER_INFO  	jpegInfo;
	DISPLAY_INFO  		displayInfo;
	memset(&encodeInfo, 0, sizeof(ENCODE_INFO));
	memset(&videoInfo, 0, sizeof(VIDEO_ENCODER_INFO) * MAX_ENC_CHANNEL);
	memset(&jpegInfo, 0, sizeof(JPEG_ENCODER_INFO));
	memset(&displayInfo, 0, sizeof(DISPLAY_INFO));

	{
		g_encoder.th.state = STAT_UNREADY;
		init_msg_queue(& g_encoder.th.msgq, ENCODER);

		pthread_mutex_init(&g_encoder.mutex_saveunit, NULL);
	}


	{
		memset(&g_EncUserParam, 0, sizeof(ENC_USER_PARAM));

		setup_device_cam_apply (& g_setup);

		foreach_cam_ch (ch)
			bitset (DEV_CODEC.recmask, ch);	// Allocate recording channels.

			g_EncUserParam.chencmode[ch] = ENCMODE_OFF;

			g_EncUserParam.prealrm_frms [ch] = 0;
		end_foreach

		encoder_prealrm_init_framebuf(g_EncUserParam.prealrm_frame_buf);
	}

	{
		encodeInfo.maxEncFrameCnt = 0;	// 0 = INFINIT
		encodeInfo.maxCapFrameCnt = 0;
		encodeInfo.vsig = 0;

		for (ch = 0; ch < MAX_ENC_CHANNEL; ch++)
			encodeInfo.pVideoInfo[ch] = &videoInfo[ch];
		encodeInfo.pJpegInfo = &jpegInfo;

		encodeInfo.pUserParam = &encodeInfo;


		foreach_cam_ch (ch)
			get_videoInfo_from_setup(ch, 1, &videoInfo[ch]);
		end_foreach



		jpegInfo.videoCh = 0;
		jpegInfo.capture_width = ALIGN(encodeInfo.pVideoInfo[jpegInfo.videoCh]->capture_width, 16);
		jpegInfo.capture_height = ALIGN(encodeInfo.pVideoInfo[jpegInfo.videoCh]->capture_height, 16);
		jpegInfo.jpegQvalue = 95;
#if (CAM_MAX > 2)
		jpegInfo.jpegFrameRate = g_setup.cam[2].res;
#else
		jpegInfo.jpegFrameRate = g_setup.cam[1].res;
#endif
	}

	int max_width = 1920;
	int max_height = 1080;
	int __max_width = videoInfo[0].capture_width;
	int __max_height = videoInfo[0].capture_height;
	static const PIC_SIZE_E __default_resol[] = { PIC_HD1080, PIC_VGA, PIC_QVGA };
	

	for (ch = 0; ch < MAX_ENC_CHANNEL; ch++)
	{
		if (videoInfo[ch].capture_width < 2UL  || videoInfo[ch].capture_height < 2UL ||
			videoInfo[ch].capture_width > max_width || videoInfo[ch].capture_height > max_height)
		{
#ifdef USE_DF2_SUPPORT
			int resol_idx = __default_resol[ch];
			videoInfo[ch].capture_width = def_resol_index_to_width(g_setup.videotype, resol_idx);
			videoInfo[ch].capture_height = def_resol_index_to_height(g_setup.videotype, resol_idx);
#endif
		}

		if (__max_width < videoInfo[ch].capture_width)
			__max_width = videoInfo[ch].capture_width;
		if (__max_height < videoInfo[ch].capture_height)
			__max_height = videoInfo[ch].capture_height;
	}


	{
		int unitbuf_text_size = 1024;
		int unitbuf_size = (__max_width * __max_height * 2);
		// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, ((unitbuf_size / 4) + 1) * 4);
		__g_unitbuf 	 	= (unsigned char*)_mem_calloc2(1, ((unitbuf_size / 4) + 1) * 4);
		s_unit 				= (ST_ENCUNIT *) __g_unitbuf;

	}

#ifdef USE_VIDEO_POOL
	video_pool_init();
#endif


	EncodeInit(&encodeInfo);
	{
		restart:
		{
			open_new_channel(&encodeInfo);
			ret = DoEncode(&encodeInfo);

			if (g_encoder.bRestart) {
				g_encoder.th.state = STAT_UNREADY;
				goto restart;
			}
		}
	}
	EncodeClose(&encodeInfo);


#ifdef USE_VIDEO_POOL
	video_pool_deinit();
#endif

	{
		if (__g_unitbuf)
			_mem_free(__g_unitbuf);

		__g_unitbuf = NULL;
	}

cleanup:

	pthread_mutex_destroy(&g_encoder.mutex_saveunit);

	g_encoder.th.state = STAT_TERMINATED;
	flush_msg_queue(&g_encoder.th.msgq);


	pthread_exit(NULL);

}






