

#ifndef	__H_UTIL_H__
#define	__H_UTIL_H__


/*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/

#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define MAX(a, b)           ((a) > (b) ? (a) : (b))

#define ARRAY_SIZE(ary)     (sizeof(ary) / sizeof(ary[0]))

/*************************************************************************/
HT_API int          get_if_nums();
HT_API uint32       get_if_ip(int index);
HT_API uint32       get_route_if_ip(uint32 dst_ip);
HT_API uint32       get_default_if_ip();
HT_API const char * get_local_ip();
HT_API int          get_default_if_mac(uint8 * mac);
HT_API uint32       get_address_by_name(const char * host_name);
HT_API const char * get_default_gateway();
HT_API int          get_prefix_len_by_mask(const char * mask);


/*************************************************************************/
HT_API char       * lowercase(char * str);
HT_API char       * uppercase(char * str);
HT_API int          unicode(char ** dst, char * src);



/*************************************************************************/

HT_API SOCKET       tcp_connect_timeout(uint32 rip, int port, int timeout);

/*************************************************************************/

#if __WINDOWS_OS__
HT_API int          gettimeofday(struct timeval* tp, int* tz);
#endif

#ifdef __cplusplus
}
#endif

#endif	//	__H_UTIL_H__



