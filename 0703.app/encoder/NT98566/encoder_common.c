#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "hs_common.h"
#include "encoder/encoder.h"
#include "encoder/encoder_common.h"
#include "encoder/video_default.h"

#include "hdal.h"
#include "math.h"
#include "vendor_md.h"
#include "libmd/libmd.h"

#include "nv_define.h"
#include "nv_mem_pool.h"
#include "nv_isp.h"
#include "nv_gpio.h"
#include "nv_video_capture.h"
#include "nv_video_process.h"
#include "nv_video_encoder.h"
#include "nv_video_motion.h"

#include "osd.h"



#define VIDEOCAP_ALG_FUNC HD_VIDEOCAP_FUNC_AE | HD_VIDEOCAP_FUNC_AWB

#pragma pack(2)
struct BmpFileHeader {
    UINT16 bfType;
    UINT32 bfSize;
    UINT16 bfReserved1;
    UINT16 bfReserved2;
    UINT32 bfOffBits;
};
struct BmpInfoHeader {
    UINT32 biSize;
    UINT32 biWidth;
    UINT32 biHeight;
    UINT16 biPlanes; // 1=defeaul, 0=custom
    UINT16 biBitCount;
    UINT32 biCompression;
    UINT32 biSizeImage;
    UINT32 biXPelsPerMeter; // 72dpi=2835, 96dpi=3780
    UINT32 biYPelsPerMeter; // 120dpi=4724, 300dpi=11811
    UINT32 biClrUsed;
    UINT32 biClrImportant;
};
#pragma pack()


static pthread_t mot_thread_id;
static int g_endFlag = 0;

#define	OCCUR_TIME			100	// ms
#define	IGNORE_NUM			8
#define	MOTION_IGNORE_MAX	OCCUR_TIME*IGNORE_NUM




static NV_VIDEO_INFO g_video_info[3+2] = {0, };
static pthread_t venc_thread_id;
typedef struct _NV_ENCODE_THREAD_ARG {
	NV_VIDEO_INFO *video_info;
	ENCODE_INFO *encode_info;
} NV_ENCODE_THREAD_ARG;



enum {
	SET_UPDATE_ISP_IQ,
	SET_UPDATE_ISP_AE,
	SET_UPDATE_ISP_BLC,
	SET_UPDATE_ISP_AWB,
	SET_UPDATE_ISP_DN,
	SET_UPDATE_ISP_MAX
};



const char *err_handle_msg(HD_RESULT id)
{
	// TODO .. 
	return "";
}

///////////////////////////////////////////////////////////////////////////////
#define HD_VIDEOPROC_CFG                0x000f0000  //vprc
#define HD_VIDEOPROC_CFG_STRIP_MASK     0x00000007  //vprc stripe rule mask: (default 0)
#define HD_VIDEOPROC_CFG_STRIP_LV1      0x00000000  //vprc "0: cut w>1280, GDC =  on, 2D_LUT off after cut (LL slow)
#define HD_VIDEOPROC_CFG_STRIP_LV2      0x00010000  //vprc "1: cut w>2048, GDC = off, 2D_LUT off after cut (LL fast)
#define HD_VIDEOPROC_CFG_STRIP_LV3      0x00020000  //vprc "2: cut w>2688, GDC = off, 2D_LUT off after cut (LL middle)(2D_LUT best)
#define HD_VIDEOPROC_CFG_STRIP_LV4      0x00030000  //vprc "3: cut w> 720, GDC =  on, 2D_LUT off after cut (LL not allow)(GDC best)
#define HD_VIDEOPROC_CFG_DISABLE_GDC    HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_LL_FAST        HD_VIDEOPROC_CFG_STRIP_LV2
#define HD_VIDEOPROC_CFG_2DLUT_BEST     HD_VIDEOPROC_CFG_STRIP_LV3
#define HD_VIDEOPROC_CFG_GDC_BEST       HD_VIDEOPROC_CFG_STRIP_LV4
///////////////////////////////////////////////////////////////////////////////





static int g_shdr = 0;
ST_ENCODER g_encoder;



int get_cap_caps(HD_PATH_ID video_cap_ctrl, HD_VIDEOCAP_SYSCAPS *p_video_cap_syscaps)
{
	int ret = 0;

	ret = hd_videocap_get(video_cap_ctrl, HD_VIDEOCAP_PARAM_SYSCAPS, p_video_cap_syscaps);
	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_get(HD_VIDEOCAP_PARAM_SYSCAPS)!!! (ret=%d)\n", ret);
		goto Fail;
	}



	return 0;
Fail:
	return -1;
}



int set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, UINT32 max_bitrate, UINT32 isp_id, VIDEO_ENCODER_INFO *p_enc)
{
	int ret = HD_OK;
	HD_VIDEOENC_PATH_CONFIG video_path_config = {0};
	HD_VIDEOENC_FUNC_CONFIG video_func_config = {0};

	//--- HD_VIDEOENC_PARAM_PATH_CONFIG ---
	if(p_enc->codec == 0) {
		video_path_config.max_mem.codec_type      = HD_CODEC_TYPE_H264;
	}
	else if(p_enc->codec == 1) {
		video_path_config.max_mem.codec_type      = HD_CODEC_TYPE_H265;
	}
	else if(p_enc->codec == 2) {
		video_path_config.max_mem.codec_type      = HD_CODEC_TYPE_JPEG;
	}


	video_path_config.max_mem.max_dim.w       = p_max_dim->w;
	video_path_config.max_mem.max_dim.h       = p_max_dim->h;
	video_path_config.max_mem.bitrate         = max_bitrate + 1024;
	video_path_config.max_mem.enc_buf_ms      = 2500;
	video_path_config.max_mem.svc_layer       = HD_SVC_4X;
	video_path_config.max_mem.ltr             = TRUE;
	video_path_config.max_mem.rotate          = TRUE;
	video_path_config.max_mem.source_output   = FALSE;
	video_path_config.isp_id                  = isp_id;

	ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_PATH_CONFIG, &video_path_config);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoenc_set()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	//video_func_config.in_func |= HD_VIDEOENC_INFUNC_LOWLATENCY; //enable low-latency

	ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_FUNC_CONFIG, &video_func_config);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoenc_set()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}


int set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, VIDEO_ENCODER_INFO *p_enc)
{

	if(p_enc->videoBitRate <= 0) {
		printf("Invalid param videoBitRate=%d)\n", p_enc->videoBitRate);
		p_enc->videoBitRate = 200000;
		p_dim->w = 352;
		p_dim->h = 240;
	}

	int ret = HD_OK;
	HD_VIDEOENC_IN  video_in_param = {0};
	HD_VIDEOENC_OUT video_out_param = {0};
	HD_H26XENC_DEBLOCK deblock_param = {0};
	HD_H26XENC_RATE_CONTROL rc_param = {0};

	//--- HD_VIDEOENC_PARAM_IN ---
	video_in_param.dir     = HD_VIDEO_DIR_NONE;
	video_in_param.pxl_fmt = HD_VIDEO_PXLFMT_YUV420;
	video_in_param.dim.w   = p_dim->w;
	video_in_param.dim.h   = p_dim->h;




	video_in_param.frc     = HD_VIDEO_FRC_RATIO(p_enc->videoFrameRate, 30);
	ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_IN, &video_in_param);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoenc_set()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	if(p_enc->codec == 0) {
		//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
		video_out_param.codec_type         = HD_CODEC_TYPE_H264;
		video_out_param.h26x.profile       = HD_H264E_HIGH_PROFILE;
		video_out_param.h26x.level_idc     = HD_H264E_LEVEL_5_1;
		video_out_param.h26x.gop_num       = p_enc->groupOfPicture;
		video_out_param.h26x.ltr_interval  = 0;
		video_out_param.h26x.ltr_pre_ref   = 0;
		video_out_param.h26x.gray_en       = 0;
		video_out_param.h26x.source_output = 0;
		video_out_param.h26x.svc_layer     = HD_SVC_DISABLE;
		video_out_param.h26x.entropy_mode  = HD_H264E_CABAC_CODING;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoenc_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}

		//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
		rc_param.rc_mode             = (p_enc->bIsCBR)?HD_RC_MODE_CBR:HD_RC_MODE_VBR;
		if(rc_param.rc_mode == HD_RC_MODE_CBR) {
			rc_param.cbr.bitrate         = p_enc->videoBitRate;
			rc_param.cbr.frame_rate_base = p_enc->videoFrameRate;
			rc_param.cbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 10;
			rc_param.cbr.max_i_qp        = 51;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 10;
			rc_param.cbr.max_p_qp        = 51;
			rc_param.cbr.static_time     = 4;
			rc_param.cbr.ip_weight       = (p_dim->w > 2500)?-30:0;
		}
		else if(rc_param.rc_mode == HD_RC_MODE_VBR) {
			rc_param.vbr.bitrate         = p_enc->videoBitRate;
			rc_param.vbr.frame_rate_base = p_enc->videoFrameRate;
			rc_param.vbr.frame_rate_incr = 1;
			rc_param.cbr.init_i_qp       = 26;
			rc_param.cbr.min_i_qp        = 10;
			rc_param.cbr.max_i_qp        = 51;
			rc_param.cbr.init_p_qp       = 26;
			rc_param.cbr.min_p_qp        = 10;
			rc_param.cbr.max_p_qp        = 51;
			rc_param.vbr.static_time     = 4;
			rc_param.vbr.ip_weight       = (p_dim->w > 2500)?-30:0;
			rc_param.vbr.change_pos      = 0;
		}
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoenc_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}
	}
	else if(p_enc->codec == 1) {
		//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
		video_out_param.codec_type         = HD_CODEC_TYPE_H265;
		video_out_param.h26x.profile       = HD_H265E_MAIN_PROFILE;
		video_out_param.h26x.level_idc     = HD_H265E_LEVEL_5;
		video_out_param.h26x.gop_num       = p_enc->groupOfPicture;
		video_out_param.h26x.ltr_interval  = 0;
		video_out_param.h26x.ltr_pre_ref   = 0;
		video_out_param.h26x.gray_en       = 0;
		video_out_param.h26x.source_output = 0;
		video_out_param.h26x.svc_layer     = HD_SVC_DISABLE;
		video_out_param.h26x.entropy_mode  = HD_H265E_CABAC_CODING;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoenc_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}

		//--- HD_VIDEOENC_PARAM_OUT_RATE_CONTROL ---
		rc_param.rc_mode             = (p_enc->bIsCBR)?HD_RC_MODE_CBR:HD_RC_MODE_VBR;
		rc_param.cbr.bitrate         = p_enc->videoBitRate;
		rc_param.cbr.frame_rate_base = p_enc->videoFrameRate;
		rc_param.cbr.frame_rate_incr = 1;
		rc_param.cbr.init_i_qp       = 26;
		rc_param.cbr.min_i_qp        = 10;
		rc_param.cbr.max_i_qp        = 51;
		rc_param.cbr.init_p_qp       = 26;
		rc_param.cbr.min_p_qp        = 10;
		rc_param.cbr.max_p_qp        = 51;
		rc_param.cbr.static_time     = 4;
		rc_param.cbr.ip_weight       = (p_dim->w > 2500)?-30:0;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_RATE_CONTROL, &rc_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoenc_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}
	}
	else if(p_enc->codec == 2) {
		//--- HD_VIDEOENC_PARAM_OUT_ENC_PARAM ---
		video_out_param.codec_type         = HD_CODEC_TYPE_JPEG;
		video_out_param.jpeg.retstart_interval = 0;
		video_out_param.jpeg.image_quality = 50;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_ENC_PARAM, &video_out_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoenc_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}

	}
	else {
		printf("not support enc_type\r\n");
		goto Fail;
	}




	if(p_enc->codec == 0 || p_enc->codec == 1) {
		deblock_param.dis_ilf_idc = 0;
		deblock_param.db_alpha = 0;
		deblock_param.db_beta = 0;
		ret = hd_videoenc_set(video_enc_path, HD_VIDEOENC_PARAM_OUT_DEBLOCK, &deblock_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoenc_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}
	}

	return 0;
Fail:
	return -1;
}





int bc_reorgS1(UINT8* inputS, UINT8* outputS, UINT32 width, UINT32 height)
{
	int detect_count = 0;
	UINT32 i, j, count,size;
	count = 0;
	size = width * height;
	for(j = 0; j < MDBC_ALIGN(size,8)/8; j++) {
		UINT8 c = inputS[j];
        for(i = 0; i < 8; i++) {
			if(count < size) {
				outputS[count] = c & 0x1;
				c = c>>1;
				count++;
				if(outputS[count]) {
					detect_count++;
				}
			}
        }
    }

	return detect_count;
}

static void md_set_para_sensitivity(VENDOR_MD_PARAM *mdbc_parm, UINT32 sensitivity)
{

	mdbc_parm->MdmatchPara.lbsp_th    = 0x0;
	mdbc_parm->MdmatchPara.d_colour   = 8;
	mdbc_parm->MdmatchPara.r_colour   = 0x10;
	mdbc_parm->MdmatchPara.d_lbsp     = 3;
	mdbc_parm->MdmatchPara.r_lbsp     = 5;
	mdbc_parm->MdmatchPara.model_num  = 0x8;
	mdbc_parm->MdmatchPara.t_alpha    = 0x33;
	mdbc_parm->MdmatchPara.dw_shift   = 0x4;
	mdbc_parm->MdmatchPara.dlast_alpha= 0x28;
	mdbc_parm->MdmatchPara.min_match  = 2;
	mdbc_parm->MdmatchPara.dlt_alpha  = 0xa;
	mdbc_parm->MdmatchPara.dst_alpha  = 0x28;
	mdbc_parm->MdmatchPara.uv_thres   = 8;
	mdbc_parm->MdmatchPara.s_alpha    = 100;
	mdbc_parm->MdmatchPara.dbg_lumDiff= 0x0;
	mdbc_parm->MdmatchPara.dbg_lumDiff_en = 0x0;

	mdbc_parm->MorPara.mor_th0 	  = 0x4;
	mdbc_parm->MorPara.mor_th1    = 0x0;
	mdbc_parm->MorPara.mor_th2    = 0x8;
	mdbc_parm->MorPara.mor_th3    = 0x0;
	mdbc_parm->MorPara.mor_th_dil = 0x0;
	mdbc_parm->MorPara.mor_sel0   = 0x1;
	mdbc_parm->MorPara.mor_sel1   = 0x1;
	mdbc_parm->MorPara.mor_sel2   = 0x1;
	mdbc_parm->MorPara.mor_sel3   = 0x0;

	mdbc_parm->UpdPara.minT           = 0x10;
	mdbc_parm->UpdPara.maxT           = 0x80;
	mdbc_parm->UpdPara.maxFgFrm       = 0x0;
	mdbc_parm->UpdPara.deghost_dth    = 0xf;
	mdbc_parm->UpdPara.deghost_sth    = 0xf0;
	mdbc_parm->UpdPara.stable_frm     = 0x78;
	mdbc_parm->UpdPara.update_dyn     = 0x80;
	mdbc_parm->UpdPara.va_distth      = 32;
	mdbc_parm->UpdPara.t_distth       = 24;
	mdbc_parm->UpdPara.dbg_frmID      = 0x0;
	mdbc_parm->UpdPara.dbg_frmID_en   = 0x0;
	mdbc_parm->UpdPara.dbg_rnd        = 0x0;
	mdbc_parm->UpdPara.dbg_rnd_en     = 0x0;
}

static HD_RESULT md_set_para(NV_MEM_RANGE *p_share_mem, UINT32 ping_pong_id, UINT32 mode, UINT32 sensi)
{
	VENDOR_MD_PARAM mdbc_parm;
	HD_RESULT ret = HD_OK;

	mdbc_parm.mode = mode;
	mdbc_parm.controlEn.update_nei_en = 1;
	mdbc_parm.controlEn.deghost_en    = 1;
	mdbc_parm.controlEn.roi_en0       = 0;
	mdbc_parm.controlEn.roi_en1       = 0;
	mdbc_parm.controlEn.roi_en2       = 0;
	mdbc_parm.controlEn.roi_en3       = 0;
	mdbc_parm.controlEn.roi_en4       = 0;
	mdbc_parm.controlEn.roi_en5       = 0;
	mdbc_parm.controlEn.roi_en6       = 0;
	mdbc_parm.controlEn.roi_en7       = 0;
	mdbc_parm.controlEn.chksum_en     = 0;
	mdbc_parm.controlEn.bgmw_save_bw_en = 1;
	mdbc_parm.controlEn.bc_y_only_en 	= 0;


	mdbc_parm.InInfo.uiInAddr0 = p_share_mem[0].addr;
	mdbc_parm.InInfo.uiInAddr1 = p_share_mem[1].addr;
	mdbc_parm.InInfo.uiInAddr2 = p_share_mem[2].addr;
	mdbc_parm.InInfo.uiInAddr3 = p_share_mem[3].addr;
	mdbc_parm.InInfo.uiInAddr4 = p_share_mem[4].addr;
	mdbc_parm.InInfo.uiInAddr5 = p_share_mem[5].addr;
	mdbc_parm.OutInfo.uiOutAddr0 = p_share_mem[6].addr;
	mdbc_parm.OutInfo.uiOutAddr1 = p_share_mem[3].addr;
	mdbc_parm.OutInfo.uiOutAddr2 = p_share_mem[4].addr;
	mdbc_parm.OutInfo.uiOutAddr3 = p_share_mem[5].addr;

	mdbc_parm.uiLLAddr          = 0x0;
	mdbc_parm.InInfo.uiLofs0    = MDBC_ALIGN(MD_IMG_WIDTH,4);//160;
	mdbc_parm.InInfo.uiLofs1    = MDBC_ALIGN(MD_IMG_WIDTH,4);//160;
	mdbc_parm.Size.uiMdbcWidth  = MD_IMG_WIDTH;
	mdbc_parm.Size.uiMdbcHeight = MD_IMG_HEIGHT;

	md_set_para_sensitivity(&mdbc_parm, sensi);

	ret = vendor_md_set(VENDOR_MD_PARAM_ALL, &mdbc_parm);
	if(HD_OK != ret) {
		printf("Failed to call vendor_md_set()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}





static int md2buff(char *va, char MDBuff[15 + 1][16 + 1])
{
	int i = 0;
	int x, y;
	int h, v;
	int ux = (MD_IMG_WIDTH/16);	// 320/16 = 20
	int uy = (MD_IMG_HEIGHT/15);	// 240/15 = 16

	for(y = 0; y < MD_IMG_HEIGHT; y++) {
		for(x = 0; x < MD_IMG_WIDTH; x++) {
			h = (x == 0)?0:x/ux;
			v = (y == 0)?0:y/uy;
			if(va[i++] || MDBuff[v][h]) {
				MDBuff[v][h] = 1;
			}
			else {
				MDBuff[v][h] = 0;
			}
		}
	}
	return 0;
}



static int md2buff2(char *va, char MDBuff[15 + 1][16 + 1], void *arg)
{
	int i = 0;
	int x, y;
	int h, v;
	int ux = (MD_IMG_WIDTH/16);
	int uy = (MD_IMG_HEIGHT/15);

	for(y = 0; y < MD_IMG_HEIGHT; y++) {
		for(x = 0; x < MD_IMG_WIDTH; x++) {
			h = (x == 0)?0:x/ux;
			v = (y == 0)?0:y/uy;
			if(va[i++] || MDBuff[v][h]) {
				MDBuff[v][h] = 1;
			}
			else {
				MDBuff[v][h] = 0;
			}
		}
	}
	return 0;
}

static int check_setup(int *sensitivity)
{
	if(*sensitivity != g_setup.cam[0].sens) {
		*sensitivity = g_setup.cam[0].sens;
		return 1;
	}
	return 0;
}

static int getMDThreshold(int sensitivity) {

	if(sensitivity == 100) return 0;
	if(sensitivity < 30) return 5;

	return 1;
}

int get_obj_size_th(int sensitivity)
{
	int factor = 200;//400;
	if(sensitivity <= 10)		factor = 100;//80;//100;
	else if(sensitivity <= 20) 	factor = 200;//100;//200;
	else if(sensitivity <= 40) 	factor = 300;//150;//300;
	else if(sensitivity <= 40) 	factor = 400;//150;//300;
	else if(sensitivity <= 50) 	factor = 500;
	else if(sensitivity <= 60) 	factor = 550;//200;//400;
	else if(sensitivity <= 80) 	factor = 600;//400;//500;
	else if(sensitivity <= 90) 	factor = 650;//500;//600;
	else factor = 700;


	return MD_IMG_WIDTH * MD_IMG_HEIGHT / factor;
}





#if 0
#define VIDEOPROC_ALG_FUNC \
	HD_VIDEOPROC_FUNC_DEFOG \
	| HD_VIDEOPROC_FUNC_WDR \
	| HD_VIDEOPROC_FUNC_3DNR \
	| HD_VIDEOPROC_FUNC_3DNR_STA \
	| HD_VIDEOPROC_FUNC_COLORNR \
	| HD_VIDEOPROC_FUNC_AF
	/*
	| HD_VIDEOPROC_FUNC_DI \
	| HD_VIDEOPROC_FUNC_SHDR \
	| HD_VIDEOPROC_FUNC_MOSAIC \
	| HD_VIDEOPROC_FUNC_SHARP \
	 */



 

#else
#define VIDEOPROC_ALG_FUNC HD_VIDEOPROC_FUNC_WDR | HD_VIDEOPROC_FUNC_COLORNR | HD_VIDEOPROC_FUNC_3DNR | HD_VIDEOPROC_FUNC_3DNR_STA
#endif

int set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_OUT_ID _out_id, int shdr, HD_PATH_ID ref_path_3dnr)
{
	int ret = HD_OK;
	HD_VIDEOPROC_SYSCAPS video_proc_syscaps = {0};
	HD_VIDEOPROC_DEV_CONFIG video_cfg_param = {0};
	HD_VIDEOPROC_CTRL video_ctrl_param = {0};
	HD_PATH_ID video_proc_ctrl = 0;
	HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

	ret = hd_videoproc_open(0, _out_id, &video_proc_ctrl); //open this for device control
	if(ret != HD_OK) {
		printf("Failed to call hd_videoproc_open()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	ret = hd_videoproc_get(video_proc_ctrl, HD_VIDEOPROC_PARAM_SYSCAPS, &video_proc_syscaps);
	if(ret == HD_OK) {
	}

	if(p_max_dim != NULL ) {
		video_cfg_param.pipe = HD_VIDEOPROC_PIPE_RAWALL;
		if((HD_CTRL_ID)_out_id == HD_VIDEOPROC_0_CTRL) {
			video_cfg_param.isp_id = 0;
		} else {
			video_cfg_param.isp_id = 1;
		}

		video_cfg_param.ctrl_max.func = VIDEOPROC_ALG_FUNC;
		video_cfg_param.in_max.pxlfmt = CAP_OUT_FMT;


		video_cfg_param.in_max.func = 0;
		video_cfg_param.in_max.dim.w = p_max_dim->w;
		video_cfg_param.in_max.dim.h = p_max_dim->h;
		video_cfg_param.in_max.frc = HD_VIDEO_FRC_RATIO(1,1);

		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_DEV_CONFIG, &video_cfg_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_open()!!! (ret=%d)\n", ret);
			goto Fail;
		}
	}


	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.in_func = 0;
		video_path_param.in_func |= HD_VIDEOPROC_INFUNC_DIRECT; //direct NOTE: enable direct
		ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}



	video_ctrl_param.func = VIDEOPROC_ALG_FUNC;
	
	
	if(shdr) {
		video_ctrl_param.func |= HD_VIDEOPROC_FUNC_SHDR;
	}
	video_ctrl_param.ref_path_3dnr = ref_path_3dnr;
	ret = hd_videoproc_set(video_proc_ctrl, HD_VIDEOPROC_PARAM_CTRL, &video_ctrl_param);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoproc_set()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	*p_video_proc_ctrl = video_proc_ctrl;

	return 0;
Fail:
	return -1;
}

int set_proc_in_param(HD_PATH_ID video_proc_path, HD_DIM *p_dim)
{
	int ret;

	HD_VIDEOPROC_IN video_in_param = {0};
	ret = hd_videoproc_get(video_proc_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoproc_get()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	video_in_param.dim.w = p_dim->w;
	video_in_param.dim.h = p_dim->h;

	ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_IN, &video_in_param);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoproc_set()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, int fps, HD_VIDEO_PXLFMT pxlfmt, int f, int pullout)
{
	int ret;

	{
		if(p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
			HD_VIDEOPROC_OUT video_out_param = {0};
			video_out_param.func = 0;
			video_out_param.dim.w = p_dim->w;
			video_out_param.dim.h = p_dim->h;
			video_out_param.pxlfmt = pxlfmt;
			video_out_param.dir = HD_VIDEO_DIR_NONE;
			video_out_param.frc = HD_VIDEO_FRC_RATIO(fps,30);
			video_out_param.depth = pullout;	// set > 0 to allow pull out (nn)

			// ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_EX, &video_out_param);
			ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);


			if(ret != HD_OK) {
				printf("Failed to call hd_videoproc_set()!!! (ret=%d)\n", ret);
				goto Fail;
			}
		}
		else {
			HD_VIDEOPROC_OUT video_out_param = {0};
			video_out_param.func = 0;
			video_out_param.dim.w = 0;
			video_out_param.dim.h = 0;
			video_out_param.pxlfmt = 0;
			video_out_param.dir = HD_VIDEO_DIR_NONE;
			video_out_param.frc = HD_VIDEO_FRC_RATIO(fps,30);
			video_out_param.depth = pullout;	// set > 0 to allow pull out (nn)
			ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT, &video_out_param);
			if(ret != HD_OK) {
				printf("Failed to call hd_videoproc_set()!!! (ret=%d)\n", ret);
				goto Fail;
			}
		}
	}


	return 0;
Fail:
	return -1;
}



int set_proc_param2(HD_PATH_ID video_proc_path, HD_DIM* p_dim, int fps, HD_VIDEO_PXLFMT pxlfmt, int f, int pullout)
{
	int ret;

#if 0
#endif
	{
		if(p_dim != NULL) { //if videoproc is already binding to dest module, not require to setting this!
			HD_VIDEOPROC_OUT_EX video_out_param = {0};
			video_out_param.src_path = HD_VIDEOPROC_0_OUT_0;
			video_out_param.dim.w = p_dim->w;
			video_out_param.dim.h = p_dim->h;
			video_out_param.pxlfmt = pxlfmt;
			video_out_param.dir = HD_VIDEO_DIR_NONE;
			video_out_param.frc = HD_VIDEO_FRC_RATIO(fps,30);
			video_out_param.depth = pullout;	// set > 0 to allow pull out (nn)
			ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_EX, &video_out_param);

			if(ret != HD_OK) {
				printf("Failed to call hd_videoproc_set()!!! (ret=%d)\n", ret);
				goto Fail;
			}
		}
		else {
			HD_VIDEOPROC_OUT_EX video_out_param = {0};
			video_out_param.src_path = HD_VIDEOPROC_0_OUT_0;
			video_out_param.dim.w = 0;
			video_out_param.dim.h = 0;
			video_out_param.pxlfmt = 0;
			video_out_param.dir = HD_VIDEO_DIR_NONE;
			video_out_param.frc = HD_VIDEO_FRC_RATIO(fps,30);
			video_out_param.depth = pullout;	// set > 0 to allow pull out (nn)
			ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_OUT_EX, &video_out_param);
			if(ret != HD_OK) {
				printf("Failed to call hd_videoproc_set()!!! (ret=%d)\n", ret);
				goto Fail;
			}
		}
	}



	{
		HD_VIDEOPROC_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func |= HD_VIDEOPROC_OUTFUNC_MD;
		ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_FUNC_CONFIG, &video_path_param);
	}



	return 0;
Fail:
	return -1;
}


int set_proc_ratio(HD_PATH_ID video_proc_ctrl, HD_PATH_ID video_proc_path, int on, HD_DIM *dim)
{
	int ret;

	HD_VIDEOPROC_CROP video_crop = {0};
	video_crop.mode = (on)?HD_CROP_ON:HD_CROP_OFF;

	ret = hd_videoproc_set(video_proc_path, HD_VIDEOPROC_PARAM_IN_CROP, &video_crop);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoproc_set(HD_VIDEOPROC_PARAM_IN_CROP)!!! (ret=%d)\n", ret);
		goto Fail;
	}

	return (video_crop.mode == HD_CROP_ON)?1:0;
Fail:
	return -1;
}



static int init_video_module()
{
	int ret;

	ret = hd_videocap_init();
	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_init()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	ret = hd_videoproc_init();
	if(ret != HD_OK) {
		printf("Failed to call hd_videoproc_init()!!! (ret=%d)\n", ret);
		goto Fail;
	}


	ret = hd_videoenc_init();
	if(ret != HD_OK) {
		printf("Failed to call hd_videoenc_init()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}



int nv_query_osg_buf_size(unsigned int width, unsigned int height)
{
	HD_VIDEO_FRAME frame = {0};
	int stamp_size;

	frame.sign = MAKEFOURCC('O','S','G','P');
	frame.dim.w = width;
	frame.dim.h = height;
	frame.pxlfmt = HD_VIDEO_PXLFMT_ARGB4444;

	//get required buffer size for a single image
	stamp_size = hd_common_mem_calc_buf_size(&frame);
	if(!stamp_size) {
		printf("Failed to call hd_common_mem_calc_buf_size()!!! (stamp_size=%d)\n", stamp_size);
		return -1;
	}

	//ping pong buffer needs double size
	// stamp_size *= 2;

	return stamp_size;
}

int nv_mem_pool_init(unsigned int width, unsigned int height, unsigned int stamp_size)
{
	int ch;
	int pool_index = 0;
	int ret;
	HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

	// config common pool (cap), dram mode
	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_COMMON_POOL;

	mem_cfg.pool_info[pool_index].blk_size = DBGINFO_BUFSIZE() + \
									VDO_RAW_BUFSIZE(width, height, CAP_OUT_FMT) + \
									VDO_CA_BUF_SIZE(CA_WIN_NUM_W, CA_WIN_NUM_H) + \
									VDO_LA_BUF_SIZE(LA_WIN_NUM_W, LA_WIN_NUM_H) + \
									VDO_VA_BUF_SIZE(VA_WIN_NUM_W, VA_WIN_NUM_H);

	mem_cfg.pool_info[pool_index].blk_cnt = 2;



	mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
	pool_index++;

	// config common pool
	for(ch = 0; ch < 3; ch++) {
		width = def_resol_index_to_width(0, def_max_resol(ch));
		height = def_resol_index_to_height(0, def_max_resol(ch));
		mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[pool_index].blk_size = DBGINFO_BUFSIZE() + \
										VDO_YUV_BUFSIZE(width, height, HD_VIDEO_PXLFMT_YUV420);


#if 1
		// mem_cfg.pool_info[pool_index].blk_cnt = 4;
		// mem_cfg.pool_info[pool_index].blk_cnt = 2;

		// if (ch==0)
		// 	mem_cfg.pool_info[pool_index].blk_cnt = 5;
		// else
		// 	mem_cfg.pool_info[pool_index].blk_cnt = 3;

		mem_cfg.pool_info[pool_index].blk_cnt = 3;
#else
#endif
		mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
		pool_index++;
	}


#if 0	
	// config common pool (ref)
	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_COMMON_POOL;
		mem_cfg.pool_info[pool_index].blk_size = DBGINFO_BUFSIZE()+VDO_YUV_BUFSIZE(width, height, HD_VIDEO_PXLFMT_YUV420);
		mem_cfg.pool_info[pool_index].blk_cnt = 1; //no bind and no pull, just need 1
		mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
		mem_cfg.pool_info[pool_index].blk_cnt += 1;
		pool_index++;
#endif

#if 1
	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[pool_index].blk_size = DBGINFO_BUFSIZE() + \
										VDO_YUV_BUFSIZE(MD_IMG_WIDTH, MD_IMG_WIDTH, HD_VIDEO_PXLFMT_YUV420);
										// VDO_YUV_BUFSIZE(MD_IMG_WIDTH, MD_IMG_WIDTH, HD_VIDEO_PXLFMT_YUV400);
#else
	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_COMMON_POOL;
	mem_cfg.pool_info[pool_index].blk_size = DBGINFO_BUFSIZE() + \
										VDO_YUV_BUFSIZE(MD_IMG_WIDTH, MD_IMG_WIDTH, HD_VIDEO_PXLFMT_YUV420);
										// VDO_YUV_BUFSIZE(MD_IMG_WIDTH, MD_IMG_WIDTH, HD_VIDEO_PXLFMT_YUV420);
#endif


	mem_cfg.pool_info[pool_index].blk_cnt = 2;
	mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
	pool_index++;

	if(stamp_size > 0) {
		// config common pool (osg)
		mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_OSG_POOL;
		mem_cfg.pool_info[pool_index].blk_size = stamp_size;
		mem_cfg.pool_info[pool_index].blk_cnt = 3;
		mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
		pool_index++;
	}

	// md pool
#if 1

	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_GLOBAL_MD_POOL;
	mem_cfg.pool_info[pool_index].blk_size = MD_IMG_BUF_SIZE;	// 320 * 240
	mem_cfg.pool_info[pool_index].blk_cnt = 2;
	mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
	pool_index++;
	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_GLOBAL_MD_POOL;
	mem_cfg.pool_info[pool_index].blk_size = MD_IMG_BUF_SIZE/2;
	mem_cfg.pool_info[pool_index].blk_cnt = 2;
	mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
	pool_index++;
	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_GLOBAL_MD_POOL;
	mem_cfg.pool_info[pool_index].blk_size = MD_IMG_BUF_SIZE*40;
	mem_cfg.pool_info[pool_index].blk_cnt = 2;
	mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
	pool_index++;
	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_GLOBAL_MD_POOL;
	mem_cfg.pool_info[pool_index].blk_size = MD_IMG_BUF_SIZE*6;
	mem_cfg.pool_info[pool_index].blk_cnt = 2;
	mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
	pool_index++;
	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_GLOBAL_MD_POOL;
	mem_cfg.pool_info[pool_index].blk_size = ((MD_IMG_BUF_SIZE+15)/16)*16*12;
	mem_cfg.pool_info[pool_index].blk_cnt = 2;
	mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
	pool_index++;
	mem_cfg.pool_info[pool_index].type = HD_COMMON_MEM_GLOBAL_MD_POOL;
	mem_cfg.pool_info[pool_index].blk_size = ((MD_IMG_BUF_SIZE+7)/8);
	mem_cfg.pool_info[pool_index].blk_cnt = 1;
	mem_cfg.pool_info[pool_index].ddr_id = DDR_ID0;
	pool_index++;

#else
#endif




	ret = hd_common_mem_init(&mem_cfg);
	if(ret != HD_OK) {
		printf("Failed to call hd_common_mem_init()!!! (ret=%d)\n", ret);
		goto Fail;
	}

#if 1
	//system("cat /proc/hdal/comm/info");
	// sleep(3);
#endif

	return 0;

Fail:
	return -1;
}

int nv_mem_pool_exit()
{
	int ret;
	ret = hd_common_mem_uninit();
	if(ret != HD_OK) {
		printf("Failed to call hd_common_mem_uninit()!!! (ret=%d)\n", ret);
		return -1;
	}
	return 0;
}

unsigned int nv_mem_alloc(UINT32 stamp_size, UINT32 *stamp_blk, UINT32 *stamp_pa)
{
	UINT32                  pa;
	HD_COMMON_MEM_VB_BLK    blk;

	if(!stamp_size) {
		printf("stamp_size is unknown!!!\n");
		return -1;
	}

	//get osd stamps' block
	blk = hd_common_mem_get_block(HD_COMMON_MEM_OSG_POOL, stamp_size, DDR_ID0);
	if(blk == HD_COMMON_MEM_VB_INVALID_BLK) {
		printf("get block fail!!!\n");
		return -1;
	}

	if(stamp_blk)
		*stamp_blk = blk;

	//translate stamp block to physical address
	pa = hd_common_mem_blk2pa(blk);
	if(pa == 0) {
		printf("blk2pa fail, blk = 0x%x\n", blk);
		return -1;
	}

	if(stamp_pa)
		*stamp_pa = pa;

	return 0;
}


static int open_video_module(NV_VIDEO_RECORD **p_stream, HD_DIM* p_proc_max_dim, int proc_ch_count)
{
	int i;
	int ret;

	// set videocap config
	ret = set_cap_cfg(&p_stream[0]->cap_ctrl, g_shdr);
	if(ret != HD_OK) {
		printf("Failed to call set_cap_cfg()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	for(i = 0; i < proc_ch_count; i++) {
		ret = set_proc_cfg(&p_stream[i]->proc_ctrl, &p_proc_max_dim[i], HD_VIDEOPROC_0_CTRL, g_shdr, (proc_ch_count > (3 + 1))?HD_VIDEOPROC_0_OUT_4:HD_VIDEOPROC_0_OUT_0);
		if(ret != HD_OK) {
			printf("Failed to call set_proc_cfg()!!! (ret=%d)\n", ret);
			goto Fail;
		}
	}
	// ret = set_proc_cfg(&p_stream[3]->proc_ctrl, &p_proc_max_dim[3], HD_VIDEOPROC_0_CTRL, g_shdr, HD_VIDEOPROC_0_OUT_3);

	ret = hd_videocap_open(HD_VIDEOCAP_0_IN_0, HD_VIDEOCAP_0_OUT_0, &p_stream[0]->cap_path);
	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_open()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	for(i = 0; i < proc_ch_count; i++) {
		if (i==4)
			ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_OUT(0, 4), &p_stream[i]->proc_path);
		else if (i==3)
			ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_OUT(0, 5), &p_stream[i]->proc_path);
		else
			ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_VIDEOPROC_OUT(0, i), &p_stream[i]->proc_path);

		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_open()!!!	(ret=%d)\n", ret);
			goto Fail;
		}
	}


	for(i = 0; i < 3; i++) {
	// for(i = 0; i < proc_ch_count; i++) {

		if (i==4)
			ret = hd_videoenc_open(HD_VIDEOENC_IN(0, i), HD_VIDEOENC_OUT(0, 4), &p_stream[i]->enc_path);
		else if (i==3)
			ret = hd_videoenc_open(HD_VIDEOENC_IN(0, i), HD_VIDEOENC_OUT(0, 6), &p_stream[i]->enc_path);
		else
			ret = hd_videoenc_open(HD_VIDEOENC_IN(0, i), HD_VIDEOENC_OUT(0, i), &p_stream[i]->enc_path);
		if(ret != HD_OK) {
			printf("Failed to call hd_videocap_open()!!! (ret=%d)\n", ret);
			goto Fail;
		}
	}

	for(i = 0;i < 3;i++) {
		ret = hd_videoproc_open(HD_VIDEOPROC_0_IN_0, HD_STAMP(i), &p_stream[0]->vp_stamp_path[i]);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_open()!!! (ret=%d;i = %d)\n", ret, i);
			goto Fail;
		}
	}

	//w4000 gpio_init();

	return 0;
Fail:
	return -1;
}

static int close_video_module(int ch, NV_VIDEO_RECORD *p_stream)
{
	int ret;
	int i;

	if(ch == 0) {
		ret = hd_videocap_close(p_stream->cap_path);
		if(ret != HD_OK) {
			printf("Failed to call hd_videocap_close()!!! (ret=%d)\n", ret);
			goto Fail;
		}


		for(i = 0;i < 3;i++) {
			ret = hd_videoproc_close(p_stream->vp_stamp_path[i]);
			if(ret != HD_OK) {
				printf("Failed to call hd_videoproc_close()!!! (ret=%d;i = %d)\n", ret, i);
				goto Fail;
			}
		}
	}

	ret = hd_videoproc_close(p_stream->proc_path);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoproc_close()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	ret = hd_videoenc_close(p_stream->enc_path);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoenc_close()!!! (ret=%d)\n", ret);
		goto Fail;
	}
	return 0;
Fail:
	return -1;
}

static int exit_video_module(void)
{
	int ret;

	ret = hd_videocap_uninit();
	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_uninit()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	ret = hd_videoproc_uninit();
	if(ret != HD_OK) {
		printf("Failed to call hd_videoproc_uninit()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	ret = hd_videoenc_uninit();
	if(ret != HD_OK) {
		printf("Failed to call hd_videoenc_uninit()!!! (ret=%d)\n", ret);
		goto Fail;
	}
	return 0;
Fail:
	return -1;
}

static int refresh_video_info(int id, NV_VIDEO_INFO *p_video_info)
{
	

	//while to get vsp, sps, pps
	int ret;
	p_video_info->codec_type = PT_BUTT;
	p_video_info->vps_size = p_video_info->sps_size = p_video_info->pps_size = 0;
	memset(p_video_info->vps, 0, sizeof(p_video_info->vps));
	memset(p_video_info->sps, 0, sizeof(p_video_info->sps));
	memset(p_video_info->pps, 0, sizeof(p_video_info->pps));
	while((ret = hd_videoenc_pull_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull, 500)) == 0) {
		if(p_video_info->data_pull.vcodec_format == HD_CODEC_TYPE_JPEG) {
			p_video_info->codec_type = PT_MJPEG;
			hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
			break;
		}
		else if (p_video_info->data_pull.vcodec_format == HD_CODEC_TYPE_H264) {
			p_video_info->codec_type = PT_H264;
			if(p_video_info->data_pull.pack_num != 3) {
				hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
				continue;
			}


			p_video_info->sps_size = p_video_info->data_pull.video_pack[0].size;
			p_video_info->pps_size = p_video_info->data_pull.video_pack[1].size;
			hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
			break;
		}
		else if(p_video_info->data_pull.vcodec_format == HD_CODEC_TYPE_H265) {
			p_video_info->codec_type = PT_H265;
			if(p_video_info->data_pull.pack_num != 4) {
				hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
				continue;
			}


			p_video_info->vps_size = p_video_info->data_pull.video_pack[0].size;
			p_video_info->sps_size = p_video_info->data_pull.video_pack[1].size;
			p_video_info->pps_size = p_video_info->data_pull.video_pack[2].size;
			hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
			break;
		}
		else {
			hd_videoenc_release_out_buf(p_video_info->p_stream->enc_path, &p_video_info->data_pull);
		}
	}
	return 0;
}

static int start_audio(ENCODE_INFO* pEncodeInfo)
{
	return 0;
}

static int stop_audio(ENCODE_INFO* pEncodeInfo)
{
	AUDIO_AiAenc_Close(pEncodeInfo, NULL);
	return 0;
}

static int conv_osd_position(int pos, int resol, int max)
{
	return (resol * pos) / max;
}
//static 
int get_osd_canvas_area_position()//w4000 add
{
	/*printf("1 canvas_area_position(%d)\n", canvas_area_position[OSD_1ST_AREA][0]);
	printf("1 canvas_area_position(%d)\n", canvas_area_position[OSD_1ST_AREA][1]);
	printf("2 canvas_area_position(%d)\n", canvas_area_position[OSD_2ND_AREA][0]);
	printf("2 canvas_area_position(%d)\n", canvas_area_position[OSD_2ND_AREA][1]);
*/
}

static int set_osd_position()
{
	OSD_ITEM *osd_item = &g_setup_ext.osd.osd_item[OSD_DATETIME];
	int x = conv_osd_position(osd_item->pos_x, 1920, 10000);
	int y = conv_osd_position(osd_item->pos_y, 1080, 10000);
	set_proc_stamp_position(g_video_info[0].p_stream->vp_stamp_path[0], x, y);
	canvas_area_position[OSD_1ST_AREA][0] = x;
	canvas_area_position[OSD_1ST_AREA][1] = y;

	osd_item = &g_setup_ext.osd.osd_item[OSD_TITLE];
	x = conv_osd_position(osd_item->pos_x, 1920, 10000);
	y = conv_osd_position(osd_item->pos_y, 1080, 10000);
	set_proc_stamp_position(g_video_info[0].p_stream->vp_stamp_path[1], x, y);
	canvas_area_position[OSD_2ND_AREA][0] = x;
	canvas_area_position[OSD_2ND_AREA][1] = y;
	get_osd_canvas_area_position();//w4000
}

static int apply_configuration(NV_VIDEO_RECORD *stream)
{


	return 0;
}

static int copy_canvas_data(int index, VFB_CANVAS_INFO* dest, NV_VIDEO_RECORD* src)
{
	dest->stamp.vp_stamp_path = src->vp_stamp_path[index];
	dest->stamp.stamp_pa = src->stamp_pa[index];
	dest->stamp.stamp_size = src->stamp_size[index];

	return 0;
}


HI_S32 DoEncode(ENCODE_INFO* pEncodeInfo)//w4000_default
{
	int err_line=0;
	int i;
	int ret = HD_OK;
	int need_proc_ch4 = 0;
	NV_VIDEO_RECORD *stream[3+2];
	NV_MEM_RANGE share_mem[11];
	HD_DIM stream_max_dim[] = {
		{ 1920, 1080 },
		{ 1280, 720},
		{  320, 240},
		{ 1920, 1080 },
		{ 1920, 1080 }
	};
	
	for(i = 0; i < 3+2; i++) {
		stream[i] = (NV_VIDEO_RECORD *)malloc(sizeof(NV_VIDEO_RECORD));
		memset(stream[i], 0, sizeof(NV_VIDEO_RECORD));
	}

	ENC_OBJECT_INFO objInfo;
	memset(&objInfo, 0, sizeof(ENC_OBJECT_INFO));
	pEncodeInfo->pObjInfo = &objInfo;
	g_encoder.pObjInfo = &objInfo;

	g_shdr = 0;

	ret = hd_common_init(0);
	if(ret != HD_OK) {
		printf("Failed to call hd_common_init()!!! (ret=%d)\n", ret);
		
		goto End;
	}


	
	hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_STRIP_LV1, 0);
	// hd_common_sysconfig(HD_VIDEOPROC_CFG, 0, HD_VIDEOPROC_CFG_STRIP_LV1, 0);


	HD_DIM dimension[3] = {{ 640, 270 }, { 640, 270 }, { 640, 270 }};
	

	for(i = 0; i < 3; i++) {
		dimension[i].w = (dimension[i].w >> 2) << 2;
		dimension[i].h = (dimension[i].h >> 2) << 2;
		stream[0]->stamp_size[i] = nv_query_osg_buf_size(dimension[i].w, dimension[i].h);
		stream[0]->img_buf[i] = (unsigned short*)calloc(dimension[i].w * dimension[i].h, sizeof(unsigned short));
		if(stream[0]->img_buf[i] == NULL){
			printf("Failed to stamp image malloc()!!! (index = %d)\n", i);
			
			goto End;
		}
	}

	
	ret = nv_mem_pool_init(1920, 1080, stream[0]->stamp_size[0]);
	if(ret != HD_OK) {
		printf("Failed to call nv_mem_pool_init()!!! (ret=%d)\n", ret);
		
		goto End;
	}
	
	for(i = 0;i < 3;i++) {
		ret = nv_mem_alloc(stream[0]->stamp_size[i], &(stream[0]->stamp_blk[i]), &(stream[0]->stamp_pa[i]));
		if(ret) {
			printf("Failed to call nv_mem_alloc()!!! (ret=%d;i = %d)\n", ret, i);
			
			goto End;
		}
	}

	
	ret = nv_share_memory_init(share_mem);
	if (ret != HD_OK) {
		printf("Failed to call nv_share_memory_init()!!! (ret=%d)\n", ret);
		
		goto End;
	}
	
	ret = init_video_module();
	if(ret != HD_OK) {
		printf("Failed to call init_video_module()!!! (ret=%d)\n", ret);
		
		goto End;
	}
	

	if(stream_max_dim[0].w != pEncodeInfo->pVideoInfo[0]->capture_width
			|| stream_max_dim[0].h != pEncodeInfo->pVideoInfo[0]->capture_height) {
		need_proc_ch4 = 1;
	}


	ret = open_video_module(stream, stream_max_dim, 3 + 1 + need_proc_ch4);
	if(ret != HD_OK) {
		printf("Failed to call open_video_module()!!! (ret=%d)\n", ret);
		
		goto End;
	}
	
	ret = get_cap_caps(stream[0]->cap_ctrl, &stream[0]->cap_syscaps);


	
	if(ret != HD_OK) {
		printf("Failed to call get_cap_caps()!!! (ret=%d)\n", ret);
		
		goto End;
	}
	


	stream[0]->cap_dim.w = stream_max_dim[0].w;
	stream[0]->cap_dim.h = stream_max_dim[0].h;
	ret = set_cap_param(stream[0]->cap_path, &stream[0]->cap_dim, 0, g_shdr);
	if(ret != HD_OK) {
		printf("Failed to call set_cap_param()!!! (ret=%d)\n", ret);
		
		goto End;
	}
	
	for(i = 0; i < 3; i++) {
		stream[i]->enc_max_dim.w = pEncodeInfo->pVideoInfo[i]->capture_width;
		stream[i]->enc_max_dim.h = pEncodeInfo->pVideoInfo[i]->capture_height;
		int pullOut = 0;
		
		set_proc_param(stream[i]->proc_path, &stream[i]->enc_max_dim, 30, HD_VIDEO_PXLFMT_YUV420, 0, pullOut);
		if(ret != HD_OK) {
			printf("Failed to call set_proc_param()!!! (ret=%d)\n", ret);
			
			goto End;
		}
	}
	


	stream[i]->enc_max_dim.w = MD_IMG_WIDTH;
	stream[i]->enc_max_dim.h = MD_IMG_HEIGHT;
	ret = set_proc_param2(stream[i]->proc_path, &stream[i]->enc_max_dim, 10, HD_VIDEO_PXLFMT_YUV420, 0, 1);


#if 0
	set_proc_param(stream[i]->proc_path, &stream[0]->enc_max_dim, 30, HD_VIDEO_PXLFMT_YUV420, 0, 0);
#endif


	if(ret != HD_OK) {
		printf("Failed to call set_proc_param()!!! (ret=%d) i = %d\n", ret, i);
		
		goto End;
	}

	if(need_proc_ch4) {
		i++;

		stream[i]->enc_max_dim.w = stream[0]->cap_dim.w;
		stream[i]->enc_max_dim.h = stream[0]->cap_dim.h;
		
		ret = set_proc_param(stream[i]->proc_path, &stream[i]->enc_max_dim, 1, HD_VIDEO_PXLFMT_YUV420, 0, 0);
		if(ret != HD_OK) {
			printf("Failed to call set_proc_param()!!! (ret=%d) i = %d\n", ret, i);
			
			goto End;
		}
	}

	for(i = 0; i < 3; i++) {

		if (i==0)
			ret = set_enc_cfg(stream[i]->enc_path, &stream_max_dim[i], 16000 * 1000, 0, pEncodeInfo->pVideoInfo[i]);
		else if (i==1)
			ret = set_enc_cfg(stream[i]->enc_path, &stream_max_dim[i], 2000 * 1000, 0, pEncodeInfo->pVideoInfo[i]);
		else 
			ret = set_enc_cfg(stream[i]->enc_path, &stream_max_dim[i], 1100 * 1000, 0, pEncodeInfo->pVideoInfo[i]);


		if(ret != HD_OK) {
			printf("Failed to call set_enc_cfg()!!! (ret=%d)\n", ret);
			
			goto End;
		}

		stream[i]->enc_dim.w = pEncodeInfo->pVideoInfo[i]->capture_width;
		stream[i]->enc_dim.h = pEncodeInfo->pVideoInfo[i]->capture_height;
		stream[i]->enc_codec = pEncodeInfo->pVideoInfo[i]->codec;



		ret = set_enc_param(stream[i]->enc_path, &stream[i]->enc_dim, pEncodeInfo->pVideoInfo[i]);
		if(ret != HD_OK) {
			printf("Failed to call set_enc_param()!!! (ret=%d)\n", ret);
			
			goto End;
		}
	}

	ret = hd_videocap_bind(HD_VIDEOCAP_0_OUT_0, HD_VIDEOPROC_0_IN_0);


	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_bind()!!! (ret=%d)\n", ret);
		
		goto End;
	}

	for(i = 0; i < 3; i++) {
		ret = hd_videoproc_bind(HD_VIDEOPROC_OUT(0, i), HD_VIDEOENC_IN(0, i));
		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_bind()!!! (ret=%d)\n", ret);
			
			goto End;
		}

		g_video_info[i].p_stream = stream[i];
	}
	g_video_info[i].p_stream = stream[i];





	ret = encode_before_start(pEncodeInfo, pEncodeInfo->pObjInfo);
	if(ret != 0) {
		printf("Failed to call encode_before_start()!!! (ret=%d)\n", ret);
		
		goto End;
	}

	ret = start_get_stream(pEncodeInfo, g_video_info);


	if(ret != HD_OK) {
		printf("Failed to call start_get_stream()!!! (ret=%d)\n", ret);
		
		goto End;
	}

	if(g_encoder.bRestart) {
		g_encoder.th.state = STAT_RUNNING;
		g_encoder.bRestart = 0;
	}


	// ret = hd_videocap_start(stream[0]->cap_path);
	// if(ret != HD_OK) {
	// 	printf("Failed to call hd_videocap_start()!!! (ret=%d)\n", ret);
	// 	
	// 	goto End;
	// }


	for(i = 0; i < 3; i++) {
		ret = hd_videoproc_start(stream[i]->proc_path);
		if(ret != HD_OK) {
			goto End;
		}
		// sleep(1);
	}

	ret = hd_videoproc_start(stream[i]->proc_path);
	if(ret != HD_OK) {
		goto End;
	}


	if(need_proc_ch4) {
		i++;
		ret = hd_videoproc_start(stream[i]->proc_path);
		if(ret != HD_OK) {
			goto End;
		}
		// sleep(1);
	}


	ret = hd_videocap_start(stream[0]->cap_path);
	if(ret != HD_OK) {
		goto End;
	}




#if 1
	VFB_CANVAS_INFO canvasInfo;
	for(i = 0;i < 3;i++) {
		memset(&canvasInfo, 0, sizeof(canvasInfo));
		copy_canvas_data(i, &canvasInfo, stream[0]);
		canvasInfo.area.width = dimension[i].w;
		canvasInfo.area.height = dimension[i].h;
		init_osd_buffer(i, &canvasInfo);
		setImageBuffer(i, stream[0]->img_buf[i]);
	}


	init_osd_language();
	run_osd_thread();

	for(i = 0;i < 3;i++) {
		ret = set_proc_stamp_param(stream[0]->vp_stamp_path[i], stream[0]->stamp_pa[i], stream[0]->stamp_size[i], stream[0]->img_buf[i], &dimension[i], canvas_area_position[i]);
		if(ret != HD_OK) {
			goto End;
		}
	}

	for(i = 0;i < 3;i++) {
		ret = hd_videoproc_start(stream[0]->vp_stamp_path[i]);
		if(ret != HD_OK) {
			//goto End;
		}
	}
#endif

	for(i = 0; i < 3; i++) {
		ret = hd_videoenc_start(stream[i]->enc_path);
		if(ret != HD_OK) {
			goto End;
		}
		stream[i]->flow_start = 1;
	}


#if 1
	ret = start_motion(g_video_info, share_mem);
	if(ret != HD_OK) {
		goto End;
	}
#endif


	// apply configuration
	ret = apply_configuration(stream[0]);
	if(ret != HD_OK) {
		goto End;
	}


	while(g_encoder.th.state != STAT_RUNNING && g_encoder.bRestart == 0) {
		ret = encode_check_video_loop(pEncodeInfo, pEncodeInfo->pObjInfo);
		if(ret != 0) {
			
			goto End;
		}
		sleep(1);
	}


	const char *conf_file = nv_sensor_desc.config_file;
	if(g_shdr) {
		conf_file = nv_sensor_desc.shdr_config_file;
	}


	ret = nv_isp_init(conf_file);
	if(ret < 0) {
		goto End;
	}


	
	usleep(1);
	isp_update_iq();
	usleep(1);
	isp_update_ae();
	usleep(1);
	isp_update_blc();
	usleep(1);
	isp_update_awb();
	usleep(1);
	isp_update_dn();

    g_encoder.bQuit = FALSE;
	while(!g_encoder.bQuit && !g_encoder.bRestart) {
		ret = encode_check_video_loop(pEncodeInfo, pEncodeInfo->pObjInfo);
		if(ret != 0) {
			
			goto End;
		}
		usleep(1);//w4000_sleep(1);
	}

	// stop osd thread
	setStopOsdThread();


	// stop stamp
	for(i = 0; i < 3; i++) {
		ret = hd_videoproc_stop(stream[0]->vp_stamp_path[i]);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_stop\n");
		}
	}




	for(i = 0; i < 3; i++) {
		ret = hd_videoproc_stop(stream[i]->proc_path);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_stop\n");
		}

	}

	// ret = hd_videoproc_stop(stream[i]->proc_path);
	// if(ret != HD_OK) {
	// 	printf("Failed to call hd_videoproc_stop\n");
	// }

	// if (need_proc_ch4)

	{
		ret = hd_videoproc_stop(stream[3]->proc_path);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_stop\n");
		}
	}



	{
		ret = hd_videoproc_stop(stream[4]->proc_path);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_stop\n");
		}
	}


	// ret = hd_videoproc_stop(stream[i]->proc_path);
	// if(ret != HD_OK) {
	// 	printf("Failed to call hd_videoproc_stop\n");
	// }


	// stop video_record modules
	ret = hd_videocap_stop(stream[0]->cap_path);
	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_stop\n");
	}


	for(i = 0; i < 3; i++) {

		ret = hd_videoenc_stop(stream[i]->enc_path);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoenc_stop\n");
		}

		// refresh to indicate no stream
		ret = refresh_video_info(i, &g_video_info[i]);
		if(ret != HD_OK) {
			printf("Failed to call refresh_video_info\n");
		}
	}


	// if (need_proc_ch4)
	// {
	// 	ret = hd_videoenc_stop(stream[4]->enc_path);
	// 	if(ret != HD_OK) {
	// 		printf("Failed to call hd_videoenc_stop\n");
	// 	}

	// }


	// stop_motion();


	g_video_info[0].p_stream->enc_exit = 1;
	stop_get_stream();

	// unbind video_record modules
	ret = hd_videocap_unbind(HD_VIDEOCAP_0_OUT_0);
	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_unbind\n");
	}

	for(i = 0; i < 3; i++) {
		ret = hd_videoproc_unbind(HD_VIDEOPROC_OUT(0, i));
		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_unbind\n");
		}
	}




End:
	stop_motion();

	// close video_record modules
	for(i = 0; i < 3; i++) {
		ret = close_video_module(i, stream[i]);
		if(ret != HD_OK) {
			printf("Failed to call close_video_module\n");
		}
	}

	// uninit all modules
	ret = exit_video_module();
	if(ret != HD_OK) {
		printf("Failed to call exit_video_module\n");
	}

	for(i = 0;i < 3;i++) {
		if(stream[0]->stamp_blk[i] > 0) {
			ret = nv_mem_release(stream[0]->stamp_blk[i]);
			if(ret != HD_OK) {
				printf("Failed to call nv_mem_release\n");
			}
		}
	}



	ret = nv_share_memory_exit(share_mem);
	if(ret != HD_OK) {
		printf("Failed to call nv_share_memory_exit\n");
	}

	ret = nv_mem_pool_exit();
	if(ret != HD_OK) {
		printf("Failed to call nv_mem_pool_exit\n");
	}

	ret = hd_common_uninit();
	if(ret != HD_OK) {
		printf("Failed to call hd_common_uninit\n");
	}

	ret = nv_isp_uninit();
	if(ret < 0) {
		printf("Failed to call nv_isp_uninit\n");
	}

	// release stamp buffer
	for(i = 0;i < 3;i++) {
		if(stream[0]->img_buf[i] != NULL) {
			free(stream[0]->img_buf[i]);
			stream[0]->img_buf[i] = NULL;
		}
	}


	for(i = 0; i < 3+2; i++) {
		free(stream[i]);
	}

	if(ret != HD_OK) {
		return -1;
	}


	return 0;
}



int StopEncode(void)
{
    g_encoder.bQuit = TRUE;

	return 0;
}

int EncodeInit(ENCODE_INFO* pEncodeInfo)
{
	return 0;
}

int EncodeClose(ENCODE_INFO* pEncodeInfo)
{
	return 0;
}

int Encoder_DynamicSetParam(int ch, ENC_OBJECT_INFO* pObjInfo, VIDEO_ENCODER_INFO* pVideoInfo)
{
	int ret = HD_OK;
	NV_VIDEO_RECORD *stream = g_video_info[ch].p_stream;

	if(stream->flow_start == 0) {
		return 0;
	}


	if(pVideoInfo->capture_width != stream->enc_max_dim.w ||
			pVideoInfo->capture_height != stream->enc_max_dim.h) {
		if(g_encoder.bRestart != 1) {
			g_encoder.bRestart = 1;
		}
		return 0;
	}


#if 1
	// Stop
	ret = hd_videoenc_stop(stream->enc_path);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoenc_stop()!!! (ret=%d)\n", ret);
	}

	ret = refresh_video_info(ch, &g_video_info[ch]);
	if(ret != HD_OK) {
		printf("Failed to call refresh_video_info()!!! (ret=%d)\n", ret);
	}

	ret = set_proc_param(stream->proc_path, &stream->enc_max_dim, 30, HD_VIDEO_PXLFMT_YUV420, 0, 0);
	if(ret != HD_OK) {
		printf("Failed to call set_proc_param()!!! (ret=%d)\n", ret);
	}

	stream->enc_codec = pVideoInfo->codec;
	ret = set_enc_param(stream->enc_path, &stream->enc_dim, pVideoInfo);
	if(ret != HD_OK) {
		printf("Failed to call set_enc_param()!!! (ret=%d)\n", ret);
	}

	// Start
	ret = hd_videoenc_start(stream->enc_path);
	if(ret != HD_OK) {
		printf("Failed to call hd_videoenc_start()!!! (ret=%d)\n", ret);
	}
#endif 


	return ret;
}


void update_camera_setup(int update_type, int is_onvif_setup)
{

	int restart_encoder = 0;
	_CAMSETUP set;
	_CAMSETUP_EXT set_ext;

#ifdef USE_SETUP_ENCRYPTION
	setup_ext_default(&set_ext);
	read_enc_setup(&set, &set_ext);
#else
	FILE *fp = NULL, *fp_ext = NULL;
	if ((fp = fopen (CGI_SETUPFILE, "rb")) == NULL) {
		perror ("fopen");
		setup_unlock();
		return -1;
	}

	if (fread ((void *) & set, 1, sizeof (set), fp) != sizeof (set)) {
		perror ("fread");
		fclose (fp);
		setup_unlock();
		return -1;
	}

	if (fp)
		fclose (fp);

	setup_ext_default(&set_ext);

	fp_ext = fopen (CGI_SETUPFILE_EXT, "rb");
	if (fp_ext)
	{
		int magic2[4], size2 = 0;
		memset(magic2, 0, sizeof(magic2));
		fread(magic2, 1, sizeof(magic2), fp_ext);
		if (magic2[0] == MAGIC_NUM_1 && magic2[1] == 0x00710225 && magic2[2] == 0x00951220 && magic2[3] == 0x00981029) {
			fread(&size2, 1, sizeof(int), fp_ext);
			if (size2 > 0) {
				int sz2, len2 = min(size2, sizeof(_CAMSETUP_EXT)) - sizeof(magic2) - sizeof(int);
				if (len2 > 0)
				{
					sz2 = fread(&(set_ext.reserved), 1, len2, fp_ext);
					if (sz2 != len2) {
						perror ("fread");
						fclose (fp_ext);
						return;
					}

					memcpy(set_ext.magic, magic2, sizeof(magic2));
					set_ext.sizeOfStruct = size2;
				}
			}
		}
		else 
		{
		}
	}

	if (fp_ext)
		fclose (fp_ext);
#endif


	g_setup = g_tmpset = set;
	g_setup_ext = g_tmpset_ext = set_ext;

	int r;
	static int on = 1;



	switch(update_type)
	{
		case SET_UPDATE_ISP_IQ:
			{
				isp_update_iq();
				if(is_onvif_setup == 0) {

					onvifserver_set_setup(&g_setup, &g_setup_ext);
					onvifserver_update_isp();
				}
			}
			break;
		case SET_UPDATE_ISP_AE:
			{
				isp_update_ae();
				if(is_onvif_setup == 0) {
					onvifserver_set_setup(&g_setup, &g_setup_ext);
					onvifserver_update_isp();
				}
			}
			break;
		case SET_UPDATE_ISP_BLC:
			{
				isp_update_blc();
				if(is_onvif_setup == 0) {
					onvifserver_set_setup(&g_setup, &g_setup_ext);
					onvifserver_update_isp();
				}
			}
			break;
		case SET_UPDATE_ISP_AWB:
			{
				isp_update_awb();
				if(is_onvif_setup == 0) {
					onvifserver_set_setup(&g_setup, &g_setup_ext);
					onvifserver_update_isp();
				}
			}
			break;
		case SET_UPDATE_ISP_DN:
			{
				isp_update_dn();
			}
			break;
	}
}





unsigned int nv_mem_release(UINT32 stamp_blk)
{
	UINT32 ret;

	// 
	ret = hd_common_mem_release_block(stamp_blk);
	if(ret != HD_OK) {
		printf("stamp_blk(%d) is not released safely\n", stamp_blk);
	}

	return ret;
}

int nv_share_memory_init(NV_MEM_RANGE *p_share_mem)
{
	HD_COMMON_MEM_VB_BLK blk;
	UINT8 i;
	UINT32 pa, va;
	UINT32 blk_size = MD_IMG_BUF_SIZE;
	HD_COMMON_MEM_DDR_ID ddr_id = DDR_ID0;
	HD_RESULT ret = HD_OK;

	for(i = 0; i < 11; i++) {
		p_share_mem[i].addr = 0x00;
		p_share_mem[i].va   = 0x00;
		p_share_mem[i].size = 0x00;
		p_share_mem[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	}

	for(i = 0; i < 11; i++) {
		if(i == 0) blk_size = MD_IMG_BUF_SIZE;
		else if(i == 1 || i == 2) blk_size = MD_IMG_BUF_SIZE/2;
		else if(i == 3 || i == 7) blk_size = MD_IMG_BUF_SIZE*40;
		else if(i == 4 || i == 8) blk_size = MD_IMG_BUF_SIZE*6;
		else if(i == 5 || i == 9) blk_size = ((MD_IMG_BUF_SIZE+15)/16)*16*12;
		else if(i == 6) blk_size = ((MD_IMG_BUF_SIZE+7)/8);
		else if(i == 10) blk_size = MD_IMG_BUF_SIZE; // result transform

		blk = hd_common_mem_get_block(HD_COMMON_MEM_GLOBAL_MD_POOL, blk_size, ddr_id);
		if(blk == HD_COMMON_MEM_VB_INVALID_BLK) {
			printf("Failed to call hd_common_mem_get_block()!!! (blk=0x%X)\n", blk);
			goto Fail;
		}

		pa = hd_common_mem_blk2pa(blk);
		if(pa == 0) {
			printf("Failed to call hd_common_mem_blk2pa()!!! (pa=0x%X)\n", pa);
			goto Fail;
		}

		if(pa > 0) {
			va = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, pa, blk_size);
			if(va == 0) {
				printf("Failed to call hd_common_mem_mmap()!!! (va=0x%X)\n", va);
				goto Fail;
			}
		}
		p_share_mem[i].addr = pa;
		p_share_mem[i].va   = va;
		p_share_mem[i].size = blk_size;
		p_share_mem[i].blk  = blk;
	}

	return 0;

Fail:
	for (; i > 0 ;) {
		i -= 1;
		// 
		ret = hd_common_mem_release_block(p_share_mem[i].blk);
		if(HD_OK != ret) {
			printf("Failed to call hd_common_mem_release_block()!!!! (ret=0x%X)\n", ret);
		}
	}
	return -1;
}

int nv_share_memory_exit(NV_MEM_RANGE *p_share_mem)
{
	UINT8 i;
	HD_RESULT ret = HD_OK;

	UINT32 blk_size = 0;

	for(i = 0; i < 11; i++){
		if(p_share_mem[i].va != 0) {
			hd_common_mem_munmap((void *)p_share_mem[i].va, p_share_mem[i].size);
			blk_size += p_share_mem[i].size;
		}

		if(p_share_mem[i].blk != HD_COMMON_MEM_VB_INVALID_BLK) {
			// printf("! %s:%d > i? %d\n", __func__, __LINE__, i);
			ret = hd_common_mem_release_block(p_share_mem[i].blk);



			if(HD_OK != ret) {
				printf("Failed to call hd_common_mem_release_block()!!! (ret=0x%X)\n", ret);
				goto Fail;
			}
		}


		p_share_mem[i].addr = 0x00;
		p_share_mem[i].va   = 0x00;
		p_share_mem[i].size = 0x00;
		p_share_mem[i].blk  = HD_COMMON_MEM_VB_INVALID_BLK;
	}

	return 0;
Fail:
	return -1;
}



int set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl, int shdr)
{
	int ret = 0;
	HD_VIDEOCAP_DRV_CONFIG cap_cfg = {0};
	HD_PATH_ID video_cap_ctrl = 0;
	HD_VIDEOCAP_CTRL iq_ctl = {0};

	ret = hd_videocap_open(0, HD_VIDEOCAP_0_CTRL, &video_cap_ctrl); //open this for device control
	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_open()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	snprintf(cap_cfg.sen_cfg.sen_dev.driver_name, HD_VIDEOCAP_SEN_NAME_LEN-1, nv_sensor_desc.driver_name);

	// printf("sensor 1 using %s \n", cap_cfg.sen_cfg.sen_dev.driver_name);

	// MIPI interface

	cap_cfg.sen_cfg.sen_dev.if_type = HD_COMMON_VIDEO_IN_MIPI_CSI;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.sensor_pinmux = 0x220;

	//Sensor interface choice
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.serial_if_pinmux = 0xF01;

#if 0
#else	// CHIP_NA51089
	cap_cfg.sen_cfg.sen_dev.pin_cfg.pinmux.cmd_if_pinmux = 0x01;//PIN_I2C_CFG_CH1
#endif

	cap_cfg.sen_cfg.sen_dev.pin_cfg.clk_lane_sel = HD_VIDEOCAP_SEN_CLANE_SEL_CSI0_USE_C0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[0] = 0;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[1] = 1;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[2] = 2;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[3] = 3;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[4] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[5] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[6] = HD_VIDEOCAP_SEN_IGNORE;
	cap_cfg.sen_cfg.sen_dev.pin_cfg.sen_2_serial_pin_map[7] = HD_VIDEOCAP_SEN_IGNORE;


	// if(shdr) {
	// 	cap_cfg.sen_cfg.shdr_map = HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_1));
	// }

	ret = hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_DRV_CONFIG, &cap_cfg);
	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_set()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	iq_ctl.func = VIDEOCAP_ALG_FUNC;
	if(shdr) {
		iq_ctl.func |= HD_VIDEOCAP_FUNC_SHDR;
	}

	ret = hd_videocap_set(video_cap_ctrl, HD_VIDEOCAP_PARAM_CTRL, &iq_ctl);
	if(ret != HD_OK) {
		printf("Failed to call hd_videocap_set()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	*p_video_cap_ctrl = video_cap_ctrl;

	return 0;
Fail:
	return -1;
}

int set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, UINT32 path, int shdr)
{
	int ret = HD_OK;
	UINT32 color_bar_width = 200;
	{
		HD_VIDEOCAP_IN video_in_param = {0};

		video_in_param.sen_mode = HD_VIDEOCAP_SEN_MODE_AUTO; //auto select sensor mode by the parameter of HD_VIDEOCAP_PARAM_OUT
		video_in_param.frc = HD_VIDEO_FRC_RATIO(30,1);

		video_in_param.dim.w = p_dim->w;
		video_in_param.dim.h = p_dim->h;
		video_in_param.pxlfmt = SEN_OUT_FMT;
		if((path == 0) && (shdr == 1))
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_2;
		else
			video_in_param.out_frame_num = HD_VIDEOCAP_SEN_FRAME_NUM_1;

		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN, &video_in_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videocap_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}
	}

	{
		HD_VIDEOCAP_CROP video_crop_param = {0};

		video_crop_param.mode = HD_CROP_OFF;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_IN_CROP, &video_crop_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videocap_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}
	}


	{
		HD_VIDEOCAP_OUT video_out_param = {0};

		//without setting dim for no scaling, using original sensor out size
		if((path == 0) && (shdr == 1))
			video_out_param.pxlfmt = SHDR_CAP_OUT_FMT;
		else
			video_out_param.pxlfmt = CAP_OUT_FMT;

		video_out_param.dir = HD_VIDEO_DIR_NONE;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_OUT, &video_out_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videocap_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}
	}


	{//direct mode
		HD_VIDEOCAP_FUNC_CONFIG video_path_param = {0};

		video_path_param.out_func = HD_VIDEOCAP_OUTFUNC_DIRECT;
		ret = hd_videocap_set(video_cap_path, HD_VIDEOCAP_PARAM_FUNC_CONFIG, &video_path_param);
		if(ret != HD_OK) {
			printf("Failed to call hd_videocap_set()!!! (ret=%d)\n", ret);
			goto Fail;
		}

	}

	return 0;
Fail:
	return -1;
}



static int stream2common(HD_VIDEOENC_BS *data_pull, UINT32 vir_addr, HD_VIDEOENC_BUFINFO phy_buf, VENC_STREAM_S *pstStream)
{
	int i;
	int refType = 1;
   	unsigned int size = 0;

	if(data_pull->frame_type == HD_FRAME_TYPE_IDR || data_pull->frame_type == HD_FRAME_TYPE_I) {
		refType = 0;
	}

	if(data_pull->vcodec_format == HD_CODEC_TYPE_H264) {
		pstStream->stH264Info.enRefType = refType;
	}
	else if(data_pull->vcodec_format == HD_CODEC_TYPE_H265) {
		pstStream->stH265Info.enRefType = refType;
	}

	// namba
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(VENC_PACK_S) * data_pull->pack_num);  //136
	pstStream->pstPack = malloc(sizeof(VENC_PACK_S) * data_pull->pack_num);
	pstStream->pstPack->u64PTS = data_pull->timestamp;
	pstStream->u32PackCount = data_pull->pack_num;
	for(i = 0; i < data_pull->pack_num; i++) {
		pstStream->pstPack[i].u32Len = data_pull->video_pack[i].size;
		pstStream->pstPack[i].u32PhyAddr = (HI_U32)data_pull->video_pack[i].phy_addr;
		pstStream->pstPack[i].pu8Addr = (HI_U8 *)ADDR_PHY2VIRT(data_pull->video_pack[i].phy_addr, vir_addr, phy_buf.buf_info.phy_addr);
		size += pstStream->pstPack[i].u32Len;
	}

	if(data_pull->vcodec_format == HD_CODEC_TYPE_JPEG) {
		pstStream->stJpegInfo.u32PicBytesNum = size;
	}

	return size;
}


static int send_stream(int ch, ENCODE_INFO* pEncodeInfo, HD_VIDEOENC_BS *data_pull, UINT32 vir_addr, HD_VIDEOENC_BUFINFO phy_buf)
{
	unsigned int size;
	VENC_STREAM_S stStream;
	PAYLOAD_TYPE_E enType;

	if(data_pull->vcodec_format == HD_CODEC_TYPE_H264) {
		enType = PT_H264;
	}
	else if(data_pull->vcodec_format == HD_CODEC_TYPE_H265) {
		enType = PT_H265;
	}
	else if(data_pull->vcodec_format == HD_CODEC_TYPE_JPEG) {
		enType = PT_JPEG;
	}
	else {
		
		goto Fail;
	}
		

	size = stream2common(data_pull, vir_addr, phy_buf, &stStream);

	if(data_pull->vcodec_format == HD_CODEC_TYPE_JPEG) {
		encode_after_video_encoded(ch, enType, (void *)pEncodeInfo, &stStream);
	}
	else {
		encode_after_video_encoded(ch, enType, (void *)pEncodeInfo, &stStream);
	}

	if(stStream.pstPack != NULL) {
		free(stStream.pstPack);
	}

	return size;
Fail:
	return -1;
}


#if 1//VENC_LIST_MODE
static void *encode_thread(void *arg)
{
	int i;
	int ret;
	int size = 0;
	NV_ENCODE_THREAD_ARG *p_arg = (NV_ENCODE_THREAD_ARG *)arg;
	NV_VIDEO_INFO *p_video_info = (NV_VIDEO_INFO *)p_arg->video_info;
	ENCODE_INFO *pEncodeInfo = (ENCODE_INFO *)p_arg->encode_info;
	HD_VIDEOENC_POLL_LIST poll_list[CAM_MAX];

	NV_VIDEO_RECORD* p_stream[CAM_MAX];
	HD_VIDEOENC_BS data_pull;
	static int preCh[CAM_MAX] = { 0, };

	for(i = 0; i < CAM_MAX; i++) {
		p_stream[i] = p_video_info[i].p_stream;
	}

	while(p_stream[0]->flow_start == 0) {
		sleep(1);	// Wait flow start
	}

	for(i = 0; i < CAM_MAX; i++) {
		hd_videoenc_get(p_stream[i]->enc_path, HD_VIDEOENC_PARAM_BUFINFO, &p_video_info[i].phy_buf);

		p_video_info[i].vir_addr = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, \
				p_video_info[i].phy_buf.buf_info.phy_addr, \
				p_video_info[i].phy_buf.buf_info.buf_size);

		poll_list[i].path_id = p_stream[i]->enc_path;
	}

	int abnormal_count = 0;
#if 0
	int64_t tick = get_tick_usec();
	UINT64 timestamp = 0;
#endif

	while(p_stream[0]->enc_exit == 0) {
		if(abnormal_count > 1000*1000) {
			printf("Encoder restart!!!\n");
			g_encoder.bRestart = 1;
		}

		ret = hd_videoenc_poll_list(poll_list, CAM_MAX, 5);
		if(ret != HD_OK) {
			abnormal_count++;
			usleep(10);
			continue;
		}
		abnormal_count = 0;
		// 240611
		for(i = 0; i < CAM_MAX; i++) {
			if(poll_list[i].revent.event) {
			
				ret = hd_videoenc_pull_out_buf(p_stream[i]->enc_path, &data_pull, -1); // -1 = blocking mode, > 0 = ms
				if(ret == HD_OK) {
					
					{
						size = send_stream(i, pEncodeInfo, &data_pull, p_video_info[i].vir_addr, p_video_info[i].phy_buf);
						if(data_pull.vcodec_format != HD_CODEC_TYPE_JPEG) {
						}
					}


					ret = hd_videoenc_release_out_buf(p_stream[i]->enc_path, &data_pull);
					if(ret != HD_OK) {
					}
				}
				else if(ret != HD_ERR_TIMEDOUT) {
					usleep(10);
				}
				else {
					usleep(10);
				}
			}
		}
	}

	// mummap for bs buffer
	for(i = 0; i < CAM_MAX; i++) {
		if(p_video_info[i].vir_addr) {
			hd_common_mem_munmap((void *)p_video_info[i].vir_addr, p_video_info[i].phy_buf.buf_info.buf_size);
		}
	}

	return 0;
}
#else
#endif


int start_get_stream(ENCODE_INFO* pEncodeInfo, NV_VIDEO_INFO *video_info)
{
	static NV_ENCODE_THREAD_ARG arg;

	int ret;

	video_info[0].p_stream->enc_exit = 0;
	arg.video_info = video_info;
	arg.encode_info = pEncodeInfo;

	ret = pthread_create(&venc_thread_id, NULL, encode_thread, (void *)&arg);
	if(ret < 0) {
		printf("Failed to call pthread_create()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int stop_get_stream()
{
	pthread_join(venc_thread_id, 0);
	return 0;
}




static void *motion_thread(void *arg)
{
	HD_VIDEO_FRAME video_frame = {0};
	UINT32 ping_pong_id = 0, is_Init = 0;
	HD_RESULT ret = HD_OK;
	UINT32 reg_id = 0;
	NV_MOTION_THREAD_ARG *p_arg = (NV_MOTION_THREAD_ARG *)arg;
	NV_VIDEO_INFO *p_video_info = (NV_VIDEO_INFO *)p_arg->video_info;
	NV_MEM_RANGE *p_share_mem = (NV_MEM_RANGE*)p_arg->share_mem;
	NV_VIDEO_RECORD* p_stream = p_video_info[3].p_stream;
	UINT32 imgY_va, imgUV_va;
	VENDOR_MD_TRIGGER_PARAM md_trig_param;
	LIB_MD_MDT_LIB_INFO mdt_lib_param;
	LIB_MD_MDT_RESULT_INFO lib_md_rst;
	int uv_size = (MD_IMG_BUF_SIZE/2);
	char MDBuff[15 + 1][16 + 1];
	int counter = 0;
	int is_detected = 0;
	int sensitivity = g_setup.cam[0].sens;
	int detect =0;


	ret = vendor_md_init();
	if(HD_OK != ret) {
		printf("Failed to call vendor_md_init()!!! (ret=%d)\n", ret);
		goto End;
	}
	md_trig_param.is_nonblock = 0;
	md_trig_param.time_out_ms = 0;

	// LibMD motion detection info
	mdt_lib_param.mdt_info.libmd_enabled = 1;
	mdt_lib_param.mdt_info.phy_md_x_num = MD_IMG_WIDTH;
	mdt_lib_param.mdt_info.phy_md_y_num = MD_IMG_HEIGHT;
	mdt_lib_param.mdt_info.phy_md_rst.p_md_bitmap = (UINT8*)p_share_mem[7].va;
	mdt_lib_param.mdt_info.phy_md_rst.md_bitmap_sz = MD_IMG_WIDTH * MD_IMG_HEIGHT;
	ret = lib_md_set(0, LIB_MD_MOTION_DETECT_INFO, &mdt_lib_param.mdt_info);
	if(ret != HD_OK) {
		printf("Failed to call lib_md_set()!!! (ret=%d)\n", ret);
		goto End;
	}
	// LibMD init
	ret = lib_md_init(0);
	if(ret != HD_OK) {
		printf("Failed to call lib_md_init()!!! (ret=%d)\n", ret);
		goto End;
	}
	// LibMD function enable
	mdt_lib_param.mdt_enable.globel_md_alarm_detect_en = 1;
	mdt_lib_param.mdt_enable.subregion_md_alarm_detect_en = 0;
	mdt_lib_param.mdt_enable.scene_change_alarm_detect_en = 0;
	mdt_lib_param.mdt_enable.md_obj_detect_en = 1;
	ret = lib_md_set(0, LIB_MD_AP_ENABLE_PARAM, &mdt_lib_param.mdt_enable);
	if(ret != HD_OK) {
		printf("Failed to call lib_md_set()!!! (ret=%d)\n", ret);
		goto End;
	}
	// LibMD global motion alarm
	mdt_lib_param.mdt_global_param.motion_alarm_th = 1; // (0~100) Motion area percentage(%)


	ret = lib_md_set(0, LIB_MD_AP_GLOBAL_MOTION_ALARM_PARAM, &mdt_lib_param.mdt_global_param);
	if(ret != HD_OK) {
		printf("Failed to call lib_md_set()!!! (ret=%d)\n", ret);
		goto End;
	}
	// LibMD sub-region motion alarm
	mdt_lib_param.mdt_subregion_param.sub_region_num = 1;
	mdt_lib_param.mdt_subregion_param.sub_region[0].enabled = 1;
	mdt_lib_param.mdt_subregion_param.sub_region[0].x_start = 0;
	mdt_lib_param.mdt_subregion_param.sub_region[0].y_start = 0;
	mdt_lib_param.mdt_subregion_param.sub_region[0].x_end = 160;
	mdt_lib_param.mdt_subregion_param.sub_region[0].y_end = 90;
	mdt_lib_param.mdt_subregion_param.sub_region[0].alarm_th = 50;
	ret = lib_md_set(0, LIB_MD_AP_SUBREGION_MOTION_ALARM_PARAM, &mdt_lib_param.mdt_subregion_param);
	if(ret != HD_OK) {
		printf("Failed to call lib_md_set()!!! (ret=%d)\n", ret);
		goto End;
	}

	// LibMD scene change alarm
	mdt_lib_param.mdt_scene_change_param.scene_change_alarm_th = 50;
	ret = lib_md_set(0, LIB_MD_AP_SCENE_CHANGE_ALARM_PARAM, &mdt_lib_param.mdt_scene_change_param);
	if(ret != HD_OK) {
		printf("Failed to call lib_md_set()!!! (ret=%d)\n", ret);
		goto End;
	}

	// LibMD Obj detect
	mdt_lib_param.mdt_obj.obj_size_th = get_obj_size_th(sensitivity);
	ret = lib_md_set(0, LIB_MD_AP_OBJ_PARAM, &mdt_lib_param.mdt_obj);
	if(ret != HD_OK) {
		printf("Failed to call lib_md_set()!!! (ret=%d)\n", ret);
		goto End;
	}


	while(g_endFlag == 0) {

		ret = hd_videoproc_pull_out_buf(p_stream->proc_path, &video_frame, -1); // -1:blocking, 0:non-blocking, >0:timeout
		if(ret != HD_OK) {
			usleep(100*1000);
			continue;
		}

		if(counter > 30) {


			imgY_va = (UINT32)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_CACHE, video_frame.phy_addr[0],
									MD_IMG_BUF_SIZE + (MD_IMG_BUF_SIZE / 2));
			imgUV_va = imgY_va + MD_IMG_BUF_SIZE;

			p_share_mem[0].addr = video_frame.phy_addr[0];
#if 0
			p_share_mem[1].addr = video_frame.phy_addr[1];
#endif
			memset((void *)p_share_mem[1].va, 0x00, MD_IMG_BUF_SIZE/2);

			md_set_para(p_share_mem, ping_pong_id, is_Init, sensitivity);

			ret = vendor_md_trigger(&md_trig_param);
			if(HD_OK != ret) {
				printf("Failed to call vendor_md_trigger()!!! (ret=%d)\n", ret);
				goto End;
			}

#if 0
			memcpy((UINT32 *)p_share_mem[2].va , (UINT32 *)imgUV_va , MD_IMG_BUF_SIZE/2);
#else
			memset((UINT32 *)p_share_mem[2].va , 0x00, MD_IMG_BUF_SIZE/2);  // this
#endif



			if(is_Init == 1) {
				detect = bc_reorgS1((UINT8*)p_share_mem[6].va, (UINT8*)p_share_mem[7].va, MD_IMG_WIDTH, MD_IMG_HEIGHT);

				memset((void *)&lib_md_rst, 0, sizeof(LIB_MD_MDT_RESULT_INFO));
				ret = lib_md_get(0, LIB_MD_RESULT_INFO, &lib_md_rst);
				if(ret != HD_OK) {
					printf("Failed to call lib_md_get() LIB_MD_RESULT_INFO!!! (ret=%d)\n", ret);
				}

				memset(MDBuff, 0, sizeof(MDBuff));

				is_detected = 0;
				if(lib_md_rst.global_motion_alarm == 1) {
					// is_detected = 1;
				}
				if(lib_md_rst.obj_num > 1) {
					is_detected = 1;
				}


				if(is_detected) {
					md2buff2((char *)p_share_mem[7].va, MDBuff, &lib_md_rst);
				}


				encode_motion_check(MDBuff, video_frame.timestamp);
			}

            hd_common_mem_munmap((void *)imgY_va, MD_IMG_BUF_SIZE + (MD_IMG_BUF_SIZE/2));

			ping_pong_id = (ping_pong_id + 1) % 2;
			if(is_Init == 0) is_Init = 1;
#if 0
			else {
				if(is_detected == 1) is_Init = 0;
			}
#endif
		}
		else if(counter <= 30){
			counter++;
		}

		ret = hd_videoproc_release_out_buf(p_stream->proc_path, &video_frame);
		if(ret != HD_OK) {
			printf("Failed to call hd_videoproc_release_out_buf()!!! (ret=%d)\n", ret);
			goto End;
		}

		if(check_setup(&sensitivity)) {
			// LibMD Obj detect
			mdt_lib_param.mdt_obj.obj_size_th = get_obj_size_th(sensitivity);
			lib_md_set(0, LIB_MD_AP_OBJ_PARAM, &mdt_lib_param.mdt_obj);
		}



	}
	
End:
	if(g_endFlag == 0) {
		g_endFlag = -1;
	}

	ret = lib_md_uninit(0);
	if(HD_OK != ret) {
		printf("Failed to call lib_md_uninit()!!! (ret=%d)\n", ret);
	}
	ret = vendor_md_uninit();
	if(HD_OK != ret) {
		printf("Failed to call vendor_md_uninit()!!! (ret=%d)\n", ret);
	}
	return NULL;
}



static NV_MOTION_THREAD_ARG arg;
int start_motion(NV_VIDEO_INFO *video_info, NV_MEM_RANGE *share_mem)
{
	int ret;

	g_endFlag = 0;
	arg.video_info = video_info;
	arg.share_mem = share_mem;
	ret = pthread_create(&mot_thread_id, NULL, motion_thread, (void *)&arg);
	if(ret < 0) {
		printf("Failed to call pthread_create()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int stop_motion()
{
	g_endFlag = 1;
	pthread_join(mot_thread_id, 0);
	return 0;
}

int watch_motion_thread()
{
	return g_endFlag;
}




int set_proc_stamp_param(HD_PATH_ID stamp_path, UINT32 stamp_pa, UINT32 stamp_size, unsigned short *img_buf, HD_DIM *dim, const int *position)
{
	int ret = HD_OK;
	HD_OSG_STAMP_BUF  buf;
	HD_OSG_STAMP_IMG  img;
	HD_OSG_STAMP_ATTR attr;
	int err_line =0;

	if(!stamp_pa) {
		printf("Invalid stamp_pa = 0x%x!!!\n", stamp_pa);
		
		goto Fail;
	}

	memset(&buf, 0, sizeof(HD_OSG_STAMP_BUF));

	//buf.type      = HD_OSG_BUF_TYPE_PING_PONG;
	buf.type      = HD_OSG_BUF_TYPE_SINGLE;
	buf.p_addr    = stamp_pa;
	buf.size      = stamp_size;

	ret = hd_videoproc_set(stamp_path, HD_VIDEOPROC_PARAM_IN_STAMP_BUF, &buf);
	if(ret != HD_OK){
		printf("fail to set stamp buffer(ret = %d)!!!\n", ret);
		
		goto Fail;
	}
	//sleep(1);

	memset(&img, 0, sizeof(HD_OSG_STAMP_IMG));

	img.fmt        = HD_VIDEO_PXLFMT_ARGB4444;
	img.dim.w      = dim->w;
	img.dim.h      = dim->h;
	img.p_addr     = (UINT32)img_buf;

	ret = hd_videoproc_set(stamp_path, HD_VIDEOPROC_PARAM_IN_STAMP_IMG, &img);
	//sleep(1);


	if(ret != HD_OK){
		printf("fail to set stamp image(ret = %d)!!!\n", ret);
		
		goto Fail;
	}

	memset(&attr, 0, sizeof(HD_OSG_STAMP_ATTR));

	if(position == NULL) {
		attr.position.x = 0;
		attr.position.y = 0;
	}
	else {
		attr.position.x = *position;
		attr.position.y = *(position + 1);
	}
	attr.alpha      = 255;
	attr.layer      = 0;
	attr.region     = 0;



	ret = hd_videoproc_set(stamp_path, HD_VIDEOPROC_PARAM_IN_STAMP_ATTR, &attr);

	if(ret != HD_OK){
		printf("fail to set stamp attr(ret = %d)!!!\n", ret);
		
		goto Fail;
	}
	//sleep(1);
	return 0;
Fail:
	if (err_line)
		printf("err line? %d, err: %s", err_line, err_handle_msg(ret));
	return -1;
}

int set_proc_stamp_position(HD_PATH_ID stamp_path, int x, int y)
{
	int ret = HD_OK;
	HD_OSG_STAMP_ATTR attr;

	memset(&attr, 0, sizeof(HD_OSG_STAMP_ATTR));
	ret = hd_videoproc_get(stamp_path, HD_VIDEOPROC_PARAM_IN_STAMP_ATTR, &attr);
	if(ret != HD_OK){
		printf("fail to get stamp attr(ret = %d)!!!\n", ret);
		goto Fail;
	}

	if(x < 0) x = 0;
	if(y < 0) y = 0;
	attr.position.x = x;
	attr.position.y = y;

	ret = hd_videoproc_set(stamp_path, HD_VIDEOPROC_PARAM_IN_STAMP_ATTR, &attr);
	if(ret != HD_OK){
		printf("fail to set stamp attr(ret = %d)!!!\n", ret);
		goto Fail;
	}
	return 0;
Fail:
	return -1;
}

