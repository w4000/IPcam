
#include <string.h>
#include <time.h>
#include "sys_inc.h"
#include "onvif_utils.h"
#include "onvif.h"

#include "../bm/cam_timezone.h"
#include "../../setup/setup.h"

void onvif_get_time_str(char * buff, int len, int sec_off)
{
	time_t nowtime;
	struct tm *gtime;

	time(&nowtime);
	nowtime += sec_off;
	gtime = gmtime(&nowtime);

	snprintf(buff, len, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		gtime->tm_year+1900, gtime->tm_mon+1, gtime->tm_mday,
		gtime->tm_hour, gtime->tm_min, gtime->tm_sec);
}

void onvif_get_time_str_s(char * buff, int len, time_t nowtime, int sec_off)
{
	struct tm *gtime;

	nowtime += sec_off;
	gtime = gmtime(&nowtime);

	snprintf(buff, len, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		gtime->tm_year+1900, gtime->tm_mon+1, gtime->tm_mday,
		gtime->tm_hour, gtime->tm_min, gtime->tm_sec);
}


void onvif_get_timezone(char * tz, int len)
{
	/*
    int time_zone;
	int local_hour, utc_hour;
	time_t time_utc;
    struct tm *tm_local;
  	struct tm *tm_utc;

    // Get the UTC time
    time(&time_utc);

    // Get the local time
    tm_local = localtime(&time_utc);
    local_hour = tm_local->tm_hour;

    // Change it to GMT tm
    tm_utc = gmtime(&time_utc);
    utc_hour = tm_utc->tm_hour;

    time_zone = local_hour - utc_hour;
    if (time_zone < -12)
    {
        time_zone += 24;
    }
    else if (time_zone > 12)
    {
        time_zone -= 24;
    }

	snprintf(tz, len, "PST%dPDT", time_zone);
	*/

	char tz_string[64] = "GMT+0BST,M3.5.0,M10.5.0";
	int i = 0;

	for (i = 0;i<TMZONE_MAX; i++)
	{
		if (strcmp(cam_tmzone2_str[i].TimezoneName, cam_tmzone_set[g_setup.tmzone]) == 0)
		{
			strcpy(tz_string, cam_tmzone2_str[i].TZ);
			break;
		}
	}
	snprintf(tz, len, "%s", tz_string);
}


const char * onvif_uuid_create()
{
	static char uuid[100];
    static int seed = 1;
	uint8 mac[6];

	if(get_default_if_mac(mac) < 0) {
		srand((uint32)time(NULL) + seed++);

		sprintf(uuid, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
				rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF,
				rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF);
	}
	else {
		sprintf(uuid, "AF68E009-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],
				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	}

	return uuid;
}

const char * onvif_get_local_ip()
{
	struct in_addr addr;
    addr.s_addr = get_default_if_ip();

	return inet_ntoa(addr);
}

time_t onvif_timegm(struct tm *T)
{
	time_t t, g, z;
	struct tm tm;

	t = mktime(T);
	if (t == (time_t)-1)
	{
		return (time_t)-1;
	}

	tm = *gmtime(&t);

	tm.tm_isdst = 0;
	g = mktime(&tm);
	if (g == (time_t)-1)
	{
		return (time_t)-1;
	}

	z = g - t;
	return t - z;
}

int onvif_parse_xaddr_(const char * pdata, char * host, char * url, int * port, int * https)
{
    const char *p1, *p2;
    int len = (int)strlen(pdata);
    int prefixlen = 0;

    *port = 80;

    if (len > 7) // skip "http://"
    {
    	if (strncmp(pdata, "https://", 8) == 0)
    	{
    		prefixlen = 8;
    		p1 = pdata+8;
    		*https = 1;
    	}
    	else
    	{
    		prefixlen = 7;
    		p1 = pdata+7;
    		*https = 0;
    	}

        p1 = strchr(pdata+prefixlen, ':');
        if (p1)
        {
			char buff[100];

            strncpy(host, pdata+prefixlen, p1-pdata-prefixlen);
            host[p1-pdata-prefixlen] = '\0';
            memset(buff, 0, 100);

            p2 = strchr(p1, '/');
            if (p2)
            {
                strncpy(url, p2, len - (p2 - pdata));
                url[len - (p2 - pdata)] = '\0';

                len = (int)(p2 - p1 - 1);
                strncpy(buff, p1+1, len);
            }
            else
            {
                len = (int)(len - (p1 - pdata));
                strncpy(buff, p1+1, len);
            }

            *port = atoi(buff);
        }
        else
        {
            p2 = strchr(pdata+prefixlen, '/');
            if (p2)
            {
                strncpy(url, p2, len - (p2 - pdata));
                url[len - (p2 - pdata)] = '\0';

                len = (int)(p2 - pdata - prefixlen);
                strncpy(host, pdata+prefixlen, len);
                host[len] = '\0';
            }
            else
            {
                len = len - prefixlen;
                strncpy(host, pdata+prefixlen, len);
                host[len] = '\0';
            }
        }
    }

    return 1;
}

int onvif_parse_xaddr(const char * pdata, char * host, char * url, int * port, int * https)
{
	int size;
    const char *p1, *p2;
    char xaddr[256];

    p2 = pdata;
    p1 = strchr(pdata, ' ');
    while (p1)
    {
    	size = (int)(p1 - p2);
    	size = size >= sizeof(xaddr) ? sizeof(xaddr)-1 : size;
        strncpy(xaddr, p2, size);
        xaddr[size] = '\0';

        onvif_parse_xaddr_(xaddr, host, url, port, https);
        if (is_ip_address(host) && strncmp(host, "169.", 4)) // if ipv4 address
        {
            return 1;
        }

        p2 = p1+1;
        p1 = strchr(p2, ' ');
    }

	if (p2)
	{
		onvif_parse_xaddr_(p2, host, url, port, https);
	}

    return 1;
}

/***************************************************************************************/

time_t onvif_datetime_to_time_t(onvif_DateTime * p_datetime)
{
    struct tm t1;

    t1.tm_year = p_datetime->Date.Year - 1900;
    t1.tm_mon = p_datetime->Date.Month - 1;
    t1.tm_mday = p_datetime->Date.Day;
    t1.tm_hour = p_datetime->Time.Hour;
    t1.tm_min = p_datetime->Time.Minute;
    t1.tm_sec = p_datetime->Time.Second;

    return mktime(&t1);
}

void onvif_time_t_to_datetime(time_t n, onvif_DateTime * p_datetime)
{
    struct tm *t1 = gmtime(&n);

	if (t1)
	{
	    p_datetime->Date.Year = t1->tm_year + 1900;
	    p_datetime->Date.Month = t1->tm_mon + 1;
	    p_datetime->Date.Day = t1->tm_mday;
	    p_datetime->Time.Hour = t1->tm_hour;
	    p_datetime->Time.Minute = t1->tm_min;
	    p_datetime->Time.Second = t1->tm_sec;
    }
}

const char * onvif_format_datetime_str(time_t n, int flag /*0 -- local, 1 -- utc*/, const char * format)
{
    struct tm *t1;
  	static char buff[100];

	memset(buff, 0, sizeof(buff));

    if (flag == 1)
    {
  	    t1 = gmtime(&n);
  	}
  	else
  	{
  	    t1 = localtime(&n);
  	}

  	if (t1)
  	{
		strftime(buff, sizeof(buff), format, t1);
	}

	return buff;
}

int onvif_parse_uri(const char * p_in, char * p_out, int outlen)
{
    int inlen = (int)strlen(p_in);
    char *p;

    if (inlen < 8 || outlen < inlen)
    {
        return -1;
    }

	strcpy(p_out, p_in);

    // replace "&amp;" with "&"

	p = strstr(p_out, "&amp;");
	while (p)
	{
		memmove(p+1, p+5, strlen(p+5));
		p = strstr(p+5, "&amp;");
		p_out[strlen(p_out)-4] = '\0';
	}

	return 0;
}

int get_mac_address(char *szMacAddress){
	const char * MACADDR_NAME = "ethaddr";
	const char * MACADDR_POS = "ethaddr=";
	const char * MACADDR_FNAME = "/sys/class/net/eth0/address";
	const int MACADDR_LEN = 17;

	FILE *fp = NULL;
	char szTemp[128];
	char bRead = 0;

	fp = fopen(MACADDR_FNAME, "r");
	if(fp) {
		char *pStart;
		int mac[6];
		int nItem;

		memset(szTemp, 0x00, sizeof(szTemp));
		fgets(szTemp, sizeof(szTemp), fp);
		fclose(fp);
		unlink(MACADDR_FNAME);

		if(strlen( szTemp + strlen(MACADDR_POS)) >= MACADDR_LEN){
			pStart = szTemp + strlen(MACADDR_POS);
			nItem = sscanf( pStart, "%s02X:%s02X:%s02X:%s02X:%s02X:%s02X", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
			if(nItem == 6) {
				sprintf(szMacAddress, "%02X-%02X-%02X-%02X-%02X-%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				bRead = 1;
			}
		}
	}

	if(bRead == 0) {
		int fd, e;
		struct ifreq ifr;
		fd = socket(AF_INET, SOCK_DGRAM, 0);
		if(fd > 0) {
			ifr.ifr_addr.sa_family = AF_INET;
			strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
			e = ioctl(fd, SIOCGIFHWADDR, &ifr);
			if( e >= 0) {
				sprintf(szMacAddress, "%02X-%02X-%02X-%02X-%02X-%02X",
						(uint8_t)ifr.ifr_hwaddr.sa_data[0],
						(uint8_t)ifr.ifr_hwaddr.sa_data[1],
						(uint8_t)ifr.ifr_hwaddr.sa_data[2],
						(uint8_t)ifr.ifr_hwaddr.sa_data[3],
						(uint8_t)ifr.ifr_hwaddr.sa_data[4],
						(uint8_t)ifr.ifr_hwaddr.sa_data[5]);
				bRead = 1;
			}
			close(fd);
		}
	}

	if(bRead == 1) return 1;
	else return -1;

}





