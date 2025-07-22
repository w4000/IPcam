
#ifndef ENCODER_CORE_HI3518A_H_
#define ENCODER_CORE_HI3518A_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "soc_common.h"
#include "dvr.h"

#include "enc_typedef.h"

#include "../../utils/utils.h"
#include "../../lib/msg.h"
#include "../../lib/datetime.h"
#include "../../lib/threadsafe.h"
#include "../../thread/thread.h"


#define	MAX_ENC_CHANNEL	CAM_MAX

enum {
	ENC_ERR_NONE = 0,

	ENC_ERR_USERERR = 1000,

	ENC_ERR_MAX
};

typedef struct {



	int					jpgForceEncode;
	int					jpgWapEncode;
	unsigned char*		jpgSaveBuf;
	unsigned int		jpgSaveBufLength, jpgSaveBufLength2;
	unsigned int		jpgSaveBufLength_forFTP;

	int					bIsSkipLoopAudio;
	unsigned char*		soundBuf;
	unsigned int		soundBufReturnSize;
	uint64				soundTimestamp;

	char*				szText;
	long				textSize;

	int					bIsSkipFrame[MAX_ENC_CHANNEL];

} ENC_OBJECT_INFO;


typedef struct encode_info {
	int						maxEncFrameCnt;
	int						maxCapFrameCnt;
	int						vsig;

	VIDEO_ENCODER_INFO* 	pVideoInfo[CAM_MAX];
	VIDEO_ENCODER_INFO 		vInfo_Variable[CAM_MAX];

	JPEG_ENCODER_INFO*  	pJpegInfo;
	DISPLAY_INFO*  			pDisplayInfo;

	int						bStartNewFlag;

	void*					pUserParam;

	ENC_OBJECT_INFO*		pObjInfo;

} ENCODE_INFO;

typedef struct
{
	_CAM_THREAD_COMMON	th;

	int 					bQuit;
	int 					bRestart;

	pthread_mutex_t 		mutex_saveunit;

	ENCODE_INFO*			pEncInfo;
	ENC_OBJECT_INFO* 		pObjInfo;

} ST_ENCODER;

extern ST_ENCODER g_encoder;

int EncodeInit(ENCODE_INFO* pEncodeInfo);
int DoEncode(ENCODE_INFO* pEncInfo);
int StopEncode(void);
int EncodeClose(ENCODE_INFO* pEncodeInfo);

int Encoder_DynamicSetParam(int ch, ENC_OBJECT_INFO* pObjInfo, VIDEO_ENCODER_INFO* pVideoInfo);
int Encoder_DynamicGOP(int ch, int gopX);


#define OSD_TEXT_LEN 64


typedef struct _RGN_BOXS
{
	int count;
	struct BOX_RECT {
		unsigned int x;
		unsigned int y;
		unsigned int width;
		unsigned int height;
		HI_U32 rgb_color;
	} rects[512];
} RGN_BOXS;

HI_S32 draw_rgn_box(HI_BOOL bShow, int x, int y, int width, int height, HI_U32 rgb_color);
HI_S32 draw_rgn_boxs(HI_BOOL bShow, RGN_BOXS *boxs);
HI_S32 draw_rgn_bitmap(HI_BOOL bShow, BITMAP_S *pstBitmap);
HI_S32 draw_osd_text(HI_BOOL bShow, int x, int y, char text[], MPP_CHN_S *pstChn);
HI_S32 draw_osd_text_venc(HI_BOOL bShow, int ch, int x, int y, char text[]);
HI_S32 draw_osd_text_vi(HI_BOOL bShow, int x, int y, char text[]);

void update_camera_setup(int update_type, int is_onvif_setup);
int get_osd_canvas_area_position();//w4000 add


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* ENCODER_CORE_HI3518A_H_ */
