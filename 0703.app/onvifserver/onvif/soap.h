

#ifndef SOAP_H
#define SOAP_H

#include "sys_inc.h"
#include "http.h"

typedef int (*ap_bd)(char * p_buf, int mlen, const char * argv);

#ifdef __cplusplus
extern "C" {
#endif

ONVIF_RET _bdsend_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, ap_bd bdxml, const char * argv, const char * action, XMLN * p_header);

void process_request(HTTPCLN * p_user, HTTPMSG * rx_msg);
void GetSnapshot(HTTPCLN * p_user, HTTPMSG * rx_msg);
void GetHttpSystemLog(HTTPCLN * p_user, HTTPMSG * rx_msg);
void GetHttpAccessLog(HTTPCLN * p_user, HTTPMSG * rx_msg);
void GetSupportInfo(HTTPCLN * p_user, HTTPMSG * rx_msg);
void GetSystemBackup(HTTPCLN * p_user, HTTPMSG * rx_msg);
void SystemRestore(HTTPCLN * p_user, HTTPMSG * rx_msg);

#ifdef __cplusplus
}
#endif

#endif


