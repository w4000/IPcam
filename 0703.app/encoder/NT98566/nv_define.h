#ifndef __NV_DEFINE_H__
#define __NV_DEFINE_H__

#include "hs_common.h"
#include "vendor_isp.h"

#define DBGINFO_BUFSIZE() (0x200)
#define VDO_RAW_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))
#define VDO_CA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 3) << 1)
#define VDO_LA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 1) << 1)
#define VDO_VA_BUF_SIZE(win_num_w, win_num_h) ALIGN_CEIL_4((win_num_w * win_num_h << 4) << 1)
#define VDO_YUV_BUFSIZE(w, h, pxlfmt) (ALIGN_CEIL_4((w) * HD_VIDEO_PXLFMT_BPP(pxlfmt) / 8) * (h))

#define SEN_OUT_FMT      HD_VIDEO_PXLFMT_RAW12
#define CAP_OUT_FMT      HD_VIDEO_PXLFMT_RAW12
#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_NRX12_SHDR2
//#define SHDR_CAP_OUT_FMT HD_VIDEO_PXLFMT_RAW12_SHDR2
#define CA_WIN_NUM_W     32
#define CA_WIN_NUM_H     32
#define LA_WIN_NUM_W     32
#define LA_WIN_NUM_H     32
#define VA_WIN_NUM_W      8
#define VA_WIN_NUM_H      8


#define MDBC_ALIGN(a, b) (((a) + ((b) - 1)) / (b) * (b))
#define MD_IMG_WIDTH           352
#define MD_IMG_HEIGHT          240//180
#define MD_IMG_BUF_SIZE        (MD_IMG_WIDTH * MD_IMG_HEIGHT)

#define MASK_COUNT	4
#define VP_STAMP 1

#define JPEG_PROC_CHANNEL 5
#define JPEG_LINK_CHANNEL 2
#define JPEG_FPS 1

#define ADDR_PHY2VIRT(pa, base_va, base_pa) (base_va + (pa - base_pa))


// MD
#define MD_HEAD_BUFSIZE()	            (0x40)
// Note , the md info w, h is vprc input w, h not out w, h
#define MD_INFO_BUFSIZE(w, h)           (ALIGN_CEIL_64(((((w >> 7) + 3) >> 2) << 2) * ((h + 15) >> 4)))


#if 1
typedef struct _NV_VIDEO_RECORD {
	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;
	HD_DIM  proc_max_dim;

	// (2)
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;

	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;
	int enc_codec;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	// (4) user pull
	pthread_t  enc_thread_id;
	UINT32     enc_exit;
	UINT32     flow_start;

	// osg
	HD_PATH_ID vp_mask_path[MASK_COUNT];
	HD_PATH_ID vp_stamp_path[3];

	// stamp
	UINT32 stamp_blk[3];
	UINT32 stamp_pa[3];
	UINT32 stamp_size[3];
	unsigned short *img_buf[3];
} NV_VIDEO_RECORD;


#else
typedef struct _VIDEO_RECORD {

	// (1)
	HD_VIDEOCAP_SYSCAPS cap_syscaps;
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;

	HD_DIM  cap_dim;
	HD_DIM  proc_max_dim;

	// (2)
	HD_VIDEOPROC_SYSCAPS proc_syscaps;
	HD_PATH_ID proc_ctrl;
	HD_PATH_ID proc_path;

	HD_DIM  enc_max_dim;
	HD_DIM  enc_dim;

	// (3)
	HD_VIDEOENC_SYSCAPS enc_syscaps;
	HD_PATH_ID enc_path;

	// (4) user pull
	pthread_t  enc_thread_id;
	UINT32     enc_exit;
	UINT32     flow_start;

} VIDEO_RECORD;
#endif


typedef struct _NV_AUDIO_RECORD {
	HD_AUDIO_SR sample_rate_max;
	HD_AUDIO_SR sample_rate;

	// (1) audio cap
	HD_PATH_ID cap_ctrl;
	HD_PATH_ID cap_path;
	HD_AUDIO_SOUND_MODE cap_mode;

	// (2) audio enc
	HD_PATH_ID enc_path;
	UINT32 enc_type;

	// (3) audio dec
	HD_PATH_ID dec_path;
	UINT32 dec_type;
	UINT32 is_init_dec;
	UINT32 dec_exit;

	// (4) audio out
	HD_PATH_ID out_ctrl;
	HD_PATH_ID out_path;

	// (5) user pull
	pthread_t  enc_thread_id;
	UINT32     enc_exit;
	UINT32     flow_start;
} NV_AUDIO_RECORD;

typedef struct _NV_VIDEO_INFO {
	NV_VIDEO_RECORD *p_stream;
	HD_VIDEOENC_BS  data_pull;
	PAYLOAD_TYPE_E codec_type;
	unsigned char vps[64];
	int vps_size;
	unsigned char sps[64];
	int sps_size;
	unsigned char pps[64];
	int pps_size;
	int ref_cnt;

	UINT32 vir_addr;
	HD_VIDEOENC_BUFINFO phy_buf;
} NV_VIDEO_INFO;

typedef struct _NV_SENSOR_DESC {
	char driver_name[HD_VIDEOCAP_SEN_NAME_LEN];
	char config_file[CFG_NAME_LENGTH];
	char shdr_config_file[CFG_NAME_LENGTH];
} NV_SENSOR_DESC;

const static NV_SENSOR_DESC nv_sensor_desc = {
	"nvt_sen_imx327", "/edvr/isp_cfg/isp_imx307_0.cfg", "/edvr/isp_cfg/isp_imx327_0_hdr.cfg"
};

typedef struct _NV_MEM_RANGE {
	UINT32 va;        ///< Memory buffer starting address
	UINT32 addr;      ///< Memory buffer starting address
	UINT32 size;      ///< Memory buffer size
	HD_COMMON_MEM_VB_BLK blk;
} NV_MEM_RANGE, *PNV_MEM_RANGE;


#endif /*__NV_DEFINE_H__*/
