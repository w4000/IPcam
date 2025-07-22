
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];

static void replace_param(char* page, int lang)
{
	char str[1024], buf[1024];
	int port;

	buf[0] = '\0';

	port = r_setup.net.port_tcp == 0 ? 8001 : r_setup.net.port_tcp;
	sprintf(str, "%d", port);
	replace_form(page, "##PARAM_BASEPORT##", str);


	port = r_setup.net.port_web == 0 ? 80 : r_setup.net.port_web;
	sprintf(str, "%d", port);
	replace_form(page, "##PARAM_WEBPORT##", str);

	port = r_setup_ext.rtsp_info.rtsp_port == 0 ? 554 : r_setup_ext.rtsp_info.rtsp_port;
	sprintf(str, "%d", port);
	replace_form(page, "##PARAM_RTSPPORT##", str);




	sprintf(str, "%d", 8001);
	replace_form(page, "##PARAM_DEFAULT_BASEPORT##", str);

	sprintf(str, "%d", 80);
	replace_form(page, "##PARAM_DEFAULT_WEBPORT##", str);
	sprintf(str, "%d", 554);
	replace_form(page, "##PARAM_DEFAULT_RTSPPORT##", str);

}

static void replace_resource_string(char* page, int lang)
{
	char* szRES_PORT_CONFIG[] = RES_ID_ARRAY(RES_PORT_CONFIG);
	char* szRES_BASE_PORT[] = RES_ID_ARRAY(RES_BASE_PORT);
	char* szRES_WEB_PORT[] = RES_ID_ARRAY(RES_WEB_PORT);
#ifdef LIGHTTPD
#endif
	char* szRES_RTSP_PORT[] = RES_ID_ARRAY(RES_RTSP_PORT);
	char* szRES_DEFAULT_PORT[] = RES_ID_ARRAY(RES_DEFAULT_PORT);
	char* szRES_SET_DEFAULT_PORT[] = RES_ID_ARRAY(RES_SET_DEFAULT_PORT);
	char* szRES_MSG_PORT_ERR[] = RES_ID_ARRAY(RES_MSG_PORT_ERR);

	replace_form(page, "##RES_PORT_CONFIG##", szRES_PORT_CONFIG[lang]);
	replace_form(page, "##RES_BASE_PORT##", szRES_BASE_PORT[lang]);
	replace_form(page, "##RES_WEB_PORT##", szRES_WEB_PORT[lang]);
#ifdef LIGHTTPD
#endif
	replace_form(page, "##RES_RTSP_PORT##", szRES_RTSP_PORT[lang]);
	replace_form(page, "##RES_DEFAULT_PORT##", szRES_DEFAULT_PORT[lang]);
	replace_form(page, "##RES_SET_DEFAULT_PORT##", szRES_SET_DEFAULT_PORT[lang]);
	replace_form(page, "##RES_MSG_PORT_ERR##", szRES_MSG_PORT_ERR[lang]);

	char* szRES_WEBGATE_WARNING[] = RES_ID_ARRAY(RES_WEBGATE_WARNING);
	replace_form(page, "##RES_WEBGATE_WARNING##", szRES_WEBGATE_WARNING[lang]);

	char* szRES_SAVE[] = RES_ID_ARRAY(RES_SAVE);
	replace_form(page, "##RES_SAVE##", szRES_SAVE[lang]);
}

static void replace_option(char* page, int lang)
{

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
	load_form(JQUERY_HTML_SETUP_AJAX_NET_PORT, page, lang);

	replace_param(page, lang);
	replace_resource_string(page, lang);
	replace_option(page, lang);

	if (ajax_check_output(page, lang) < 0) {
		if (lang != 0) {
			lang = 0;
			goto retry;
		}
	}

	cgi_end();
	return 0;
}
