

/***************************************************************************************/
#include "sys_inc.h"


/***************************************************************************************/
HT_API void * sys_os_create_mutex()
{
	void * p_mutex = NULL;

#ifdef IOS
#elif __WINDOWS_OS__
#elif __LINUX_OS__
	
	int ret;

	p_mutex = (sem_t *)malloc(sizeof(sem_t));
	ret = sem_init((sem_t *)p_mutex, 0, 1);
	if (ret != 0)
	{
		free(p_mutex);
		return NULL;
	}

#endif

	return p_mutex;
}

HT_API void * sys_os_create_sig()
{	
	void * p_sig = NULL;
    
#ifdef IOS
#elif __WINDOWS_OS__
#elif __LINUX_OS__

	int ret;

	p_sig = malloc(sizeof(sem_t));
	ret = sem_init((sem_t *)p_sig, 0, 0);
	if (ret != 0)
	{
		free(p_sig);
		return NULL;
	}

#endif

	return p_sig;
}

HT_API void sys_os_destroy_sig_mutex(void * ptr)
{
	if (ptr == NULL)
	{
		return;
	}
	
#ifdef IOS
#elif __WINDOWS_OS__
#elif __LINUX_OS__

	sem_destroy((sem_t *)ptr);
	free(ptr);

#endif
}

HT_API int sys_os_mutex_enter(void * p_sem)
{
	int ret;
	
	if (p_sem == NULL)
	{
		return -1;
	}
	
#if	__LINUX_OS__

	ret = sem_wait((sem_t *)p_sem);
	if (ret != 0)
	{
		return -1;
	}
	
#elif __WINDOWS_OS__
#endif

	return 0;
}

HT_API void sys_os_mutex_leave(void * p_sem)
{
	if (p_sem == NULL)
	{
		return;
	}
	
#if	__LINUX_OS__

	sem_post((sem_t *)p_sem);
	
#elif __WINDOWS_OS__
#endif
}

HT_API int sys_os_sig_wait(void * p_sig)
{
	int ret;
		
	if (p_sig == NULL)
	{
		return -1;
	}
	
#if	__LINUX_OS__

	ret = sem_wait((sem_t *)p_sig);
	if (ret != 0)
	{
		return -1;
	}
	
#elif __WINDOWS_OS__
#endif

	return 0;
}

HT_API int sys_os_sig_wait_timeout(void * p_sig, uint32 ms)
{
#ifdef IOS
    
	if (p_sig == NULL)
	{
		return -1;
	}
	
    while (ms > 0)
    {
        if (sem_trywait((sem_t *)p_sig) == 0)
        {
            return 0;
        }
        
        usleep(1000);
        
        ms -= 1;
    }
    
    return -1;
    
#elif __LINUX_OS__

    int ret;
	struct timespec ts;
	struct timeval tt;

	if (p_sig == NULL) 
	{
		return -1;
	}
	
	gettimeofday(&tt,NULL);

	tt.tv_sec = tt.tv_sec + ms / 1000;
	tt.tv_usec = tt.tv_usec + (ms % 1000) * 1000;
	tt.tv_sec += tt.tv_usec / (1000 * 1000);
	tt.tv_usec = tt.tv_usec % (1000 * 1000);
	
	ts.tv_sec = tt.tv_sec;
	ts.tv_nsec = tt.tv_usec * 1000;

	ret = sem_timedwait((sem_t *)p_sig, &ts);
	if (ret == -1 && errno == ETIMEDOUT)
	{
		return -1;
	}	
	else
	{
		return 0;
	}
	
#elif __WINDOWS_OS__
#endif

	return 0;
}

HT_API void sys_os_sig_sign(void * p_sig)
{
	if (p_sig == NULL)
	{
		return;
	}
	
#if	__LINUX_OS__

	sem_post((sem_t *)p_sig);
	
#elif __WINDOWS_OS__
#endif
}

HT_API pthread_t sys_os_create_thread(void * thread_func, void * argv)
{
	pthread_t tid = 0;

#if	__LINUX_OS__

	int ret = pthread_create(&tid, NULL, (void *(*)(void *))thread_func, argv);
	if (ret != 0)
	{
	}

	pthread_detach(tid);

#elif __WINDOWS_OS__
#endif

	return tid;
}

HT_API uint32 sys_os_get_ms()
{
	uint32 ms = 0;

#if __LINUX_OS__

	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	ms = tv.tv_sec * 1000 + tv.tv_usec/1000;

#elif __WINDOWS_OS__
#endif

	return ms;
}

HT_API uint32 sys_os_get_uptime()
{
	uint32 upt = 0;

#ifdef ANDROID

    upt = (uint32)time(NULL);
    
#elif __LINUX_OS__

    int rlen;
	char bufs[512];
	char * ptr;	
	FILE * file;

	file = fopen("/proc/uptime", "rb");
	if (NULL == file)
	{
		return 0;
    }
    
	rlen = fread(bufs, 1, sizeof(bufs)-1, file);
	fclose(file);

	if (rlen <= 0)
	{
		return (uint32)time(NULL);
	}
	
	bufs[rlen] = '\0';
	ptr = bufs;
	
	while (*ptr != '\0' && *ptr != ' ' && *ptr != '\r' && *ptr != '\n')
	{
	    ptr++;
	}
	
	if (*ptr == ' ')
	{
		*ptr = '\0';
	}	
	else
	{
		return (uint32)time(NULL);
    }
    
	upt = (uint32)strtod(bufs, NULL);

#elif __WINDOWS_OS__
#endif

	return upt;
}

HT_API char * sys_os_get_socket_error()
{
	char * p_estr = NULL;
	
#if __LINUX_OS__

	p_estr = strerror(errno);
	
#elif __WINDOWS_OS__
#endif

	return p_estr;
}

HT_API int sys_os_get_socket_error_num()
{
	int sockerr;
	
#if __LINUX_OS__

	sockerr = errno;
	
#elif __WINDOWS_OS__
#endif

	return sockerr;
}

static FILE * pMemLogFile = NULL;
static void * logMemFileMutex = NULL;

HT_API void mem_log_quit()
{
	if (pMemLogFile)
	{
		fclose(pMemLogFile);
		pMemLogFile = NULL;
	}

	if (logMemFileMutex)
	{
		sys_os_destroy_sig_mutex(logMemFileMutex);
		logMemFileMutex = NULL;
	}
}


HT_API void * xmalloc(size_t size, const char * pFileName, int line)
{

	void * ptr = malloc(size);
	return ptr;
}

HT_API void xfree(void * ptr, const char * pFileName, int line)
{
	free(ptr);
}



