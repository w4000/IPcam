#ifndef __ONVIFSERVER_H__
#define __ONVIFSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "setup/setup.h"

int onvifserver_init(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext);
void onvifserver_deinit();

void onvifserver_set_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext);
void onvifserver_get_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext);
void onvifserver_get_rtsp_uri(char *uri, int ch);

void video_encoder_ch_to_token(int ch, char *token);
int video_encoder_token_to_ch(char *token);

typedef enum {
	ONVIF_EVENT_MOTION,
	ONVIF_EVENT_MOTION_REGION,	// Only Internal
	ONVIF_EVENT_SENSOR_IN,
	ONVIF_EVENT_ALARM_OUT,
	ONVIF_EVENT_COUNT
} ONVIF_EVENT_E;




void onvifserver_notify_event(ONVIF_EVENT_E event, int value);
void onvifserver_init_NotificationMessage(const char *p_eua, char *topic_expression);

void onvifserver_update_basic();
void onvifserver_update_datetime();
void onvifserver_update_user();
void onvifserver_update_scopes();
void onvifserver_update_network_protocol();
void onvifserver_update_ntp();
void onvifserver_update_video_encoder();
void onvifserver_update_isp();

#ifdef __cplusplus
}
#endif

#endif


