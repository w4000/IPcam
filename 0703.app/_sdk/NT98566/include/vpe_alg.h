#ifndef _VPE_ALG_H_
#define _VPE_ALG_H_

#if defined(__KERNEL__) || defined(__FREERTOS)
#include "kwrap/type.h"
#endif

//=============================================================================
// struct & definition
//=============================================================================
#define VPE_GDC_LUT_NUMS 65
#define VPE_2DLUT_NUM 260*257
#define VPE_DRT_PATH_NUM 4

/**
	VPE process id
*/
typedef enum _VPE_ID {
	VPE_ID_1 = 0,                      ///< vpe id 1
	VPE_ID_2,                          ///< vpe id 2
	VPE_ID_3,                          ///< vpe id 3
	VPE_ID_4,                          ///< vpe id 4
	VPE_ID_5,                          ///< vpe id 5
	VPE_ID_6,                          ///< vpe id 6
	VPE_ID_7,                          ///< vpe id 7
	VPE_ID_8,                          ///< vpe id 8
	VPE_ID_MAX_NUM,
	ENUM_DUMMY4WORD(VPE_ID)
} VPE_ID;

typedef enum {
	VPE_ISP_DCE_MODE_GDC_ONLY = 0,
	VPE_ISP_DCE_MODE_2DLUT_ONLY,
	VPE_ISP_DCE_MODE_2DLUT_DCTG,
	VPE_ISP_DCE_MODE_MAX,
} VPE_ISP_DCE_MODE;

typedef enum {
	VPE_ISP_2DLUT_SZ_9X9 = 0,
	VPE_ISP_2DLUT_SZ_65X65 = 3,
	VPE_ISP_2DLUT_SZ_129X129 = 4,
	VPE_ISP_2DLUT_SZ_257X257 = 5,
} VPE_ISP_2DLUT_SZ;

typedef struct _VPE_SHARPEN_PARAM {
	UINT8 enable;
	UINT8 edge_weight_gain;
	UINT8 edge_sharp_str1;
	UINT8 edge_sharp_str2;
	UINT8 flat_sharp_str;
} VPE_SHARPEN_PARAM;

typedef struct _VPE_DCE_CTL_PARAM {
	UINT8 enable;
	VPE_ISP_DCE_MODE dce_mode;
	UINT32 lens_radius;
} VPE_DCE_CTL_PARAM;

typedef struct _VPE_GDC_PARAM {
	UINT16 	fovgain;
	UINT16 	gdc_lut[VPE_GDC_LUT_NUMS];
	UINT16 	cent_x_ratio;		/* cent_x will be calculate: (img_width * ratio / ratio_base) */
	UINT16 	cent_y_ratio;		/* cent_y will be calculate: (img_height * ratio / ratio_base) */
} VPE_GDC_PARAM;

typedef struct _VPE_2DLUT_PARAM {
	VPE_ISP_2DLUT_SZ lut_sz;
	UINT32 	lut[VPE_2DLUT_NUM];
} VPE_2DLUT_PARAM;

typedef enum {
	VPE_YUV_CVT_NONE = 0,
	VPE_YUV_CVT_PC,
	VPE_YUV_CVT_TV,
	VPE_YUV_CVT_MAX,
} VPE_YUV_CVT;

typedef struct _VPE_DRT_PARAM {
	VPE_YUV_CVT cvt_sel[VPE_DRT_PATH_NUM];
} VPE_DRT_PARAM;

typedef enum {
	VPE_ISP_DCTG_2DLUT_SZ_9X9 = 0,
	VPE_ISP_DCTG_2DLUT_SZ_65X65 = 3,
} VPE_ISP_DCTG_2DLUT_SZ;

typedef struct _VPE_DCTG_PARAM {
	UINT8  mount_type;
	VPE_ISP_DCTG_2DLUT_SZ  lut2d_sz;
	UINT16 lens_r;
	UINT16 lens_x_st;
	UINT16 lens_y_st;
	INT32  theta_st;
	INT32  theta_ed;
	INT32  phi_st;
	INT32  phi_ed;
	INT32  rot_z;
	INT32  rot_y;
} VPE_DCTG_PARAM;

typedef struct _VPE_PARAM_PTR {
	VPE_SHARPEN_PARAM *sharpen;
	VPE_DCE_CTL_PARAM *dce_ctl;
	VPE_GDC_PARAM     *gdc;
	VPE_2DLUT_PARAM   *lut2d;
	VPE_DRT_PARAM     *drt;
	VPE_DCTG_PARAM    *dctg;
} VPE_PARAM_PTR;

#endif
