#ifndef __VIDEO_POOL_H__
#define __VIDEO_POOL_H__

#include "hi_common.h"
#include "hi_comm_venc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VIDEO_POOL_HEADER_S {
	uint64_t tm;
	unsigned int iframe;	// 0:P-Frame, 1:I-Frame, 10:JPEG
	unsigned int size;
	unsigned char data[0];
} __attribute__((packed)) VIDEO_POOL_HEADER_S;

typedef void* vp_h;

int video_pool_init();
void video_pool_deinit();

int video_pool_send(int ch, void *data, PAYLOAD_TYPE_E enType);

vp_h video_pool_handle_init(int ch);
void video_pool_handle_deinit(vp_h h);

#ifdef __cplusplus
}
#endif

#endif /*__VIDEO_POOL_H__*/
