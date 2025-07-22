#ifndef __NV_VIDEO_ENCODER_H__
#define __NV_VIDEO_ENCODER_H__

#include "hdal.h"
#include "encoder_common.h"

int set_enc_cfg(HD_PATH_ID video_enc_path, HD_DIM *p_max_dim, UINT32 max_bitrate, UINT32 isp_id, VIDEO_ENCODER_INFO *p_enc);
int set_enc_param(HD_PATH_ID video_enc_path, HD_DIM *p_dim, VIDEO_ENCODER_INFO *p_enc); 
int start_get_stream(ENCODE_INFO* pEncodeInfo, NV_VIDEO_INFO *video_info);
int stop_get_stream();

#endif /*__NV_VIDEO_ENCODER_H__*/
