#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "lib/datetime.h"
#include "lib/net.h"
#include "curl/curl.h"
#include "openssl/hmac.h"
#include "jansson.h"
#include "push_md_http.h"

typedef struct _PUSH_MD_CONFIG_S {
	int use;
	char url[128];
	int dwell_msec;
	int interval_msec;

	int64_t cur_time;
   	int64_t expire_time;

	PUSH_MD_STATUS status;	
} PUSH_MD_CONFIG_S;

static PUSH_MD_CONFIG_S g_config = {0, "", 5000, 1000, PUSH_MD_DISABLE};

static int s_bStopSignal = 1;
static pthread_t s_hThread = 0;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

int push_md_config(int use, char *server_url, int dwell_msec, int interval_msec)
{
	pthread_mutex_lock(&g_mutex);

	g_config.use = use;
	if(server_url != NULL) {	
		sprintf(g_config.url, "%s", server_url);
	}
	if(dwell_msec > 0) {
		g_config.dwell_msec = dwell_msec;
	}
	if(interval_msec > 0) {
		g_config.interval_msec = interval_msec;
	}

	pthread_mutex_unlock(&g_mutex);

	return 0;
}


struct MemoryStruct {
	char *memory;
	size_t size;
};


int push_md_detected()
{
	if(g_config.use == 0) {
		return 0;
	}

	if(s_bStopSignal == 1) {
		s_bStopSignal = 0;
		g_config.cur_time = get_time_msec();
		g_config.expire_time = g_config.cur_time + g_config.dwell_msec;
	}
	else {
		pthread_mutex_lock(&g_mutex);
		g_config.cur_time = get_time_msec();
		g_config.expire_time = g_config.cur_time + g_config.dwell_msec;
		pthread_mutex_unlock(&g_mutex);
	}

	return 0;
Fail:
	return -1;
}

void push_md_cleanup() {
	if(s_hThread != 0) {
		s_bStopSignal = 1;
		pthread_join(s_hThread, NULL);
		s_hThread = 0;
	}
}
