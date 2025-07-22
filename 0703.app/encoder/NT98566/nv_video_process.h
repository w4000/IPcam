#ifndef __NV_VIDEO_PROCESS_H__
#define __NV_VIDEO_PROCESS_H__

#include "hdal.h"

int set_proc_cfg(HD_PATH_ID *p_video_proc_ctrl, HD_DIM* p_max_dim, HD_OUT_ID _out_id, int dfg, HD_PATH_ID ref_path_3dnr);
int set_proc_in_param(HD_PATH_ID video_proc_path, HD_DIM *p_dim);
int set_proc_param(HD_PATH_ID video_proc_path, HD_DIM* p_dim, int fps, HD_VIDEO_PXLFMT pxlfmt, int ext, int pullout);
int set_proc_ratio_crop(HD_PATH_ID video_proc_ctrl, HD_PATH_ID video_proc_path, int on, HD_DIM *dim);
int set_proc_stamp_param(HD_PATH_ID stamp_path, UINT32 stamp_pa, UINT32 stamp_size, unsigned short *img_buf, HD_DIM*, const int*);
int set_proc_stamp_position(HD_PATH_ID stamp_path, int x, int y);

#endif /*__NV_VIDEO_PROCESS_H__*/
