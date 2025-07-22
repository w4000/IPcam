#ifndef __NV_VIDEO_MOTION_H__
#define __NV_VIDEO_MOTION_H__

#include "hdal.h"
#include "encoder_common.h"

typedef struct _NV_MOTION_THREAD_ARG {
	NV_VIDEO_INFO *video_info;
	NV_MEM_RANGE *share_mem;
} NV_MOTION_THREAD_ARG;

int start_motion(NV_VIDEO_INFO *video_info, NV_MEM_RANGE *share_mem);
int stop_motion();
int watch_motion_thread();

#endif /*__NV_VIDEO_MOTION_H__*/
