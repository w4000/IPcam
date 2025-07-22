#ifndef __FCGI_COMMON_H__
#define __FCGI_COMMON_H__

#include "cgi.h"
#include "fcgiapp.h"

typedef struct _CGI_LINK {
	char uri[32];
	int (*func)(FCGX_Stream *in, FCGX_Stream *out, FCGX_ParamArray envp);
	int need_auth;
} CGI_LINK;

formvars *get_params(FCGX_Stream *in, FCGX_Stream *out, FCGX_ParamArray envp);

void http_response(FCGX_Stream *out, const char *resp, int http_status);

#endif // __FCGI_COMMON_H__
