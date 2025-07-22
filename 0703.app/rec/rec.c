#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include "rec.h"
#include "common.h"
#include "mp4encoder.h"
#include "hi_comm_venc.h"

typedef struct _REC_PRIV_S {
	int ready;
	char *rec_path;
	int consumer_id[CAM_MAX];
	int recording[CAM_MAX];
	char rec_file[CAM_MAX][36];
	MP4ENC mp4enc[CAM_MAX];
	int close_file;
} REC_PRIV_S;

static REC_PRIV_S *g_rec = NULL;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static int stamp2tmstr(time_t stamp, char *tmstr)
{
	strftime(tmstr, 36, "%y%m%d%H%M%S", localtime(&stamp));
	return 0;
}

static int close_file(int ch)
{
	g_rec->close_file = 0;
}



static int recv_data(int id, struct st_frame *frame, int ch)
{
	pthread_mutex_lock(&g_mutex);

	if(g_rec == NULL) {
		return -1;
	}


	// First
	if(g_rec->mp4enc[ch] == NULL) {
		g_rec->close_file = 0;
	}


End:
	pthread_mutex_unlock(&g_mutex);
	return 0;
}

int rec_init(const char *rec_path)
{
	int i;
	size_t szpath = 0;

	if(g_rec != NULL) {
		return -1;
	}


	szpath = strlen(rec_path);
	if(szpath == 0) {
		printf("Invalid record path.(%s)\n", rec_path);
		return -1;
	}

	char sys_cmd[256];
	snprintf(sys_cmd, sizeof(sys_cmd), "mkdir %s;echo \"%s\" > /tmp/rec_init", rec_path, rec_path);
	printf("sys_cmd = %s\n", sys_cmd);
	popen(sys_cmd, "w");

	if(!isdirectory(rec_path)) {
		return -1;
	}

	g_rec = (REC_PRIV_S *)malloc(sizeof(REC_PRIV_S));
	g_rec->rec_path = (char *)malloc(szpath + 1);
	strncpy(g_rec->rec_path, rec_path, szpath + 1);

	g_rec->ready = 1;
	return 0;
}
