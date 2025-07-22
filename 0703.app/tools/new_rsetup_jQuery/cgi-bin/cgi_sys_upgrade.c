
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

#define PROGRESS_FILE	"/tmp/progress"

static char page[MAX_PAGE_SZ];

static void replace_param(char* page, int lang)
{
	char model[64], sensor[32], wait_cnt[32];

	replace_form_once(page, "##PARAM_MODEL##", get_model(model, &r_setup));
	replace_software_version(page, &r_setup);

	sprintf(sensor, model_name());

	replace_form(page, "##PARAM_SENSOR_NAME##", sensor);

	snprintf(wait_cnt, sizeof(wait_cnt), "%d", REBOOT_TIME);
	replace_form(page, "##PARAM_WAIT_CNT##", wait_cnt);

}

static void replace_resource_string(char* page, int lang)
{
	char* szRES_UPGRADE[] = RES_ID_ARRAY(RES_UPGRADE);
	char* szRES_MSG_SELECT_UPGRADE_FILE[] = RES_ID_ARRAY(RES_MSG_SELECT_UPGRADE_FILE);
	char* szRES_MSG_INVALID_UPGRADE_FILE[] = RES_ID_ARRAY(RES_MSG_INVALID_UPGRADE_FILE);
	char* szRES_MSG_UPGRADE1[] = RES_ID_ARRAY(RES_MSG_UPGRADE1);
	char* szRES_MSG_UPGRADE2[] = RES_ID_ARRAY(RES_MSG_UPGRADE2);
	char* szRES_MSG_UPGRADE_PROGRESS[] = RES_ID_ARRAY(RES_MSG_UPGRADE_PROGRESS);
	char* szRES_MSG_UPGRADE_END_1[] = RES_ID_ARRAY(RES_MSG_UPGRADE_END_1);
	char* szRES_MSG_UPGRADE_END_2[] = RES_ID_ARRAY(RES_MSG_UPGRADE_END_2);
	char* szRES_MSG_UPGRADE_DONE[] = RES_ID_ARRAY(RES_MSG_UPGRADE_DONE);
	char* szRES_MSG_UPGRADE_FAIL[] = RES_ID_ARRAY(RES_MSG_UPGRADE_FAIL);
	char* szRES_MSG_REBOOT_TIME[] = RES_ID_ARRAY(RES_MSG_REBOOT_TIME);
	char* szRES_UPGRADE_CONFIRM[] = RES_ID_ARRAY(RES_UPGRADE_CONFIRM);
	char* szRES_UPGRADE_FAIL[] = RES_ID_ARRAY(RES_UPGRADE_FAIL);
	char* szRES_UPGRADE_OK[] = RES_ID_ARRAY(RES_UPGRADE_OK);
	char* szRES_PROGRESS_STATUS[] = RES_ID_ARRAY(RES_PROGRESS_STATUS);
	char* szRES_STATUS[] = RES_ID_ARRAY(RES_STATUS);
	char* szRES_UPLOAD_PROGRESS[] = RES_ID_ARRAY(RES_UPLOAD_PROGRESS);
	char* szRES_WRITING_PROGRESS[] = RES_ID_ARRAY(RES_WRITING_PROGRESS);
	char* szRES_SEC[] = RES_ID_ARRAY(RES_SEC);
	replace_form(page, "##RES_UPGRADE##", szRES_UPGRADE[lang]);
	replace_form(page, "##RES_MSG_SELECT_UPGRADE_FILE##", szRES_MSG_SELECT_UPGRADE_FILE[lang]);
	replace_form(page, "##RES_MSG_INVALID_UPGRADE_FILE##", szRES_MSG_INVALID_UPGRADE_FILE[lang]);
	replace_form(page, "##RES_MSG_UPGRADE1##", szRES_MSG_UPGRADE1[lang]);
	replace_form(page, "##RES_MSG_UPGRADE2##", szRES_MSG_UPGRADE2[lang]);
	replace_form(page, "##RES_MSG_UPGRADE_PROGRESS##", szRES_MSG_UPGRADE_PROGRESS[lang]);
	replace_form(page, "##RES_MSG_UPGRADE_END_1##", szRES_MSG_UPGRADE_END_1[lang]);
	replace_form(page, "##RES_MSG_UPGRADE_END_2##", szRES_MSG_UPGRADE_END_2[lang]);
	replace_form(page, "##RES_MSG_UPGRADE_DONE##", szRES_MSG_UPGRADE_DONE[lang]);
	replace_form(page, "##RES_MSG_UPGRADE_FAIL##", szRES_MSG_UPGRADE_FAIL[lang]);
	replace_form(page, "##RES_MSG_REBOOT_TIME##", szRES_MSG_REBOOT_TIME[lang]);
	replace_form(page, "##RES_UPGRADE_CONFIRM##", szRES_UPGRADE_CONFIRM[lang]);
	replace_form(page, "##RES_UPGRADE_FAIL##", szRES_UPGRADE_FAIL[lang]);
	replace_form(page, "##RES_UPGRADE_OK##", szRES_UPGRADE_OK[lang]);
	replace_form(page, "##RES_PROGRESS_STATUS##", szRES_PROGRESS_STATUS[lang]);
	replace_form(page, "##RES_STATUS##", szRES_STATUS[lang]);
	replace_form(page, "##RES_UPLOAD_PROGRESS##", szRES_UPLOAD_PROGRESS[lang]);
	replace_form(page, "##RES_WRITING_PROGRESS##", szRES_WRITING_PROGRESS[lang]);
	replace_form(page, "##RES_SEC##", szRES_SEC[lang]);

	char* szRES_MODEL[] = RES_ID_ARRAY(RES_MODEL);
	replace_form_once(page, "##RES_MODEL##", szRES_MODEL[lang]);

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


	if(access(PROGRESS_FILE, 0) == 0) {
		remove(PROGRESS_FILE);
	}


	load_setup (&r_setup, &r_setup_ext);
	int lang = r_setup.lang;

retry:
	load_form(JQUERY_HTML_SETUP_AJAX_SYSTEM_UPGRADE, page, lang);

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
