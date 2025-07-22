
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

/* DB MANAGER - START */
#include "lib/dbmanager.h"

const char *preFixNewLine = "<tr>";
const char *preFixKey = "<td class=\"sub_key_right\">";
const char *preFixValue = "<td align=\"center\" class=\"sub_value\">";
const char *postFix = "</td>";
const char *postFixNewLine = "</tr>";

static char g_buf[4096];
/* DB MANAGER - END */

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];

#include <sys/sysinfo.h>

int callback(void *data, int argc, char **argv, char **azColName)
{
	static int num = 1;
	char str[4096];
	char buf[4096];

	 

	strcat(buf, preFixNewLine);
	memset(buf, 0, sizeof(buf));
	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%s%d%s", preFixKey, num, postFix);
	strcat(buf, str);
	// todo : IP Addrees
	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%s%s%s", preFixValue, argv[0], postFix);
	strcat(buf, str);
	// todo : Stream type
	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%s%s%s", preFixValue, argv[1], postFix);
	strcat(buf, str);
	// todo : Protocol
	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%s%s%s", preFixValue, argv[2], postFix);
	strcat(buf, str);
	// todo : Access time
	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%s%s%s", preFixValue, argv[3], postFix);
	strcat(buf, str);
	strcat(buf, postFixNewLine);

	num++;
	strcat(g_buf, buf);

	return 0;
}


static void replace_param(char *page, int lang)
{
	char str[4096];
	char buf[4096];
	char model[64];
	char base_model[64];
	char *selptr;
	int i;

	buf[0] = '\0';
	sprintf(buf, "%s", get_model(model, &r_setup));
	if (strcmp(get_model(model, &r_setup), get_base_model(base_model, &r_setup)) != 0)
	{
		buf[0] = '\0';
		sprintf(buf, "%s", get_model(model, &r_setup));
	}
	replace_form_once(page, "##PARAM_MODEL##", buf);
	replace_form_once(page, "##PARAM_SW_VERSION##", get_software_version(buf, &r_setup));

	char **lang_val = get_lang_val(0);

	buf[0] = '\0';
	for (i = 0; i < MAX_LANGUAGE; i++)
	{
		selptr = (r_setup.lang == i) ? "selected" : "";
		sprintf(str, "<option %s value=\"%d\">%s</option>\n", selptr, i, lang_val[i]);
		strcat(buf, str);
	}


	replace_form_once(page, "##PARAM_LANGUAGE##", buf);



	buf[0] = '\0';
	replace_form_once(page, "##PARAM_WAIT_CNT##", "20");
}

static void replace_option(char *page, int lang)
{
}

static void replace_resource_string(char *page, int lang)
{
	char *szRES_SYSTEM_INFO[] = RES_ID_ARRAY(RES_SYSTEM_INFO);
	replace_form(page, "##RES_SYSTEM_INFO##", szRES_SYSTEM_INFO[lang]);

	char *szRES_MODEL[] = RES_ID_ARRAY(RES_MODEL);
	char *szRES_VERSION[] = RES_ID_ARRAY(RES_VERSION);
	replace_form_once(page, "##RES_MODEL##", szRES_MODEL[lang]);
	replace_form(page, "##RES_VERSION##", szRES_VERSION[lang]);

	char *szRES_LANGUAGE[] = RES_ID_ARRAY(RES_LANGUAGE);
	replace_form_once(page, "##RES_LANGUAGE##", szRES_LANGUAGE[lang]);

	char *szRES_SAVE[] = RES_ID_ARRAY(RES_SAVE);
	replace_form(page, "##RES_SAVE##", szRES_SAVE[lang]);


	char *szRES_NO[] = RES_ID_ARRAY(RES_NO);
	replace_form(page, "##RES_NO##", szRES_NO[lang]);

	char *szRES_IP_ADDRESS[] = RES_ID_ARRAY(RES_IP_ADDRESS);
	replace_form(page, "##RES_IP_ADDRESS##", szRES_IP_ADDRESS[lang]);


	char *szRES_VIDEO_STREAM[] = RES_ID_ARRAY(RES_VIDEO_STREAM);
	replace_form(page, "##RES_VIDEO_STREAM##", szRES_VIDEO_STREAM[lang]);





	char *szRES_OK[] = RES_ID_ARRAY(RES_OK);
	replace_form_once(page, "##RES_OK##", szRES_OK[lang]);
	char *szRES_NETWORK[] = RES_ID_ARRAY(RES_NETWORK);
	replace_form(page, "##RES_NETWORK##", szRES_NETWORK[lang]);
	char *szRES_SYSTEM[] = RES_ID_ARRAY(RES_SYSTEM);
	replace_form(page, "##RES_SYSTEM##", szRES_SYSTEM[lang]);
	char *szRES_UPLOAD_OK[] = RES_ID_ARRAY(RES_UPLOAD_OK);
	replace_form(page, "##RES_UPLOAD_OK##", szRES_UPLOAD_OK[lang]);
	char *szRES_MSG_REBOOT_TIME[] = RES_ID_ARRAY(RES_MSG_REBOOT_TIME);
	replace_form(page, "##RES_MSG_REBOOT_TIME##", szRES_MSG_REBOOT_TIME[lang]);
	char *szRES_UPLOAD_FAIL[] = RES_ID_ARRAY(RES_UPLOAD_FAIL);
	replace_form(page, "##RES_UPLOAD_FAIL##", szRES_UPLOAD_FAIL[lang]);
	char *szRES_SEC[] = RES_ID_ARRAY(RES_SEC);
	replace_form(page, "##RES_SEC##", szRES_SEC[lang]);
	char *szRES_APPLY_AFTER_REBOOT[] = RES_ID_ARRAY(RES_APPLY_AFTER_REBOOT);
	replace_form(page, "##RES_APPLY_AFTER_REBOOT##", szRES_APPLY_AFTER_REBOOT[lang]);
}

// #define _DEBUG_OUTPUT_

int main(void)
{
	cgi_init();
	cgi_session_start();

	cgi_process_form();
	cgi_init_headers();

	if (!check_session())
	{
		cgi_end();
		return 0;
	}

	load_setup(&r_setup, &r_setup_ext);
	int lang = r_setup.lang;

retry:
	load_form(JQUERY_HTML_SETUP_AJAX_SYSTEM_INFO, page, lang);

	replace_param(page, lang);
	replace_resource_string(page, lang);
	replace_option(page, lang);

	if (ajax_check_output(page, lang) < 0)
	{
		if (lang != 0)
		{
			lang = 0;
			goto retry;
		}
	}

	cgi_end();

	return 0;
}
