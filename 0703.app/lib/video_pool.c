#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include "video_pool.h"
#include "lib/pool.h"

static pthread_mutex_t g_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
static pool_t stream_pool[4];
static int is_init = 0;

int video_pool_init()
{
	pool_config_t pool_config;
	pool_config.max_frame_size = 512 * 1024;
	pool_config.min_frame_size = 1024;
	pool_config.pool_size = 6 * (pool_config.max_frame_size);
	pool_init(&stream_pool[0], pool_config);

	pool_config.max_frame_size = 256 * 1024;
	pool_config.min_frame_size = 1024;
	pool_config.pool_size = 6 * (pool_config.max_frame_size);
	pool_init(&stream_pool[1], pool_config);

	pool_config.max_frame_size = 256 * 1024;
	pool_config.min_frame_size = 1024;
	pool_config.pool_size = 6 * (pool_config.max_frame_size);
	pool_init(&stream_pool[2], pool_config);

	pool_config.max_frame_size = 256 * 1024;
	pool_config.min_frame_size = 512;
	pool_config.pool_size = 6 * (pool_config.max_frame_size);
	pool_init(&stream_pool[3], pool_config);

	is_init = 1;

	return 0;
}

void video_pool_deinit()
{
	is_init = 0;
	pool_deinit(&stream_pool[0]);
	pool_deinit(&stream_pool[1]);
	pool_deinit(&stream_pool[2]);
	pool_deinit(&stream_pool[3]);
}

int video_pool_send(int ch, void *data, PAYLOAD_TYPE_E enType)
{
	if(is_init == 0) {
		return -1;
	}
	pthread_mutex_lock(&g_pool_mutex);

	VENC_STREAM_S *frame = (VENC_STREAM_S *)(enType != 99)?data:NULL; 
	int i;
	vbuf_t vbuf;
	VIDEO_POOL_HEADER_S hdr;

	if(frame != NULL) {
		hdr.tm = frame->pstPack->u64PTS / 1000;
		if(enType == PT_H265) {
			hdr.iframe = (frame->stH265Info.enRefType == 0)?1:0;
		}
		else if(enType == PT_H264) {
			hdr.iframe = (frame->stH264Info.enRefType == 0)?1:0;
		}
		else if(enType == PT_JPEG) {
			hdr.iframe = 10;
		}
		else {
			pthread_mutex_unlock(&g_pool_mutex);
			return -1;
		}

		hdr.size = 0;

		vbuf.valid_num = 0;
		vbuf.pdata[vbuf.valid_num] = (char *)&hdr;
		vbuf.data_len[vbuf.valid_num] = sizeof(VIDEO_POOL_HEADER_S);
		vbuf.valid_num++;
		for(i = 0; i < frame->u32PackCount; i++) {
			vbuf.pdata[vbuf.valid_num] = frame->pstPack[i].pu8Addr;
			vbuf.data_len[vbuf.valid_num] = frame->pstPack[i].u32Len;
			hdr.size += vbuf.data_len[vbuf.valid_num];
			vbuf.valid_num++;
		}
	}
	else {
		char *text = (char *)data;
		hdr.tm = 0;;
		hdr.iframe = enType;
		hdr.size = 0;

		vbuf.valid_num = 0;
		vbuf.pdata[vbuf.valid_num] = (char *)&hdr;
		vbuf.data_len[vbuf.valid_num] = sizeof(VIDEO_POOL_HEADER_S);
		vbuf.valid_num++;
		vbuf.pdata[vbuf.valid_num] = text;
		vbuf.data_len[vbuf.valid_num] = strlen(text);
		hdr.size += vbuf.data_len[vbuf.valid_num];
		vbuf.valid_num++;
	}
	
	pthread_mutex_unlock(&g_pool_mutex);

	return pool_send(&stream_pool[ch], vbuf);
}

vp_h video_pool_handle_init(int ch)
{
	pool_rpos_t *rpos;
	// printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, sizeof(pool_rpos_t));
	rpos = (pool_rpos_t *)malloc(sizeof(pool_rpos_t));

	pool_handle_init(&stream_pool[ch], rpos);

	return (vp_h)rpos;
}

void video_pool_handle_deinit(vp_h h)
{
	pool_rpos_t *rpos = (pool_rpos_t *)h;

	pool_handle_deinit(rpos);
	free(rpos);
}


