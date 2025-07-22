/****************************************************************************
 * cgi_common.h
 *
 * purpose:
 *	- cgi common functions header.
 ****************************************************************************/
#ifndef _CGI_COMMON_H
#define _CGI_COMMON_H

#include "cgi.h"
#include "setup/setup.h"
#include "setup/setup_ini.h"

#include "jQuery_language.h"
#include "cgi_encryption.h"

typedef struct _st_cgi {

	char	login_id[32];
	char	login_pwd[32];
	int		login_no;
} ST_CGI;

extern ST_CGI g_cgi;
extern const char* g_errfmt;

#define MAX_PAGE_SZ					(1024*120)

// xwebviewer version
#define JQUERY_HTML_LOGIN						"../html/login.html"
#define JQUERY_HTML_LIVE_MONITORING				"../html/live_fview.html"
#define JQUERY_HTML_SETUP						"../html/websetup.html"
#define JQUERY_HTML_SETUP_AJAX_VIDEO_STREAM		"../html/web_ipc_video_stream.html"

#define JQUERY_HTML_SETUP_CAMERA_SETUP			"../html/setup_camera_setup.html"
#define JQUERY_HTML_SETUP_CAMERA_TAB_IQ			"../html/cam_image_setup_iq.html"
#define JQUERY_HTML_SETUP_CAMERA_TAB_AE			"../html/cam_image_setup_ae.html"
#define JQUERY_HTML_SETUP_CAMERA_TAB_BLC		"../html/cam_image_setup_blc.html"
#define JQUERY_HTML_SETUP_CAMERA_TAB_AWB		"../html/cam_image_setup_awb.html"
#define JQUERY_HTML_SETUP_CAMERA_TAB_DN			"../html/cam_image_setup_daynight.html"





#define JQUERY_HTML_SETUP_AJAX_NET_INTERFACE	"../html/web_net_interface.html"
#define JQUERY_HTML_SETUP_AJAX_NET_PORT			"../html/web_net_port.html"
#define JQUERY_HTML_SETUP_AJAX_NET_RTSP_ONVIF	"../html/web_net_rtsp_onvif.html"


#define JQUERY_HTML_SETUP_AJAX_SYSTEM_INFO		"../html/web_sys_info.html"
#define JQUERY_HTML_SETUP_AJAX_SYSTEM_DATE		"../html/web_sys_date.html"
#define JQUERY_HTML_SETUP_AJAX_SYSTEM_USER		"../html/web_sys_user.html"
#define JQUERY_HTML_SETUP_AJAX_SYSTEM_DEFAULT	"../html/web_sys_default.html"
#define JQUERY_HTML_SETUP_AJAX_SYSTEM_REBOOT	"../html/web_sys_reboot.html"
#define JQUERY_HTML_SETUP_AJAX_SYSTEM_UPGRADE	"../html/web_sys_upgrade.html"


#define JQUERY_HTML_VIDEO_PLAYER					"../html/video_player.html"


extern pthread_mutex_t	g_malloc_lock;




extern char * tmzone_val [TMZONE_MAX];
extern char * tmzone_str_2 [TMZONE_MAX][4];

extern void replace_jquery_include(char* page);

extern int replace_form(char *text,char *src,char *rep);
extern int replace_form_once(char *text,char *src,char *rep);
extern int replace_charset(char *text, int lang);
extern void load_form(char *path, char* form, int lang);
extern int load_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext);

extern void force_redirect(int sec, const char* url);

extern boolean check_session();

extern char* get_model(char* szModel, _CAMSETUP* _r_setup);
extern char* get_base_model(char* szModel, _CAMSETUP* _r_setup);
extern char* get_software_version(char* szVersion, _CAMSETUP* _r_setup);
extern char* get_sensor_name(char* szSensorName);

extern void replace_logo(char* page);

extern void replace_date_string(char* page, int lang);

extern char** get_month_val(int lang);
extern char** get_short_month_val(int lang);
extern char** get_datefmt_val(int lang);
extern char** get_timefmt_val(int lang);
extern char** get_week_val(int lang);
extern char** get_short_week_val(int lang);
extern char** get_lang_val(int lang);
extern char** get_quality_val(int lang);

extern char * get_date_string (int lang, int iDateFormat, char *buf, struct tm *pstTm);
extern char * get_time_string (int lang, int iTimeFormat, char *buf, struct tm *pstTm);


extern void RemoveLastChar(char* src, char cLast);
extern int get_essid_ip_ra0(char* essid, int essidlen, char* ipaddr, int ipaddrlen);
extern int ifconfig_ra0_(int bIsUp, _CAMSETUP* pSetup);
extern int __if_get_mac (const char *dev, uchar * buff);
extern int __if_get_ip(const char *dev, struct in_addr * addr);


extern void replace_software_version(char* page, _CAMSETUP* _r_setup);


#if 1
extern int isp_open();
extern void isp_close(int htty);
extern void isp_write(int hTTY, char * buff, int size);
extern void isp_get_data(int htty, int addr, char* str);
extern int isp_get_data_int(int htty, int addr);
extern int check_isp_version(int* major, int* minor);
#endif

extern void datetime_tzset_2 (_CAMSETUP* r_setup, int index, char* tzstr);
extern int  _file_exist(const char *filename);
extern void replace_java_player_path(char* page);
extern void update_setup(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext);

extern int preview_size(_CAMSETUP *setup, int *width, int *height);
#endif	// _CGI_COMMON_H

