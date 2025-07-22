
#ifndef _WEBSOCKET_SERVER_H
#define _WEBSOCKET_SERVER_H


#define MAX_WEBSOCKET_SESSION		4

#define WEBSOCKET_TIMEOUT_SEC		(30)
#define WEBSOCKET_TIMEOUT			(WEBSOCKET_TIMEOUT_SEC * 1000)
#define WEBSOCKET_BUF_THRESHOLD		(99)

#define DEFAULT_PORT_WEBSOCKET		(8080)
// #define DEFAULT_PORT_WEBSOCKET		(9080)

#include "../lib/CircularBuffer.h"


// SESSION HANDLER
typedef struct __websocket_session
{
	int status;
	int session_fd;
	char forwarded_ip[32];
	CCircularBuffer_t queue;// recv queue

} websocket_session_t;

extern pthread_mutex_t	g_mutex_websocket_session[MAX_WEBSOCKET_SESSION];

#define WEBSOCKET_SESSION_LOCK(i)		pthread_mutex_lock(&g_mutex_websocket_session[i])
#define WEBSOCKET_SESSION_UNLOCK(i)		pthread_mutex_unlock(&g_mutex_websocket_session[i])

#define VALID_WEBSOCKET_SESSION_IDX(idx)		((idx) >= 0 && (idx) < MAX_WEBSOCKET_SESSION)

void websocket_init();
void websocket_terminate();

void websocket_start();
void websocket_stop();

websocket_session_t* websocket_get_session(int idx);



#endif
