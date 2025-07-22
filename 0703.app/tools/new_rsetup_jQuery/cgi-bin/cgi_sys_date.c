
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];

static void replace_param(char* page, int lang)
{
	char str[1024], buf[1024 * 10], timezone[128] = {0}, date_ex[64], time_ex[64];;
	char* selptr;
	int i;

	time_t t;
	struct tm tm;

	//w4000 datetime_tzset_2(&r_setup, r_setup.tmzone, timezone);

	t = ({ struct timeval tv; gettimeofday (& tv, NULL); tv.tv_sec; });

	localtime_r (&t, &tm);
#if 0//w4000
	{
		FILE* fp = fopen("/edvr/time_setting", "rb");
		if (fp)
		{
			fread(&tm, 1, sizeof(struct tm), fp);
			fclose(fp);

			unlink("/edvr/time_setting");
		}
	}
#endif
	sprintf(str, "%d", tm.tm_year + 1900);
	replace_form(page, "##PARAM_CUR_YEAR##", str);
	sprintf(str, "%d", tm.tm_mon);
	replace_form(page, "##PARAM_CUR_MONTH##", str);
	sprintf(str, "%d", tm.tm_mday);
	replace_form(page, "##PARAM_CUR_DAY##", str);
	sprintf(str, "%d", tm.tm_hour);
	replace_form(page, "##PARAM_CUR_HOUR##", str);
	sprintf(str, "%d", tm.tm_min);
	replace_form(page, "##PARAM_CUR_MIN##", str);
	sprintf(str, "%d", tm.tm_sec);
	replace_form(page, "##PARAM_CUR_SEC##", str);
	buf[0] = '\0';
	for (i=0; i<24; i++) {

		{
			sprintf(str, "<option value=%02d>%02d</option>\n", i,  i);
		}

		strcat(buf, str);
	}
	replace_form(page, "<!--##PARAM_SYS_HOUR##-->", buf);

	buf[0] = '\0';
	for (i=0; i<60; i++) {
		{
			sprintf(str, "<option value=%02d>%02d</option>\n", i,  i);
		}

		strcat(buf, str);
	}
	replace_form(page, "<!--##PARAM_SYS_MIN##-->", buf);

	buf[0] = '\0';
	for (i=0; i<60; i++) {
		{
			sprintf(str, "<option value=%02d>%02d</option>\n", i,  i);
		}

		strcat(buf, str);
	}
	replace_form(page, "<!--##PARAM_SYS_SEC##-->", buf);



	// time zone
	buf[0] = '\0';
	for (i = 0; i < TMZONE_MAX; i++) {
		selptr = (r_setup.tmzone == i)? "selected" : "";
		sprintf(str, "<option %s value=\"%d\">%s</option>\n", selptr, i, tmzone_val[i]);
		strcat(buf, str);
	}
	replace_form(page, "<!--##PARAM_TIME_ZONE##-->", buf);
	replace_form(page, "##PARAM_CUR_TIME_ZONE##", tmzone_val[r_setup.tmzone]);

	buf[0] = '\0';
	for (i = 0; i < TMZONE_MAX; i++) {
		sprintf(str, "<input type=\"hidden\" id=\"timezone_dst_%d\" value=\"%d\" />\n", i, (strlen(tmzone_str_2[i][1]) > 0) ? 1 : 0);
		strcat(buf, str);
	}
	replace_form(page, "<!--##PARAM_TIMEZONE_DST##-->", buf);


	// date format
	char **datefmt_val = get_datefmt_val(lang);
	char **timefmt_val = get_timefmt_val(lang);
	buf[0] = '\0';
	for (i = 0; i < 6; i++) {
		selptr = (r_setup.datefmt == i)? "selected" : "";
		get_date_string(lang, i, date_ex, &tm);
		sprintf(str, "<option %s value=\"%d\">%s (%s)</option>\n", selptr, i, datefmt_val[i], date_ex);
		strcat(buf, str);
	}
	replace_form(page, "<!--##PARAM_DATE_FORMAT##-->", buf);

	buf[0] = '\0';
	for (i=0; i<3; i++) {
		selptr = (r_setup.timefmt == i)? "selected" : "";
		get_time_string(lang, i, time_ex, &tm);
		sprintf(str, "<option %s value=\"%d\">%s (%s)</option>\n", selptr, i, timefmt_val[i], time_ex);
		strcat(buf, str);
	}
	replace_form(page, "<!--##PARAM_TIME_FORMAT##-->", buf);



	buf[0] = '\0';
	sprintf(str, "%d", r_setup.tmsync.usage);//w4000_tm
	replace_form(page, "##PARAM_TIME_SYNC_MODE##", str);



	int cycle[8] = {1, 2, 3, 4, 6, 8, 12, 24};
	char* szHOUR[] = RES_ID_ARRAY(RES_HOUR);
	buf[0] = '\0';
	for (i = 0; i < 1; i++) {
		selptr = (r_setup.tmsync.usage == cycle[i])? "selected" : "" ;
		sprintf(str, "<option %s value=%d>%d %s</option>\n", selptr, cycle[i], cycle[i], szHOUR[lang]);
		strcat(buf, str);
	}
	replace_form(page, "<!--##PARAM_SYNC_CYCLE##-->", buf);




	strcpy(buf, r_setup.tmsync.ntpsvr);
	if (strlen(buf) == 0)
		strcpy(buf, "pool.ntp.org");
	replace_form(page, "##PARAM_TIME_SYNC_NTP_SERVER##", buf);


}

static void replace_resource_string(char* page, int lang)
{
	{
		char* szRES_DATE[] = RES_ID_ARRAY(RES_DATE);
		char* szRES_TIME[] = RES_ID_ARRAY(RES_TIME);
		char* szRES_SEC[] = RES_ID_ARRAY(RES_SEC);
		char* szRES_MIN[] = RES_ID_ARRAY(RES_MIN);
		char* szRES_HOUR[] = RES_ID_ARRAY(RES_HOUR);
		char* szRES_DST[] = RES_ID_ARRAY(RES_DST);
		char* szRES_DAY_LIGHT_SAVING[] = RES_ID_ARRAY(RES_DAY_LIGHT_SAVING);

		replace_form_once(page, "##RES_DATE##", szRES_DATE[lang]);
		replace_form_once(page, "##RES_TIME##", szRES_TIME[lang]);
		replace_form_once(page, "##RES_SEC##", szRES_SEC[lang]);
		replace_form_once(page, "##RES_MIN##", szRES_MIN[lang]);
		replace_form_once(page, "##RES_HOUR##", szRES_HOUR[lang]);
		replace_form_once(page, "##RES_DST##", szRES_DST[lang]);
		replace_form_once(page, "##RES_DAY_LIGHT_SAVING##", szRES_DAY_LIGHT_SAVING[lang]);

		replace_date_string(page, lang);
	}


	char* szRES_TIME_SYNC_CYCLE[] = RES_ID_ARRAY(RES_TIME_SYNC_CYCLE);
	char* szRES_TIME_SYNC_SERVER[] = RES_ID_ARRAY(RES_TIME_SYNC_SERVER);
	char* szRES_TIME_SYNC_NTP_SERVER[] = RES_ID_ARRAY(RES_TIME_SYNC_NTP_SERVER);
	
	replace_form(page, "##RES_TIME_SYNC_SERVER##", szRES_TIME_SYNC_SERVER[lang]);
	replace_form(page, "##RES_TIME_SYNC_NTP_SERVER##", szRES_TIME_SYNC_NTP_SERVER[lang]);
	replace_form(page, "##RES_TIME_SYNC_CYCLE##", szRES_TIME_SYNC_CYCLE[lang]);

	char* szRES_TIME_SYNC_NTP[] = RES_ID_ARRAY(RES_TIME_SYNC_NTP);
	replace_form(page, "##RES_TIME_SYNC_NTP##", szRES_TIME_SYNC_NTP[lang]);

	char* szRES_MOVE_NW[] = RES_ID_ARRAY(RES_MOVE_NW);
	char* szRES_MOVE_SW[] = RES_ID_ARRAY(RES_MOVE_SW);
	char* szRES_MOVE_NE[] = RES_ID_ARRAY(RES_MOVE_NE);
	char* szRES_MOVE_SE[] = RES_ID_ARRAY(RES_MOVE_SE);
	char* szRES_WHITE[] = RES_ID_ARRAY(RES_WHITE);
	char* szRES_BLACK[] = RES_ID_ARRAY(RES_BLACK);
	char* szRES_RED[] = RES_ID_ARRAY(RES_RED);
	char* szRES_BLUE[] = RES_ID_ARRAY(RES_BLUE);
	char* szRES_GREEN[] = RES_ID_ARRAY(RES_GREEN);
	char* szRES_TIME_SYNC[] = RES_ID_ARRAY(RES_TIME_SYNC);
	char* szRES_NONE[] = RES_ID_ARRAY(RES_NONE);
	char* szRES_SERVER[] = RES_ID_ARRAY(RES_SERVER);
	char* szRES_USE_NTP_SERVER[] = RES_ID_ARRAY(RES_USE_NTP_SERVER);
	replace_form(page, "##RES_MOVE_NW##", szRES_MOVE_NW[lang]);
	replace_form(page, "##RES_MOVE_SW##", szRES_MOVE_SW[lang]);
	replace_form(page, "##RES_MOVE_NE##", szRES_MOVE_NE[lang]);
	replace_form(page, "##RES_MOVE_SE##", szRES_MOVE_SE[lang]);
	replace_form(page, "##RES_WHITE##", szRES_WHITE[lang]);
	replace_form(page, "##RES_BLACK##", szRES_BLACK[lang]);
	replace_form(page, "##RES_RED##", szRES_RED[lang]);
	replace_form(page, "##RES_BLUE##", szRES_BLUE[lang]);
	replace_form(page, "##RES_GREEN##", szRES_GREEN[lang]);

	replace_form(page, "##RES_TIME_SYNC##", szRES_TIME_SYNC[lang]);
	replace_form(page, "##RES_NONE##", szRES_NONE[lang]);
	replace_form(page, "##RES_SERVER##", szRES_SERVER[lang]);
	replace_form(page, "##RES_USE_NTP_SERVER##", szRES_USE_NTP_SERVER[lang]);

	char* szRES_SYSTEM_TIME[] = RES_ID_ARRAY(RES_SYSTEM_TIME);
	replace_form(page, "##RES_SYSTEM_TIME##", szRES_SYSTEM_TIME[lang]);
	char* szRES_USE_CURRENT_SYSTEM_TIME[] = RES_ID_ARRAY(RES_USE_CURRENT_SYSTEM_TIME);
	replace_form(page, "##RES_USE_CURRENT_SYSTEM_TIME##", szRES_USE_CURRENT_SYSTEM_TIME[lang]);

	char* szRES_AM[] = RES_ID_ARRAY(RES_AM);
	char* szRES_PM[] = RES_ID_ARRAY(RES_PM);
	replace_form(page, "##RES_AM##", szRES_AM[lang]);
	replace_form(page, "##RES_PM##", szRES_PM[lang]);

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
	load_form(JQUERY_HTML_SETUP_AJAX_SYSTEM_DATE, page, lang);

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
