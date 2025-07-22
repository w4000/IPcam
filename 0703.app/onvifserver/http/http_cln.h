

#ifndef HTTP_CLN_H
#define HTTP_CLN_H

#include "http.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL http_onvif_trans(HTTPREQ * p_req, int timeout, const char * bufs, int len);

#ifdef __cplusplus
}
#endif

#endif


