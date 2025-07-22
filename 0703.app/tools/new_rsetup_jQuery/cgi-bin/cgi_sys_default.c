
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];

static void replace_param(char* page, int lang)
{
	char wait_cnt[32];
	snprintf(wait_cnt, sizeof(wait_cnt), "%d", REBOOT_TIME);
	replace_form(page, "##PARAM_WAIT_CNT##", wait_cnt);
}

static void replace_resource_string(char* page, int lang)
{
	char* szRES_SYSTEM_DEFAULT[] = RES_ID_ARRAY(RES_SYSTEM_DEFAULT);
	replace_form(page, "##RES_SYSTEM_DEFAULT##", szRES_SYSTEM_DEFAULT[lang]);

	char* szRES_DEFAULT[] = RES_ID_ARRAY(RES_DEFAULT);
	char* szRES_DEFAULT_CONFIRM[] = RES_ID_ARRAY(RES_DEFAULT_CONFIRM);
	char* szRES_MSG_DEFAULT[] = RES_ID_ARRAY(RES_MSG_DEFAULT);
	char* szRES_KEEP_NETWORK[] = RES_ID_ARRAY(RES_KEEP_NETWORK);
	char* szRES_REBOOT[] = RES_ID_ARRAY(RES_REBOOT);
	char* szRES_WARNING[] = RES_ID_ARRAY(RES_WARNING);
	replace_form(page, "##RES_DEFAULT##", szRES_DEFAULT[lang]);
	replace_form(page, "##RES_DEFAULT_CONFIRM##", szRES_DEFAULT_CONFIRM[lang]);
	replace_form(page, "##RES_MSG_DEFAULT##", szRES_MSG_DEFAULT[lang]);
	replace_form(page, "##RES_KEEP_NETWORK##", szRES_KEEP_NETWORK[lang]);
	replace_form(page, "##RES_REBOOT##", szRES_REBOOT[lang]);
	replace_form(page, "##RES_WARNING##", szRES_WARNING[lang]);

	char* szRES_MSG_REBOOT_TIME[] = RES_ID_ARRAY(RES_MSG_REBOOT_TIME);
	char* szRES_SEC[] = RES_ID_ARRAY(RES_SEC);
	replace_form(page, "##RES_MSG_REBOOT_TIME##", szRES_MSG_REBOOT_TIME[lang]);
	replace_form(page, "##RES_SEC##", szRES_SEC[lang]);

	char* szRES_MSG_UPGRADE_END_2[] = RES_ID_ARRAY(RES_MSG_UPGRADE_END_2);
	replace_form(page, "##RES_MSG_UPGRADE_END_2##", szRES_MSG_UPGRADE_END_2[lang]);

	char* szRES_OK[] = RES_ID_ARRAY(RES_OK);
	char* szRES_CANCEL[] = RES_ID_ARRAY(RES_CANCEL);
	replace_form(page, "##RES_OK##", szRES_OK[lang]);
	replace_form(page, "##RES_CANCEL##", szRES_CANCEL[lang]);

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
	load_form(JQUERY_HTML_SETUP_AJAX_SYSTEM_DEFAULT, page, lang);

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
