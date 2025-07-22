#ifndef __NV_ISP_H__
#define __NV_ISP_H__

#include "hdal.h"

typedef enum _ISP_ID {
	ISP_ID_1 = 0,
	ISP_ID_2,
	ISP_ID_3,
	ISP_ID_4,
	ISP_ID_5,
	ISP_ID_MAX_NUM,
	ISP_ID_IGNORE = 0xffffffff,
} ISP_ID;

int nv_isp_init(const char *conf_file);
int nv_isp_uninit();


#endif /*__NV_ISP_H__*/
