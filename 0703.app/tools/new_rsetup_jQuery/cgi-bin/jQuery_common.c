/****************************************************************************
 * cgi_comon.c
 *
 * purpose:
 *	- cgi common functions.
 ****************************************************************************/
#include "common2.h"
#include "jQuery_common.h"

#define SETUP_EXT_IMPLEMENTATION
#include "setup/setup_ini.h"
#include "setup/setup_ext.h"
#include "load_setup.h"

pthread_mutex_t	g_malloc_lock;
ST_CGI g_cgi;

char * tmzone_val [TMZONE_MAX] = {
	"None",
	"(GMT-12) International Date Line West",
	"(GMT-11) Midway Island, Samoa",
	"(GMT-10) Hawaii",
	"(GMT-9) Alaska",
	"(GMT-8) Pacific Time(US&Canada)",
	"(GMT-8) Tijuana, Baja California",
	"(GMT-7) Arizona",
	"(GMT-7) Chihuahua, La Paz, Mazatha",
	"(GMT-7) Mountain Time(US&Canada)",
	"(GMT-6) Central America",
	"(GMT-6) Central Time(US&Canada)",
	"(GMT-6) Guadalajara, Mexico City, Monterrey",
	"(GMT-6) Saskatchewan",
	"(GMT-5) Bogota, Lima, Quito, Rio Branco",
	"(GMT-5) Eastern Time(US&Canada)",
	"(GMT-5) Indiana(East)",
	"(GMT-4:30) Caracas",
	"(GMT-4) Atlantic Time(Canada)",
	"(GMT-4) La Paz",
	"(GMT-4) Manaus",
	"(GMT-4) Santiago",
	"(GMT-3:30) Newfoundland",
	"(GMT-3) Brasilia",
	"(GMT-3) Buenos Aires",
	"(GMT-3) Georgetown",
	"(GMT-3) Greenland",
	"(GMT-3) Montevideo",
	"(GMT-2) Mid-Atlantic",
	"(GMT-1) Azores",
	"(GMT-1) Cape Verde Is.",
	"(GMT+0) Casablanca",
	"(GMT+0) Dublin, Edinburgh, Lisbon, London",
	"(GMT+0) Monrovia, Reykjavik",
	"(GMT+1) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna",
	"(GMT+1) Belgrade, Bratislava, Budapest, Ljubljana, Prague",
	"(GMT+1) Brussels, Copenhagen, Madrid, Paris",
	"(GMT+1) Sarajevo, Skopje, Warsaw, Zagreb",
	"(GMT+1) West Central Africa",
	"(GMT+2) Amman",
	"(GMT+2) Athens, Bucharest, Istanbul",
	"(GMT+2) Beirut",
	"(GMT+2) Cairo",
	"(GMT+2) Harare, Pretoria",
	"(GMT+2) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius",
	"(GMT+2) Jerusalem",
	"(GMT+2) Minsk",
	"(GMT+2) Windhoek",
	"(GMT+3) Baghdad",
	"(GMT+3) Kuwait, Riyadh",
	"(GMT+3) Moscos, St. Petersburg, Volgograd",
	"(GMT+3) Nairobi",
	"(GMT+3) Tbilisi",
	"(GMT+3:30) Tehran",
	"(GMT+4) Abu Dhabi, Muscat",
	"(GMT+4) Baku",
	"(GMT+4) Caucasus Standard Time",
	"(GMT+4) Port Louis",
	"(GMT+4) Yerevan",
	"(GMT+4:30) Kabul",
	"(GMT+5) Ekaterinburg",
	"(GMT+5) Islamabad, Karachi",
	"(GMT+5) Tashkent",
	"(GMT+5:30) Chennai, Kalkata, Mumbai, New Delhi",
	"(GMT+5:30) Sri Jayawardenepura",
	"(GMT+5:45) Kathmandu",
	"(GMT+6) Almaty, Novosibirsk",
	"(GMT+6) Astana, Dhaka",
	"(GMT+6:30) Yangon(Rangoon)",
	"(GMT+7) Bangkok, Hanoi, Jakarta",
	"(GMT+7) Krasnoyarsk",
	"(GMT+8) Beijing, Chonging, Hong Kong, Urumqi",
	"(GMT+8) Irkutsk, Ulaan Bataar",
	"(GMT+8) Kuala Lumpur, Singapore",
	"(GMT+8) Perth",
	"(GMT+8) Taipei",
	"(GMT+9) Osaka, Sapporo, Tokyo",
	"(GMT+9) Seoul",
	"(GMT+9) Yakutsk",
	"(GMT+9:30) Adelaide",
	"(GMT+9:30) Darwin",
	"(GMT+10) Brisbane",
	"(GMT+10) Canberra, Melbourne, Sydney",
	"(GMT+10) Guam, Port Moresby",
	"(GMT+10) Hobart",
	"(GMT+10) Vladivostok",
	"(GMT+11) Magadan, Solomon Is., New Caledonia",
	"(GMT+12) Auckland, Wellington",
	"(GMT+12) Fiji, Kamchatka, Marshall Is.",
	"(GMT+13) Nuku'alofa"
};

char * tmzone_str_2 [TMZONE_MAX][4] = {
	{"GMT", 			"",					"", 				""},
	{"STD+12", 			"",					"", 				""},
	{"STD+11", 			"", 				"", 				""},
	{"STD+10", 			"", 				"", 				""},
	{"STD+9",			"SMR", 				"M3.2.0/02:00",		"M11.1.0/02:00"},
	{"STD+8",			"SMR", 				"M3.2.0/02:00", 	"M11.1.0/02:00"},
	{"STD+8",			"SMR", 				"M4.1.0/02:00", 	"M10.5.0/02:00"},
	{"STD+7", 			"", 				"", 				""},
	{"STD+7",			"SMR", 				"M4.1.0/02:00", 	"M10.5.0/02:00"},
	{"STD+7",			"SMR", 				"M3.2.0/02:00", 	"M11.1.0/02:00"},
	{"STD+6", 			"", 				"", 				""},
	{"STD+6",			"SMR", 				"M3.2.0/02:00", 	"M11.1.0/02:00"},
	{"STD+6",			"SMR", 				"M4.1.0/02:00", 	"M10.5.0/02:00"},
	{"STD+6", 			"", 				"", 				""},
	{"STD+5", 			"", 				"", 				""},
	{"STD+5",			"SMR", 				"M3.2.0/02:00", 	"M11.1.0/02:00"},
	{"STD+5", 			"", 				"", 				""},
	{"STD+5:30", 		"", 				"", 				""},
	{"STD+4",			"SMR", 				"M3.2.0/02:00", 	"M11.1.0/02:00"},
	{"STD+4", 			"", 				"", 				""},
	{"STD+4",			"SMR", 				"M10.4.0/00:00", 	"M2.2.6/00:00"},
	{"STD+4",			"SMR", 				"M10.3.0/00:00", 	"M3.2.6/0:00"},
	{"STD+3:30", 		"", 				"", 				""},
	{"STD+3",			"SMR", 				"M10.4.0/00:00", 	"M2.2.6/00:00"},
	{"STD+3",			"SMR", 				"M10.4.0/00:00", 	"M3.2.6/00:00"},
	{"STD+3", 			"", 				"", 				""},
	{"STD+3",			"SMR", 				"M4.1.0/02:00", 	"M10.5.0/02:00"},
	{"STD+3",			"SMR", 				"M10.1.0/02:00", 	"M3.2.0/02:00"},
	{"STD+2",			"SMR", 				"M3.5.0/02:00", 	"M9.5.0/02:00"},
	{"STD+1",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD+1", 			"", 				"", 				""},
	{"STD+0", 			"", 				"", 				""},
	{"STD+0",			"SMR", 				"M3.5.0/01:00", 	"M10.5.0/02:00"},
	{"STD+0", 			"", 				"", 				""},
	{"STD-1",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-1",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-1",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-1",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-1", 			"", 				"", 				""},
	{"STD-2",			"SMR", 				"M3.4.4/00:00", 	"M10.5.5/01:00"},
	{"STD-2",			"SMR", 				"M3.5.0/03:00", 	"M10.5.0/04:00"},
	{"STD-2",			"SMR", 				"M3.5.0/00:00", 	"M10.5.6/00:00"},
	{"STD-2",			"SMR", 				"M4.5.5/00:00", 	"M9.5.4/00:00"},
	{"STD-2", 			"", 				"", 				""},
	{"STD-2",			"SMR", 				"M3.5.0/03:00", 	"M10.5.0/04:00"},
	{"STD-2",			"SMR", 				"M3.5.5/02:00", 	"M9.5.0/02:00"},
	{"STD-2",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-2",			"SMR", 				"M4.1.0/02:00", 	"M9.1.0/04:00"},
	{"STD-3", 			"", 				"", 				""},
	{"STD-3", 			"", 				"", 				""},
	{"STD-3",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-3", 			"", 				"", 				""},
	{"STD-3", 			"", 				"", 				""},
	{"STD-3:30",		"SMR", 				"M3.4.0/00:00", 	"M9.3.1/00:00"},
	{"STD-4", 			"", 				"", 				""},
	{"STD-4",			"SMR", 				"M3.5.0/04:00", 	"M10.5.0/05:00"},
	{"STD-4", 			"", 				"", 				""},
	{"STD-4",			"SMR", 				"M10.5.0/02:00", 	"M3.5.0/02:00"},
	{"STD-4",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-4:30", 		"", 				"", 				""},
	{"STD-5",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-5", 			"", 				"", 				""},
	{"STD-5", 			"", 				"", 				""},
	{"STD-5:30", 		"", 				"", 				""},
	{"STD-5:30", 		"", 				"", 				""},
	{"STD-5:45", 		"", 				"", 				""},
	{"STD-6",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-6", 			"", 				"", 				""},
	{"STD-6:30", 		"", 				"", 				""},
	{"STD-7", 			"", 				"", 				""},
	{"STD-7",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-8", 			"", 				"", 				""},
	{"STD-8",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-8", 			"", 				"", 				""},
	{"STD-8",			"SMR", 				"M10.5.0/02:00", 	"M3.5.0/03:00"},
	{"STD-8", 			"", 				"", 				""},
	{"STD-9", 			"", 				"", 				""},
	{"STD-9", 			"", 				"", 				""},
	{"STD-9",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-9:30",		"SMR", 				"M10.1.0/02:00", 	"M4.1.0/03:00"},
	{"STD-9:30", 		"", 				"", 				""},
	{"STD-10", 			"", 				"", 				""},
	{"STD-10",			"SMR", 				"M10.1.0/02:00", 	"M4.1.0/03:00"},
	{"STD-10", 			"", 				"", 				""},
	{"STD-10",			"SMR", 				"M10.1.0/02:00", 	"M4.1.0/03:00"},
	{"STD-10",			"SMR", 				"M3.5.0/02:00", 	"M10.5.0/03:00"},
	{"STD-11", 			"", 				"", 				""},
	{"STD-12",			"SMR", 				"M9.5.0/02:00", 	"M4.1.0/03:00"},
	{"STD-12", 			"", 				"", 				""},
	{"STD-13", 			"", 				"", 				""}
};

const char* g_errfmt =
				"<table border='0' cellspacing='0' cellpadding='2px' width='100%%'>"
					"<tr>"
						"<td bgcolor='#FEF8F6' align='center' valign='center'>"
							"<div class='ui-widget'>"
								"<div class='ui-state-error ui-corner-all' style='padding: 0 .7em;'>"
									"<p><span class=\"ui-icon ui-icon-alert\" style=\"display:inline-block;\"></span><br>%s</p>"
								"</div>"
							"</div>"
						"</td>"
					"</tr>"
					"<tr height='10px'></tr>"
				"</table>";

#define _CSS_REDMOND_

static char* jquery_redmond =
#ifdef _CSS_REDMOND_
			"<link href=\"/css/jquery-ui-1.9.2.custom.min.css?v=" VER_STR "\" rel=\"stylesheet\" />"
			"<link href=\"/css/custom.css?v=" VER_STR "\" rel=\"stylesheet\" />"
#endif
			"<script src=\"/js/jquery-1.8.3.js?v=" VER_STR "\"></script>"
			"<script src=\"/js/jquery-ui-1.9.2.custom.min.js?v=" VER_STR "\"></script>"
			"<script src=\"/js/jsencrypt.min.js?v=" VER_STR "\"></script>"
			"<script src=\"/js/common.js?v=" VER_STR "\"></script>"  
			"<script src=\"/js/jcanvas.min.js?v=" VER_STR "\"></script>";


void replace_jquery_include(char* page)
{
	replace_form_once(page, "<!--##PARAM_JQUERY_INCLUDE##-->", jquery_redmond);
}

int replace_form(char *text,char *src,char *rep)
{
	char *pfound;
	char page_buf[MAX_PAGE_SZ];

	if ((strlen(src) == 0)/* || (strlen(rep) == 0)*/)
		return -1;

	while ((pfound = strstr(text, src)) != NULL) {

		strcpy(page_buf, text);
		page_buf[pfound -text] = '\0';

		strcat(page_buf, rep);

		pfound += strlen(src);

		strcat(page_buf, pfound);
		strcpy(text, page_buf);
	}

	return 0;
}

int replace_form_once(char *text,char *src,char *rep)
{
	char *pfound;
	char page_buf[MAX_PAGE_SZ];

	if ((strlen(src) == 0)/* || (strlen(rep) == 0)*/)
		return -1;

	if ((pfound = strstr(text, src)) == NULL)
		return -1;

	strcpy(page_buf, text);
	page_buf[pfound -text] = '\0';

	strcat(page_buf, rep);

	pfound += strlen(src);

	strcat(page_buf, pfound);
	strcpy(text, page_buf);

	return 0;
}

int replace_charset(char *text, int lang)
{
#ifdef _USE_UTF8_
	return replace_form_once(text, "; charset=EUC-KR", "; charset=utf-8");	
#else
	char* szResCharSet[] = RES_ID_ARRAY(RES_COMMON_CHARSET);
	return replace_form_once(text, "; charset=EUC-KR", szResCharSet[lang]); 
#endif
}

static void replace_web_encrypt_code(char* page)
{
	char* code1 =
		"var encrypt = new JSEncrypt();"
		"encrypt.setPublicKey(_pub_key);"
		"var rnd_key = randomString(64);"
		"var enc_key = encrypt.encrypt(rnd_key);"
		"var ses_key = encrypt.encrypt(_session_key);"
		"var enc_category = encrypt.encrypt"; // ("category");

	replace_form(page, "//##PARAM_WEB_ENCRYPT_CODE_1##", code1);

	char* code2 =
		"category: 		enc_category"
		",rsa_session:	_session_key"
		",rnd_key:		rnd_key"
		",enc_key:		enc_key"
		",ses_key:		ses_key";

	replace_form(page, "//##PARAM_WEB_ENCRYPT_CODE_2##", code2);
}

static void replace_char(char *str, char s, char r)
{
	int i;
	int len = strlen(str);
	for(i = 0; i < len; i++) {
		if(str[i] == s) {
			str[i] = r;
		}
	}
}

void load_form(char *path, char* form, int lang)
{
	FILE *fp;
	char buf[1024];

	form[0] = '\0';

	fp = fopen (path, "rt");
	if (fp) {
		while (fgets (buf, 1024, fp) != NULL) {
			strcat(form, buf);
		}
		fclose(fp);
	}

	replace_charset(form, lang);
	replace_jquery_include(form);
	replace_web_encrypt_code(form);
}

void replace_logo(char* page)
{
	replace_form_once(page, "##PARAM_LOGO##", "/img/logo.png");
}

void force_redirect(int sec, const char* url)
{
	printf("<meta http-equiv=\"refresh\" content=\"%d;url='%s'\" />\r\n\r\n", sec, url);
}


boolean check_session()
{
	if (!cgi_session_var_exists("logon_user_no"))
	{
		force_redirect(0, "./login.cgi");
		return FALSE;
	}

	return TRUE;
}
char* get_model(char* szModel, _CAMSETUP* _r_setup)
{
	szModel[0] = 0;
	setup_ini_load();
	sprintf(szModel, "%s", setup_ini_get_str("cam_setup", "INFO_MODEL_NAME", "21C26IP6"));

	return szModel;
}

char* get_base_model(char* szBaseModel, _CAMSETUP* _r_setup)
{
	szBaseModel[0] = 0;

	// sprintf(szBaseModel, "%s", base_model_name());
	sprintf(szBaseModel, "%s", setup_ini_get_str("cam_setup", "INFO_MODEL_NAME", "21C26IP6"));

	return szBaseModel;
}

void replace_software_version(char* page, _CAMSETUP* _r_setup)
{
	char str[128];
	//char cmd[256];
	// sprintf(str, "v%d.%02d.%02d-" REVISION, _r_setup->sw_ver[0], _r_setup->sw_ver[1], _r_setup->sw_ver[2]);
	sprintf(str, "v%d.%02d.%02d", atoi(RELVER), atoi(MAJVER), atoi(MINVER));//w4000
	//w4000 sprintf(str, "v%d.%02d.%02d", atoi(RELVER), atoi(MAJVER), 29);

		//sprintf (cmd, "echo \"replace_software_version > version: %d %d %d, VER_STR? %s \" | cat >> /mnt/ipm/log.txt", atoi(RELVER), atoi(MAJVER), atoi(MINVER), VER_STR);
		//system(cmd);

	replace_form(page, "##PARAM_CURRENT_VERSION##", str);
}


char* get_software_version(char* szVersion, _CAMSETUP* _r_setup)
{
	sprintf(szVersion, "v%d.%02d.%02d", atoi(RELVER), atoi(MAJVER), atoi(MINVER));

	{
		//char cmd[256];
		//sprintf (cmd, "echo \"get_software_version > szVersion ? %s, version: %d %d %d / %d %d %d, VER_STR? %s \" | cat >> /mnt/ipm/log.txt", szVersion, _r_setup->sw_ver[0], _r_setup->sw_ver[1], _r_setup->sw_ver[2], atoi(RELVER), atoi(MAJVER), atoi(MINVER), VER_STR);
		//system(cmd);
	}

	return szVersion;
}

char* get_sensor_name(char* szSensorName)
{
	sprintf(szSensorName, "2M IMX327");
	return szSensorName;
}

void replace_date_string(char* page, int lang)
{
	char* szRES_JAN[] = RES_ID_ARRAY(RES_JAN);
	char* szRES_FEB[] = RES_ID_ARRAY(RES_FEB);
	char* szRES_MAR[] = RES_ID_ARRAY(RES_MAR);
	char* szRES_APR[] = RES_ID_ARRAY(RES_APR);
	char* szRES_MAY[] = RES_ID_ARRAY(RES_MAY);
	char* szRES_JUN[] = RES_ID_ARRAY(RES_JUN);
	char* szRES_JUL[] = RES_ID_ARRAY(RES_JUL);
	char* szRES_AUG[] = RES_ID_ARRAY(RES_AUG);
	char* szRES_SEP[] = RES_ID_ARRAY(RES_SEP);
	char* szRES_OCT[] = RES_ID_ARRAY(RES_OCT);
	char* szRES_NOV[] = RES_ID_ARRAY(RES_NOV);
	char* szRES_DEC[] = RES_ID_ARRAY(RES_DEC);

	char* szRES_JAN_2[] = RES_ID_ARRAY(RES_JAN_2);
	char* szRES_FEB_2[] = RES_ID_ARRAY(RES_FEB_2);
	char* szRES_MAR_2[] = RES_ID_ARRAY(RES_MAR_2);
	char* szRES_APR_2[] = RES_ID_ARRAY(RES_APR_2);
	char* szRES_MAY_2[] = RES_ID_ARRAY(RES_MAY_2);
	char* szRES_JUN_2[] = RES_ID_ARRAY(RES_JUN_2);
	char* szRES_JUL_2[] = RES_ID_ARRAY(RES_JUL_2);
	char* szRES_AUG_2[] = RES_ID_ARRAY(RES_AUG_2);
	char* szRES_SEP_2[] = RES_ID_ARRAY(RES_SEP_2);
	char* szRES_OCT_2[] = RES_ID_ARRAY(RES_OCT_2);
	char* szRES_NOV_2[] = RES_ID_ARRAY(RES_NOV_2);
	char* szRES_DEC_2[] = RES_ID_ARRAY(RES_DEC_2);

	char* szRES_SUN[] = RES_ID_ARRAY(RES_SUN);
	char* szRES_MON[] = RES_ID_ARRAY(RES_MON);
	char* szRES_TUE[] = RES_ID_ARRAY(RES_TUE);
	char* szRES_WED[] = RES_ID_ARRAY(RES_WED);
	char* szRES_THU[] = RES_ID_ARRAY(RES_THU);
	char* szRES_FRI[] = RES_ID_ARRAY(RES_FRI);
	char* szRES_SAT[] = RES_ID_ARRAY(RES_SAT);

	char* szRES_SUN_2[] = RES_ID_ARRAY(RES_SUN_2);
	char* szRES_MON_2[] = RES_ID_ARRAY(RES_MON_2);
	char* szRES_TUE_2[] = RES_ID_ARRAY(RES_TUE_2);
	char* szRES_WED_2[] = RES_ID_ARRAY(RES_WED_2);
	char* szRES_THU_2[] = RES_ID_ARRAY(RES_THU_2);
	char* szRES_FRI_2[] = RES_ID_ARRAY(RES_FRI_2);
	char* szRES_SAT_2[] = RES_ID_ARRAY(RES_SAT_2);

	replace_form(page, "##RES_JAN##", szRES_JAN[lang]);
	replace_form(page, "##RES_FEB##", szRES_FEB[lang]);
	replace_form(page, "##RES_MAR##", szRES_MAR[lang]);
	replace_form(page, "##RES_APR##", szRES_APR[lang]);
	replace_form(page, "##RES_MAY##", szRES_MAY[lang]);
	replace_form(page, "##RES_JUN##", szRES_JUN[lang]);
	replace_form(page, "##RES_JUL##", szRES_JUL[lang]);
	replace_form(page, "##RES_AUG##", szRES_AUG[lang]);
	replace_form(page, "##RES_SEP##", szRES_SEP[lang]);
	replace_form(page, "##RES_OCT##", szRES_OCT[lang]);
	replace_form(page, "##RES_NOV##", szRES_NOV[lang]);
	replace_form(page, "##RES_DEC##", szRES_DEC[lang]);

	replace_form(page, "##RES_JAN_2##", szRES_JAN_2[lang]);
	replace_form(page, "##RES_FEB_2##", szRES_FEB_2[lang]);
	replace_form(page, "##RES_MAR_2##", szRES_MAR_2[lang]);
	replace_form(page, "##RES_APR_2##", szRES_APR_2[lang]);
	replace_form(page, "##RES_MAY_2##", szRES_MAY_2[lang]);
	replace_form(page, "##RES_JUN_2##", szRES_JUN_2[lang]);
	replace_form(page, "##RES_JUL_2##", szRES_JUL_2[lang]);
	replace_form(page, "##RES_AUG_2##", szRES_AUG_2[lang]);
	replace_form(page, "##RES_SEP_2##", szRES_SEP_2[lang]);
	replace_form(page, "##RES_OCT_2##", szRES_OCT_2[lang]);
	replace_form(page, "##RES_NOV_2##", szRES_NOV_2[lang]);
	replace_form(page, "##RES_DEC_2##", szRES_DEC_2[lang]);

	replace_form(page, "##RES_SUN##", szRES_SUN[lang]);
	replace_form(page, "##RES_MON##", szRES_MON[lang]);
	replace_form(page, "##RES_TUE##", szRES_TUE[lang]);
	replace_form(page, "##RES_WED##", szRES_WED[lang]);
	replace_form(page, "##RES_THU##", szRES_THU[lang]);
	replace_form(page, "##RES_FRI##", szRES_FRI[lang]);
	replace_form(page, "##RES_SAT##", szRES_SAT[lang]);

	replace_form(page, "##RES_SUN_2##", szRES_SUN_2[lang]);
	replace_form(page, "##RES_MON_2##", szRES_MON_2[lang]);
	replace_form(page, "##RES_TUE_2##", szRES_TUE_2[lang]);
	replace_form(page, "##RES_WED_2##", szRES_WED_2[lang]);
	replace_form(page, "##RES_THU_2##", szRES_THU_2[lang]);
	replace_form(page, "##RES_FRI_2##", szRES_FRI_2[lang]);
	replace_form(page, "##RES_SAT_2##", szRES_SAT_2[lang]);
}

#ifdef _LANG_ENG_ONLY_

char *g_month_val_ENG[12] = RES_MONTH_ARRAY(ENG);
char *g_month_val_CHI[12] = RES_MONTH_ARRAY(ENG);
char *g_month_val_PRC[12] = RES_MONTH_ARRAY(ENG);
char *g_month_val_KOR[12] = RES_MONTH_ARRAY(ENG);
char *g_month_val_GER[12] = RES_MONTH_ARRAY(ENG);

char *g_short_month_val_ENG[12] = RES_S_MONTH_ARRAY(ENG);
char *g_short_month_val_CHI[12] = RES_S_MONTH_ARRAY(ENG);
char *g_short_month_val_PRC[12] = RES_S_MONTH_ARRAY(ENG);
char *g_short_month_val_KOR[12] = RES_S_MONTH_ARRAY(ENG);
char *g_short_month_val_GER[12] = RES_S_MONTH_ARRAY(ENG);

char *g_week_val_ENG[] = RES_WEEK_ARRAY(ENG);
char *g_week_val_CHI[] = RES_WEEK_ARRAY(ENG);
char *g_week_val_PRC[] = RES_WEEK_ARRAY(ENG);
char *g_week_val_KOR[] = RES_WEEK_ARRAY(ENG);
char *g_week_val_GER[] = RES_WEEK_ARRAY(ENG);

char *g_short_week_val_ENG[] = RES_S_WEEK_ARRAY(ENG);
char *g_short_week_val_CHI[] = RES_S_WEEK_ARRAY(ENG);
char *g_short_week_val_PRC[] = RES_S_WEEK_ARRAY(ENG);
char *g_short_week_val_KOR[] = RES_S_WEEK_ARRAY(ENG);
char *g_short_week_val_GER[] = RES_S_WEEK_ARRAY(ENG);

char *g_lang_val_ENG[] = RES_LANG_ARRAY(ENG);
char *g_lang_val_CHI[] = RES_LANG_ARRAY(ENG);
char *g_lang_val_PRC[] = RES_LANG_ARRAY(ENG);
char *g_lang_val_KOR[] = RES_LANG_ARRAY(ENG);
char *g_lang_val_GER[] = RES_LANG_ARRAY(ENG);

char *g_datefmt_val_ENG[6] = RES_DATEFMT_ARRAY(ENG);
char *g_datefmt_val_CHI[6] = RES_DATEFMT_ARRAY(ENG);
char *g_datefmt_val_PRC[6] = RES_DATEFMT_ARRAY(ENG);
char *g_datefmt_val_KOR[6] = RES_DATEFMT_ARRAY(ENG);
char *g_datefmt_val_GER[6] = RES_DATEFMT_ARRAY(ENG);

char *g_timefmt_val_ENG[3] = RES_TIMEFMT_ARRAY(ENG);
char *g_timefmt_val_CHI[3] = RES_TIMEFMT_ARRAY(ENG);
char *g_timefmt_val_PRC[3] = RES_TIMEFMT_ARRAY(ENG);
char *g_timefmt_val_KOR[3] = RES_TIMEFMT_ARRAY(ENG);
char *g_timefmt_val_GER[3] = RES_TIMEFMT_ARRAY(ENG);

char *g_qty_val_V_ENG[QTY_MAX] = RES_QUALITY_ARRAY(ENG);
char *g_qty_val_V_CHI[QTY_MAX] = RES_QUALITY_ARRAY(ENG);
char *g_qty_val_V_PRC[QTY_MAX] = RES_QUALITY_ARRAY(ENG);
char *g_qty_val_V_KOR[QTY_MAX] = RES_QUALITY_ARRAY(ENG);
char *g_qty_val_V_GER[QTY_MAX] = RES_QUALITY_ARRAY(ENG);

#else

	#ifdef _LANG_ENG_KOR_ONLY_
		char *g_month_val_ENG[12] = RES_MONTH_ARRAY(ENG);
		char *g_month_val_CHI[12] = RES_MONTH_ARRAY(ENG);
		char *g_month_val_PRC[12] = RES_MONTH_ARRAY(ENG);
		char *g_month_val_KOR[12] = RES_MONTH_ARRAY(KOR);
		char *g_month_val_GER[12] = RES_MONTH_ARRAY(ENG);

		char *g_short_month_val_ENG[12] = RES_S_MONTH_ARRAY(ENG);
		char *g_short_month_val_CHI[12] = RES_S_MONTH_ARRAY(ENG);
		char *g_short_month_val_PRC[12] = RES_S_MONTH_ARRAY(ENG);
		char *g_short_month_val_KOR[12] = RES_S_MONTH_ARRAY(KOR);
		char *g_short_month_val_GER[12] = RES_S_MONTH_ARRAY(ENG);

		char *g_week_val_ENG[] = RES_WEEK_ARRAY(ENG);
		char *g_week_val_CHI[] = RES_WEEK_ARRAY(ENG);
		char *g_week_val_PRC[] = RES_WEEK_ARRAY(ENG);
		char *g_week_val_KOR[] = RES_WEEK_ARRAY(KOR);
		char *g_week_val_GER[] = RES_WEEK_ARRAY(ENG);

		char *g_short_week_val_ENG[] = RES_WEEK_ARRAY(ENG);
		char *g_short_week_val_CHI[] = RES_WEEK_ARRAY(ENG);
		char *g_short_week_val_PRC[] = RES_WEEK_ARRAY(ENG);
		char *g_short_week_val_KOR[] = RES_WEEK_ARRAY(KOR);
		char *g_short_week_val_GER[] = RES_WEEK_ARRAY(ENG);

		char *g_lang_val_ENG[] = RES_LANG_ARRAY(ENG);
		char *g_lang_val_CHI[] = RES_LANG_ARRAY(ENG);
		char *g_lang_val_PRC[] = RES_LANG_ARRAY(ENG);
		char *g_lang_val_KOR[] = RES_LANG_ARRAY(KOR);
		char *g_lang_val_GER[] = RES_LANG_ARRAY(ENG);

		char *g_datefmt_val_ENG[6] = RES_DATEFMT_ARRAY(ENG);
		char *g_datefmt_val_CHI[6] = RES_DATEFMT_ARRAY(ENG);
		char *g_datefmt_val_PRC[6] = RES_DATEFMT_ARRAY(ENG);
		char *g_datefmt_val_KOR[6] = RES_DATEFMT_ARRAY(KOR);
		char *g_datefmt_val_GER[6] = RES_DATEFMT_ARRAY(ENG);

		char *g_timefmt_val_ENG[3] = RES_TIMEFMT_ARRAY(ENG);
		char *g_timefmt_val_CHI[3] = RES_TIMEFMT_ARRAY(ENG);
		char *g_timefmt_val_PRC[3] = RES_TIMEFMT_ARRAY(ENG);
		char *g_timefmt_val_KOR[3] = RES_TIMEFMT_ARRAY(KOR);
		char *g_timefmt_val_GER[3] = RES_TIMEFMT_ARRAY(ENG);

		char *g_qty_val_V_ENG[QTY_MAX] = RES_QUALITY_ARRAY(ENG);
		char *g_qty_val_V_CHI[QTY_MAX] = RES_QUALITY_ARRAY(ENG);
		char *g_qty_val_V_PRC[QTY_MAX] = RES_QUALITY_ARRAY(ENG);
		char *g_qty_val_V_KOR[QTY_MAX] = RES_QUALITY_ARRAY(KOR);
		char *g_qty_val_V_GER[QTY_MAX] = RES_QUALITY_ARRAY(ENG);

	#else

		char *g_month_val_ENG[12] = RES_MONTH_ARRAY(ENG);
		char *g_month_val_KOR[12] = RES_MONTH_ARRAY(KOR);
		char *g_month_val_JPN[12] = RES_MONTH_ARRAY(JPN);

		char *g_short_month_val_ENG[12] = RES_S_MONTH_ARRAY(ENG);
		char *g_short_month_val_KOR[12] = RES_S_MONTH_ARRAY(KOR);
		char *g_short_month_val_JPN[12] = RES_S_MONTH_ARRAY(JPN);

		char *g_week_val_ENG[] = RES_WEEK_ARRAY(ENG);
		char *g_week_val_KOR[] = RES_WEEK_ARRAY(KOR);
		char *g_week_val_JPN[] = RES_WEEK_ARRAY(JPN);

		char *g_short_week_val_ENG[] = RES_WEEK_ARRAY(ENG);
		char *g_short_week_val_KOR[] = RES_WEEK_ARRAY(KOR);
		char *g_short_week_val_JPN[] = RES_WEEK_ARRAY(JPN);

		char *g_lang_val_ENG[] = RES_LANG_ARRAY(ENG);
		char *g_lang_val_KOR[] = RES_LANG_ARRAY(KOR);
		char *g_lang_val_JPN[] = RES_LANG_ARRAY(JPN);

		char *g_datefmt_val_ENG[6] = RES_DATEFMT_ARRAY(ENG);
		char *g_datefmt_val_KOR[6] = RES_DATEFMT_ARRAY(KOR);
		char *g_datefmt_val_JPN[6] = RES_DATEFMT_ARRAY(JPN);

		char *g_timefmt_val_ENG[3] = RES_TIMEFMT_ARRAY(ENG);
		char *g_timefmt_val_KOR[3] = RES_TIMEFMT_ARRAY(KOR);
		char *g_timefmt_val_JPN[3] = RES_TIMEFMT_ARRAY(JPN);

		char *g_qty_val_V_ENG[QTY_MAX] = RES_QUALITY_ARRAY(ENG);
		char *g_qty_val_V_KOR[QTY_MAX] = RES_QUALITY_ARRAY(KOR);
		char *g_qty_val_V_JPN[QTY_MAX] = RES_QUALITY_ARRAY(JPN);

	#endif
#endif

char** get_month_val(int lang)
{
	char** month_val = g_month_val_ENG;
	switch (lang)
	{
		case 1: month_val = g_month_val_KOR;	break;
		case 2: month_val = g_month_val_JPN;	break;
	}
	return month_val;
}

char** get_short_month_val(int lang)
{
	char** month_val = g_short_month_val_ENG;
	switch (lang)
	{
		case 1: month_val = g_short_month_val_KOR;	break;
		case 2: month_val = g_short_month_val_JPN;	break;
	}
	return month_val;
}

char** get_datefmt_val(int lang)
{
	char** val = g_datefmt_val_ENG;
	switch (lang)
	{
		case 1: val = g_datefmt_val_KOR;	break;
		case 2: val = g_datefmt_val_JPN;	break;
	}
	return val;
}

char** get_timefmt_val(int lang)
{
	char** val = g_timefmt_val_ENG;
	switch (lang)
	{
		case 1: val = g_timefmt_val_KOR;	break;
		case 2: val = g_timefmt_val_JPN;	break;
	}
	return val;
}

char** get_week_val(int lang)
{
	char** week_val = g_week_val_ENG;
	switch (lang)
	{
		case 1: week_val = g_week_val_KOR;	break;
		case 2: week_val = g_week_val_JPN;	break;
	}
	return week_val;
}

char** get_short_week_val(int lang)
{
	char** week_val = g_short_week_val_ENG;
	switch (lang)
	{
		case 1: week_val = g_short_week_val_KOR;	break;
		case 2: week_val = g_short_week_val_JPN;	break;
	}
	return week_val;
}

char** get_lang_val(int lang)
{
	char** lang_val = g_lang_val_ENG;
	switch (lang)
	{
		case 1: lang_val = g_lang_val_KOR;	break;
		case 2: lang_val = g_lang_val_JPN;	break;
	}
	return lang_val;
}

char** get_quality_val(int lang)
{
	char** qty_val_V = g_qty_val_V_ENG;
	switch (lang)
	{
		case 1:	qty_val_V = g_qty_val_V_KOR;	break;
		case 2:	qty_val_V = g_qty_val_V_JPN;	break;
	}
	return qty_val_V;
}

char * get_date_string (int lang, int iDateFormat, char *buf, struct tm *pstTm)
{
	char** pMonthStr = get_short_month_val(lang);

	if( iDateFormat == FORMAT_YYYYMMDD )
	{
		sprintf(buf, "%04d/%02d/%02d",
				pstTm->tm_year + 1900,
				pstTm->tm_mon + 1,
				pstTm->tm_mday
				);
	}
	else if( iDateFormat == FORMAT_DDMMYYYY )
	{
		sprintf(buf, "%02d/%02d/%04d",
				pstTm->tm_mday,
				pstTm->tm_mon + 1,
				pstTm->tm_year + 1900
				);
	}
	else if( iDateFormat == FORMAT_DDmmYYYY )
	{
		sprintf(buf, "%02d %s, %04d",
				pstTm->tm_mday,
				pMonthStr[pstTm->tm_mon],
				pstTm->tm_year + 1900
				);
	}
	else if( iDateFormat == FORMAT_mmDDYYYY )
	{
		sprintf(buf, "%s %02d, %04d",
				pMonthStr[pstTm->tm_mon],
				pstTm->tm_mday,
				pstTm->tm_year + 1900
				);
	}
	else if( iDateFormat == FORMAT_YYYYmmDD )
	{
		sprintf(buf, "%04d, %s %02d",
				pstTm->tm_year + 1900,
				pMonthStr[pstTm->tm_mon],
				pstTm->tm_mday
				);
	}
	else // FORMAT_MMDDYYYY
	{
		sprintf(buf, "%02d/%02d/%04d",
				pstTm->tm_mon + 1,
				pstTm->tm_mday,
				pstTm->tm_year + 1900
				);
	}

	return buf;
}

char* g_szAM[] = RES_ID_ARRAY(RES_AM);
char* g_szPM[] = RES_ID_ARRAY(RES_PM);

char * get_time_string (int lang, int iTimeFormat, char *buf, struct tm *pstTm) {

	if( iTimeFormat == FORMAT_AMPM )
	{
		sprintf(buf , "%02d:%02d:%02d %s",
				(pstTm->tm_hour + 11 ) % 12 + 1,
				pstTm->tm_min,
				pstTm->tm_sec,
				pstTm->tm_hour / 12 ? g_szPM[lang] : g_szAM[lang]
				);
	}
	else if( iTimeFormat == FORMAT_AMPM_FRONT )
	{
		sprintf(buf , "%s %02d:%02d:%02d",
				pstTm->tm_hour / 12 ? g_szPM[lang] : g_szAM[lang] ,
				(pstTm->tm_hour + 11 ) % 12 + 1,
				pstTm->tm_min,
				pstTm->tm_sec
				);
	}
	else
	{
		sprintf(buf, "%02d:%02d:%02d",
				pstTm->tm_hour,
				pstTm->tm_min,
				pstTm->tm_sec
				);
	}
	return buf;
}


void RemoveLastChar(char* src, char cLast)
{
	int len = strlen(src);
	while (--len >= 0)
	{
		if (src[len] == cLast)
		{
			src[len] = 0;
			break;
		}
	}
}

int get_essid_ip_ra0(char* essid, int essidlen, char* ipaddr, int ipaddrlen)
{
	// todo
	return 0;
}

static int _if_set_gateway (char * device, char *gw)
{
	// todo
	return 0;
}

int ifconfig_ra0_(int bIsUp, _CAMSETUP* pSetup)
{
	// todo
	return 0;
}

// Return 6 bytes of MAC.
int __if_get_mac (const char *dev, uchar * buff)
{
	int i;
    int sockfd = -1;
    uchar *pdata = NULL;
    struct ifreq  ifr;
    int ret = -1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto out;

    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    ifr.ifr_hwaddr.sa_family = AF_INET;

    if (ioctl (sockfd, SIOCGIFHWADDR, &ifr) < 0)
		goto out;

    pdata = (uchar*)ifr.ifr_hwaddr.sa_data;

    for (i = 0; i < 6; i++)
		buff[i] = *pdata++;

    ret = 0;

out:
    if (sockfd >= 0)
		close (sockfd);

	return ret;
}






int __if_get_ip(const char *dev, struct in_addr * addr)
{
	int   sockfd = -1;

    struct ifreq ifr;
    struct sockaddr_in *sap;
    int ret = -1;

    assert (dev);
    assert (addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto out;

    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ-1] = '\0';
    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl (sockfd, SIOCGIFADDR, &ifr) < 0)
		goto out;

    sap  = (struct sockaddr_in*)&ifr.ifr_addr;
    * addr = * ((struct in_addr *)& sap->sin_addr);

    ret = 0;

out:
    if (sockfd >= 0)
		close (sockfd);

    return ret;
}



#include "tty.h"



#if 1
int isp_open()
{
	return 1;
}

void isp_close(int htty)
{
}

void isp_write(int hTTY, char * buff, int size)
{
}

void isp_get_data(int htty, int addr, char* str)
{
}

int isp_get_data_int(int htty, int addr)
{
	return 0;
}

int check_isp_version(int* major, int* minor)
{
	return 1;
}


#endif

void datetime_tzset_2 (_CAMSETUP* r_setup, int index, char* tzstr) {

	assert (index >=0 && index < TMZONE_MAX);

	if (r_setup->dst && (strlen(tmzone_str_2[index][1]) > 0))
		sprintf(tzstr, "%s%s,%s,%s", tmzone_str_2[index][0], tmzone_str_2[index][1], tmzone_str_2[index][2], tmzone_str_2[index][3]);
	else
		sprintf(tzstr, "%s", tmzone_str_2[index][0]);


	//char cmd[128];
	//sprintf (cmd, "echo \"tzstr:%s  (index:%d)\" | cat >> /mnt/ipm/tz.txt", tzstr, index);
	//system (cmd);

	setenv ("TZ", tzstr, 1);

	tzset();
}

void replace_java_player_path(char* page)
{
}


int preview_size(_CAMSETUP *setup, int *width, int *height)
{
	float w = (float)def_resol_index_to_width(0, setup->cam[0].res);
	float h = (float)def_resol_index_to_height(0, setup->cam[0].res);
	float rwh = w/h;

	*height = *width/rwh;

	return 0;
}
