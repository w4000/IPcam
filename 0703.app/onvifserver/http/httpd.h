

#ifndef HTTPD_H
#define HTTPD_H

#include "sys_inc.h"
#include "http.h"


struct mime_handler 
{
	const char * pattern;
	const char * mime_type;
	int          auth;
	
	int (*output)(const char *filename, char *buff, int buflen);
};


#ifdef __cplusplus
extern "C" {
#endif

void http_process_request(HTTPCLN * p_user, HTTPMSG * rx_msg);

#ifdef __cplusplus
}
#endif

#endif

