#include "common.h"
#include "vfb.h"
#include "osd.h"
#include "osd_datetime.h"

#if 1
void woo_e(void)
{
		//char str[1024], buf[1024 * 10], timezone[128] = {0}, date_ex[64], time_ex[64];;
		char str[1024];
		//char* selptr;
		//int i;
	
		time_t t;
		struct tm tm;
	
		//datetime_tzset_2(&r_setup, r_setup.tmzone, timezone);
	
		t = ({ struct timeval tv; gettimeofday (& tv, NULL); tv.tv_sec; });
	
		localtime_r (&t, &tm);
	#if 0
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
		printf("%s/",str);
		sprintf(str, "%d", tm.tm_mon);
		printf("%s/",str);
		sprintf(str, "%d", tm.tm_mday);
		printf("%s   ",str);
		sprintf(str, "%d", tm.tm_hour);
		printf("%s:",str);
		sprintf(str, "%d", tm.tm_min);
		printf("%s:",str);
		sprintf(str, "%d", tm.tm_sec);
		printf("%s\n",str);
	
		
	
	}


int getDateTimeToText(char* osd)
{
	char str[1024];
    struct tm tm;
	#if 1
	time_t t = time(NULL);
    localtime_r(&t, &tm);
	#else//org
    memcpy(&tm, &IPCAM_THREAD_TIMER->current, sizeof(struct tm));
	#endif
    int datefmt = g_setup.datefmt;
    int timefmt = g_setup.timefmt;
    TMToStringDelimiter(datefmt, timefmt, osd, &tm, " ");
	//printf("w4000 m  %s, %d\n",__FUNCTION__,__LINE__);
    //printf("osd: %s\n", osd);
    //woo_e();

    return 0;
}

#elif 1
double t_zone[]={
	0 ,//-13
	-12 ,
	-11 ,
	-10 ,
	-9	,
	-8	,
	-8	,
	-7	,
	-7	,
	-7	,
	-6	,
	-6	,
	-6	,
	-6	,
	-5	,
	-5	,
	-5	,
	-4.5	,
	-4	,
	-4	,
	-4	,
	-4	,
	-3.5	,
	-3	,
	-3	,
	-3	,
	-3	,
	-3	,
	-2	,
	-1	,
	-1	,
	0	,
	0	,
	0	,
	1	,
	1	,
	1	,
	1	,
	1	,
	2	,
	2	,
	2	,
	2	,
	2	,
	2	,
	2	,
	2	,
	2	,
	3	,
	3	,
	3	,
	3	,
	3	,
	3.5 ,
	4	,
	4	,
	4	,
	4	,
	4	,
	4.5 ,
	5	,
	5	,
	5	,
	5.5 ,
	5.5 ,
	5.5 ,
	6	,
	6	,
	6.5 ,
	7	,
	7	,
	8	,
	8	,
	8	,
	8	,
	8	,
	9	,
	9	,
	9	,
	9.5 ,
	9.5 ,
	10	,
	10	,
	10	,
	10	,
	10	,
	11	,
	12	,
	12	,
	13	,

};
int getDateTimeToText(char* osd)
{
	static int i=0;
    struct tm tm;
	char* osd1,osd2,osd3;
    memcpy(&tm, &IPCAM_THREAD_TIMER->current, sizeof(struct tm));
    int datefmt = g_setup.datefmt;
    int timefmt = g_setup.timefmt;
    TMToStringDelimiter(datefmt, timefmt, osd, &tm, " ");

	if(i)i--;else i=400;
	if(i%100==0)
    TMToStringDelimiter(datefmt, timefmt, osd1, &g_setup.dst_stm, " ");
	if(i%201==0)
    TMToStringDelimiter(datefmt, timefmt, osd2, &g_setup.dst_stm, " ");
	if(i%302==0)
    TMToStringDelimiter(datefmt, timefmt, osd2, &g_setup.sv_tm, " ");

	
    printf("osd: %s\n", osd);
    printf("osd1: %s\n", osd1);
    printf("osd2: %s\n", osd2);
    printf("osd3: %s\n", osd3);

    return 0;
}

#elif 1
int getDateTimeToText(char* osd) {
    struct tm tm;
    time_t now = time(NULL);
    if (localtime_r(&now, &tm) == NULL) {
        perror("localtime_r failed");
        return -1;
    }

    size_t t_zone_size = sizeof(t_zone) / sizeof(t_zone[0]);
    int tmzone = g_setup.tmzone;
    if (tmzone < 0 || tmzone >= t_zone_size) {
        fprintf(stderr, "Invalid tmzone value: %d.  Value must be between 0 and %zu\n", tmzone, t_zone_size -1);
        return -1;
    }

    double offset = t_zone[tmzone];
    tm.tm_hour += (int)offset;
    tm.tm_min += (int)((offset - (int)offset) * 60);

    tm.tm_hour += tm.tm_min / 60;
    tm.tm_min %= 60;

    int datefmt = g_setup.datefmt;
    int timefmt = g_setup.timefmt;

    if (TMToStringDelimiter(datefmt, timefmt, osd, &tm, " ") != 0) 
		{
		//return -1;
    	}

    printf("time >> %s\n", osd); // 시간 출력

    return 0;
}



#elif 1

int getDateTimeToText(char* osd)
{
    struct tm tm;
    memcpy(&tm, &IPCAM_THREAD_TIMER->current, sizeof(struct tm));
    int datefmt = g_setup.datefmt;
    int timefmt = g_setup.timefmt;

	// 시간대 보정 (t_zone 배열 사용)
	double offset = t_zone[g_setup.tmzone]; //  t_zone 배열의 인덱스를 이용하여 오프셋 가져오기
	tm.tm_hour += (int)offset; // 시간 보정
	tm.tm_min += (int)((offset - (int)offset) * 60); // 분 보정 (소수점 이하 분 처리)
	
	//	분이 60을 초과하거나 미만인 경우 조정
	tm.tm_hour += tm.tm_min / 60;
	tm.tm_min %= 60;
	
    TMToStringDelimiter(datefmt, timefmt, osd, &tm, " ");

    return 0;
}

#elif 1
int get_tmzone(int val)//w4000
{
	switch(val)
	{
		
		
		case 0: return	0	;	//	None,
		case 1: return	2	;	//	(GMT-12) International Date Line West,
		case 2: return	4	;	//	(GMT-11) Midway Island, Samoa,
		case 3: return	6	;	//	(GMT-10) Hawaii,
		case 4: return	8	;	//	(GMT-9) Alaska,
		case 5: return	10	;	//	(GMT-8) Pacific Time(US&Canada),
		case 6: return	10	;	//	(GMT-8) Tijuana, Baja California,
		case 7: return	12	;	//	(GMT-7) Arizona,
		case 8: return	12	;	//	(GMT-7) Chihuahua, La Paz, Mazatha,
		case 9: return	12	;	//	(GMT-7) Mountain Time(US&Canada),
		case 10:	return	14	;	//	(GMT-6) Central America,
		case 11:	return	14	;	//	(GMT-6) Central Time(US&Canada),
		case 12:	return	14	;	//	(GMT-6) Guadalajara, Mexico City, Monterrey,
		case 13:	return	14	;	//	(GMT-6) Saskatchewan,
		case 14:	return	16	;	//	(GMT-5) Bogota, Lima, Quito, Rio Branco,
		case 15:	return	16	;	//	(GMT-5) Eastern Time(US&Canada),
		case 16:	return	16	;	//	(GMT-5) Indiana(East),
		case 17:	return	17	;	//	(GMT-4:30) Caracas,
		case 18:	return	18	;	//	(GMT-4) Atlantic Time(Canada),
		case 19:	return	18	;	//	(GMT-4) La Paz,
		case 20:	return	18	;	//	(GMT-4) Manaus,
		case 21:	return	18	;	//	(GMT-4) Santiago,
		case 22:	return	19	;	//	(GMT-3:30) Newfoundland,
		case 23:	return	20	;	//	(GMT-3) Brasilia,
		case 24:	return	20	;	//	(GMT-3) Buenos Aires,
		case 25:	return	20	;	//	(GMT-3) Georgetown,
		case 26:	return	20	;	//	(GMT-3) Greenland,
		case 27:	return	20	;	//	(GMT-3) Montevideo,
		case 28:	return	22	;	//	(GMT-2) Mid-Atlantic,
		case 29:	return	24	;	//	(GMT-1) Azores,
		case 30:	return	24	;	//	(GMT-1) Cape Verde Is.,
		case 31:	return	26	;	//	(GMT+0) Casablanca,
		case 32:	return	26	;	//	(GMT+0) Dublin, Edinburgh, Lisbon, London,
		case 33:	return	26	;	//	(GMT+0) Monrovia, Reykjavik,
		case 34:	return	28	;	//	(GMT+1) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna,
		case 35:	return	28	;	//	(GMT+1) Belgrade, Bratislava, Budapest, Ljubljana, Prague,
		case 36:	return	28	;	//	(GMT+1) Brussels, Copenhagen, Madrid, Paris,
		case 37:	return	28	;	//	(GMT+1) Sarajevo, Skopje, Warsaw, Zagreb,
		case 38:	return	28	;	//	(GMT+1) West Central Africa,
		case 39:	return	30	;	//	(GMT+2) Amman,
		case 40:	return	30	;	//	(GMT+2) Athens, Bucharest, Istanbul,
		case 41:	return	30	;	//	(GMT+2) Beirut,
		case 42:	return	30	;	//	(GMT+2) Cairo,
		case 43:	return	30	;	//	(GMT+2) Harare, Pretoria,
		case 44:	return	30	;	//	(GMT+2) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius,
		case 45:	return	30	;	//	(GMT+2) Jerusalem,
		case 46:	return	30	;	//	(GMT+2) Minsk,
		case 47:	return	30	;	//	(GMT+2) Windhoek,
		case 48:	return	32	;	//	(GMT+3) Baghdad,
		case 49:	return	32	;	//	(GMT+3) Kuwait, Riyadh,
		case 50:	return	32	;	//	(GMT+3) Moscos, St. Petersburg, Volgograd,
		case 51:	return	32	;	//	(GMT+3) Nairobi,
		case 52:	return	32	;	//	(GMT+3) Tbilisi,
		case 53:	return	33	;	//	(GMT+3:30) Tehran,
		case 54:	return	34	;	//	(GMT+4) Abu Dhabi, Muscat,
		case 55:	return	34	;	//	(GMT+4) Baku,
		case 56:	return	34	;	//	(GMT+4) Caucasus Standard Time,
		case 57:	return	34	;	//	(GMT+4) Port Louis,
		case 58:	return	34	;	//	(GMT+4) Yerevan,
		case 59:	return	35	;	//	(GMT+4:30) Kabul,
		case 60:	return	36	;	//	(GMT+5) Ekaterinburg,
		case 61:	return	36	;	//	(GMT+5) Islamabad, Karachi,
		case 62:	return	36	;	//	(GMT+5) Tashkent,
		case 63:	return	37	;	//	(GMT+5:30) Chennai, Kalkata, Mumbai, New Delhi,
		case 64:	return	37	;	//	(GMT+5:30) Sri Jayawardenepura,
		case 65:	return	37	;	//	(GMT+5:45) Kathmandu,
		case 66:	return	38	;	//	(GMT+6) Almaty, Novosibirsk,
		case 67:	return	38	;	//	(GMT+6) Astana, Dhaka,
		case 68:	return	39	;	//	(GMT+6:30) Yangon(Rangoon),
		case 69:	return	40	;	//	(GMT+7) Bangkok, Hanoi, Jakarta,
		case 70:	return	40	;	//	(GMT+7) Krasnoyarsk,
		case 71:	return	42	;	//	(GMT+8) Beijing, Chonging, Hong Kong, Urumqi,
		case 72:	return	42	;	//	(GMT+8) Irkutsk, Ulaan Bataar,
		case 73:	return	42	;	//	(GMT+8) Kuala Lumpur, Singapore,
		case 74:	return	42	;	//	(GMT+8) Perth,
		case 75:	return	42	;	//	(GMT+8) Taipei,
		case 76:	return	44	;	//	(GMT+9) Osaka, Sapporo, Tokyo,
		case 77:	return	44	;	//	(GMT+9) Seoul,
		case 78:	return	44	;	//	(GMT+9) Yakutsk,
		case 79:	return	45	;	//	(GMT+9:30) Adelaide,
		case 80:	return	45	;	//	(GMT+9:30) Darwin,
		case 81:	return	46	;	//	(GMT+10) Brisbane,
		case 82:	return	46	;	//	(GMT+10) Canberra, Melbourne, Sydney,
		case 83:	return	46	;	//	(GMT+10) Guam, Port Moresby,
		case 84:	return	46	;	//	(GMT+10) Hobart,
		case 85:	return	46	;	//	(GMT+10) Vladivostok,
		case 86:	return	48	;	//	(GMT+11) Magadan, Solomon Is., New Caledonia,
		case 87:	return	50	;	//	(GMT+12) Auckland, Wellington,
		case 88:	return	50	;	//	(GMT+12) Fiji, Kamchatka, Marshall Is.,
		case 89:	return	52	;	//	(GMT+13) Nuku'alofa
		
		default:
			break;
	}

	return	255	;
}
int getDateTimeToText(char* osd)
{
    struct tm tm;
    memcpy(&tm, &IPCAM_THREAD_TIMER->current, sizeof(struct tm));

    // 날짜 및 시간 포맷 가져오기
    int datefmt = g_setup.datefmt;
    int timefmt = g_setup.timefmt;

    // 날짜와 시간을 문자열로 변환
    TMToStringDelimiter(datefmt, timefmt, osd, &tm, " ");

    // 타임존 오프셋을 콘솔에 출력
    printf("Current Timezone Offset: %d hours\n", get_tmzone(g_setup.tmzone)/2);

    // 타임존을 적용한 시간을 구하기 위해 raw_time 계산
    time_t raw_time = timegm(&tm);   // UTC로 변환
    raw_time =raw_time-(((get_tmzone(g_setup.tmzone)/2)-11) * 3600); // 타임존 보정

    // 업데이트된 시간 문자열을 프린트
    char time_buffer[100];  // 시간을 출력할 버퍼
    struct tm *local_tm = gmtime(&raw_time); // UTC와 비교하려면 gmtime 사용
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_tm);  // 원하는 형식으로 시간 변환
    printf("Updated Date and Time after applying timezone: %s\n", time_buffer);  // 출력

    return 0;
}


#else //w4000
int getDateTimeToText(char* osd)
{
    struct tm tm;
    memcpy(&tm, &IPCAM_THREAD_TIMER->current, sizeof(struct tm));
    
    // 타임존 적용을 위해 tm_gmtoff를 이용해 UTC를 로컬 시간으로 변환합니다.
    time_t raw_time = timegm(&tm);  // tm을 UTC 시간으로 변환
    raw_time += g_setup.tmzone * 3600; // tmzone 시간(초 단위) 추가

    // 변환된 시간으로 tm 구조체를 다시 가져옵니다.
    struct tm *local_tm = localtime(&raw_time);
    
    // 날짜 및 시간 포맷 가져오기
    int datefmt = g_setup.datefmt;
    int timefmt = g_setup.timefmt;
    
    // 형식화 함수 호출
    TMToStringDelimiter(datefmt, timefmt, osd, local_tm, " ");

    return 0;
}

#endif
int getDateTimeOsdArea(int id, OSD_AREA_S *tm_area)
{
	if(id != OSD_1ST_AREA && id != OSD_2ND_AREA) {
		return -1;
	}

	int posIndex = 0;
	tm_area->x = osd_position[posIndex][OSD_AREA_X];
	tm_area->y = osd_position[posIndex][OSD_AREA_Y];
	tm_area->max_width = INIT_MAX_WIDTH;
	tm_area->max_height = INIT_MAX_HEIGHT;
	tm_area->areaPosIndex = posIndex;

    return posIndex;
}

int getDateTimeOsdId(void)
{
	if(0 == g_setup.tmosd.position ) {
        return OSD_1ST_AREA;
    }
    else {
        return OSD_2ND_AREA;
    }
}

void osd_datetime_novatek(void)
{

	static int tm_usage = 0;
	if(g_setup.tmosd.usage == 0 && tm_usage == 0) {
		//w4000 
		return;
	}

	static int tm_pos = -1;
	int id;
	id = tm_pos;

    // current id by position
	if(tm_pos < 0) {
        id = getDateTimeOsdId();
	}

	OSD_AREA_S tm_area;
	tm_area.areaPosIndex = -1;
	tm_area.areaIndex = OSD_DATETIME_ID;

	if(g_setup.tmosd.usage == 0 && tm_usage == 1) {
		tm_usage = 0;
		if(tm_pos >= 0) {
			getDateTimeOsdArea(tm_pos, &tm_area);
			osd_hide_text(tm_pos, &tm_area);
		}
        return;
	}

    tm_usage = 1;
    if(tm_pos != getDateTimeOsdId()) {
        if(tm_pos >= 0) {
			getDateTimeOsdArea(tm_pos, &tm_area);
            osd_hide_text(id, &tm_area);
        }
		tm_pos = getDateTimeOsdId();
		getDateTimeOsdArea(tm_pos, &tm_area);
    }

    OSD_COLOR_E color;
    color = getDatetimeOsdColor();

	if(tm_area.areaPosIndex < 0) {
		getDateTimeOsdArea(tm_pos, &tm_area);
	}

    setDefaultTextSize();

    enum { OSD_BUF_SIZE = 128, };
	char osd[OSD_BUF_SIZE];
    getDateTimeToText(osd);
	//printf("w4000 osd_datatime=%s\r\n",osd);

    osd_draw_text(tm_pos, &tm_area, osd, color, getTextSize());
}
