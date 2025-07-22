

#ifndef HTTP_SRV_H
#define HTTP_SRV_H

#include "sys_inc.h"
#include "http.h"
#include "http_parse.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************/
HT_API int      http_srv_init(HTTPSRV * p_srv, const char * saddr, uint16 sport, int cln_num, BOOL https, const char * cert_file, const char * key_file);
HT_API void     http_srv_deinit(HTTPSRV * p_srv);
HT_API void     http_set_msg_callback(HTTPSRV * p_srv, http_msg_callback cb, void * p_userdata);
HT_API void     http_set_data_callback(HTTPSRV * p_srv, http_data_callback cb, void * p_userdata);
HT_API int      http_srv_cln_tx(HTTPCLN * p_user, uint8 * p_data, int len);

/***************************************************************************************/
HT_API uint32   http_cln_index(HTTPSRV * p_srv, HTTPCLN * p_cln);
HT_API HTTPCLN* http_get_cln_by_index(HTTPSRV * p_srv, unsigned long index);
HT_API HTTPCLN* http_get_idle_cln(HTTPSRV * p_srv);
HT_API void     http_free_used_cln(HTTPSRV * p_srv, HTTPCLN * p_cln);

#ifdef __cplusplus
}
#endif

#endif


