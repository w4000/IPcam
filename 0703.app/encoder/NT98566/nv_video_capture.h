#ifndef __NV_VIDEO_CAPTURE_H__
#define __NV_VIDEO_CAPTURE_H__

#include "hdal.h"

int get_cap_caps(HD_PATH_ID video_cap_ctrl, HD_VIDEOCAP_SYSCAPS *p_video_cap_syscaps);
int set_cap_cfg(HD_PATH_ID *p_video_cap_ctrl, int shdr);
int set_cap_param(HD_PATH_ID video_cap_path, HD_DIM *p_dim, UINT32 path, int shdr);

#endif /*__NV_VIDEO_CAPTURE_H__*/
