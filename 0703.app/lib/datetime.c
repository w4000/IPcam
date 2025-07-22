/* datetime.c
 *
 * 	purpose
 *		- Date & time string manipulation.
 **********************************************************************************/
#include "common.h"
#if 1
#include "../tools/new_rsetup_jQuery/cgi-bin/jQuery_language.h"
#else 
#endif 
char *g_short_month_val_ENG[12] = RES_S_MONTH_ARRAY(ENG);
char *g_short_month_val_KOR[12] = RES_S_MONTH_ARRAY(KOR);


#define HOURTOSEC	(3600)

/**********************************************************************************
 * Timezone setting
 *********************************************************************************/
// We set /etc/TZ file or TZ environment variable with these.
// This is uClibc style setting.
char * tmzone_set [TMZONE_MAX] = {
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

char * tmzone_str [TMZONE_MAX][4] = {
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



#include "datetime_glibc.h"

#define printf_datetime(x,y) printf(x,y) //w4000
void datetime_tzset (int index) {
	char cmd [512] = {0};

printf_datetime("%s \r\n",__func__);

	do {
		if ((index < 0) || (index > (TMZONE_MAX -1)))
			break;

	

		snprintf (cmd, sizeof(cmd), "ln -sf /usr/share/zoneinfo/%s /etc/localtime", tmzone_zoneifo [index]);
		system (cmd);


	} while (0);

	tzset();
}

/**********************************************************************************
 * System time setting.
 *********************************************************************************/

bool datetime_isdst_end(struct tm *ptm)
{
	time_t end, now;
	struct tm *dst = &g_main.dst_end;
	printf_datetime("%s \r\n",__func__);


	if (strlen(tmzone_str[g_setup.tmzone][1]) == 0)
		return FALSE;

	end = __mktime(dst);
	now = __mktime(ptm);

	if (now >= (end -HOURTOSEC) && now < end)
		return TRUE;

	return FALSE;
}

#include <linux/rtc.h>
#define RTC_DEV "/dev/rtc0"

// Set system datetime.
void datetime_systohc ()//w4000_tm
{
	const char *dev_name = RTC_DEV;
	int fd = open(dev_name, O_RDWR);
	printf_datetime("%s \r\n",__func__);
	if (fd) {
		time_t     current_time;
		struct tm *struct_time;
		time( &current_time);
		struct_time = localtime( &current_time);

		struct rtc_time rtc_time;
		rtc_time.tm_year = struct_time->tm_year;
		rtc_time.tm_mon = struct_time->tm_mon;
		rtc_time.tm_mday = struct_time->tm_mday;
		rtc_time.tm_hour = struct_time->tm_hour;
		rtc_time.tm_min = struct_time->tm_min;
		rtc_time.tm_sec = struct_time->tm_sec;
		if(ioctl(fd, RTC_SET_TIME, &rtc_time) < 0) {
			// rtc fail !
		}
		close(fd);
	}
}

static void reset_rtc_time()
{
	struct rtc_time rtc_time;
	rtc_time.tm_year = 2000-1900;
	rtc_time.tm_mon = 0;
	rtc_time.tm_mday = 1;
	rtc_time.tm_hour = 0;
	rtc_time.tm_min = 0;
	rtc_time.tm_sec = 0;

	char szDate[64];
	printf_datetime("%s \r\n",__func__);
	sprintf(szDate, "date -s %04d.%02d.%02d-%02d:%02d:%02d",
			rtc_time.tm_year+1900, rtc_time.tm_mon+1, rtc_time.tm_mday, rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);
	system(szDate);

	printf("RTC Time Reset (%04d-%02d-%02d %02d:%02d:%02d)\n",
			rtc_time.tm_year+1900, rtc_time.tm_mon+1, rtc_time.tm_mday, rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);


	datetime_systohc();
}

void datetime_hctosys (void)
{
	int reset_time = 0;
	const char *dev_name = RTC_DEV;
	int fd = open(dev_name, O_RDWR);
	printf_datetime("%s \r\n",__func__);

	if (fd) {
		struct rtc_time rtc_time;
		int ret = ioctl(fd, RTC_RD_TIME, &rtc_time);
		if (ret >= 0) {
			if(rtc_time.tm_year+1900 < 2000) {
				reset_time = 1;
			}
			else {
				char szDate[64];
				sprintf(szDate, "date -s %04d.%02d.%02d-%02d:%02d:%02d",
						rtc_time.tm_year+1900, rtc_time.tm_mon+1, rtc_time.tm_mday, rtc_time.tm_hour, rtc_time.tm_min, rtc_time.tm_sec);
				system(szDate);
			}
		}
		else {
			reset_time = 1;
			printf("Failed to ioctl RTC_RD_TIME!!!\n");
		}

		close(fd);
	}
	else {
		reset_time = 1;
		printf("Failed to open %s!!!\n", RTC_DEV);
	}

	if(reset_time) {
		reset_rtc_time();
	}

}


char** get_short_month_val(int lang)
{
	char** month_val = g_short_month_val_ENG;
	printf_datetime("%s \r\n",__func__);
	switch (lang)
	{
		case 1: month_val = g_short_month_val_KOR;	break;
	}
	return month_val;
}

/**********************************************************************************
 * Datetime string conversion.
 *********************************************************************************/
char * __DateToString (int iDateFormat, char *buf, struct tm *pstTm) {

	int lang = SETUP_LANG(&g_setup);

	char** pMonthStr = get_short_month_val(lang);
	printf_datetime("%s \r\n",__func__);

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

char * __TimeToString (int iTimeFormat, char *buf, struct tm *pstTm) {

	int lang = SETUP_LANG(&g_setup);
	printf_datetime("%s \r\n",__func__);

	if( iTimeFormat == FORMAT_AMPM )
	{
		sprintf(buf , " %02d:%02d:%02d %s",
				(pstTm->tm_hour + 11 ) % 12 + 1,
				pstTm->tm_min,
				pstTm->tm_sec,
				pstTm->tm_hour / 12 ? g_szPM[lang] : g_szAM[lang]
				);
	}
	else if( iTimeFormat == FORMAT_AMPM_FRONT )
	{
		sprintf(buf , " %s %02d:%02d:%02d",
				pstTm->tm_hour / 12 ? g_szPM[lang] : g_szAM[lang],
				(pstTm->tm_hour + 11 ) % 12 + 1,
				pstTm->tm_min,
				pstTm->tm_sec
				);
	}
	else
	{
		sprintf(buf, " %02d:%02d:%02d",
				pstTm->tm_hour,
				pstTm->tm_min,
				pstTm->tm_sec
				);
	}
	return buf;
}


char *TMToStringDelimiter(int iDateFormat, int iTimeFormat, char * buf, struct tm *pstTm, char * delimiter) {

	char date[32], time[32];
	printf_datetime("%s \r\n",__func__);
	__DateToString (iDateFormat, date, pstTm);
	__TimeToString (iTimeFormat, time, pstTm);
	sprintf(buf, "%s%s%s", date, delimiter, time);

	return buf;
}

/******************************************************************************
 * Calculating time.
 ******************************************************************************/
// Add given secs to local time.

void tm_minussec(struct tm * tm, int sec)
{
	time_t t;

	assert (tm);
	printf_datetime("%s \r\n",__func__);

	t = __mktime (tm);
	t = t - sec;
	localtime_r (& t, tm);
}

void tm_addsec2 (struct tm * tm, int s)
{
	int hour, min, sec;

	hour = s /3600;
	s -= (hour *3600);
	min = s / 60;
	s -= (min *60);
	sec = s;
	printf_datetime("%s \r\n",__func__);

	tm->tm_hour += hour;
	tm->tm_min += min;
	tm->tm_sec += sec;
}





/* Sleep while requested duration without
 * affecting any signals.
 */
void sleep_ex (long usec) {
	struct timespec req, rem;

	if (usec < 0)
		return;
	//printf_datetime("%s \r\n",__func__);

	req.tv_sec  = (usec / 1000000);
	req.tv_nsec = (usec % 1000000) * 1000;

	if (nanosleep (& req, & rem) < 0) {
		perror ("nanosleep");
	}
}


