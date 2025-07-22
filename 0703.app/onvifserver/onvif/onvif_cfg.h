

#ifndef ONVIF_CONFIG_H
#define ONVIF_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

void onvif_load_cfg(const char * filename);
void onvif_parse_cfg(char * xml_buff, int rlen);
BOOL onvif_read_device_uuid(char * buff, int len);
BOOL onvif_save_device_uuid(char * buff);

#ifdef PROFILE_Q_SUPPORT
int  onvif_read_device_state();
BOOL onvif_save_device_state(int state);
#endif

void onvif_save_profile_cfg(const char * filename, ONVIF_PROFILE *profiles);
void onvif_load_profile_cfg(const char * filename);
void onvif_parse_profile_cfg(char * xml_buff, int rlen);

#ifdef __cplusplus
}
#endif

#endif


