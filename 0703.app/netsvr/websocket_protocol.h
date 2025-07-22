#ifndef __WEBSOCKET_PROTOCOL_H__
#define __WEBSOCKET_PROTOCOL_H__

#include "jansson.h"

typedef struct _WS_PACKET
{
	const char *proc_name;
	json_t *param;
} WS_PACKET;

int start_video(void *arg, WS_PACKET *packet);
int stop_video(void *arg, WS_PACKET *packet);

#define MAKE_API_MAP(api) {#api, api}
struct _WS_PROC 
{
	const char *name;
	int (*func)(void *, WS_PACKET *);
} ws_proc[] = {
	MAKE_API_MAP(start_video),
	MAKE_API_MAP(stop_video),
};

#endif /*__WEBSOCKET_PROTOCOL_H__*/
