#ifndef _MOTION_DETECTION_LIB_H
#define _MOTION_DETECTION_LIB_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "hdal.h"
#include "hd_type.h"
#include "hd_gfx.h"


#include "vendor_ai.h"
#include "kflow_ai_net/kflow_ai_net.h"

#define MD_ALG              0

#define SHARE_MEM_NUM		8

#define MD_IMG_WIDTH           320	
#define MD_IMG_HEIGHT          180
#define IMG_BUF_SIZE        (MD_IMG_WIDTH * MD_IMG_HEIGHT)

#define MDBC_ALIGN(a, b) (((a) + ((b) - 1)) / (b) * (b))
#define OUTPUT_BMP		1
#define DEBUG_FILE 0

#define USE_YUV         DISABLE
#define SAVE_SCALE		DISABLE

#define MD_PROF			DISABLE


#undef MIN
#define MIN(a, b)           ((FLOAT)(a) < (FLOAT)(b) ? (FLOAT)(a) : (FLOAT)(b))
#undef MAX
#define MAX(a, b)           ((FLOAT)(a) > (FLOAT)(b) ? (FLOAT)(a) : (FLOAT)(b))

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


typedef enum {
	LOW_SENSI         = 0,
	MED_SENSI         = 1,
	HIGH_SENSI        = 2,
	DEFAULT_SENSI     = 3,
	ENUM_DUMMY4WORD(MDBC_PARA_SENSI)
} MDBC_PARA_SENSI;


typedef struct _MDMEM_RANGE {
	UINT32               va;        ///< Memory buffer starting address
	UINT32               addr;      ///< Memory buffer starting address
	UINT32               size;      ///< Memory buffer size
	HD_COMMON_MEM_VB_BLK blk;
} MDMEM_RANGE, *PMDMEM_RANGE;

typedef struct _MD_IRECT {
	FLOAT  x1;                           ///< x coordinate of the top-left point of the rectangle
	FLOAT  y1;                           ///< y coordinate of the top-left point of the rectangle
	FLOAT  x2;                           ///< rectangle width
	FLOAT  y2;                           ///< rectangle height
} MD_IRECT;

typedef struct _MDCNN_RESULT
{
	INT32   category;
	FLOAT 	score;
	FLOAT 	x1;
	FLOAT 	y1;
	FLOAT 	x2;
	FLOAT 	y2;
}MDCNN_RESULT;



UINT32 g_sensi = HIGH_SENSI;//DEFAULT_SENSI;

HD_RESULT md_process (MDMEM_RANGE* share_mem, VENDOR_MD_TRIGGER_PARAM* md_trig_param, LIB_MD_MDT_LIB_INFO* mdt_lib_param, LIB_MD_MDT_RESULT_INFO* lib_md_rst, VENDOR_AI_BUF* src_img, MD_IRECT roi, UINT32 is_Init, UINT32 img_name);
HD_RESULT md_set_para(MDMEM_RANGE *p_share_mem, UINT32 mode, UINT32 sensi);
BOOL md_check_overlap(LIB_MD_MDT_RESULT_INFO* p_box1, MDCNN_RESULT p_box2, FLOAT ratio, FLOAT md_ratio_w, FLOAT md_ratio_h, INT32 method);


#endif
