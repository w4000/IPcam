

#include "sys_inc.h"
#include "onvif.h"
#include "onvif_event.h"
#include "onvif_timer.h"

#ifdef SCHEDULE_SUPPORT
#include "onvif_schedule.h"
#endif

#ifdef PROFILE_G_SUPPORT
#include "onvif_recording.h"
#endif

/**************************************************************************************/

extern ONVIF_CLS g_onvif_cls;
extern ONVIF_CFG g_onvif_cfg;

static PPSN_CTX* g_timer_fl;
static PPSN_CTX* g_timer_ul;

/**************************************************************************************/

void timer_init()
{
    g_timer_fl = pps_ctx_fl_init(32, sizeof(TIMER_UA), TRUE);
    g_timer_ul = pps_ctx_ul_init(g_timer_fl, TRUE);
}

void timer_deinit()
{
    if (g_timer_ul)
	{
	    pps_ul_free(g_timer_ul);
	    g_timer_ul = NULL;
	}

	if (g_timer_fl)
	{
	    pps_fl_free(g_timer_fl);
	    g_timer_fl = NULL;
	}
}

void * timer_add(uint32 interval, timer_func func, void * param, int mode)
{
    TIMER_UA * p_tua = (TIMER_UA *)pps_fl_pop(g_timer_fl);
	if (p_tua)
	{
		memset(p_tua, 0, sizeof(TIMER_UA));

		p_tua->interval = interval;
		p_tua->func = func;
		p_tua->mode = mode;
		p_tua->param = param;
		p_tua->start = time(NULL);

		pps_ctx_ul_add(g_timer_ul, p_tua);

		return p_tua;
	}

	return NULL;
}

void timer_del(void * p_tua)
{
    if (NULL == p_tua)
    {
        return;
    }
    
    pps_ctx_ul_del(g_timer_ul, p_tua);
	
	memset(p_tua, 0, sizeof(TIMER_UA));
	
	pps_fl_push(g_timer_fl, p_tua);
}

void timer_task()
{
    time_t curtime = time(NULL);
    
    TIMER_UA * p_tua = (TIMER_UA *)pps_lookup_start(g_timer_ul);
    while (p_tua)
    {
        if (curtime - p_tua->start >= p_tua->interval)
        {
            if (p_tua->func)
            {
                p_tua->func(p_tua->param);
            }

            if (p_tua->mode == TIMER_MODE_REPEAT)
            {
                p_tua->start = curtime;
            }
            else
            {
                TIMER_UA * p_next = (TIMER_UA *)pps_lookup_next(g_timer_ul, p_tua);

                pps_ctx_ul_del_unlock(g_timer_ul, p_tua);

                p_tua = p_next;
            }
        }
        
        p_tua = (TIMER_UA *)pps_lookup_next(g_timer_ul, p_tua);
    }
    pps_lookup_end(g_timer_ul);
}

/**************************************************************************************/

void onvif_timer()
{
	timer_task();

	onvif_event_renew();

#ifdef SCHEDULE_SUPPORT
    onvif_DoSchedule();
#endif

#ifdef PROFILE_G_SUPPORT
    onvif_SearchTimeoutCheck();
#endif
}


#if	__WINDOWS_OS__

#pragma comment(lib, "winmm.lib")

#ifdef _WIN64
void CALLBACK onvif_win_timer(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
#else
void CALLBACK onvif_win_timer(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
#endif
{
    OIMSG msg;
	memset(&msg, 0, sizeof(OIMSG));
	
	msg.msg_src = ONVIF_TIMER_SRC;
	
	hqBufPut(g_onvif_cls.msg_queue, (char *)&msg);
}

void onvif_timer_init()
{
    timer_init();
    
	g_onvif_cls.timer_id = timeSetEvent(1000, 0, onvif_win_timer, 0, TIME_PERIODIC);
}

void onvif_timer_deinit()
{
	timeKillEvent(g_onvif_cls.timer_id);

	timer_deinit();
}

#else

void * onvif_timer_task(void * argv)
{
	struct timeval tv;	
	OIMSG msg;
	memset(&msg, 0, sizeof(OIMSG));
	
	while (g_onvif_cls.sys_timer_run == 1)
	{		
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		
		select(1, NULL, NULL, NULL, &tv);
    	
    	msg.msg_src = ONVIF_TIMER_SRC;
    	
    	hqBufPut(g_onvif_cls.msg_queue, (char *)&msg);
	}

	g_onvif_cls.timer_id = 0;
	

	return NULL;
}

void onvif_timer_init()
{
    pthread_t tid;

    timer_init();
    
	g_onvif_cls.sys_timer_run = 1;

	tid = sys_os_create_thread((void *)onvif_timer_task, NULL);
	if (tid == 0)
	{
		return;
	}

    g_onvif_cls.timer_id = (uint32)tid;

}

void onvif_timer_deinit()
{
	g_onvif_cls.sys_timer_run = 0;

	while (g_onvif_cls.timer_id != 0)
	{
		usleep(10*1000);
	}

	timer_deinit();
}

#endif



