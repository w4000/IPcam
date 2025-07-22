/*-----------------------------------------------------------------------------*/
/* Including Files                                                             */
/*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "hdal.h"
#include "hd_type.h"
#include "hd_debug.h"
#include "vendor_ai.h"
#include "vendor_ai_cpu/vendor_ai_cpu.h"
#include "kflow_ai_net/kflow_ai_net.h"
#include <arm_neon.h>

#define PVD_VERSION_A  "1"   //model version
#define PVD_VERSION_B  "1"   //pdcnn version
#define PVD_VERSION_C  "3"   //vdcnn version
#define PVD_VERSION_D  "00"   //pvdcnn code version

#define REDUCE_FP		1

#define PVD_MAX_MEM_SIZE			 23 * 1024 * 1024
#define PVD_SRCIMG_MEM_SIZE      	  2 * 2560 * 1440
#define COMMON_IOBUF	1

#define AI_VDO_H		540
#define AI_VDO_W		960

typedef struct _PROPOSAL_PARAM {
	FLOAT pd_anchor_params[3][8][4];
	FLOAT vd_anchor_params[3][8][4];
	INT32 strides[3];
	FLOAT pd_conf_thr;
	FLOAT vd_conf_thr;
	FLOAT nms_thr;
	INT32 pdout_num;
	INT32 vdout_num;
} PROPOSAL_PARAM;

typedef struct _LIMIT_PARAM {
	FLOAT score_thrs[25];
	INT32 sizes[25];
	INT32 limit_size;
	INT32 sm_thr_num;
	FLOAT k[25];
	INT32 limit_module;
} LIMIT_PARAM;

typedef struct REDUCE_FP_PARAM{
	FLOAT ratiow;
	FLOAT ratioh;
	FLOAT thr;
	UINT32 rfpnet_id;
	UINT32 rfp_proc;
}REDUCE_FP_PARAM;

typedef struct _REDUCE_FP_MEM {
	VENDOR_AIS_FLOW_MEM_PARM io_mem;
	VENDOR_AIS_FLOW_MEM_PARM bin_mem;
	VENDOR_AIS_FLOW_MEM_PARM input_mem;
} REDUCE_FP_MEM;


typedef struct _PVDCNN_PARAM {
	UINT32 pdnet_id;
	UINT32 vdnet_id;
	UINT32 pdcnn_proc;
	UINT32 vdcnn_proc;
	UINT32 pd_distance_mode;
	UINT32 vd_distance_mode;
	PROPOSAL_PARAM proposal_param;
	LIMIT_PARAM pd_limit_param;
	LIMIT_PARAM vd_limit_param;
	REDUCE_FP_PARAM rfp_param;
} PVDCNN_PARAM;

typedef struct _PVDCNN_MEM {
    INT32 out_num;
	VENDOR_AIS_FLOW_MEM_PARM io_mem;
#if (!COMMON_IOBUF)
	VENDOR_AIS_FLOW_MEM_PARM vdio_mem;
#endif
	VENDOR_AIS_FLOW_MEM_PARM pdbin_mem;
	VENDOR_AIS_FLOW_MEM_PARM vdbin_mem;
	VENDOR_AIS_FLOW_MEM_PARM input_mem;
	VENDOR_AIS_FLOW_MEM_PARM pps_rslt;
    VENDOR_AIS_FLOW_MEM_PARM pdout_rslt;
	VENDOR_AIS_FLOW_MEM_PARM vdout_rslt;
	VENDOR_AIS_FLOW_MEM_PARM final_rslt;
	VENDOR_AIS_FLOW_MEM_PARM pvdout_rslt;
	VENDOR_AIS_FLOW_MEM_PARM scale_buf;
	REDUCE_FP_MEM rfp_buf;
} PVDCNN_MEM;

typedef struct _PVDCNN_RESULT
{
	INT32   category; //pd category = 1, vd category = 1
	FLOAT 	score;
	FLOAT 	x1;
	FLOAT 	y1;
	FLOAT 	x2;
	FLOAT 	y2;
}PVDCNN_RESULT;

typedef struct _PVD_IRECT {
	FLOAT  x1;                           ///< xmin
	FLOAT  y1;                           ///< ymin
	FLOAT  x2;                           ///< xmax
	FLOAT  y2;                           ///< ymax
} PVD_IRECT;

UINT32 calculate_pvdcnn_memsize(PVDCNN_MEM *pvd_mem, PVDCNN_PARAM *pvd_param);
HD_RESULT get_mem(VENDOR_AIS_FLOW_MEM_PARM *buf, VENDOR_AIS_FLOW_MEM_PARM *req_mem, UINT32 req_size, UINT32 align_size);
UINT32 pvdcnn_nonmax_suppress(PVDCNN_RESULT *p_boxes, INT32 num, FLOAT ratio, INT32 method, INT32 after_num);
HD_RESULT network_get_ai_outbuf_by_name(INT32 proc_id, VENDOR_AI_BUF *p_outbuf, CHAR *p_buf_name);
HD_RESULT pvdcnn_crop_img(HD_GFX_IMG_BUF *dst_img, VENDOR_AI_BUF *src_img, HD_GFX_SCALE_QUALITY method, PVD_IRECT *roi);
VOID pvd_gfx_img_to_vendor(VENDOR_AI_BUF* out_img, HD_GFX_IMG_BUF* in_img, UINT32 va);
HD_RESULT pvdcnn_process(PVDCNN_PARAM *pvd_param, PVDCNN_MEM *pvd_mem, VENDOR_AI_BUF *src_img);
HD_RESULT pvdcnn_init(PVDCNN_PARAM *pvd_param, PVDCNN_MEM *pvd_mem, VENDOR_AIS_FLOW_MEM_PARM *in_buf);
HD_RESULT pvdcnn_uninit(PVDCNN_PARAM *pvd_param);
VOID dump_pvdcnn_detectout(PVDCNN_MEM *pvd_mem, FLOAT ratiow, FLOAT ratioh);

