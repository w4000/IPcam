

#ifndef _ONVIF_UTILS_H_
#define _ONVIF_UTILS_H_

#include "sys_inc.h"
#include "onvif.h" 

#if 1
#define onvif_bprint(fmt, args...) 
#else 
#define onvif_bprint(fmt, args...)  fprintf(stdout, fmt, args...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

void 	     onvif_get_time_str(char * buff, int len, int sec_off);
void 	     onvif_get_time_str_s(char * buff, int len, time_t nowtime, int sec_off);
void 	     onvif_get_timezone(char * tz, int len);

const char * onvif_uuid_create();
const char * onvif_get_local_ip();

time_t 	     onvif_timegm(struct tm *T);
int  	     onvif_parse_xaddr(const char * pdata, char * host, char * url, int * port, int * https);

time_t       onvif_datetime_to_time_t(onvif_DateTime * p_datetime);
void         onvif_time_t_to_datetime(time_t n, onvif_DateTime * p_datetime);
const char * onvif_format_datetime_str(time_t n, int flag, const char * format);
time_t	     onvif_timegm(struct tm *T);
int          onvif_parse_uri(const char * p_in, char * p_out, int outlen);

#ifdef __cplusplus
}
#endif


#endif


