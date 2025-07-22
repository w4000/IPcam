
#ifndef MAIN_ENCODER_H_
#define MAIN_ENCODER_H_

#include "common.h"
#include "soc_common.h"


/* Frame buffer for a pre-alarmed channel.   */
typedef struct st_prealrm_framebuf {
	pthread_mutex_t 	mutex;		/* Mutex lock for buf_head */
	struct clist 		buf_head;			/* Frame buffer header(list of st_frame). */
	volatile int 		numframe;			/* Current number frame buffer in the list. */
} ST_PREALRM_FRAMEBUF;

typedef struct
{
	//	User Parameters..
	int					nAliveCount;

	unsigned 			wsched [CAM_MAX];
	unsigned			chencmode [CAM_MAX];	// Current encoding mode(enum CHENCMODE) of each channel.
	unsigned			netmode [CAM_MAX];

	int 				recFlagbyMotion;


	int 						bIsTherePrealarmData;
	unsigned					prealrm_frms [CAM_MAX];
	struct st_prealrm_framebuf 	prealrm_frame_buf[CAM_MAX];	

} ENC_USER_PARAM;

extern ENC_USER_PARAM	g_EncUserParam;

#define CAMERA_ENCMODE(ch)		(g_EncUserParam.chencmode[ch])		
#define NET_ENCMODE(ch)			(g_EncUserParam.netmode[ch])

#define PREALRM_FRAME_BUF_CH(ch) (&g_EncUserParam.prealrm_frame_buf[(int)(ch)])

#define VBR_MAX_QP(q)  49 

void encoder_prealrm_init_framebuf(struct st_prealrm_framebuf buf[]);
int encoder_prealrm_release_all_frame(int ch);

void* main_encoder_thread(void* pParam);

int encode_before_start(void* pUserParam, ENC_OBJECT_INFO* pObjInfo);
int encode_after_video_encoded(int ch, PAYLOAD_TYPE_E enType, void* pUserParam, VENC_STREAM_S *pstStream);
int encode_after_stop(void* pUserParam, ENC_OBJECT_INFO* pObjInfo);
int encode_exit(void* pUserParam);
int encode_check_video_loop(void* pUserParam, ENC_OBJECT_INFO* pObjInfo);
int encode_motion_check(char MDBuff[15 + 1][16 + 1], HI_U64 u64Pts);
#endif /* MAIN_ENCODER_H_ */
