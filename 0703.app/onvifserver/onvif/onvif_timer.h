

#ifndef ONVIF_TIMER_H
#define ONVIF_TIMER_H


#define TIMER_MODE_SINGLE   0
#define TIMER_MODE_REPEAT   1

typedef void (*timer_func)(void *);

typedef struct
{
    time_t      start;
    uint32      interval;
    int         mode;
    timer_func  func;
    void *      param;
} TIMER_UA;

#ifdef __cplusplus
extern "C" {
#endif

void *  timer_add(uint32 interval, timer_func func, void * param, int mode);
void    timer_del(void * p_tua);

void    onvif_timer_init();
void    onvif_timer_deinit();
void    onvif_timer();

#ifdef __cplusplus
}
#endif

#endif


