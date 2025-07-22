

#ifndef	__SYS_INC_H__
#define	__SYS_INC_H__

#if defined(_WIN32) || defined(_WIN64)
#define __WINDOWS_OS__	1
#define __LINUX_OS__	0
#else
#define __WINDOWS_OS__	0
#define __LINUX_OS__	1
#endif

#if __WINDOWS_OS__
	#ifdef HT_EXPORTS
		#define HT_API __declspec(dllexport)
	#else
		#define HT_API __declspec(dllimport)
	#endif
	
	#ifdef HT_STATIC
		#undef  HT_API 	
		#define HT_API
	#endif
#else
	#define HT_API
#endif

/***************************************************************************************/
typedef int 			int32;
typedef unsigned int 	uint32;
typedef unsigned short 	uint16;
typedef unsigned char 	uint8;


/***************************************************************************************/
#if	__WINDOWS_OS__

#include "stdafx.h"

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <process.h>    /* _beginthread, _endthread */
#include <iphlpapi.h>
#include <assert.h>

#define sleep(x) 		    Sleep((x) * 1000)
#define usleep(x) 		    Sleep((x) / 1000)

#define strcasecmp 		    stricmp
#define strncasecmp 	    strnicmp
#define snprintf 		    _snprintf

#define pthread_t 		    DWORD

typedef __int64             int64;
typedef unsigned __int64    uint64;

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#elif __LINUX_OS__

#include <sys/types.h>
#include <sys/ipc.h>

#ifndef ANDROID
#include <sys/sem.h>
#endif

#include <semaphore.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#ifndef IOS
#include <linux/netlink.h>
#include <netinet/udp.h>
#include <sys/prctl.h>
#include <sys/epoll.h>
#endif

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>
#include <dlfcn.h>
#include <dirent.h>

#ifdef IOS
#include <ifaddrs.h>
#endif

#include "onvifserver.h"

#ifndef BOOL_DEF
typedef int				    BOOL;
#define BOOL_DEF
#endif

typedef unsigned int	    UINT;
typedef int				    SOCKET;
typedef uint8 *			    LPBYTE;

typedef long long           int64;
typedef unsigned long long  uint64;


#define TRUE			    1
#define	FALSE			    0

#define	closesocket 	    close

#if !(defined(ANDROID) || defined(IOS))
#define HANDLE	sem_t*
#endif

#endif

/*************************************************************************/
#include "ppstack.h"
#include "word_analyse.h"
#include "sys_buf.h"
#include "util.h"


#ifdef __cplusplus
extern "C" {
#endif

HT_API void       * sys_os_create_mutex();
HT_API void       * sys_os_create_sig();

HT_API void         sys_os_destroy_sig_mutex(void * ptr);

HT_API int          sys_os_mutex_enter(void * p_sem);
HT_API void         sys_os_mutex_leave(void * p_sem);

HT_API int          sys_os_sig_wait(void * p_sig);
HT_API int          sys_os_sig_wait_timeout(void * p_sig, uint32 ms);
HT_API void         sys_os_sig_sign(void * p_sig);

HT_API pthread_t    sys_os_create_thread(void * thread_func, void * argv);

HT_API uint32       sys_os_get_ms();
HT_API uint32       sys_os_get_uptime();
HT_API char       * sys_os_get_socket_error();
HT_API int          sys_os_get_socket_error_num();

/*************************************************************************/
HT_API void       * xmalloc(size_t size, const char * pFileName, int line);
HT_API void         xfree(void * ptr, const char * pFileName, int line);

#define XMALLOC(x) 	xmalloc(x, __FILE__, __LINE__)
#define XFREE(x) 	xfree(x, __FILE__, __LINE__)

HT_API void         mem_log_quit();

#ifdef __cplusplus
}
#endif

#endif	// __SYS_INC_H__



