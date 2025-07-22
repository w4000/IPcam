
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];

static void replace_param(char* page, int lang)
{
	// int i;
	char str[1024], buf[1024];

	buf[0] = '\0';
	str[0] = '\0';


	sprintf(str, "%d", r_setup_ext.rtsp_info.use_rtsp ? 1 : 0);
	replace_form(page, "##PARAM_USE_RTSP##", str);

	sprintf(str, "%d", r_setup_ext.rtsp_info.use_security ? 1 : 0);
	replace_form(page, "##PARAM_RTSP_SECURITY##", str);

	if (r_setup_ext.rtsp_info.rtsp_port == 0)
		r_setup_ext.rtsp_info.rtsp_port = 554;

	sprintf(str, "%d", r_setup_ext.rtsp_info.rtsp_port);
	replace_form(page, "##PARAM_RTSP_PORT##", str);



	{
		char essid[64], ipaddr[32], ipaddr_wifi[32];
		memset(essid, 0, sizeof(essid));
		memset(ipaddr, 0, sizeof(ipaddr));
		memset(ipaddr_wifi, 0, sizeof(ipaddr_wifi));

		strcpy(ipaddr, inet_ntoa(r_setup.net.ip));

#if	0
#else

		for(int i = 0; i < 3; i++) {

#if 0
#else 
			sprintf(str, "rtsp://%s/%s", ipaddr, r_setup_ext.rtsp_uri[i]);
#endif 
			sprintf(buf, "##PARAM_RTSP_ADDR%d##", i + 1);

			replace_form(page, buf, str);
		}
#endif

		replace_form(page, "##PARAM_IPADDR##", ipaddr);


		int bIsWifi = 0;

		sprintf(str, "%d", bIsWifi);
		replace_form(page, "##PARAM_RTSP_WIFI##", str);
		replace_form(page, "##PARAM_IPADDR_WIFI##", ipaddr_wifi);
	}

	// Onvif
	sprintf(str, "%d", r_setup_ext.onvif_security ? 1 : 0);
	replace_form(page, "##PARAM_ONVIF_SECURITY##", str);
#if defined(USE_NEW_ONVIF)
	sprintf(str, "%d", r_setup_ext.onvif_use_https ? 1 : 0);
	replace_form(page, "##PARAM_ONVIF_USE_HTTPS##", str);
#if defined(USE_ONVIF_PORT_PROXY)
	replace_form(page, "##PARAM_ONVIF_PORT##", "80");
#else
	sprintf(str, "%d", r_setup_ext.onvif_port);
	replace_form(page, "##PARAM_ONVIF_PORT##", str);
#endif	/* USE_ONVIF_PORT_PROXY */
#endif	/* USE_NEW_ONVIF */


#ifdef USE_NEW_ONVIF
	replace_form_once(page, "<!--##USE_OLD_ONVIF_START##-->", "<!--");
	replace_form_once(page, "<!--##USE_OLD_ONVIF_END##-->", "-->");
#else
	replace_form_once(page, "<!--##USE_NEW_ONVIF_START##-->", "<!--");
	replace_form_once(page, "<!--##USE_NEW_ONVIF_END##-->", "-->");
#endif
}

static void replace_resource_string(char* page, int lang)
{
	char* szRES_MAIN_STREAM[] = RES_ID_ARRAY(RES_MAIN_STREAM);
	char* szRES_SUB_STREAM[] = RES_ID_ARRAY(RES_SUB_STREAM);
	char* szRES_SUB2_STREAM[] = RES_ID_ARRAY(RES_SUB2_STREAM);
	replace_form(page, "##RES_MAIN_STREAM##", szRES_MAIN_STREAM[lang]);
	replace_form(page, "##RES_SUB_STREAM##", szRES_SUB_STREAM[lang]);
	replace_form(page, "##RES_SUB2_STREAM##", szRES_SUB2_STREAM[lang]);

	char* szRES_RTSP_CONFIG[] = RES_ID_ARRAY(RES_RTSP_CONFIG);
	char* szRES_USE_RTSP[] = RES_ID_ARRAY(RES_USE_RTSP);
	char* szRES_RTSP_SECURITY[] = RES_ID_ARRAY(RES_RTSP_SECURITY);
	char* szRES_ACCESS_CONTROL[] = RES_ID_ARRAY(RES_ACCESS_CONTROL);
	char* szRES_RTSP_PORT[] = RES_ID_ARRAY(RES_RTSP_PORT);
	char* szRES_RTSP_ADDRESS[] = RES_ID_ARRAY(RES_RTSP_ADDRESS);
	char* szRES_ONVIF_CONFIG[] = RES_ID_ARRAY(RES_ONVIF_CONFIG);
	char* szRES_ONVIF_SECURITY[] = RES_ID_ARRAY(RES_ONVIF_SECURITY);
	char* szRES_ONVIF_PORT[] = RES_ID_ARRAY(RES_PORT);

	char* szRES_MSG_RTSP_PORT[] = RES_ID_ARRAY(RES_MSG_INVALID_PORT);
	char* szRES_MSG_ONVIF_PORT[] = RES_ID_ARRAY(RES_MSG_INVALID_PORT);

	replace_form(page, "##RES_MSG_RTSP_PORT##", szRES_MSG_RTSP_PORT[lang]);
	replace_form(page, "##RES_MSG_ONVIF_PORT##", szRES_MSG_ONVIF_PORT[lang]);
	replace_form(page, "##RES_RTSP_CONFIG##", szRES_RTSP_CONFIG[lang]);
	replace_form(page, "##RES_USE_RTSP##", szRES_USE_RTSP[lang]);
	replace_form(page, "##RES_RTSP_SECURITY##", szRES_RTSP_SECURITY[lang]);
	replace_form(page, "##RES_ACCESS_CONTROL##", szRES_ACCESS_CONTROL[lang]);
	replace_form(page, "##RES_RTSP_PORT##", szRES_RTSP_PORT[lang]);
	replace_form(page, "##RES_RTSP_ADDRESS##", szRES_RTSP_ADDRESS[lang]);
	replace_form(page, "##RES_ONVIF_CONFIG##", szRES_ONVIF_CONFIG[lang]);
	replace_form(page, "##RES_ONVIF_SECURITY##", szRES_ONVIF_SECURITY[lang]);
	replace_form(page, "##RES_ONVIF_PORT##", szRES_ONVIF_PORT[lang]);

	char* szRES_WEBGATE_WARNING[] = RES_ID_ARRAY(RES_WEBGATE_WARNING);
	replace_form(page, "##RES_WEBGATE_WARNING##", szRES_WEBGATE_WARNING[lang]);

	char* szRES_SAVE[] = RES_ID_ARRAY(RES_SAVE);
	replace_form(page, "##RES_SAVE##", szRES_SAVE[lang]);
}

int main(void)
{

	cgi_init();
	cgi_session_start();

	cgi_process_form();
	cgi_init_headers();

	if (!check_session()) {
		cgi_end();
		return 0;
	}

	load_setup (&r_setup, &r_setup_ext);
	int lang = r_setup.lang;

retry:
	load_form(JQUERY_HTML_SETUP_AJAX_NET_RTSP_ONVIF, page, lang);

	replace_param(page, lang);
	replace_resource_string(page, lang);

	if (ajax_check_output(page, lang) < 0) {
		if (lang != 0) {
			lang = 0;
			goto retry;
		}
	}

	cgi_end();
	return 0;
}
