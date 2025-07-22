

#ifndef ONVIF_API_H
#define ONVIF_API_H


#define ONVIF_VERSION_STRING    "V8.5"


#ifdef __cplusplus
extern "C" {
#endif

int   onvif_start1();
void   onvif_start(int);
void   onvif_stop();

void * onvif_task(void * argv);
void   onvif_free_device();

#ifdef __cplusplus
}
#endif

#endif


