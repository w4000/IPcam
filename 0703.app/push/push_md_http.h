#ifndef __PUSH_MD_HTTP_H__
#define __PUSH_MD_HTTP_H__

typedef enum _PUSH_MD_STATUS {
	PUSH_MD_OK,
	PUSH_MD_DISABLE,
	PUSH_MD_ERR_CONNECT,
	PUSH_MD_ERR_HTTP,
	PUSH_MD_ERR_CONTENTS
} PUSH_MD_STATUS;

int push_md_config(int use, char *server_url, int dwell_msec, int interval_msec);
int push_md_detected();
void push_md_cleanup();

#endif /*__PUSH_MD_HTTP_H__*/
