

#include "sys_inc.h"
#include "onvif.h"
#include "soap_parser.h"
#include "onvif_utils.h"
#include "util.h"


/***************************************************************************************/
#define printf_function_soap_parser()printf("w4000 1 %s, %d \n",__FUNCTION__,__LINE__)//w4000


#if 0
#define	_TZ_DBG_	1

#define ATOI(n)		(n-'0')
#define ISNULL(n) 	(n == NULL || n == ' ')? 1:0


int parse_tz2(char *src, char dst[], int sz, char *h, char *m, char *lb, char token[])
{
	char *start = src;
	char *pos = NULL;

	if (strstr(src, "GMT"))
	{
		pos = strstr(src, "GMT");		// (GMT+9) Seoul"
	}
	else if (strstr(src, "PST"))
	{
		pos = strstr(src, "PST");
	}

	if (pos == NULL)
	{
		return -1;
	}

	pos = pos+3;
	int name_cp_flag =0;

	while ( (int)(pos-start) < sz )
	{
		if (*pos == token[0] || *pos == token[1])
		{
			strncpy(lb, pos, sizeof(char));
		}
		else if (isdigit(*pos))
		{
			if ( !isdigit(*(pos+1)) )
			{
				sprintf(h, "%d", ATOI(*pos));
				name_cp_flag++;
			}
			else if ( !ISNULL(*(pos+1)) && isdigit(*(pos+1)) )
			{
				sprintf(h, "%d%d", ATOI(*pos), ATOI(*(pos+1)) );
				pos++;
				name_cp_flag++;
			}
		}
		else if (*pos == token[2])
		{
			if (( !ISNULL(*(pos+1)) && isdigit(*(pos+1)) ) && ( !ISNULL(*(pos+2)) && isdigit(*(pos+2)) ) )
			{
				sprintf(m, "%d%d", ATOI(*(pos+1)), ATOI(*(pos+2)) );

				pos = pos +2;
				name_cp_flag++;
			}
			else if ( !ISNULL(*(pos+1)) && isdigit(*(pos+1)) )
			{
				sprintf(m, "%d", ATOI(*(pos+1)));

				pos++;
				name_cp_flag++;
			}
		}
		else if (name_cp_flag && !isdigit(*pos) && *pos != token[3])
		{
			while (!isdigit(*pos) && *pos != '\0' && (int)(pos-start) < sz)
			{
				strncat(dst, pos, sizeof(char));
				pos++;
			}
		}
		pos++;
	}
	return 0;
}


BOOL *parse_tz(char *src, manager_tz_dt *p_dt)
{
	char lb[2] = {0x00, };
	char  h[4] = {0x00, };
	char  m[4] = {0x00, };


	char tz_name[64];
	memset(tz_name, 0x00, 64);

	const char label[] = {'-', '+', ':', ')'};

	if (src == NULL)
	{
		return NULL;
	}

	char *start = src;
	char *p = src;
	int end =0;

	if (strstr(p, "GMT"))
	{
		p_dt->tz_type = GMT;
		parse_tz2(src, tz_name, strlen(src), h, m, lb, label);
	}
	else if (strstr(p, "PST"))
	{
		p_dt->tz_type = PST;
		parse_tz2(src, tz_name, strlen(src), h, m, lb, label);
	}
	else
	{
		p_dt->tz_type = UNKNOWN;
		while ( !isdigit(*p) && *p != label[0] && *p != label[1] && (int)(p-start) < strlen(src) )
		{
			end++;
			p++;
		}

		if (end)
		{
			strncpy(tz_name, start, end);
		}
		else
		{
			return NULL;
		}

		start = p;
		end =0;

		while ( p-src < strlen(src) && end == 0)
		{
			if (*p == label[0] || *p == label[1])
			{
				strncpy(lb, p, sizeof(char));
			}
			else if (isdigit(*p))
			{
				if ( ( !isdigit(*(p+1)) && !ISNULL(*(p+1)) && *(p+1) == label[2] ) || ISNULL(*(p+1)) && ISNULL(*(p+2)))
				{
					sprintf(h, "%d", ATOI(*p));
				}
				else if ( !ISNULL(*(p+1)) && isdigit(*(p+1)) && !ISNULL( *(p+2)) && !isdigit(*(p+2)) )
				{
					sprintf(h, "%d%d", ATOI(*p), ATOI(*(p+1)) );
					p++;
				}
			}
			else if (*p == label[2])
			{
				if (  ( !ISNULL(*(p+1)) && isdigit(*(p+1)) ) && ( !ISNULL(*(p+2)) && isdigit(*(p+2)) ) )
				{
					sprintf(m, "%d%d", ATOI(*(p+1)), ATOI(*(p+2)) );

					end++;
					p++;
				}
			}
			else
			{
				break;
			}

			if ( (!isdigit(*(p)) && !isdigit(*(p+1))) && (!ISNULL(*(p)) && !ISNULL(*(p+1))) )
			{
				break;
			}

			p++;
		}
	}


#if _TZ_DBG_
	// printf("!![%s:%s:%d] timezone info ==> %s: %s %s:%s \n\n\n", __FILE__, __func__, __LINE__, tz_name, lb, h, m);
#endif

	char buf[1024];
	memset(buf, 0x00, 1024);

	if (strlen(tz_name) && strlen(h))
	{
		p_dt->valid_flag =1;

		sprintf(p_dt->tz_name, "%s",tz_name);
		if (strlen(lb))
			sprintf(p_dt->label, label);
#if 1
		else
			sprintf(p_dt->label, "+");
#endif
		p_dt->h = atoi(h);
		p_dt->m = strlen(m)? atoi(m):0;
	}


	p = NULL;
	start = NULL;
	return 1;
}

#endif

BOOL parse_Bool(const char * pdata)
{
    if (strcasecmp(pdata, "true") == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL parse_XSDDatetime(const char * s, time_t * p)
{
	if (s)
	{
		char zone[32];
		struct tm T;
		const char *t;

		*zone = '\0';
		memset(&T, 0, sizeof(T));

		if (strchr(s, '-'))
		{
			t = "%d-%d-%dT%d:%d:%d%31s";
		}
		else if (strchr(s, ':'))
		{
			t = "%4d%2d%2dT%d:%d:%d%31s";
		}
		else /* parse non-XSD-standard alternative ISO 8601 format */
		{
			t = "%4d%2d%2dT%2d%2d%2d%31s";
		}

		if (sscanf(s, t, &T.tm_year, &T.tm_mon, &T.tm_mday, &T.tm_hour, &T.tm_min, &T.tm_sec, zone) < 6)
		{
			return FALSE;
		}

		if (T.tm_year == 1)
		{
			T.tm_year = 70;
		}
		else
		{
			T.tm_year -= 1900;
		}

		T.tm_mon--;

		if (*zone == '.')
		{
			for (s = zone + 1; *s; s++)
			{
				if (*s < '0' || *s > '9')
				{
					break;
				}
			}
		}
    	else
    	{
      		s = zone;
      	}

		if (*s)
		{
			if (*s == '+' || *s == '-')
			{
				int h = 0, m = 0;
				if (s[3] == ':')
				{
					/* +hh:mm */
					sscanf(s, "%d:%d", &h, &m);
					if (h < 0)
						m = -m;
				}
				else /* +hhmm */
				{
					m = (int)strtol(s, NULL, 10);
					h = m / 100;
					m = m % 100;
				}

				T.tm_min -= m;
				T.tm_hour -= h;
				/* put hour and min in range */
				T.tm_hour += T.tm_min / 60;
				T.tm_min %= 60;

				if (T.tm_min < 0)
				{
					T.tm_min += 60;
					T.tm_hour--;
				}

				T.tm_mday += T.tm_hour / 24;
				T.tm_hour %= 24;

				if (T.tm_hour < 0)
				{
					T.tm_hour += 24;
					T.tm_mday--;
				}
				/* note: day of the month may be out of range, timegm() handles it */
			}

			*p = onvif_timegm(&T);
		}
		else /* no UTC or timezone, so assume we got a localtime */
		{
			T.tm_isdst = -1;
			*p = mktime(&T);
		}
	}

	return TRUE;
}

BOOL parse_XSDDuration(const char *s, int *a)
{
	int sign = 1, Year = 0, Mon = 0, Day = 0, Hour = 0, Min = 0, Sec = 0;
	float f = 0;
	*a = 0;
	if (s)
	{
		if (*s == '-')
		{
			sign = -1;
			s++;
		}
		if (*s++ != 'P')
			return FALSE;

		/* date part */
		while (s && *s)
		{
			int n;
			char k;
			if (*s == 'T')
			{
				s++;
				break;
			}

			if (sscanf(s, "%d%c", &n, &k) != 2)
				return FALSE;

			s = strchr(s, k);
			if (!s)
				return FALSE;

			switch (k)
			{
			case 'Y':
				Year = n;
				break;

			case 'M':
				Mon = n;
				break;

			case 'D':
				Day = n;
				break;

			default:
				return FALSE;
			}

			s++;
		}

	    /* time part */
	    while (s && *s)
		{
			int n;
			char k;
			if (sscanf(s, "%d%c", &n, &k) != 2)
				return FALSE;

			s = strchr(s, k);
			if (!s)
				return FALSE;

			switch (k)
			{
			case 'H':
				Hour = n;
				break;

			case 'M':
				Min = n;
				break;

			case '.':
				Sec = n;
				if (sscanf(s, "%g", &f) != 1)
					return FALSE;
				s = NULL;
				continue;

			case 'S':
				Sec = n;
				break;

			default:
				return FALSE;
			}

			s++;
		}
	    /* convert Y-M-D H:N:S.f to signed int */
	    *a = sign * ((((((((((Year * 12) + Mon) * 30) + Day) * 24) + Hour) * 60) + Min) * 60) + Sec);
	}

	return TRUE;
}


/***************************************************************************************/

BOOL parse_Vector(XMLN * p_node, onvif_Vector * p_req)
{
	const char * p_x;
	const char * p_y;

	p_x = _attr_get(p_node, "x");
	if (p_x)
	{
		p_req->x = (float)atof(p_x);
	}

	p_y = _attr_get(p_node, "y");
	if (p_y)
	{
		p_req->y = (float)atof(p_y);
	}

	return TRUE;
}

BOOL parse_Vector1D(XMLN * p_node, onvif_Vector1D * p_req)
{
	const char * p_x;

	p_x = _attr_get(p_node, "x");
	if (p_x)
	{
		p_req->x = (float)atof(p_x);
	}

	return TRUE;
}

BOOL parse_Dot11Configuration(XMLN * p_node, onvif_Dot11Configuration * p_req)
{
    XMLN * p_SSID;
    XMLN * p_Mode;
    XMLN * p_Alias;
    XMLN * p_Priority;
    XMLN * p_Security;

    p_SSID = _node_soap_get(p_node, "SSID");
	if (p_SSID && p_SSID->data)
	{
	    strncpy(p_req->SSID, p_SSID->data, sizeof(p_req->SSID)-1);
	}

	p_Mode = _node_soap_get(p_node, "Mode");
	if (p_Mode && p_Mode->data)
	{
	    p_req->Mode = onvif_StringToDot11StationMode(p_Mode->data);
	}

	p_Alias = _node_soap_get(p_node, "Alias");
	if (p_Alias && p_Alias->data)
	{
	    strncpy(p_req->Alias, p_Alias->data, sizeof(p_req->Alias)-1);
	}

	p_Priority = _node_soap_get(p_node, "Priority");
	if (p_Priority && p_Priority->data)
	{
	    p_req->Priority = atoi(p_Priority->data);
	}

	p_Security = _node_soap_get(p_node, "Security");
    if (p_Security)
    {
        XMLN * p_Mode;
        XMLN * p_Algorithm;
        XMLN * p_PSK;
        XMLN * p_Dot1X;

        p_Mode = _node_soap_get(p_Security, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    	    p_req->Security.Mode = onvif_StringToDot11SecurityMode(p_Mode->data);
    	}

    	p_Algorithm = _node_soap_get(p_Security, "Algorithm");
    	if (p_Algorithm && p_Algorithm->data)
    	{
    	    p_req->Security.AlgorithmFlag = 1;
    	    p_req->Security.Algorithm = onvif_StringToDot11Cipher(p_Algorithm->data);
    	}

    	p_PSK = _node_soap_get(p_Security, "PSK");
    	if (p_PSK)
    	{
    	    XMLN * p_Key;
            XMLN * p_Passphrase;

            p_req->Security.PSKFlag = 1;

            p_Key = _node_soap_get(p_PSK, "Key");
        	if (p_Key && p_Key->data)
        	{
        	    p_req->Security.PSK.KeyFlag = 1;
        	    strncpy(p_req->Security.PSK.Key, p_Key->data, sizeof(p_req->Security.PSK.Key)-1);
        	}

        	p_Passphrase = _node_soap_get(p_PSK, "Passphrase");
        	if (p_Passphrase && p_Passphrase->data)
        	{
        	    p_req->Security.PSK.PassphraseFlag = 1;
        	    strncpy(p_req->Security.PSK.Passphrase, p_Passphrase->data, sizeof(p_req->Security.PSK.Passphrase)-1);
        	}
    	}

    	p_Dot1X = _node_soap_get(p_Security, "Dot1X");
    	if (p_Dot1X && p_Dot1X->data)
    	{
    	    p_req->Security.Dot1XFlag = 1;
    	    strncpy(p_req->Security.Dot1X, p_Dot1X->data, sizeof(p_req->Security.Dot1X)-1);
    	}
    }

	return TRUE;
}

ONVIF_RET prase_User(XMLN * p_node, onvif_User * p_req)
{
    XMLN * p_Username;
	XMLN * p_Password;
	XMLN * p_UserLevel;

	p_Username = _node_soap_get(p_node, "Username");
	if (p_Username && p_Username->data)
	{
		strncpy(p_req->Username, p_Username->data, sizeof(p_req->Username)-1);
	}

	p_Password = _node_soap_get(p_node, "Password");
	if (p_Password && p_Password->data)
	{
		if (strlen(p_Password->data) >= sizeof(p_req->Password))
		{
			return ONVIF_ERR_PasswordTooLong;
		}

		strncpy(p_req->Password, p_Password->data, sizeof(p_req->Password)-1);
	}

	p_UserLevel = _node_soap_get(p_node, "UserLevel");
	if (p_UserLevel && p_UserLevel->data)
	{
		p_req->UserLevel = onvif_StringToUserLevel(p_UserLevel->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_StreamSetup(XMLN * p_node, onvif_StreamSetup * p_req)
{
	XMLN * p_Stream;
	XMLN * p_Transport;

	p_Stream = _node_soap_get(p_node, "Stream");
	if (p_Stream && p_Stream->data)
	{
		p_req->Stream = onvif_StringToStreamType(p_Stream->data);
		if (StreamType_Invalid == p_req->Stream)
		{
			return ONVIF_ERR_InvalidStreamSetup;
		}
	}

	p_Transport = _node_soap_get(p_node, "Transport");
	if (p_Transport)
	{
		XMLN * p_Protocol = _node_soap_get(p_Transport, "Protocol");
		if (p_Protocol && p_Protocol->data)
		{
			p_req->Transport.Protocol = onvif_StringToTransportProtocol(p_Protocol->data);
			if (TransportProtocol_Invalid == p_req->Transport.Protocol)
			{
				return ONVIF_ERR_InvalidStreamSetup;
			}
		}
	}

	return ONVIF_OK;
}

/***************************************************************************************/

ONVIF_RET parse_tds_GetCapabilities(XMLN * p_node, tds_GetCapabilities_REQ * p_req)
{
    XMLN * p_Category;

    p_Category = _node_soap_get(p_node, "Category");
	if (p_Category && p_Category->data)
	{
		p_req->Category = onvif_StringToCapabilityCategory(p_Category->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_GetServices(XMLN * p_node, tds_GetServices_REQ * p_req)
{
    XMLN * p_IncludeCapability;

    p_IncludeCapability = _node_soap_get(p_node, "IncludeCapability");
    if (p_IncludeCapability && p_IncludeCapability->data)
    {
        p_req->IncludeCapability = parse_Bool(p_IncludeCapability->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tds_SetSystemDateAndTime(XMLN * p_node, tds_SetSystemDateAndTime_REQ * p_req, manager_tz_dt *p_dt)
{
    XMLN * p_DateTimeType;
	XMLN * p_DaylightSavings;
	XMLN * p_TimeZone;
	XMLN * p_UTCDateTime;
	
	p_DateTimeType = _node_soap_get(p_node, "DateTimeType");
    if (p_DateTimeType && p_DateTimeType->data)
    {
        p_req->SystemDateTime.DateTimeType = onvif_StringToSetDateTimeType(p_DateTimeType->data);
		printf("[ONVIF] DateTimeType is changed (=> %d)", p_req->SystemDateTime.DateTimeType);
#if defined(__ONVIF_DATE_TIME_TEST__)
#endif	/* __ONVIF_DATE_TIME_TEST__ */
    }
    p_DaylightSavings = _node_soap_get(p_node, "DaylightSavings");
    if (p_DaylightSavings && p_DaylightSavings->data)
    {
        p_req->SystemDateTime.DaylightSavings = parse_Bool(p_DaylightSavings->data);
    }
    p_TimeZone = _node_soap_get(p_node, "TimeZone");
    if (p_TimeZone)
    {
    	XMLN * p_TZ;
        p_TZ = _node_soap_get(p_TimeZone, "TZ");
		if (p_TZ && p_TZ->data)
		{
			strncpy(p_req->SystemDateTime.TimeZone.TZ, p_TZ->data, sizeof(p_req->SystemDateTime.TimeZone.TZ)-1);
#if defined(__ONVIF_DATE_TIME_TEST__)
			printf("[ONVIF] Timezone is changed (TZ => %s)", p_req->SystemDateTime.TimeZone.TZ);
#endif	/* __ONVIF_DATE_TIME_TEST__ */
#if 0
			parse_tz(p_req->SystemDateTime.TimeZone.TZ, p_dt);
#endif
		}
    }

    p_UTCDateTime = _node_soap_get(p_node, "UTCDateTime");
    if (p_UTCDateTime)
    {
    	XMLN * p_Time;
		XMLN * p_Hour;
		XMLN * p_Minute;
		XMLN * p_Second;
		XMLN * p_Date;
		XMLN * p_Year;
		XMLN * p_Month;
		XMLN * p_Day;

    	p_req->UTCDateTimeFlag = 1;
        p_Time = _node_soap_get(p_UTCDateTime, "Time");
	    if (!p_Time)
	    {
			
	        return ONVIF_ERR_MissingAttribute;
	    }
	    p_Hour = _node_soap_get(p_Time, "Hour");
	    if (!p_Hour || !p_Hour->data)
	    {
			
	        return ONVIF_ERR_MissingAttribute;
	    }
	    p_req->UTCDateTime.Time.Hour = atoi(p_Hour->data);
	    p_Minute = _node_soap_get(p_Time, "Minute");
	    if (!p_Minute || !p_Minute->data)
	    {
			
	        return ONVIF_ERR_MissingAttribute;
	    }
	    p_req->UTCDateTime.Time.Minute = atoi(p_Minute->data);

	    p_Second = _node_soap_get(p_Time, "Second");
	    if (!p_Second || !p_Second->data)
	    {
			
	        return ONVIF_ERR_MissingAttribute;
	    }	
	    p_req->UTCDateTime.Time.Second = atoi(p_Second->data);

	    p_Date = _node_soap_get(p_UTCDateTime, "Date");
	    if (!p_Date)
	    {
			
	        return ONVIF_ERR_MissingAttribute;
	    }

		
	    p_Year = _node_soap_get(p_Date, "Year");
	    if (!p_Year || !p_Year->data)
	    {
			
	        return ONVIF_ERR_MissingAttribute;
	    }
	    p_req->UTCDateTime.Date.Year = atoi(p_Year->data);
	    p_Month = _node_soap_get(p_Date, "Month");
	    if (!p_Month || !p_Month->data)
	    {
			
	        return ONVIF_ERR_MissingAttribute;
	    }

	    p_req->UTCDateTime.Date.Month = atoi(p_Month->data);
	    p_Day = _node_soap_get(p_Date, "Day");
	    if (!p_Day || !p_Day->data)
	    {
			
	        return ONVIF_ERR_MissingAttribute;
	    }
	    p_req->UTCDateTime.Date.Day = atoi(p_Day->data);

    }


	
    return ONVIF_OK;
}

ONVIF_RET parse_tds_AddScopes(XMLN * p_node, tds_AddScopes_REQ * p_req)
{
	int i = 0;

	XMLN * p_ScopeItem = _node_soap_get(p_node, "ScopeItem");
	while (p_ScopeItem && soap_strcmp(p_ScopeItem->name, "ScopeItem") == 0)
	{
		if (i < ARRAY_SIZE(p_req->ScopeItem))
		{
			strncpy(p_req->ScopeItem[i], p_ScopeItem->data, sizeof(p_req->ScopeItem[i])-1);

			++i;
		}
		else
		{
			return ONVIF_ERR_TooManyScopes;
		}

		p_ScopeItem = p_ScopeItem->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetScopes(XMLN * p_node, tds_SetScopes_REQ * p_req)
{
	int i = 0;

	XMLN * p_Scopes = _node_soap_get(p_node, "Scopes");
	while (p_Scopes && soap_strcmp(p_Scopes->name, "Scopes") == 0)
	{
		if (i < ARRAY_SIZE(p_req->Scopes))
		{
			strncpy(p_req->Scopes[i], p_Scopes->data, sizeof(p_req->Scopes[i])-1);

			++i;
		}
		else
		{
			return ONVIF_ERR_TooManyScopes;
		}

		p_Scopes = p_Scopes->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_RemoveScopes(XMLN * p_node, tds_RemoveScopes_REQ * p_req)
{
    int i = 0;

	XMLN * p_ScopeItem = _node_soap_get(p_node, "ScopeItem");
	while (p_ScopeItem && soap_strcmp(p_ScopeItem->name, "ScopeItem") == 0)
	{
		if (i < ARRAY_SIZE(p_req->ScopeItem))
		{
			strncpy(p_req->ScopeItem[i], p_ScopeItem->data, sizeof(p_req->ScopeItem[i])-1);

			++i;
		}
		else
		{
			return ONVIF_ERR_TooManyScopes;
		}

		p_ScopeItem = p_ScopeItem->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetHostname(XMLN * p_node, tds_SetHostname_REQ * p_req)
{
    XMLN * p_Name;

    p_Name = _node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetHostnameFromDHCP(XMLN * p_node, tds_SetHostnameFromDHCP_REQ * p_req)
{
    XMLN * p_FromDHCP;

    p_FromDHCP = _node_soap_get(p_node, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
	    p_req->FromDHCP = parse_Bool(p_FromDHCP->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetDiscoveryMode(XMLN * p_node, tds_SetDiscoveryMode_REQ * p_req)
{
	XMLN * p_DiscoveryMode = _node_soap_get(p_node, "DiscoveryMode");
	if (p_DiscoveryMode && p_DiscoveryMode->data)
	{
		p_req->DiscoveryMode = onvif_StringToDiscoveryMode(p_DiscoveryMode->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetDNS(XMLN * p_node, tds_SetDNS_REQ * p_req)
{
	int i = 0;
	XMLN * p_FromDHCP;
	XMLN * p_SearchDomain;
	XMLN * p_DNSManual;

	assert(p_node);

	p_FromDHCP = _node_soap_get(p_node, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
		p_req->DNSInformation.FromDHCP = parse_Bool(p_FromDHCP->data);
	}

	p_SearchDomain = _node_soap_get(p_node, "SearchDomain");
	while (p_SearchDomain && soap_strcmp(p_SearchDomain->name, "SearchDomain") == 0)
	{
		p_req->DNSInformation.SearchDomainFlag = 1;

		if (p_SearchDomain->data && i < ARRAY_SIZE(p_req->DNSInformation.SearchDomain))
		{
			strncpy(p_req->DNSInformation.SearchDomain[i], p_SearchDomain->data, sizeof(p_req->DNSInformation.SearchDomain[i])-1);
			++i;
		}

		p_SearchDomain = p_SearchDomain->next;
	}

	i = 0;

	p_DNSManual = _node_soap_get(p_node, "DNSManual");
	while (p_DNSManual && soap_strcmp(p_DNSManual->name, "DNSManual") == 0)
	{
		XMLN * p_Type;
		XMLN * p_IPv4Address;

		p_Type = _node_soap_get(p_DNSManual, "Type");
		if (p_Type && p_Type->data)
		{
			if (strcasecmp(p_Type->data, "IPv4") != 0) // todo : now only support ipv4
			{
				p_DNSManual = p_DNSManual->next;
				continue;
			}
		}

		p_IPv4Address = _node_soap_get(p_DNSManual, "IPv4Address");
		if (p_IPv4Address && p_IPv4Address->data)
		{
			if (is_ip_address(p_IPv4Address->data) == FALSE)
			{
				return ONVIF_ERR_InvalidIPv4Address;
			}
			else if (i < ARRAY_SIZE(p_req->DNSInformation.DNSServer))
			{
				strncpy(p_req->DNSInformation.DNSServer[i], p_IPv4Address->data, sizeof(p_req->DNSInformation.DNSServer[i])-1);
				++i;
			}
		}

		p_DNSManual = p_DNSManual->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetDynamicDNS(XMLN * p_node, tds_SetDynamicDNS_REQ * p_req)
{
    XMLN * p_Type;
    XMLN * p_Name;
    XMLN * p_TTL;

    p_Type = _node_soap_get(p_node, "Type");
	if (p_Type && p_Type->data)
	{
	    p_req->DynamicDNSInformation.Type = onvif_StringToDynamicDNSType(p_Type->data);
	}

	p_Name = _node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
	    p_req->DynamicDNSInformation.NameFlag = 1;
	    strncpy(p_req->DynamicDNSInformation.Name, p_Name->data, sizeof(p_req->DynamicDNSInformation.Name)-1);
	}

	// p_TTL = _node_soap_get(p_node, "TTL");
	// if (p_TTL && p_TTL->data)
	// {
	//     p_req->DynamicDNSInformation.TTLFlag = 1;
	//     p_req->DynamicDNSInformation.TTL = atoi(p_TTL->data);
	// }

    return ONVIF_OK;
}

ONVIF_RET parse_tds_SetNTP(XMLN * p_node, tds_SetNTP_REQ * p_req)
{
	int i = 0;
	XMLN * p_FromDHCP;
	XMLN * p_NTPManual;

	assert(p_node);

	p_FromDHCP = _node_soap_get(p_node, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
		p_req->NTPInformation.FromDHCP = parse_Bool(p_FromDHCP->data);
	}

	p_NTPManual = _node_soap_get(p_node, "NTPManual");
	while (p_NTPManual && soap_strcmp(p_NTPManual->name, "NTPManual") == 0)
	{
		XMLN * p_Type;
		XMLN * p_IPv4Address;
		XMLN * p_DNSname;

		p_Type = _node_soap_get(p_NTPManual, "Type");
		if (p_Type && p_Type->data)
		{
			if (strcasecmp(p_Type->data, "IPv4") != 0 && strcasecmp(p_Type->data, "DNS") != 0) // todo : now only support ipv4
			{
				p_NTPManual = p_NTPManual->next;
				continue;
			}
		}

		p_IPv4Address = _node_soap_get(p_NTPManual, "IPv4Address");
		if (p_IPv4Address && p_IPv4Address->data)
		{
			if (is_ip_address(p_IPv4Address->data) == FALSE)
			{
				return ONVIF_ERR_InvalidIPv4Address;
			}
			else if (i < ARRAY_SIZE(p_req->NTPInformation.NTPServer))
			{
				strncpy(p_req->NTPInformation.NTPServer[i], p_IPv4Address->data, sizeof(p_req->NTPInformation.NTPServer[i])-1);
				++i;
			}
		}

		p_DNSname = _node_soap_get(p_NTPManual, "DNSname");
		if (p_DNSname && p_DNSname->data)
		{
			if (i < ARRAY_SIZE(p_req->NTPInformation.NTPServer))
			{
				strncpy(p_req->NTPInformation.NTPServer[i], p_DNSname->data, sizeof(p_req->NTPInformation.NTPServer[i])-1);
				++i;
			}
		}

		p_NTPManual = p_NTPManual->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetZeroConfiguration(XMLN * p_node, tds_SetZeroConfiguration_REQ * p_req)
{
    XMLN * p_InterfaceToken;
    XMLN * p_Enabled;

    p_InterfaceToken = _node_soap_get(p_node, "InterfaceToken");
	if (p_InterfaceToken && p_InterfaceToken->data)
	{
	    strncpy(p_req->InterfaceToken, p_InterfaceToken->data, sizeof(p_req->InterfaceToken)-1);
	}

	p_Enabled = _node_soap_get(p_node, "Enabled");
	if (p_Enabled && p_Enabled->data)
	{
	    p_req->Enabled = parse_Bool(p_Enabled->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_GetDot11Status(XMLN * p_node, tds_GetDot11Status_REQ * p_req)
{
    XMLN * p_InterfaceToken;

    p_InterfaceToken = _node_soap_get(p_node, "InterfaceToken");
	if (p_InterfaceToken && p_InterfaceToken->data)
	{
	    strncpy(p_req->InterfaceToken, p_InterfaceToken->data, sizeof(p_req->InterfaceToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_ScanAvailableDot11Networks(XMLN * p_node, tds_ScanAvailableDot11Networks_REQ * p_req)
{
    XMLN * p_InterfaceToken;

    p_InterfaceToken = _node_soap_get(p_node, "InterfaceToken");
	if (p_InterfaceToken && p_InterfaceToken->data)
	{
	    strncpy(p_req->InterfaceToken, p_InterfaceToken->data, sizeof(p_req->InterfaceToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetNetworkProtocols(XMLN * p_node, tds_SetNetworkProtocols_REQ * p_req)
{
	char name[32];
	BOOL enable;
	int  port[MAX_SERVER_PORT];
	XMLN * p_NetworkProtocols;

	assert(p_node);

	p_NetworkProtocols = _node_soap_get(p_node, "NetworkProtocols");
	while (p_NetworkProtocols && soap_strcmp(p_NetworkProtocols->name, "NetworkProtocols") == 0)
	{
		int i = 0;
		XMLN * p_Name;
		XMLN * p_Enabled;
		XMLN * p_Port;

		enable = FALSE;
		memset(name, 0, sizeof(name));
		memset(port, 0, sizeof(int)*MAX_SERVER_PORT);

		p_Name = _node_soap_get(p_NetworkProtocols, "Name");
		if (p_Name && p_Name->data)
		{
			strncpy(name, p_Name->data, sizeof(name)-1);
		}

		p_Enabled = _node_soap_get(p_NetworkProtocols, "Enabled");
		if (p_Enabled && p_Enabled->data)
		{
			if (strcasecmp(p_Enabled->data, "true") == 0)
			{
				enable = TRUE;
			}
		}

		p_Port = _node_soap_get(p_NetworkProtocols, "Port");
		while (p_Port && p_Port->data && soap_strcmp(p_Port->name, "Port") == 0)
		{
			if (i < ARRAY_SIZE(port))
			{
				port[i++] = atoi(p_Port->data);
			}

			p_Port = p_Port->next;
		}

		if (strcasecmp(name, "HTTP") == 0)
		{
			p_req->NetworkProtocol.HTTPFlag = 1;
			p_req->NetworkProtocol.HTTPEnabled = enable;
			memcpy(p_req->NetworkProtocol.HTTPPort, port, sizeof(p_req->NetworkProtocol.HTTPPort));
		}
		else if (strcasecmp(name, "RTSP") == 0)
		{
			p_req->NetworkProtocol.RTSPFlag = 1;
			p_req->NetworkProtocol.RTSPEnabled = enable;
			memcpy(p_req->NetworkProtocol.RTSPPort, port, sizeof(p_req->NetworkProtocol.RTSPPort));
		}
		else
		{
			return ONVIF_ERR_ServiceNotSupported;
		}

		p_NetworkProtocols = p_NetworkProtocols->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetNetworkDefaultGateway(XMLN * p_node, tds_SetNetworkDefaultGateway_REQ * p_req)
{
	int i = 0;
	XMLN * p_IPv4Address;

	assert(p_node);

	p_IPv4Address = _node_soap_get(p_node, "IPv4Address");
	while (p_IPv4Address && p_IPv4Address->data && soap_strcmp(p_IPv4Address->name, "IPv4Address") == 0)
	{
		if (is_ip_address(p_IPv4Address->data) == FALSE)
		{
			return ONVIF_ERR_InvalidIPv4Address;
		}

		if (i < ARRAY_SIZE(p_req->IPv4Address))
		{
			strncpy(p_req->IPv4Address[i++], p_IPv4Address->data, sizeof(p_req->IPv4Address[0])-1);
		}

		p_IPv4Address = p_IPv4Address->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetNetworkInterfaces(XMLN * p_node, tds_SetNetworkInterfaces_REQ * p_req)
{
    XMLN * p_InterfaceToken;
	XMLN * p_NetworkInterface;

	p_InterfaceToken = _node_soap_get(p_node, "InterfaceToken");
	if (p_InterfaceToken && p_InterfaceToken->data)
	{
	    strncpy(p_req->NetworkInterface.token, p_InterfaceToken->data, sizeof(p_req->NetworkInterface.token)-1);
	}
	else
	{
	    return ONVIF_ERR_MissingAttribute;
	}

	p_NetworkInterface = _node_soap_get(p_node, "NetworkInterface");
	if (p_NetworkInterface)
	{
		XMLN * p_Enabled;
		XMLN * p_MTU;
		XMLN * p_IPv4;
		XMLN * p_Extension;

	    p_req->NetworkInterface.Enabled = TRUE;

	    p_Enabled = _node_soap_get(p_NetworkInterface, "Enabled");
	    if (p_Enabled && p_Enabled->data)
	    {
	        p_req->NetworkInterface.Enabled = parse_Bool(p_Enabled->data);
	    }

	    p_MTU = _node_soap_get(p_NetworkInterface, "MTU");
	    if (p_MTU && p_MTU->data)
	    {
	    	p_req->NetworkInterface.InfoFlag = 1;
	    	p_req->NetworkInterface.Info.MTUFlag = 1;
	        p_req->NetworkInterface.Info.MTU = atoi(p_MTU->data);
	    }

	    p_IPv4 = _node_soap_get(p_NetworkInterface, "IPv4");
	    if (p_IPv4)
	    {
	    	XMLN * p_Enabled;
			XMLN * p_DHCP;

	        p_req->NetworkInterface.IPv4Flag = 1;

	        p_Enabled = _node_soap_get(p_IPv4, "Enabled");
	        if (p_Enabled && p_Enabled->data)
    	    {
    	        p_req->NetworkInterface.IPv4.Enabled = parse_Bool(p_Enabled->data);
    	    }

    	    p_DHCP = _node_soap_get(p_IPv4, "DHCP");
	        if (p_DHCP && p_DHCP->data)
	        {
	            p_req->NetworkInterface.IPv4.Config.DHCP = parse_Bool(p_DHCP->data);
	        }

	        if (p_req->NetworkInterface.IPv4.Config.DHCP == FALSE)
	        {
	            XMLN * p_Manual = _node_soap_get(p_IPv4, "Manual");
	            if (p_Manual)
	            {
	                XMLN * p_Address;
					XMLN * p_PrefixLength;

					p_Address = _node_soap_get(p_Manual, "Address");
	                if (p_Address && p_Address->data)
	                {
	                    strncpy(p_req->NetworkInterface.IPv4.Config.Address, p_Address->data, sizeof(p_req->NetworkInterface.IPv4.Config.Address)-1);
	                }

	                p_PrefixLength = _node_soap_get(p_Manual, "PrefixLength");
	                if (p_PrefixLength && p_PrefixLength->data)
	                {
	                    p_req->NetworkInterface.IPv4.Config.PrefixLength = atoi(p_PrefixLength->data);
	                }
	            }
	        }
	    }

	    p_Extension = _node_soap_get(p_NetworkInterface, "Extension");
	    if (p_Extension)
	    {
	        XMLN * p_InterfaceType;
	        XMLN * p_Dot11;

	        p_req->NetworkInterface.ExtensionFlag = 1;

	        p_InterfaceType = _node_soap_get(p_Extension, "InterfaceType");
            if (p_InterfaceType && p_InterfaceType->data)
            {
                p_req->NetworkInterface.Extension.InterfaceType = atoi(p_InterfaceType->data);
            }

	        p_Dot11 = _node_soap_get(p_Extension, "Dot11");
	        while (p_Dot11 && soap_strcmp(p_Dot11->name, "Dot11") == 0)
	        {
	            int idx = p_req->NetworkInterface.Extension.sizeDot11;

                parse_Dot11Configuration(p_Dot11, &p_req->NetworkInterface.Extension.Dot11[idx]);

	            p_req->NetworkInterface.Extension.sizeDot11++;
	            if (p_req->NetworkInterface.Extension.sizeDot11 >= ARRAY_SIZE(p_req->NetworkInterface.Extension.Dot11))
	            {
	                break;
	            }

	            p_Dot11 = p_Dot11->next;
	        }
	    }
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetSystemFactoryDefault(XMLN * p_node, tds_SetSystemFactoryDefault_REQ * p_req)
{
    XMLN * p_FactoryDefault;

    p_FactoryDefault = _node_soap_get(p_node, "FactoryDefault");
    if (p_FactoryDefault && p_FactoryDefault->data)
    {
    	p_req->FactoryDefault = onvif_StringToFactoryDefaultType(p_FactoryDefault->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tds_CreateUsers(XMLN * p_node, tds_CreateUsers_REQ * p_req)
{
	int i = 0;
	ONVIF_RET ret;

	XMLN * p_User = _node_soap_get(p_node, "User");
	while (p_User)
	{
		if (i < ARRAY_SIZE(p_req->User))
		{
		    ret = prase_User(p_User, &p_req->User[i]);
			if (ONVIF_OK != ret)
			{
			    return ret;
			}

			++i;
		}
		else
		{
			return ONVIF_ERR_TooManyUsers;
		}

		p_User = p_User->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_DeleteUsers(XMLN * p_node, tds_DeleteUsers_REQ * p_req)
{
	int i = 0;

	XMLN * p_Username = _node_soap_get(p_node, "Username");
	while (p_Username)
	{
		if (i < ARRAY_SIZE(p_req->Username))
		{
			strncpy(p_req->Username[i], p_Username->data, sizeof(p_req->Username[i])-1);

			++i;
		}
		else
		{
			break;
		}

		p_Username = p_Username->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetUser(XMLN * p_node, tds_SetUser_REQ * p_req)
{printf_function_soap_parser();
    int i = 0;
	ONVIF_RET ret;

	XMLN * p_User = _node_soap_get(p_node, "User");
	while (p_User)
	{
		if (i < ARRAY_SIZE(p_req->User))
		{
		    ret = prase_User(p_User, &p_req->User[i]);
			if (ONVIF_OK != ret)
			{
			    return ret;
			}

			++i;
		}
		else
		{
			return ONVIF_ERR_TooManyUsers;
		}

		p_User = p_User->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tds_SetRemoteUser(XMLN * p_node, tds_SetRemoteUser_REQ * p_req)
{printf_function_soap_parser();
	XMLN * p_RemoteUser = _node_soap_get(p_node, "RemoteUser");
	if (p_RemoteUser)
	{
		XMLN * p_Username;
		XMLN * p_Password;
		XMLN * p_UseDerivedPassword;

		p_req->RemoteUserFlag = 1;

		p_Username = _node_soap_get(p_RemoteUser, "Username");
    	if (p_Username && p_Username->data)
    	{
    	    strncpy(p_req->RemoteUser.Username, p_Username->data, sizeof(p_req->RemoteUser.Username)-1);
    	}

    	p_Password = _node_soap_get(p_RemoteUser, "Password");
    	if (p_Password && p_Password->data)
    	{
    	    p_req->RemoteUser.PasswordFlag = 1;
    	    strncpy(p_req->RemoteUser.Password, p_Password->data, sizeof(p_req->RemoteUser.Password)-1);
    	}

    	p_UseDerivedPassword = _node_soap_get(p_RemoteUser, "UseDerivedPassword");
    	if (p_UseDerivedPassword && p_UseDerivedPassword->data)
    	{
    	    p_req->RemoteUser.UseDerivedPassword = parse_Bool(p_UseDerivedPassword->data);
    	}
	}

	return ONVIF_OK;
}

#ifdef DEVICEIO_SUPPORT

ONVIF_RET parse_tds_SetRelayOutputSettings(XMLN * p_node, tmd_SetRelayOutputSettings_REQ * p_req)
{
    XMLN * p_RelayOutputToken;
    XMLN * p_Properties;

    p_RelayOutputToken = _node_soap_get(p_node, "RelayOutputToken");
    if (p_RelayOutputToken && p_RelayOutputToken->data)
    {
        strncpy(p_req->RelayOutput.token, p_RelayOutputToken->data, sizeof(p_req->RelayOutput.token)-1);
    }

    p_Properties = _node_soap_get(p_node, "Properties");
    if (p_Properties)
    {
        parse_RelayOutputSettings(p_Properties, &p_req->RelayOutput.Properties);
    }

    return ONVIF_OK;
}

#endif // DEVICEIO_SUPPORT

/***************************************************************************************/

ONVIF_RET parse_Filter(XMLN * p_node, ONVIF_FILTER * p_req)
{
    int i = 0;
    XMLN * p_TopicExpression;
    XMLN * p_MessageContent;

    p_TopicExpression = _node_soap_get(p_node, "TopicExpression");
    while (p_TopicExpression && soap_strcmp(p_TopicExpression->name, "TopicExpression") == 0)
    {
        if (p_TopicExpression->data && i < ARRAY_SIZE(p_req->TopicExpression))
        {
            strncpy(p_req->TopicExpression[i], p_TopicExpression->data, sizeof(p_req->TopicExpression[i])-1);
            i++;
        }

        p_TopicExpression = p_TopicExpression->next;
    }

    i = 0;

    p_MessageContent = _node_soap_get(p_node, "MessageContent");
    while (p_MessageContent && soap_strcmp(p_MessageContent->name, "MessageContent") == 0)
    {
        if (p_MessageContent->data && i < ARRAY_SIZE(p_req->MessageContent))
        {
            strncpy(p_req->MessageContent[i], p_MessageContent->data, sizeof(p_req->MessageContent[i])-1);
            i++;
        }

        p_MessageContent = p_MessageContent->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tev_Subscribe(XMLN * p_node, tev_Subscribe_REQ * p_req)
{
	XMLN * p_ConsumerReference;
	XMLN * p_Address;
	XMLN * p_InitialTerminationTime;
	XMLN * p_Filter;

	p_ConsumerReference = _node_soap_get(p_node, "ConsumerReference");
	if (NULL == p_ConsumerReference)
	{
		return ONVIF_ERR_MissingAttribute;
	}

	p_Address = _node_soap_get(p_ConsumerReference, "Address");
	if (p_Address && p_Address->data)
	{
		strncpy(p_req->ConsumerReference, p_Address->data, sizeof(p_req->ConsumerReference)-1);
	}

	p_InitialTerminationTime = _node_soap_get(p_node, "InitialTerminationTime");
	if (p_InitialTerminationTime && p_InitialTerminationTime->data)
	{
		p_req->InitialTerminationTimeFlag = 1;
		parse_XSDDuration(p_InitialTerminationTime->data, &p_req->InitialTerminationTime);
	}

    p_Filter = _node_soap_get(p_node, "Filter");
	if (p_Filter)
	{
	    p_req->FiltersFlag = 1;
		parse_Filter(p_Filter, &p_req->Filters);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tev_Renew(XMLN * p_node, tev_Renew_REQ * p_req)
{
	XMLN * p_TerminationTime;

	p_TerminationTime = _node_soap_get(p_node, "TerminationTime");
	if (p_TerminationTime && p_TerminationTime->data)
	{
	    if (p_TerminationTime->data[0] == 'P' ||
	        (p_TerminationTime->data[0] == '-' && p_TerminationTime->data[1] == 'P'))
	    {
	        p_req->TerminationTimeType = 1;
		    parse_XSDDuration(p_TerminationTime->data, (int *)&p_req->TerminationTime);
	    }
	    else
	    {
	        p_req->TerminationTimeType = 0;

		    parse_XSDDatetime(p_TerminationTime->data, &p_req->TerminationTime);
		}
	}
	else
	{
		return ONVIF_ERR_MissingAttribute;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tev_CreatePullPointSubscription(XMLN * p_node, tev_CreatePullPointSubscription_REQ * p_req)
{
	XMLN * p_InitialTerminationTime;
	XMLN * p_Filter;

	p_InitialTerminationTime = _node_soap_get(p_node, "InitialTerminationTime");
	if (p_InitialTerminationTime && p_InitialTerminationTime->data)
	{
		p_req->InitialTerminationTimeFlag = 1;
		parse_XSDDuration(p_InitialTerminationTime->data, &p_req->InitialTerminationTime);
	}

    p_Filter = _node_soap_get(p_node, "Filter");
	if (p_Filter)
	{
	    p_req->FiltersFlag = 1;
		parse_Filter(p_Filter, &p_req->Filters);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tev_PullMessages(XMLN * p_node, tev_PullMessages_REQ * p_req)
{
	XMLN * p_Timeout;
	XMLN * p_MessageLimit;

	p_Timeout = _node_soap_get(p_node, "Timeout");
	if (p_Timeout && p_Timeout->data)
	{
		parse_XSDDuration(p_Timeout->data, &p_req->Timeout);
	}

	p_MessageLimit = _node_soap_get(p_node, "MessageLimit");
	if (p_MessageLimit && p_MessageLimit->data)
	{
		p_req->MessageLimit = atoi(p_MessageLimit->data);
	}

	return ONVIF_OK;
}

/***************************************************************************************/

#ifdef IMAGE_SUPPORT

ONVIF_RET parse_img_GetImagingSettings(XMLN * p_node, img_GetImagingSettings_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }
    else
    {
    	return ONVIF_ERR_MissingAttribute;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_img_SetImagingSettings(XMLN * p_node, img_SetImagingSettings_REQ * p_req)
{
	XMLN * p_VideoSourceToken;
	XMLN * p_ImagingSettings;
	XMLN * p_BacklightCompensation;
	XMLN * p_Brightness;
	XMLN * p_ColorSaturation;
	XMLN * p_Contrast;
	XMLN * p_Exposure;
	XMLN * p_Focus;
	XMLN * p_IrCutFilter;
	XMLN * p_Sharpness;
	XMLN * p_WideDynamicRange;
	XMLN * p_WhiteBalance;
	XMLN * p_ForcePersistence;

	p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }
    else
    {
    	return ONVIF_ERR_MissingAttribute;
    }

    p_ImagingSettings = _node_soap_get(p_node, "ImagingSettings");
    if (NULL == p_ImagingSettings)
    {
    	return ONVIF_ERR_MissingAttribute;
    }

    p_BacklightCompensation = _node_soap_get(p_ImagingSettings, "BacklightCompensation");
    if (p_BacklightCompensation)
    {
    	XMLN * p_Mode;
		XMLN * p_Level;

		p_req->ImagingSettings.BacklightCompensationFlag = 1;

		p_Mode = _node_soap_get(p_BacklightCompensation, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		p_req->ImagingSettings.BacklightCompensation.Mode = onvif_StringToBacklightCompensationMode(p_Mode->data);
    	}

    }

    p_Brightness = _node_soap_get(p_ImagingSettings, "Brightness");
    if (p_Brightness && p_Brightness->data)
    {
    	p_req->ImagingSettings.BrightnessFlag = 1;
    	p_req->ImagingSettings.Brightness = (float)atof(p_Brightness->data);
    }

    p_ColorSaturation = _node_soap_get(p_ImagingSettings, "ColorSaturation");
    if (p_ColorSaturation && p_ColorSaturation->data)
    {
    	p_req->ImagingSettings.ColorSaturationFlag = 1;
    	p_req->ImagingSettings.ColorSaturation = (float)atof(p_ColorSaturation->data);
    }

    p_Contrast = _node_soap_get(p_ImagingSettings, "Contrast");
    if (p_Contrast && p_Contrast->data)
    {
    	p_req->ImagingSettings.ContrastFlag = 1;
    	p_req->ImagingSettings.Contrast = (float)atof(p_Contrast->data);
    }

    p_Exposure = _node_soap_get(p_ImagingSettings, "Exposure");
    if (p_Exposure)
    {
    	XMLN * p_Mode;
		XMLN * p_Priority;
		XMLN * p_Window;
		XMLN * p_MinExposureTime;
		XMLN * p_MaxExposureTime;
		XMLN * p_MinGain;
		XMLN * p_MaxGain;
		XMLN * p_ExposureTime;
		XMLN * p_Gain;
		XMLN * p_Iris;

		p_req->ImagingSettings.ExposureFlag = 1;

		p_Mode = _node_soap_get(p_Exposure, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		p_req->ImagingSettings.Exposure.Mode = onvif_StringToExposureMode(p_Mode->data);
    	}

        p_Priority = _node_soap_get(p_Exposure, "Priority");
    	if (p_Priority && p_Priority->data)
    	{
    		p_req->ImagingSettings.Exposure.PriorityFlag = 1;
    		p_req->ImagingSettings.Exposure.Priority = onvif_StringToExposurePriority(p_Priority->data);
    	}

    	p_MinExposureTime = _node_soap_get(p_Exposure, "MinExposureTime");
    	if (p_MinExposureTime && p_MinExposureTime->data)
    	{
    		p_req->ImagingSettings.Exposure.MinExposureTimeFlag = 1;
    		p_req->ImagingSettings.Exposure.MinExposureTime = (float)atof(p_MinExposureTime->data);
    	}

    	p_MaxExposureTime = _node_soap_get(p_Exposure, "MaxExposureTime");
    	if (p_MaxExposureTime && p_MaxExposureTime->data)
    	{
    		p_req->ImagingSettings.Exposure.MaxExposureTimeFlag = 1;
    		p_req->ImagingSettings.Exposure.MaxExposureTime = (float)atof(p_MaxExposureTime->data);
    	}

    	p_Gain = _node_soap_get(p_Exposure, "Gain");
    	if (p_Gain && p_Gain->data)
    	{
    		p_req->ImagingSettings.Exposure.GainFlag = 1;
    		p_req->ImagingSettings.Exposure.Gain = (float)atof(p_Gain->data);
    	}

    }


    p_IrCutFilter = _node_soap_get(p_ImagingSettings, "IrCutFilter");
    if (p_IrCutFilter && p_IrCutFilter->data)
    {
    	p_req->ImagingSettings.IrCutFilterFlag = 1;
    	p_req->ImagingSettings.IrCutFilter = onvif_StringToIrCutFilterMode(p_IrCutFilter->data);
    }

    p_Sharpness = _node_soap_get(p_ImagingSettings, "Sharpness");
    if (p_Sharpness && p_Sharpness->data)
    {
    	p_req->ImagingSettings.SharpnessFlag = 1;
    	p_req->ImagingSettings.Sharpness = (float)atof(p_Sharpness->data);
    }


    p_WhiteBalance = _node_soap_get(p_ImagingSettings, "WhiteBalance");
    if (p_WhiteBalance)
    {
    	XMLN * p_Mode;
		XMLN * p_CrGain;
		XMLN * p_CbGain;

		p_req->ImagingSettings.WhiteBalanceFlag = 1;

		p_Mode = _node_soap_get(p_WhiteBalance, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		p_req->ImagingSettings.WhiteBalance.Mode = onvif_StringToWhiteBalanceMode(p_Mode->data);
    	}

    	p_CrGain = _node_soap_get(p_WhiteBalance, "CrGain");
    	if (p_CrGain && p_CrGain->data)
    	{
    		p_req->ImagingSettings.WhiteBalance.CrGainFlag = 1;
    	    p_req->ImagingSettings.WhiteBalance.CrGain = (float)atof(p_CrGain->data);
    	}

    	p_CbGain = _node_soap_get(p_WhiteBalance, "CbGain");
    	if (p_CbGain && p_CbGain->data)
    	{
    		p_req->ImagingSettings.WhiteBalance.CbGainFlag = 1;
    	    p_req->ImagingSettings.WhiteBalance.CbGain = (float)atof(p_CbGain->data);
    	}
    }

    p_ForcePersistence = _node_soap_get(p_node, "ForcePersistence");
    if (p_ForcePersistence && p_ForcePersistence->data)
    {
    	p_req->ForcePersistenceFlag = 1;
    	p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
    }

	return ONVIF_OK;
}

ONVIF_RET parse_img_GetOptions(XMLN * p_node, img_GetOptions_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }
    else
    {
    	return ONVIF_ERR_MissingAttribute;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_img_GetMoveOptions(XMLN * p_node, img_GetMoveOptions_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }
    else
    {
    	return ONVIF_ERR_MissingAttribute;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_img_GetStatus(XMLN * p_node, img_GetStatus_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }
    else
    {
    	return ONVIF_ERR_MissingAttribute;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_img_Stop(XMLN * p_node, img_Stop_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }
    else
    {
    	return ONVIF_ERR_MissingAttribute;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_img_GetPresets(XMLN * p_node, img_GetPresets_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_img_GetCurrentPreset(XMLN * p_node, img_GetCurrentPreset_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_img_SetCurrentPreset(XMLN * p_node, img_SetCurrentPreset_REQ * p_req)
{
    XMLN * p_VideoSourceToken;
    XMLN * p_PresetToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }

    p_PresetToken = _node_soap_get(p_node, "PresetToken");
    if (p_PresetToken && p_PresetToken->data)
    {
        strncpy(p_req->PresetToken, p_PresetToken->data, sizeof(p_req->PresetToken)-1);
    }

    return ONVIF_OK;
}

#endif // IMAGE_SUPPORT

/***************************************************************************************/

#if defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)

ONVIF_RET parse_MulticastConfiguration(XMLN * p_node, onvif_MulticastConfiguration * p_req)
{
	XMLN * p_Multicast;
	XMLN * p_Address;
	XMLN * p_Port;
	XMLN * p_TTL;
	XMLN * p_AutoStart;

	p_Multicast = _node_soap_get(p_node, "Multicast");
    if (NULL == p_Multicast)
    {
        return ONVIF_ERR_MissingAttribute;
    }

    p_Address = _node_soap_get(p_Multicast, "Address");
    if (p_Address)
    {
    	XMLN * p_IPv4Address;

        p_IPv4Address = _node_soap_get(p_Address, "IPv4Address");
	    if (p_IPv4Address && p_IPv4Address->data)
	    {
	        strncpy(p_req->IPv4Address, p_IPv4Address->data, sizeof(p_req->IPv4Address)-1);
	    }
    }

    p_Port = _node_soap_get(p_Multicast, "Port");
    if (p_Port && p_Port->data)
    {
    	p_req->Port = atoi(p_Port->data);
    }


    p_AutoStart = _node_soap_get(p_Multicast, "AutoStart");
    if (p_AutoStart && p_AutoStart->data)
    {
    	p_req->AutoStart = parse_Bool(p_AutoStart->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_OSDColor(XMLN * p_node, onvif_OSDColor * p_req)
{
	XMLN * p_Color;
	const char * p_Transparent;

	p_Transparent = _attr_get(p_node, "Transparent");
	if (p_Transparent)
	{
		p_req->TransparentFlag = 1;
		p_req->Transparent = atoi(p_Transparent);
	}

	p_Color = _node_soap_get(p_node, "Color");
	if (p_Color)
	{
		const char * p_X;
		const char * p_Y;
		const char * p_Z;

		p_X = _attr_get(p_Color, "X");
		if (p_X)
		{
			p_req->X = (float) atof(p_X);
		}

		p_Y = _attr_get(p_Color, "Y");
		if (p_X)
		{
			p_req->Y = (float) atof(p_Y);
		}

		p_Z = _attr_get(p_Color, "Z");
		if (p_Z)
		{
			p_req->Z = (float) atof(p_Z);
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_OSDConfiguration(XMLN * p_node, onvif_OSDConfiguration * p_req)
{
	XMLN * p_OSD;
	XMLN * p_VideoSourceConfigurationToken;
	XMLN * p_Type;
	XMLN * p_Position;
	XMLN * p_TextString;
	XMLN * p_Image;
	const char * p_token;

	p_OSD = _node_soap_get(p_node, "OSD");
	if (NULL == p_OSD)
	{
		return ONVIF_ERR_MissingAttribute;
	}

	p_token = _attr_get(p_OSD, "token");
	if (p_token)
	{
		strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
	}

	p_VideoSourceConfigurationToken = _node_soap_get(p_OSD, "VideoSourceConfigurationToken");
	if (p_VideoSourceConfigurationToken && p_VideoSourceConfigurationToken->data)
	{
		strncpy(p_req->VideoSourceConfigurationToken, p_VideoSourceConfigurationToken->data, sizeof(p_req->VideoSourceConfigurationToken)-1);
	}

	p_Type = _node_soap_get(p_OSD, "Type");
	if (p_Type && p_Type->data)
	{
		p_req->Type = onvif_StringToOSDType(p_Type->data);
	}

	p_Position = _node_soap_get(p_OSD, "Position");
	if (p_Position)
	{
		XMLN * p_Type;
		XMLN * p_Pos;

		p_Type = _node_soap_get(p_Position, "Type");
		if (p_Type && p_Type->data)
		{
			p_req->Position.Type = onvif_StringToOSDPosType(p_Type->data);
		}

		p_Pos = _node_soap_get(p_Position, "Pos");
		if (p_Pos)
		{
			p_req->Position.PosFlag = parse_Vector(p_Pos, &p_req->Position.Pos);
		}
	}

	p_TextString = _node_soap_get(p_OSD, "TextString");
	if (p_TextString)
	{
		XMLN * p_Type;
		XMLN * p_DateFormat;
		XMLN * p_TimeFormat;
		XMLN * p_FontSize;
		XMLN * p_FontColor;
		XMLN * p_BackgroundColor;
		XMLN * p_PlainText;

		p_req->TextStringFlag = 1;

		p_Type = _node_soap_get(p_TextString, "Type");
		if (p_Type && p_Type->data)
		{
			p_req->TextString.Type = onvif_StringToOSDTextType(p_Type->data);
		}

		p_DateFormat = _node_soap_get(p_TextString, "DateFormat");
		if (p_DateFormat && p_DateFormat->data)
		{
			p_req->TextString.DateFormatFlag = 1;
			strncpy(p_req->TextString.DateFormat, p_DateFormat->data, sizeof(p_req->TextString.DateFormat)-1);
		}

		p_TimeFormat = _node_soap_get(p_TextString, "TimeFormat");
		if (p_TimeFormat && p_TimeFormat->data)
		{
			p_req->TextString.TimeFormatFlag = 1;
			strncpy(p_req->TextString.TimeFormat, p_TimeFormat->data, sizeof(p_req->TextString.TimeFormat)-1);
		}

		p_FontSize = _node_soap_get(p_TextString, "FontSize");
		if (p_FontSize && p_FontSize->data)
		{
			p_req->TextString.FontSizeFlag = 1;
			p_req->TextString.FontSize = atoi(p_FontSize->data);
		}

		p_FontColor = _node_soap_get(p_TextString, "FontColor");
		if (p_FontColor)
		{
			p_req->TextString.FontColorFlag = 1;

			parse_OSDColor(p_FontColor, &p_req->TextString.FontColor);
		}

		p_BackgroundColor = _node_soap_get(p_TextString, "BackgroundColor");
		if (p_BackgroundColor)
		{
			p_req->TextString.BackgroundColorFlag = 1;

			parse_OSDColor(p_BackgroundColor, &p_req->TextString.BackgroundColor);
		}

		p_PlainText = _node_soap_get(p_TextString, "PlainText");
		if (p_PlainText && p_PlainText->data)
		{
			p_req->TextString.PlainTextFlag = 1;
			strncpy(p_req->TextString.PlainText, p_PlainText->data, sizeof(p_req->TextString.PlainText)-1);
		}
	}

	p_Image = _node_soap_get(p_OSD, "Image");
	if (p_Image)
	{
		XMLN * p_ImgPath;

		p_req->ImageFlag = 1;

		p_ImgPath = _node_soap_get(p_Image, "ImgPath");
		if (p_ImgPath && p_ImgPath->data)
		{
			strncpy(p_req->Image.ImgPath, p_ImgPath->data, sizeof(p_req->Image.ImgPath)-1);
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_VideoSourceConfiguration(XMLN * p_node, onvif_VideoSourceConfiguration * p_req)
{
    XMLN * p_Configuration;
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_SourceToken;
	XMLN * p_Bounds;

	p_Configuration = _node_soap_get(p_node, "Configuration");
	if (p_Configuration)
	{
		const char * token = _attr_get(p_Configuration, "token");
		if (token)
		{
			strncpy(p_req->token, token, sizeof(p_req->token)-1);
		}
		else
		{
			return ONVIF_ERR_MissingAttribute;
		}
	}
	else
	{
		return ONVIF_ERR_MissingAttribute;
	}

	p_Name = _node_soap_get(p_Configuration, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}

	p_UseCount = _node_soap_get(p_Configuration, "UseCount");
	if (p_UseCount && p_UseCount->data)
	{
		p_req->UseCount = atoi(p_UseCount->data);
	}

	p_SourceToken = _node_soap_get(p_Configuration, "SourceToken");
	if (p_SourceToken && p_SourceToken->data)
	{
		strncpy(p_req->SourceToken, p_SourceToken->data, sizeof(p_req->SourceToken)-1);
	}

	p_Bounds = _node_soap_get(p_Configuration, "Bounds");
	if (p_Bounds)
	{
		const char * p_x;
		const char * p_y;
		const char * p_width;
		const char * p_height;

		p_x = _attr_get(p_Bounds, "x");
		if (p_x)
		{
			p_req->Bounds.x = atoi(p_x);
		}

		p_y = _attr_get(p_Bounds, "y");
		if (p_y)
		{
			p_req->Bounds.y = atoi(p_y);
		}

		p_width = _attr_get(p_Bounds, "width");
		if (p_width)
		{
			p_req->Bounds.width = atoi(p_width);
		}

		p_height = _attr_get(p_Bounds, "height");
		if (p_height)
		{
			p_req->Bounds.height = atoi(p_height);
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_MetadataConfiguration(XMLN * p_node, onvif_MetadataConfiguration * p_req)
{
    XMLN * p_Configuration;
	XMLN * p_Name;
	XMLN * p_PTZStatus;
	XMLN * p_Events;
	XMLN * p_Analytics;
	XMLN * p_SessionTimeout;
	const char * token;

	p_Configuration = _node_soap_get(p_node, "Configuration");
	if (NULL == p_Configuration)
	{
		return ONVIF_ERR_MissingAttribute;
	}

	token = _attr_get(p_Configuration, "token");
	if (token)
	{
		strncpy(p_req->token, token, sizeof(p_req->token)-1);
	}

	p_Name = _node_soap_get(p_Configuration, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}

	p_PTZStatus = _node_soap_get(p_Configuration, "PTZStatus");
	if (p_PTZStatus)
	{
		XMLN * p_Status;
		XMLN * p_Position;

		p_req->PTZStatusFlag = 1;

		p_Status = _node_soap_get(p_PTZStatus, "Status");
		if (p_Status && p_Status->data)
		{
			p_req->PTZStatus.Status = parse_Bool(p_Status->data);
		}

		p_Position = _node_soap_get(p_PTZStatus, "Position");
		if (p_Position && p_Position->data)
		{
			p_req->PTZStatus.Position = parse_Bool(p_Position->data);
		}
	}

    p_Events = _node_soap_get(p_Configuration, "Events");
	if (p_Events)
	{
	    XMLN * p_Filter;

	    p_req->EventsFlag = 1;

	    p_Filter = _node_soap_get(p_Events, "Filter");
		if (p_Filter)
		{
		    XMLN * p_TopicExpression;

		    p_TopicExpression = _node_soap_get(p_Filter, "TopicExpression");
    		if (p_TopicExpression && p_TopicExpression->data)
    		{
    		    const char * p_Dialect;

    		    p_Dialect = _attr_get(p_TopicExpression, "Dialect");
    		    if (p_Dialect)
    		    {
    		        strncpy(p_req->Events.Dialect, p_Dialect, sizeof(p_req->Events.Dialect)-1);
    		    }

			    strncpy(p_req->Events.TopicExpression, p_TopicExpression->data, sizeof(p_req->Events.TopicExpression)-1);
			}
		}
	}

	p_Analytics = _node_soap_get(p_Configuration, "Analytics");
	if (p_Analytics && p_Analytics->data)
	{
		p_req->AnalyticsFlag = 1;
		p_req->Analytics = parse_Bool(p_Analytics->data);
	}

	parse_MulticastConfiguration(p_Configuration, &p_req->Multicast);

	p_SessionTimeout = _node_soap_get(p_Configuration, "SessionTimeout");
	if (p_SessionTimeout && p_SessionTimeout->data)
	{
		parse_XSDDuration(p_SessionTimeout->data, &p_req->SessionTimeout);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_SetOSD(XMLN * p_node, trt_SetOSD_REQ * p_req)
{
	return parse_OSDConfiguration(p_node, &p_req->OSD);
}

ONVIF_RET _trt_CreateOSD(XMLN * p_node, trt_CreateOSD_REQ * p_req)
{
	return parse_OSDConfiguration(p_node, &p_req->OSD);
}

ONVIF_RET _trt_DeleteOSD(XMLN * p_node, trt_DeleteOSD_REQ * p_req)
{
	XMLN * p_OSDToken = _node_soap_get(p_node, "OSDToken");
	if (p_OSDToken && p_OSDToken->data)
	{
		strncpy(p_req->OSDToken, p_OSDToken->data, sizeof(p_req->OSDToken)-1);
	}

	return ONVIF_OK;
}

#ifdef DEVICEIO_SUPPORT

ONVIF_RET parse_AudioOutputConfiguration(XMLN * p_node, onvif_AudioOutputConfiguration * p_req)
{
    XMLN * p_Name;
    XMLN * p_UseCount;
    XMLN * p_OutputToken;
    XMLN * p_SendPrimacy;
    XMLN * p_OutputLevel;
    const char * p_token;

    p_token = _attr_get(p_node, "token");
    if (p_token)
    {
        strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
    }

    p_Name = _node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
    }

    p_UseCount = _node_soap_get(p_node, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        p_req->UseCount = atoi(p_UseCount->data);
    }

    p_OutputToken = _node_soap_get(p_node, "OutputToken");
    if (p_OutputToken && p_OutputToken->data)
    {
        strncpy(p_req->OutputToken, p_OutputToken->data, sizeof(p_req->OutputToken)-1);
    }

    p_SendPrimacy = _node_soap_get(p_node, "SendPrimacy");
    if (p_SendPrimacy && p_SendPrimacy->data)
    {
        p_req->SendPrimacyFlag = 1;
        strncpy(p_req->SendPrimacy, p_SendPrimacy->data, sizeof(p_req->SendPrimacy)-1);
    }

    p_OutputLevel = _node_soap_get(p_node, "OutputLevel");
    if (p_OutputLevel && p_OutputLevel->data)
    {
        p_req->OutputLevel = atoi(p_OutputLevel->data);
    }

    return ONVIF_OK;
}


#endif // DEVICEIO_SUPPORT

#endif // defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)




ONVIF_RET _trt_GetAudioEncoderConfigurationOptions(XMLN * p_node, trt_GetAudioEncoderConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;

	assert(p_node);

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}


#ifdef MEDIA_SUPPORT

ONVIF_RET _trt_SetVideoEncoderConfiguration(XMLN * p_node, trt_SetVideoEncoderConfiguration_REQ * p_req)
{
    XMLN * p_Configuration;
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_Encoding;
	XMLN * p_Resolution;
	XMLN * p_RateControl;
	XMLN * p_SessionTimeout;
	XMLN * p_ForcePersistence;
	const char * token;

	p_Configuration = _node_soap_get(p_node, "Configuration");
    if (NULL == p_Configuration)
    {
        return ONVIF_ERR_MissingAttribute;
    }

    token = _attr_get(p_Configuration, "token");
    if (token)
    {
        strncpy(p_req->Configuration.token, token, sizeof(p_req->Configuration.token)-1);
    }

    p_Name = _node_soap_get(p_Configuration, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_req->Configuration.Name, p_Name->data, sizeof(p_req->Configuration.Name)-1);
    }

    p_UseCount = _node_soap_get(p_Configuration, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        p_req->Configuration.UseCount = atoi(p_UseCount->data);
    }

    p_Encoding = _node_soap_get(p_Configuration, "Encoding");
    if (p_Encoding && p_Encoding->data)
    {
		p_req->Configuration.Encoding = onvif_StringToVideoEncoding(p_Encoding->data);
    }

    p_Resolution = _node_soap_get(p_Configuration, "Resolution");
    if (p_Resolution)
    {
        XMLN * p_Width;
		XMLN * p_Height;

		p_Width = _node_soap_get(p_Resolution, "Width");
	    if (p_Width && p_Width->data)
	    {
	        p_req->Configuration.Resolution.Width = atoi(p_Width->data);
	    }

	    p_Height = _node_soap_get(p_Resolution, "Height");
	    if (p_Height && p_Height->data)
	    {
	        p_req->Configuration.Resolution.Height = atoi(p_Height->data);
	    }
    }

    p_RateControl = _node_soap_get(p_Configuration, "RateControl");
    if (p_RateControl)
    {
    	XMLN * p_FrameRateLimit;
		XMLN * p_EncodingInterval;
		XMLN * p_BitrateLimit;  

        p_FrameRateLimit = _node_soap_get(p_RateControl, "FrameRateLimit");
	    if (p_FrameRateLimit && p_FrameRateLimit->data)
	    {
	        p_req->Configuration.RateControl.FrameRateLimit = atoi(p_FrameRateLimit->data);
	    }


	    p_EncodingInterval = _node_soap_get(p_RateControl, "EncodingInterval");
	    if (p_EncodingInterval && p_EncodingInterval->data)
	    {
	        p_req->Configuration.RateControl.EncodingInterval = atoi(p_EncodingInterval->data);
	    }


	    p_BitrateLimit = _node_soap_get(p_RateControl, "BitrateLimit");
	    if (p_BitrateLimit && p_BitrateLimit->data)
	    {
	        p_req->Configuration.RateControl.BitrateLimit = atoi(p_BitrateLimit->data);
	    }
    }

    if (p_req->Configuration.Encoding == VideoEncoding_H264)
    {
    	XMLN * p_H264 = _node_soap_get(p_Configuration, "H264");
    	if (p_H264)
    	{
    		XMLN * p_GovLength;
			XMLN * p_H264Profile;

    		p_req->Configuration.H264Flag = 1;

    		p_GovLength = _node_soap_get(p_H264, "GovLength");
		    if (p_GovLength && p_GovLength->data)
		    {
		        p_req->Configuration.H264.GovLength = atoi(p_GovLength->data);
		    }

		    p_H264Profile = _node_soap_get(p_H264, "H264Profile");
		    if (p_H264Profile && p_H264Profile->data)
		    {
				p_req->Configuration.H264.H264Profile = onvif_StringToH264Profile(p_H264Profile->data);
		    }
    	}
    }

	parse_MulticastConfiguration(p_Configuration, &p_req->Configuration.Multicast);

	p_SessionTimeout = _node_soap_get(p_Configuration, "SessionTimeout");
	if (p_SessionTimeout && p_SessionTimeout->data)
	{
		parse_XSDDuration(p_SessionTimeout->data, &p_req->Configuration.SessionTimeout);
	}

	p_ForcePersistence = _node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}

    return ONVIF_OK;
}

ONVIF_RET _trt_SetSynchronizationPoint(XMLN * p_node, trt_SetSynchronizationPoint_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET _trt_GetProfile(XMLN * p_node, trt_GetProfile_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_CreateProfile(XMLN * p_node, trt_CreateProfile_REQ * p_req)
{
	XMLN * p_Name;
	XMLN * p_Token;

	assert(p_node);

	p_Name = _node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}

	p_Token = _node_soap_get(p_node, "Token");
	if (p_Token && p_Token->data)
	{
		p_req->TokenFlag = 1;
		strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_DeleteProfile(XMLN * p_node, trt_DeleteProfile_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_AddVideoSourceConfiguration(XMLN * p_node, trt_AddVideoSourceConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;

	assert(p_node);

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_RemoveVideoSourceConfiguration(XMLN * p_node, trt_RemoveVideoSourceConfiguration_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_AddVideoEncoderConfiguration(XMLN * p_node, trt_AddVideoEncoderConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;

	assert(p_node);

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_RemoveVideoEncoderConfiguration(XMLN * p_node, trt_RemoveVideoEncoderConfiguration_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_GetStreamUri(XMLN * p_node, trt_GetStreamUri_REQ * p_req)
{
	ONVIF_RET ret = ONVIF_OK;
	XMLN * p_StreamSetup;
	XMLN * p_ProfileToken;

	p_StreamSetup = _node_soap_get(p_node, "StreamSetup");
	if (p_StreamSetup)
	{
		ret = parse_StreamSetup(p_StreamSetup, &p_req->StreamSetup);
	}

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ret;
}

ONVIF_RET _trt_GetSnapshotUri(XMLN * p_node, trt_GetSnapshotUri_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_GetVideoSourceConfigurationOptions(XMLN * p_node, trt_GetVideoSourceConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;

	assert(p_node);

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_SetVideoSourceConfiguration(XMLN * p_node, trt_SetVideoSourceConfiguration_REQ * p_req)
{
	XMLN * p_ForcePersistence;

    parse_VideoSourceConfiguration(p_node, &p_req->Configuration);

	p_ForcePersistence = _node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_GetVideoEncoderConfigurationOptions(XMLN * p_node, trt_GetVideoEncoderConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;

	assert(p_node);

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_GetOSDs(XMLN * p_node, trt_GetOSDs_REQ * p_req)
{
	XMLN * p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_GetOSD(XMLN * p_node, trt_GetOSD_REQ * p_req)
{
	XMLN * p_OSDToken = _node_soap_get(p_node, "OSDToken");
	if (p_OSDToken && p_OSDToken->data)
	{
		strncpy(p_req->OSDToken, p_OSDToken->data, sizeof(p_req->OSDToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_SetMetadataConfiguration(XMLN * p_node, trt_SetMetadataConfiguration_REQ * p_req)
{
	XMLN * p_ForcePersistence;

	parse_MetadataConfiguration(p_node, &p_req->Configuration);

	p_ForcePersistence = _node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_AddMetadataConfiguration(XMLN * p_node, trt_AddMetadataConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_GetMetadataConfigurationOptions(XMLN * p_node, trt_GetMetadataConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_GetVideoSourceModes(XMLN * p_node, trt_GetVideoSourceModes_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
		strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_SetVideoSourceMode(XMLN * p_node, trt_SetVideoSourceMode_REQ * p_req)
{
    XMLN * p_VideoSourceToken;
    XMLN * p_VideoSourceModeToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
		strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	p_VideoSourceModeToken = _node_soap_get(p_node, "VideoSourceModeToken");
	if (p_VideoSourceModeToken && p_VideoSourceModeToken->data)
	{
		strncpy(p_req->VideoSourceModeToken, p_VideoSourceModeToken->data, sizeof(p_req->VideoSourceModeToken)-1);
	}

	return ONVIF_OK;
}
ONVIF_RET _trt_GetAudioSourceConfigurationOptions(XMLN * p_node, trt_GetAudioSourceConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;

	assert(p_node);

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}


#ifdef DEVICEIO_SUPPORT

ONVIF_RET _trt_GetAudioOutputConfiguration(XMLN * p_node, trt_GetAudioOutputConfiguration_REQ * p_req)
{
    XMLN * p_ConfigurationToken;

    p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
    if (p_ConfigurationToken && p_ConfigurationToken->data)
    {
        strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET _trt_SetAudioOutputConfiguration(XMLN * p_node, tmd_SetAudioOutputConfiguration_REQ * p_req)
{
    XMLN * p_Configuration;
    XMLN * p_ForcePersistence;

    p_Configuration = _node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
        parse_AudioOutputConfiguration(p_Configuration, &p_req->Configuration);
    }

    p_ForcePersistence = _node_soap_get(p_node, "ForcePersistence");
    if (p_ForcePersistence && p_ForcePersistence->data)
    {
        p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
    }

    return ONVIF_OK;
}

ONVIF_RET _trt_GetCompatibleAudioOutputConfigurations(XMLN * p_node, trt_GetCompatibleAudioOutputConfigurations_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET _trt_AddAudioOutputConfiguration(XMLN * p_node, trt_AddAudioOutputConfiguration_REQ * p_req)
{
    XMLN * p_ProfileToken;
    XMLN * p_ConfigurationToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
    if (p_ConfigurationToken && p_ConfigurationToken->data)
    {
        strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET _trt_RemoveAudioOutputConfiguration(XMLN * p_node, trt_RemoveAudioOutputConfiguration_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    return ONVIF_OK;
}

#endif // DEVICEIO_SUPPORT

#ifdef VIDEO_ANALYTICS

ONVIF_RET parse_AnalyticsEngineConfiguration(XMLN * p_node, onvif_AnalyticsEngineConfiguration * p_req)
{
	XMLN * p_AnalyticsModule;
	ConfigList * p_config;

	p_AnalyticsModule = _node_soap_get(p_node, "AnalyticsModule");
	while (p_AnalyticsModule && soap_strcmp(p_AnalyticsModule->name, "AnalyticsModule") == 0)
	{
		p_config = onvif_add_Config(&p_req->AnalyticsModule);
		if (p_config)
		{
			parse_Config(p_AnalyticsModule, &p_config->Config);
		}

		p_AnalyticsModule = p_AnalyticsModule->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_RuleEngineConfiguration(XMLN * p_node, onvif_RuleEngineConfiguration * p_req)
{
	XMLN * p_Rule;
	ConfigList * p_config;

	p_Rule = _node_soap_get(p_node, "Rule");
	while (p_Rule && soap_strcmp(p_Rule->name, "Rule") == 0)
	{
		p_config = onvif_add_Config(&p_req->Rule);
		if (p_config)
		{
			parse_Config(p_Rule, &p_config->Config);
		}

		p_Rule = p_Rule->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_VideoAnalyticsConfiguration(XMLN * p_node, onvif_VideoAnalyticsConfiguration * p_req)
{
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_AnalyticsEngineConfiguration;
	XMLN * p_RuleEngineConfiguration;
	const char * p_token;
	ONVIF_RET ret = ONVIF_OK;

	p_token = _attr_get(p_node, "token");
	if (p_token)
	{
		strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
	}

	p_Name = _node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}

	p_UseCount = _node_soap_get(p_node, "UseCount");
	if (p_UseCount && p_UseCount->data)
	{
		p_req->UseCount = atoi(p_UseCount->data);
	}

	p_AnalyticsEngineConfiguration = _node_soap_get(p_node, "AnalyticsEngineConfiguration");
	if (p_AnalyticsEngineConfiguration)
	{
		ret = parse_AnalyticsEngineConfiguration(p_AnalyticsEngineConfiguration, &p_req->AnalyticsEngineConfiguration);
		if (ONVIF_OK != ret)
		{
			return ret;
		}
	}

	p_RuleEngineConfiguration = _node_soap_get(p_node, "RuleEngineConfiguration");
	if (p_RuleEngineConfiguration)
	{
		ret = parse_RuleEngineConfiguration(p_RuleEngineConfiguration, &p_req->RuleEngineConfiguration);
	}

	return ret;
}

ONVIF_RET _trt_AddVideoAnalyticsConfiguration(XMLN * p_node, trt_AddVideoAnalyticsConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_GetVideoAnalyticsConfiguration(XMLN * p_node, trt_GetVideoAnalyticsConfiguration_REQ * p_req)
{
	XMLN * p_ConfigurationToken;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_RemoveVideoAnalyticsConfiguration(XMLN * p_node, trt_RemoveVideoAnalyticsConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET _trt_SetVideoAnalyticsConfiguration(XMLN * p_node, trt_SetVideoAnalyticsConfiguration_REQ * p_req)
{
	XMLN * p_Configuration;
	XMLN * p_ForcePersistence;
	ONVIF_RET ret = ONVIF_ERR_MissingAttribute;

	p_Configuration = _node_soap_get(p_node, "Configuration");
	if (p_Configuration)
	{
		ret = parse_VideoAnalyticsConfiguration(p_Configuration, &p_req->Configuration);
	}

	p_ForcePersistence = _node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}

	return ret;
}

ONVIF_RET _trt_GetCompatibleVideoAnalyticsConfigurations(XMLN * p_node, trt_GetCompatibleVideoAnalyticsConfigurations_REQ * p_req)
{
    XMLN * p_ProfileToken;

	p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken));
	}

	return ONVIF_OK;
}

#endif // VIDEO_ANALYTICS

#endif // MEDIA_SUPPORT

#ifdef PROFILE_G_SUPPORT

ONVIF_RET parse_RecordingConfiguration(XMLN * p_node, onvif_RecordingConfiguration * p_req)
{
	XMLN * p_Source;
	XMLN * p_Content;
	XMLN * p_MaximumRetentionTime;

	p_Source = _node_soap_get(p_node, "Source");
	if (p_Source)
	{
		XMLN * p_SourceId;
		XMLN * p_Name;
		XMLN * p_Location;
		XMLN * p_Description;
		XMLN * p_Address;

		p_SourceId = _node_soap_get(p_Source, "SourceId");
		if (p_SourceId && p_SourceId->data)
		{
			strncpy(p_req->Source.SourceId, p_SourceId->data, sizeof(p_req->Source.SourceId)-1);
		}

		p_Name = _node_soap_get(p_Source, "Name");
		if (p_Name && p_Name->data)
		{
			strncpy(p_req->Source.Name, p_Name->data, sizeof(p_req->Source.Name)-1);
		}

		p_Location = _node_soap_get(p_Source, "Location");
		if (p_Location && p_Location->data)
		{
			strncpy(p_req->Source.Location, p_Location->data, sizeof(p_req->Source.Location)-1);
		}

		p_Description = _node_soap_get(p_Source, "Description");
		if (p_Description && p_Description->data)
		{
			strncpy(p_req->Source.Description, p_Description->data, sizeof(p_req->Source.Description)-1);
		}

		p_Address = _node_soap_get(p_Source, "Address");
		if (p_Address && p_Address->data)
		{
			strncpy(p_req->Source.Address, p_Address->data, sizeof(p_req->Source.Address)-1);
		}
	}

	p_Content = _node_soap_get(p_node, "Content");
	if (p_Content && p_Content->data)
	{
		strncpy(p_req->Content, p_Content->data, sizeof(p_req->Content)-1);
	}

	p_MaximumRetentionTime = _node_soap_get(p_node, "MaximumRetentionTime");
	if (p_MaximumRetentionTime && p_MaximumRetentionTime->data)
	{
		p_req->MaximumRetentionTimeFlag = parse_XSDDuration(p_MaximumRetentionTime->data, (int*)&p_req->MaximumRetentionTime);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_TrackConfiguration(XMLN * p_node, onvif_TrackConfiguration * p_req)
{
	XMLN * p_TrackType;
	XMLN * p_Description;

	p_TrackType = _node_soap_get(p_node, "TrackType");
	if (p_TrackType && p_TrackType->data)
	{
		p_req->TrackType = onvif_StringToTrackType(p_TrackType->data);
		if (TrackType_Invalid == p_req->TrackType)
		{
			return ONVIF_ERR_BadConfiguration;
		}
	}

	p_Description = _node_soap_get(p_node, "Description");
	if (p_Description && p_Description->data)
	{
		strncpy(p_req->Description, p_Description->data, sizeof(p_req->Description)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_JobConfiguration(XMLN * p_node, onvif_RecordingJobConfiguration * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_Mode;
	XMLN * p_Priority;
	XMLN * p_Source;

	p_RecordingToken = _node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_Mode = _node_soap_get(p_node, "Mode");
	if (p_Mode && p_Mode->data)
	{
		strncpy(p_req->Mode, p_Mode->data, sizeof(p_req->Mode)-1);
	}

	p_Priority = _node_soap_get(p_node, "Priority");
	if (p_Priority && p_Priority->data)
	{
		p_req->Priority = atoi(p_Priority->data);
	}

	p_Source = _node_soap_get(p_node, "Source");
	while (p_Source && soap_strcmp(p_Source->name, "Source") == 0)
	{
		int i = p_req->sizeSource;
		XMLN * p_SourceToken;
		XMLN * p_AutoCreateReceiver;
		XMLN * p_Tracks;

		p_SourceToken = _node_soap_get(p_Source, "SourceToken");
		if (p_SourceToken)
		{
			const char * p_Type;
			XMLN * p_Token;

			p_req->Source[i].SourceTokenFlag = 1;

			p_Type = _attr_get(p_SourceToken, "Type");
			if (p_Type)
			{
				p_req->Source[i].SourceToken.TypeFlag = 1;
				strncpy(p_req->Source[i].SourceToken.Type, p_Type, sizeof(p_req->Source[i].SourceToken.Type)-1);
			}

			p_Token = _node_soap_get(p_SourceToken, "Token");
			if (p_Token && p_Token->data)
			{
				strncpy(p_req->Source[i].SourceToken.Token, p_Token->data, sizeof(p_req->Source[i].SourceToken.Token)-1);
			}
		}

		p_AutoCreateReceiver = _node_soap_get(p_Source, "AutoCreateReceiver");
		if (p_AutoCreateReceiver && p_AutoCreateReceiver->data)
		{
			p_req->Source[i].AutoCreateReceiverFlag = 1;
			p_req->Source[i].AutoCreateReceiver = parse_Bool(p_AutoCreateReceiver->data);
		}

		p_Tracks = _node_soap_get(p_Source, "Tracks");
		while (p_Tracks && soap_strcmp(p_Tracks->name, "Tracks") == 0)
		{
			int j = p_req->Source[i].sizeTracks;
			XMLN * p_SourceTag;
			XMLN * p_Destination;

			p_SourceTag = _node_soap_get(p_Tracks, "SourceTag");
			if (p_SourceTag && p_SourceTag->data)
			{
				strncpy(p_req->Source[i].Tracks[j].SourceTag, p_SourceTag->data, sizeof(p_req->Source[i].Tracks[j].SourceTag)-1);
			}

			p_Destination = _node_soap_get(p_Tracks, "Destination");
			if (p_Destination && p_Destination->data)
			{
				strncpy(p_req->Source[i].Tracks[j].Destination, p_Destination->data, sizeof(p_req->Source[i].Tracks[j].Destination)-1);
			}

			p_req->Source[i].sizeTracks++;
			if (p_req->Source[i].sizeTracks >= ARRAY_SIZE(p_req->Source[i].Tracks))
			{
				break;
			}

			p_Tracks = p_Tracks->next;
		}

		p_req->sizeSource++;
		if (p_req->sizeSource >= ARRAY_SIZE(p_req->Source))
		{
			break;
		}

		p_Source = p_Source->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SearchScope(XMLN * p_node, onvif_SearchScope * p_req)
{
	XMLN * p_IncludedSources;
	XMLN * p_IncludedRecordings;
	XMLN * p_RecordingInformationFilter;

	p_IncludedSources = _node_soap_get(p_node, "IncludedSources");
	while (p_IncludedSources && soap_strcmp(p_IncludedSources->name, "IncludedSources") == 0)
	{
		int idx = p_req->sizeIncludedSources;
		const char * p_Type;
		XMLN * p_Token;

		p_Type = _attr_get(p_IncludedSources, "Type");
		if (p_Type)
		{
			p_req->IncludedSources[idx].TypeFlag = 1;
			strncpy(p_req->IncludedSources[idx].Type, p_Type, sizeof(p_req->IncludedSources[idx].Type));
		}

		p_Token = _node_soap_get(p_IncludedSources, "Token");
		if (p_Token && p_Token->data)
		{
			strncpy(p_req->IncludedSources[idx].Token, p_Token->data, sizeof(p_req->IncludedSources[idx].Token));
		}

		p_req->sizeIncludedSources++;
		if (p_req->sizeIncludedSources >= ARRAY_SIZE(p_req->IncludedSources))
		{
			break;
		}

		p_IncludedSources = p_IncludedSources->next;
	}

	p_IncludedRecordings = _node_soap_get(p_node, "IncludedRecordings");
	while (p_IncludedRecordings && p_IncludedRecordings->data && soap_strcmp(p_IncludedRecordings->name, "IncludedRecordings") == 0)
	{
		int idx = p_req->sizeIncludedRecordings;

		strncpy(p_req->IncludedRecordings[idx], p_IncludedRecordings->data, sizeof(p_req->IncludedRecordings[idx])-1);

		p_req->sizeIncludedRecordings++;
		if (p_req->sizeIncludedRecordings >= ARRAY_SIZE(p_req->IncludedRecordings))
		{
			break;
		}

		p_IncludedRecordings = p_IncludedRecordings->next;
	}

	p_RecordingInformationFilter = _node_soap_get(p_node, "RecordingInformationFilter");
	if (p_RecordingInformationFilter && p_RecordingInformationFilter->data)
	{
		strncpy(p_req->RecordingInformationFilter, p_RecordingInformationFilter->data, sizeof(p_req->RecordingInformationFilter));
	}

	return ONVIF_OK;
}

ONVIF_RET parse_trc_CreateRecording(XMLN * p_node, trc_CreateRecording_REQ * p_req)
{
	XMLN * p_RecordingConfiguration;

	p_RecordingConfiguration = _node_soap_get(p_node, "RecordingConfiguration");
	if (NULL == p_RecordingConfiguration)
	{
		return ONVIF_ERR_BadConfiguration;
	}
	else
	{
		return parse_RecordingConfiguration(p_RecordingConfiguration, &p_req->RecordingConfiguration);
	}
}

ONVIF_RET parse_trc_SetRecordingConfiguration(XMLN * p_node, trc_SetRecordingConfiguration_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_RecordingConfiguration;

	p_RecordingToken = _node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_RecordingConfiguration = _node_soap_get(p_node, "RecordingConfiguration");
	if (p_RecordingConfiguration)
	{
		return parse_RecordingConfiguration(p_RecordingConfiguration, &p_req->RecordingConfiguration);
	}
	else
	{
		return ONVIF_ERR_BadConfiguration;
	}
}

ONVIF_RET parse_trc_CreateTrack(XMLN * p_node, trc_CreateTrack_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_TrackConfiguration;

	p_RecordingToken = _node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_TrackConfiguration = _node_soap_get(p_node, "TrackConfiguration");
	if (p_TrackConfiguration)
	{
		return parse_TrackConfiguration(p_TrackConfiguration, &p_req->TrackConfiguration);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_trc_DeleteTrack(XMLN * p_node, trc_DeleteTrack_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_TrackToken;

	p_RecordingToken = _node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_TrackToken = _node_soap_get(p_node, "TrackToken");
	if (p_TrackToken && p_TrackToken->data)
	{
		strncpy(p_req->TrackToken, p_TrackToken->data, sizeof(p_req->TrackToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_trc_GetTrackConfiguration(XMLN * p_node, trc_GetTrackConfiguration_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_TrackToken;

	p_RecordingToken = _node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_TrackToken = _node_soap_get(p_node, "TrackToken");
	if (p_TrackToken && p_TrackToken->data)
	{
		strncpy(p_req->TrackToken, p_TrackToken->data, sizeof(p_req->TrackToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_trc_SetTrackConfiguration(XMLN * p_node, trc_SetTrackConfiguration_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_TrackToken;
	XMLN * p_TrackConfiguration;

	p_RecordingToken = _node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_TrackToken = _node_soap_get(p_node, "TrackToken");
	if (p_TrackToken && p_TrackToken->data)
	{
		strncpy(p_req->TrackToken, p_TrackToken->data, sizeof(p_req->TrackToken)-1);
	}

	p_TrackConfiguration = _node_soap_get(p_node, "TrackConfiguration");
	if (p_TrackConfiguration)
	{
		return parse_TrackConfiguration(p_TrackConfiguration, &p_req->TrackConfiguration);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_trc_CreateRecordingJob(XMLN * p_node, trc_CreateRecordingJob_REQ * p_req)
{
	XMLN * p_JobConfiguration;

	p_JobConfiguration = _node_soap_get(p_node, "JobConfiguration");
	if (p_JobConfiguration)
	{
		return parse_JobConfiguration(p_JobConfiguration, &p_req->JobConfiguration);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_trc_SetRecordingJobConfiguration(XMLN * p_node, trc_SetRecordingJobConfiguration_REQ * p_req)
{
	XMLN * p_JobToken;
	XMLN * p_JobConfiguration;

	p_JobToken = _node_soap_get(p_node, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{
		strncpy(p_req->JobToken, p_JobToken->data, sizeof(p_req->JobToken)-1);
	}

	p_JobConfiguration = _node_soap_get(p_node, "JobConfiguration");
	if (p_JobConfiguration)
	{
		return parse_JobConfiguration(p_JobConfiguration, &p_req->JobConfiguration);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_trc_SetRecordingJobMode(XMLN * p_node, trc_SetRecordingJobMode_REQ * p_req)
{
	XMLN * p_JobToken;
	XMLN * p_Mode;

	p_JobToken = _node_soap_get(p_node, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{
		strncpy(p_req->JobToken, p_JobToken->data, sizeof(p_req->JobToken)-1);
	}

	p_Mode = _node_soap_get(p_node, "Mode");
	if (p_Mode && p_Mode->data)
	{
		strncpy(p_req->Mode, p_Mode->data, sizeof(p_req->Mode));
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_GetRecordingInformation(XMLN * p_node, tse_GetRecordingInformation_REQ * p_req)
{
    XMLN * p_RecordingToken;

    p_RecordingToken = _node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_GetMediaAttributes(XMLN * p_node, tse_GetMediaAttributes_REQ * p_req)
{
	int idx;
	XMLN * p_RecordingTokens;
	XMLN * p_Time;

	p_RecordingTokens = _node_soap_get(p_node, "RecordingTokens");
	while (p_RecordingTokens && p_RecordingTokens->data && soap_strcmp(p_RecordingTokens->name, "RecordingTokens") == 0)
	{
		idx = p_req->sizeRecordingTokens;
		strncpy(p_req->RecordingTokens[idx], p_RecordingTokens->data, sizeof(p_req->RecordingTokens[idx])-1);

		p_req->sizeRecordingTokens++;
		if (p_req->sizeRecordingTokens >= ARRAY_SIZE(p_req->RecordingTokens))
		{
			break;
		}

		p_RecordingTokens = p_RecordingTokens->next;
	}

	p_Time = _node_soap_get(p_node, "Time");
	if (p_Time && p_Time->data)
	{
		parse_XSDDatetime(p_Time->data, &p_req->Time);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_FindRecordings(XMLN * p_node, tse_FindRecordings_REQ * p_req)
{
	XMLN * p_Scope;
	XMLN * p_MaxMatches;
	XMLN * p_KeepAliveTime;

	p_Scope = _node_soap_get(p_node, "Scope");
	if (p_Scope)
	{
		parse_SearchScope(p_Scope, &p_req->Scope);
	}

	p_MaxMatches = _node_soap_get(p_node, "MaxMatches");
	if (p_MaxMatches && p_MaxMatches->data)
	{
		p_req->MaxMatchesFlag = 1;
		p_req->MaxMatches = atoi(p_MaxMatches->data);
	}

	p_KeepAliveTime = _node_soap_get(p_node, "KeepAliveTime");
	if (p_KeepAliveTime && p_KeepAliveTime->data)
	{
		parse_XSDDuration(p_KeepAliveTime->data, &p_req->KeepAliveTime);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_GetRecordingSearchResults(XMLN * p_node, tse_GetRecordingSearchResults_REQ * p_req)
{
	XMLN * p_SearchToken;
	XMLN * p_MinResults;
	XMLN * p_MaxResults;
	XMLN * p_WaitTime;

	p_SearchToken = _node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_req->SearchToken, p_SearchToken->data, sizeof(p_req->SearchToken)-1);
	}

	p_MinResults = _node_soap_get(p_node, "MinResults");
	if (p_MinResults && p_MinResults->data)
	{
		p_req->MinResultsFlag = 1;
		p_req->MinResults = atoi(p_MinResults->data);
	}

	p_MaxResults = _node_soap_get(p_node, "MaxResults");
	if (p_MaxResults && p_MaxResults->data)
	{
		p_req->MaxResultsFlag = 1;
		p_req->MaxResults = atoi(p_MaxResults->data);
	}

	p_WaitTime = _node_soap_get(p_node, "WaitTime");
	if (p_WaitTime && p_WaitTime->data)
	{
		p_req->WaitTimeFlag = 1;
		parse_XSDDuration(p_WaitTime->data, &p_req->WaitTime);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_FindEvents(XMLN * p_node, tse_FindEvents_REQ * p_req)
{
	XMLN * p_StartPoint;
	XMLN * p_EndPoint;
	XMLN * p_Scope;
	XMLN * p_IncludeStartState;
	XMLN * p_MaxMatches;
	XMLN * p_KeepAliveTime;

	p_StartPoint = _node_soap_get(p_node, "StartPoint");
	if (p_StartPoint && p_StartPoint->data)
	{
		parse_XSDDatetime(p_StartPoint->data, &p_req->StartPoint);
	}

	p_EndPoint = _node_soap_get(p_node, "EndPoint");
	if (p_EndPoint && p_EndPoint->data)
	{
		p_req->EndPointFlag = 1;
		parse_XSDDatetime(p_EndPoint->data, &p_req->EndPoint);
	}

	p_Scope = _node_soap_get(p_node, "Scope");
	if (p_Scope)
	{
		parse_SearchScope(p_Scope, &p_req->Scope);
	}

	p_IncludeStartState = _node_soap_get(p_node, "IncludeStartState");
	if (p_IncludeStartState && p_IncludeStartState->data)
	{
		p_req->IncludeStartState = parse_Bool(p_IncludeStartState->data);
	}

	p_MaxMatches = _node_soap_get(p_node, "MaxMatches");
	if (p_MaxMatches && p_MaxMatches->data)
	{
		p_req->MaxMatchesFlag = 1;
		p_req->MaxMatches = atoi(p_MaxMatches->data);
	}

	p_KeepAliveTime = _node_soap_get(p_node, "KeepAliveTime");
	if (p_KeepAliveTime && p_KeepAliveTime->data)
	{
		p_req->KeepAliveTimeFlag = 1;
		parse_XSDDuration(p_KeepAliveTime->data, &p_req->KeepAliveTime);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_GetEventSearchResults(XMLN * p_node, tse_GetEventSearchResults_REQ * p_req)
{
	XMLN * p_SearchToken;
	XMLN * p_MinResults;
	XMLN * p_MaxResults;
	XMLN * p_WaitTime;

	p_SearchToken = _node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_req->SearchToken, p_SearchToken->data, sizeof(p_req->SearchToken)-1);
	}

	p_MinResults = _node_soap_get(p_node, "MinResults");
	if (p_MinResults && p_MinResults->data)
	{
		p_req->MinResultsFlag = 1;
		p_req->MinResults = atoi(p_MinResults->data);
	}

	p_MaxResults = _node_soap_get(p_node, "MaxResults");
	if (p_MaxResults && p_MaxResults->data)
	{
		p_req->MaxResultsFlag = 1;
		p_req->MaxResults = atoi(p_MaxResults->data);
	}

	p_WaitTime = _node_soap_get(p_node, "WaitTime");
	if (p_WaitTime && p_WaitTime->data)
	{
		p_req->WaitTimeFlag = 1;
		parse_XSDDuration(p_WaitTime->data, &p_req->WaitTime);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_FindMetadata(XMLN * p_node, tse_FindMetadata_REQ * p_req)
{
    XMLN * p_StartPoint;
	XMLN * p_EndPoint;
	XMLN * p_Scope;
	XMLN * p_MetadataFilter;
	XMLN * p_MaxMatches;
	XMLN * p_KeepAliveTime;

	p_StartPoint = _node_soap_get(p_node, "StartPoint");
	if (p_StartPoint && p_StartPoint->data)
	{
		parse_XSDDatetime(p_StartPoint->data, &p_req->StartPoint);
	}

	p_EndPoint = _node_soap_get(p_node, "EndPoint");
	if (p_EndPoint && p_EndPoint->data)
	{
		p_req->EndPointFlag = 1;
		parse_XSDDatetime(p_EndPoint->data, &p_req->EndPoint);
	}

	p_Scope = _node_soap_get(p_node, "Scope");
	if (p_Scope)
	{
		parse_SearchScope(p_Scope, &p_req->Scope);
	}

	p_MetadataFilter = _node_soap_get(p_node, "MetadataFilter");
	if (p_MetadataFilter)
	{
	    XMLN * p_MetadataStreamFilter;

	    p_MetadataStreamFilter = _node_soap_get(p_MetadataFilter, "MetadataStreamFilter");
	    if (p_MetadataStreamFilter && p_MetadataStreamFilter->data)
	    {
	        strncpy(p_req->MetadataFilter.MetadataStreamFilter, p_MetadataStreamFilter->data,
	            sizeof(p_req->MetadataFilter.MetadataStreamFilter)-1);
	    }
	}

	p_MaxMatches = _node_soap_get(p_node, "MaxMatches");
	if (p_MaxMatches && p_MaxMatches->data)
	{
		p_req->MaxMatchesFlag = 1;
		p_req->MaxMatches = atoi(p_MaxMatches->data);
	}

	p_KeepAliveTime = _node_soap_get(p_node, "KeepAliveTime");
	if (p_KeepAliveTime && p_KeepAliveTime->data)
	{
		p_req->KeepAliveTimeFlag = 1;
		parse_XSDDuration(p_KeepAliveTime->data, &p_req->KeepAliveTime);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_GetMetadataSearchResults(XMLN * p_node, tse_GetMetadataSearchResults_REQ * p_req)
{
    XMLN * p_SearchToken;
	XMLN * p_MinResults;
	XMLN * p_MaxResults;
	XMLN * p_WaitTime;

	p_SearchToken = _node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_req->SearchToken, p_SearchToken->data, sizeof(p_req->SearchToken)-1);
	}

	p_MinResults = _node_soap_get(p_node, "MinResults");
	if (p_MinResults && p_MinResults->data)
	{
		p_req->MinResultsFlag = 1;
		p_req->MinResults = atoi(p_MinResults->data);
	}

	p_MaxResults = _node_soap_get(p_node, "MaxResults");
	if (p_MaxResults && p_MaxResults->data)
	{
		p_req->MaxResultsFlag = 1;
		p_req->MaxResults = atoi(p_MaxResults->data);
	}

	p_WaitTime = _node_soap_get(p_node, "WaitTime");
	if (p_WaitTime && p_WaitTime->data)
	{
		p_req->WaitTimeFlag = 1;
		parse_XSDDuration(p_WaitTime->data, &p_req->WaitTime);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_EndSearch(XMLN * p_node, tse_EndSearch_REQ * p_req)
{
	XMLN * p_SearchToken;

	p_SearchToken = _node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_req->SearchToken, p_SearchToken->data, sizeof(p_req->SearchToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tse_GetSearchState(XMLN * p_node, tse_GetSearchState_REQ * p_req)
{
	XMLN * p_SearchToken;

	p_SearchToken = _node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_req->SearchToken, p_SearchToken->data, sizeof(p_req->SearchToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_trp_GetReplayUri(XMLN * p_node, trp_GetReplayUri_REQ * p_req)
{
	ONVIF_RET ret = ONVIF_OK;
	XMLN * p_StreamSetup;
	XMLN * p_RecordingToken;

	p_StreamSetup = _node_soap_get(p_node, "StreamSetup");
	if (p_StreamSetup)
	{
		ret = parse_StreamSetup(p_StreamSetup, &p_req->StreamSetup);
	}

	p_RecordingToken = _node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	return ret;
}

ONVIF_RET parse_trp_SetReplayConfiguration(XMLN * p_node, trp_SetReplayConfiguration_REQ * p_req)
{
	XMLN * p_Configuration;

	p_Configuration = _node_soap_get(p_node, "Configuration");
	if (p_Configuration)
	{
		XMLN * p_SessionTimeout;

		p_SessionTimeout = _node_soap_get(p_Configuration, "SessionTimeout");
		if (p_SessionTimeout && p_SessionTimeout->data)
		{
			parse_XSDDuration(p_SessionTimeout->data, &p_req->SessionTimeout);
		}
	}

	return ONVIF_OK;
}

#endif	// end of PROFILE_G_SUPPORT

#ifdef VIDEO_ANALYTICS

ONVIF_RET parse_SimpleItem(XMLN * p_node, onvif_SimpleItem * p_req)
{
	const char * p_Name;
	const char * p_Value;

	p_Name = _attr_get(p_node, "Name");
	if (p_Name)
	{
		strncpy(p_req->Name, p_Name, sizeof(p_req->Name));
	}

	p_Value = _attr_get(p_node, "Value");
	if (p_Value)
	{
		strncpy(p_req->Value, p_Value, sizeof(p_req->Value));
	}

	return ONVIF_OK;
}

ONVIF_RET parse_ElementItem(XMLN * p_node, onvif_ElementItem * p_req)
{
	const char * p_Name;

	p_Name = _attr_get(p_node, "Name");
	if (p_Name)
	{
		strncpy(p_req->Name, p_Name, sizeof(p_req->Name));

		if (p_node->f_child)
		{
		    int len = _calc_buf_len(p_node->f_child);

		    p_req->AnyFlag = 1;
		    p_req->Any = (char *) malloc(len+8);

		    memset(p_req->Any, 0, len+8);

		    _nwrite_buf(p_node->f_child, p_req->Any, len);
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_Config(XMLN * p_node, onvif_Config * p_req)
{
	XMLN * p_Parameters;
	const char * p_Name;
	const char * p_Type;
	ONVIF_RET ret;

	p_Name = _attr_get(p_node, "Name");
	if (p_Name)
	{
		strncpy(p_req->Name, p_Name, sizeof(p_req->Name));
	}

	p_Type = _attr_get(p_node, "Type");
	if (p_Type)
	{
		strncpy(p_req->Type, p_Type, sizeof(p_req->Type));

		if (p_node->l_attrib)
		{
		    p_req->attrFlag = 1;
			snprintf(p_req->attr, sizeof(p_req->attr)-1, "%s=\"%s\"", p_node->l_attrib->name, p_node->l_attrib->data);
		}
	}

	p_Parameters = _node_soap_get(p_node, "Parameters");
	if (p_Parameters)
	{
		XMLN * p_SimpleItem;
		XMLN * p_ElementItem;

		p_SimpleItem = _node_soap_get(p_Parameters, "SimpleItem");
		while (p_SimpleItem && soap_strcmp(p_SimpleItem->name, "SimpleItem") == 0)
		{
			SimpleItemList * p_simple_item = onvif_add_SimpleItem(&p_req->Parameters.SimpleItem);
			if (p_simple_item)
			{
				ret = parse_SimpleItem(p_SimpleItem, &p_simple_item->SimpleItem);
				if (ONVIF_OK != ret)
				{
					onvif_free_SimpleItems(&p_req->Parameters.SimpleItem);
					break;
				}
			}

			p_SimpleItem = p_SimpleItem->next;
		}

		p_ElementItem = _node_soap_get(p_Parameters, "ElementItem");
		while (p_ElementItem && soap_strcmp(p_ElementItem->name, "ElementItem") == 0)
		{
			ElementItemList * p_element_item = onvif_add_ElementItem(&p_req->Parameters.ElementItem);
			if (p_element_item)
			{
				ret = parse_ElementItem(p_ElementItem, &p_element_item->ElementItem);
				if (ONVIF_OK != ret)
				{
					onvif_free_ElementItems(&p_req->Parameters.ElementItem);
					break;
				}
			}

			p_ElementItem = p_ElementItem->next;
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_GetSupportedRules(XMLN * p_node, tan_GetSupportedRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_CreateRules(XMLN * p_node, tan_CreateRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_Rule;
	ONVIF_RET ret;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_Rule = _node_soap_get(p_node, "Rule");
	while (p_Rule && soap_strcmp(p_Rule->name, "Rule") == 0)
	{
		ConfigList * p_config = onvif_add_Config(&p_req->Rule);
		if (p_config)
		{
			ret = parse_Config(p_Rule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_Configs(&p_req->Rule);
				return ret;
			}
		}

		p_Rule = p_Rule->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_DeleteRules(XMLN * p_node, tan_DeleteRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_RuleName;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_RuleName = _node_soap_get(p_node, "RuleName");
	while (p_RuleName && p_RuleName->data && soap_strcmp(p_RuleName->name, "RuleName") == 0)
	{
		int idx = p_req->sizeRuleName;

		strncpy(p_req->RuleName[idx], p_RuleName->data, sizeof(p_req->RuleName[idx])-1);

		p_req->sizeRuleName++;
		if (p_req->sizeRuleName >= ARRAY_SIZE(p_req->RuleName))
		{
			break;
		}

		p_RuleName = p_RuleName->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_GetRules(XMLN * p_node, tan_GetRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_ModifyRules(XMLN * p_node, tan_ModifyRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_Rule;
	ONVIF_RET ret;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_Rule = _node_soap_get(p_node, "Rule");
	while (p_Rule && soap_strcmp(p_Rule->name, "Rule") == 0)
	{
		ConfigList * p_config = onvif_add_Config(&p_req->Rule);
		if (p_config)
		{
			ret = parse_Config(p_Rule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_Configs(&p_req->Rule);
				return ret;
			}
		}

		p_Rule = p_Rule->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_CreateAnalyticsModules(XMLN * p_node, tan_CreateAnalyticsModules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_AnalyticsModule;
	ONVIF_RET ret;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_AnalyticsModule = _node_soap_get(p_node, "AnalyticsModule");
	while (p_AnalyticsModule && soap_strcmp(p_AnalyticsModule->name, "AnalyticsModule") == 0)
	{
		ConfigList * p_config = onvif_add_Config(&p_req->AnalyticsModule);
		if (p_config)
		{
			ret = parse_Config(p_AnalyticsModule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_Configs(&p_req->AnalyticsModule);
				return ret;
			}
		}

		p_AnalyticsModule = p_AnalyticsModule->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_DeleteAnalyticsModules(XMLN * p_node, tan_DeleteAnalyticsModules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_AnalyticsModuleName;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_AnalyticsModuleName = _node_soap_get(p_node, "AnalyticsModuleName");
	while (p_AnalyticsModuleName && p_AnalyticsModuleName->data && soap_strcmp(p_AnalyticsModuleName->name, "AnalyticsModuleName") == 0)
	{
		int idx = p_req->sizeAnalyticsModuleName;

		strncpy(p_req->AnalyticsModuleName[idx], p_AnalyticsModuleName->data, sizeof(p_req->AnalyticsModuleName[idx])-1);

		p_req->sizeAnalyticsModuleName++;
		if (p_req->sizeAnalyticsModuleName >= ARRAY_SIZE(p_req->AnalyticsModuleName))
		{
			break;
		}

		p_AnalyticsModuleName = p_AnalyticsModuleName->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_GetAnalyticsModules(XMLN * p_node, tan_GetAnalyticsModules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_ModifyAnalyticsModules(XMLN * p_node, tan_ModifyAnalyticsModules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_AnalyticsModule;
	ONVIF_RET ret;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_AnalyticsModule = _node_soap_get(p_node, "AnalyticsModule");
	while (p_AnalyticsModule && soap_strcmp(p_AnalyticsModule->name, "AnalyticsModule") == 0)
	{
		ConfigList * p_config = onvif_add_Config(&p_req->AnalyticsModule);
		if (p_config)
		{
			ret = parse_Config(p_AnalyticsModule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_Configs(&p_req->AnalyticsModule);
				return ret;
			}
		}

		p_AnalyticsModule = p_AnalyticsModule->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_GetRuleOptions(XMLN * p_node, tan_GetRuleOptions_REQ * p_req)
{
    XMLN * p_RuleType;
	XMLN * p_ConfigurationToken;

	p_RuleType = _node_soap_get(p_node, "RuleType");
	if (p_RuleType && p_RuleType->data)
	{
		strncpy(p_req->RuleType, p_RuleType->data, sizeof(p_req->RuleType));
	}

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken));
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_GetSupportedAnalyticsModules(XMLN * p_node, tan_GetSupportedAnalyticsModules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken));
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tan_GetAnalyticsModuleOptions(XMLN * p_node, tan_GetAnalyticsModuleOptions_REQ * p_req)
{
    XMLN * p_Type;
	XMLN * p_ConfigurationToken;

	p_Type = _node_soap_get(p_node, "Type");
	if (p_Type && p_Type->data)
	{
		strncpy(p_req->Type, p_Type->data, sizeof(p_req->Type));
	}

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken));
	}

	return ONVIF_OK;
}

#endif	// end of VIDEO_ANALYTICS

#ifdef PROFILE_C_SUPPORT

ONVIF_RET parse_DoorCapabilities(XMLN * p_node, onvif_DoorCapabilities * p_req)
{
    const char * p_Access;
    const char * p_AccessTimingOverride;
    const char * p_Lock;
    const char * p_Unlock;
    const char * p_Block;
    const char * p_DoubleLock;
    const char * p_LockDown;
    const char * p_LockOpen;
    const char * p_DoorMonitor;
    const char * p_LockMonitor;
    const char * p_DoubleLockMonitor;
    const char * p_Alarm;
    const char * p_Tamper;
    const char * p_Fault;

    p_Access = _attr_get(p_node, "Access");
    if (p_Access)
    {
        p_req->Access = parse_Bool(p_Access);
    }

    p_AccessTimingOverride = _attr_get(p_node, "AccessTimingOverride");
    if (p_AccessTimingOverride)
    {
        p_req->AccessTimingOverride = parse_Bool(p_AccessTimingOverride);
    }

    p_Lock = _attr_get(p_node, "Lock");
    if (p_Lock)
    {
        p_req->Lock = parse_Bool(p_Lock);
    }

    p_Unlock = _attr_get(p_node, "Unlock");
    if (p_Unlock)
    {
        p_req->Unlock = parse_Bool(p_Unlock);
    }

    p_Block = _attr_get(p_node, "Block");
    if (p_Block)
    {
        p_req->Block = parse_Bool(p_Block);
    }

    p_DoubleLock = _attr_get(p_node, "DoubleLock");
    if (p_DoubleLock)
    {
        p_req->DoubleLock = parse_Bool(p_DoubleLock);
    }

    p_LockDown = _attr_get(p_node, "LockDown");
    if (p_LockDown)
    {
        p_req->LockDown = parse_Bool(p_LockDown);
    }

    p_LockOpen = _attr_get(p_node, "LockOpen");
    if (p_LockOpen)
    {
        p_req->LockOpen = parse_Bool(p_LockOpen);
    }

    p_DoorMonitor = _attr_get(p_node, "DoorMonitor");
    if (p_DoorMonitor)
    {
        p_req->DoorMonitor = parse_Bool(p_DoorMonitor);
    }

    p_LockMonitor = _attr_get(p_node, "LockMonitor");
    if (p_LockMonitor)
    {
        p_req->LockMonitor = parse_Bool(p_LockMonitor);
    }

    p_DoubleLockMonitor = _attr_get(p_node, "DoubleLockMonitor");
    if (p_DoubleLockMonitor)
    {
        p_req->DoubleLockMonitor = parse_Bool(p_DoubleLockMonitor);
    }

    p_Alarm = _attr_get(p_node, "Alarm");
    if (p_Alarm)
    {
        p_req->Alarm = parse_Bool(p_Alarm);
    }

    p_Tamper = _attr_get(p_node, "Tamper");
    if (p_Tamper)
    {
        p_req->Tamper = parse_Bool(p_Tamper);
    }

    p_Fault = _attr_get(p_node, "Fault");
    if (p_Fault)
    {
        p_req->Fault = parse_Bool(p_Fault);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_AccessPointCapabilities(XMLN * p_node, onvif_AccessPointCapabilities * p_req)
{
    const char * p_DisableAccessPoint;
    const char * p_Duress;
    const char * p_AnonymousAccess;
    const char * p_AccessTaken;
    const char * p_ExternalAuthorization;
    const char * p_SupportedRecognitionTypes;
    const char * p_IdentiferAccess;
    const char * p_SupportedFeedbackTypes;

    p_DisableAccessPoint = _attr_get(p_node, "DisableAccessPoint");
    if (p_DisableAccessPoint)
    {
        p_req->DisableAccessPoint = parse_Bool(p_DisableAccessPoint);
    }

    p_Duress = _attr_get(p_node, "Duress");
    if (p_Duress)
    {
        p_req->Duress = parse_Bool(p_Duress);
    }

    p_AnonymousAccess = _attr_get(p_node, "AnonymousAccess");
    if (p_AnonymousAccess)
    {
        p_req->AnonymousAccess = parse_Bool(p_AnonymousAccess);
    }

    p_AccessTaken = _attr_get(p_node, "AccessTaken");
    if (p_AccessTaken)
    {
        p_req->AccessTaken = parse_Bool(p_AccessTaken);
    }

    p_ExternalAuthorization = _attr_get(p_node, "ExternalAuthorization");
    if (p_ExternalAuthorization)
    {
        p_req->ExternalAuthorization = parse_Bool(p_ExternalAuthorization);
    }

    p_SupportedRecognitionTypes = _attr_get(p_node, "SupportedRecognitionTypes");
    if (p_SupportedRecognitionTypes)
    {
        p_req->SupportedRecognitionTypesFlag = 1;
        strncpy(p_req->SupportedRecognitionTypes, p_ExternalAuthorization, sizeof(p_req->SupportedRecognitionTypes)-1);
    }

    p_IdentiferAccess = _attr_get(p_node, "IdentiferAccess");
    if (p_IdentiferAccess)
    {
        p_req->IdentiferAccess = parse_Bool(p_IdentiferAccess);
    }

    p_SupportedFeedbackTypes = _attr_get(p_node, "SupportedFeedbackTypes");
    if (p_SupportedFeedbackTypes)
    {
        p_req->SupportedFeedbackTypesFlag = 1;
        strncpy(p_req->SupportedFeedbackTypes, p_SupportedFeedbackTypes, sizeof(p_req->SupportedFeedbackTypes)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_AccessPointInfo(XMLN * p_node, onvif_AccessPointInfo * p_req)
{
    const char * p_token;
    XMLN * p_Name;
    XMLN * p_Description;
    XMLN * p_AreaFrom;
    XMLN * p_AreaTo;
    XMLN * p_EntityType;
    XMLN * p_Entity;
    XMLN * p_Capabilities;

    p_token = _attr_get(p_node, "token");
    if (p_token)
    {
        strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
    }

    p_Name = _node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name) - 1);
	}

	p_Description = _node_soap_get(p_node, "Description");
	if (p_Description && p_Description->data)
	{
		p_req->DescriptionFlag = 1;
		strncpy(p_req->Description, p_Description->data, sizeof(p_req->Description) - 1);
	}

	p_AreaFrom = _node_soap_get(p_node, "AreaFrom");
	if (p_AreaFrom && p_AreaFrom->data)
	{
	    p_req->AreaFromFlag = 1;
		strncpy(p_req->AreaFrom, p_AreaFrom->data, sizeof(p_req->AreaFrom) - 1);
	}

	p_AreaTo = _node_soap_get(p_node, "AreaTo");
	if (p_AreaTo && p_AreaTo->data)
	{
	    p_req->AreaToFlag = 1;
		strncpy(p_req->AreaTo, p_AreaTo->data, sizeof(p_req->AreaTo) - 1);
	}

	p_EntityType = _node_soap_get(p_node, "EntityType");
	if (p_EntityType && p_EntityType->data)
	{
	    p_req->EntityTypeFlag = 1;
		strncpy(p_req->EntityType, p_EntityType->data, sizeof(p_req->EntityType) - 1);

		if (p_EntityType->l_attrib && p_EntityType->l_attrib->data)
		{
		    p_req->EntityTypeAttrFlag = 1;
		    snprintf(p_req->EntityTypeAttr, sizeof(p_req->EntityTypeAttr) - 1,
		        "%s=\"%s\"", p_EntityType->l_attrib->name, p_EntityType->l_attrib->data);
		}
	}

	p_Entity = _node_soap_get(p_node, "Entity");
	if (p_Entity && p_Entity->data)
	{
		strncpy(p_req->Entity, p_Entity->data, sizeof(p_req->Entity) - 1);
	}

	p_Capabilities = _node_soap_get(p_node, "Capabilities");
	if (p_Capabilities)
	{
	    parse_AccessPointCapabilities(p_Capabilities, &p_req->Capabilities);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_AreaInfo(XMLN * p_node, onvif_AreaInfo * p_req)
{
    const char * p_token;
    XMLN * p_Name;
    XMLN * p_Description;

    p_token = _attr_get(p_node, "token");
    if (p_token)
    {
        strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
    }

    p_Name = _node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name) - 1);
	}

	p_Description = _node_soap_get(p_node, "Description");
	if (p_Description && p_Description->data)
	{
		p_req->DescriptionFlag = 1;
		strncpy(p_req->Description, p_Description->data, sizeof(p_req->Description) - 1);
	}

    return ONVIF_OK;
}

ONVIF_RET parse_DoorInfo(XMLN * p_node, onvif_DoorInfo * p_req)
{
	XMLN * p_Name;
	XMLN * p_Description;
	XMLN * p_Capabilities;
	const char * p_token;

	p_token = _attr_get(p_node, "token");
	if (p_token)
	{
		strncpy(p_req->token, p_token, sizeof(p_req->token) - 1);
	}

	p_Name = _node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name) - 1);
	}

	p_Description = _node_soap_get(p_node, "Description");
	if (p_Description && p_Description->data)
	{
		p_req->DescriptionFlag = 1;
		strncpy(p_req->Description, p_Description->data, sizeof(p_req->Description) - 1);
	}

	p_Capabilities = _node_soap_get(p_node, "Capabilities");
	if (p_Capabilities)
	{
		const char * p_Access;
		const char * p_AccessTimingOverride;
		const char * p_Lock;
		const char * p_Unlock;
		const char * p_Block;
		const char * p_DoubleLock;
		const char * p_LockDown;
		const char * p_LockOpen;
		const char * p_DoorMonitor;
		const char * p_LockMonitor;
		const char * p_DoubleLockMonitor;
		const char * p_Alarm;
		const char * p_Tamper;
		const char * p_Fault;

		p_Access = _attr_get(p_Capabilities, "Access");
		if (p_Access)
		{
			p_req->Capabilities.Access = parse_Bool(p_Access);
		}

		p_AccessTimingOverride = _attr_get(p_Capabilities, "AccessTimingOverride");
		if (p_AccessTimingOverride)
		{
			p_req->Capabilities.AccessTimingOverride = parse_Bool(p_AccessTimingOverride);
		}

		p_Lock = _attr_get(p_Capabilities, "Lock");
		if (p_Lock)
		{
			p_req->Capabilities.Lock = parse_Bool(p_Lock);
		}

		p_Unlock = _attr_get(p_Capabilities, "Unlock");
		if (p_Unlock)
		{
			p_req->Capabilities.Unlock = parse_Bool(p_Unlock);
		}

		p_Block = _attr_get(p_Capabilities, "Block");
		if (p_Block)
		{
			p_req->Capabilities.Block = parse_Bool(p_Block);
		}

		p_DoubleLock = _attr_get(p_Capabilities, "DoubleLock");
		if (p_DoubleLock)
		{
			p_req->Capabilities.DoubleLock = parse_Bool(p_DoubleLock);
		}

		p_LockDown = _attr_get(p_Capabilities, "LockDown");
		if (p_LockDown)
		{
			p_req->Capabilities.LockDown = parse_Bool(p_LockDown);
		}

		p_LockOpen = _attr_get(p_Capabilities, "LockOpen");
		if (p_LockOpen)
		{
			p_req->Capabilities.LockOpen = parse_Bool(p_LockOpen);
		}

		p_DoorMonitor = _attr_get(p_Capabilities, "DoorMonitor");
		if (p_DoorMonitor)
		{
			p_req->Capabilities.DoorMonitor = parse_Bool(p_DoorMonitor);
		}

		p_LockMonitor = _attr_get(p_Capabilities, "LockMonitor");
		if (p_LockMonitor)
		{
			p_req->Capabilities.LockMonitor = parse_Bool(p_LockMonitor);
		}

		p_DoubleLockMonitor = _attr_get(p_Capabilities, "DoubleLockMonitor");
		if (p_DoubleLockMonitor)
		{
			p_req->Capabilities.DoubleLockMonitor = parse_Bool(p_DoubleLockMonitor);
		}

		p_Alarm = _attr_get(p_Capabilities, "Alarm");
		if (p_Alarm)
		{
			p_req->Capabilities.Alarm = parse_Bool(p_Alarm);
		}

		p_Tamper = _attr_get(p_Capabilities, "Tamper");
		if (p_Tamper)
		{
			p_req->Capabilities.Tamper = parse_Bool(p_Tamper);
		}

		p_Fault = _attr_get(p_Capabilities, "Fault");
		if (p_Fault)
		{
			p_req->Capabilities.Fault = parse_Bool(p_Fault);
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_Timings(XMLN * p_node, onvif_Timings * p_req)
{
    XMLN * p_ReleaseTime;
    XMLN * p_OpenTime;
    XMLN * p_ExtendedReleaseTime;
    XMLN * p_DelayTimeBeforeRelock;
    XMLN * p_ExtendedOpenTime;
    XMLN * p_PreAlarmTime;

    p_ReleaseTime = _node_soap_get(p_node, "ReleaseTime");
    if (p_ReleaseTime && p_ReleaseTime->data)
    {
        parse_XSDDuration(p_ReleaseTime->data, (int*)&p_req->ReleaseTime);
    }

    p_OpenTime = _node_soap_get(p_node, "OpenTime");
    if (p_OpenTime && p_OpenTime->data)
    {
        parse_XSDDuration(p_OpenTime->data, (int *)&p_req->OpenTime);
    }

    p_ExtendedReleaseTime = _node_soap_get(p_node, "ExtendedReleaseTime");
    if (p_ExtendedReleaseTime && p_ExtendedReleaseTime->data)
    {
        p_req->ExtendedReleaseTimeFlag = 1;
        parse_XSDDuration(p_ExtendedReleaseTime->data, (int *)&p_req->ExtendedReleaseTime);
    }

    p_DelayTimeBeforeRelock = _node_soap_get(p_node, "DelayTimeBeforeRelock");
    if (p_DelayTimeBeforeRelock && p_DelayTimeBeforeRelock->data)
    {
        p_req->DelayTimeBeforeRelockFlag = 1;
        parse_XSDDuration(p_DelayTimeBeforeRelock->data, (int *)&p_req->DelayTimeBeforeRelock);
    }

    p_ExtendedOpenTime = _node_soap_get(p_node, "ExtendedOpenTime");
    if (p_ExtendedOpenTime && p_ExtendedOpenTime->data)
    {
        p_req->ExtendedOpenTimeFlag = 1;
        parse_XSDDuration(p_ExtendedOpenTime->data, (int *)&p_req->ExtendedOpenTime);
    }

    p_PreAlarmTime = _node_soap_get(p_node, "PreAlarmTime");
    if (p_PreAlarmTime && p_PreAlarmTime->data)
    {
        p_req->PreAlarmTimeFlag = 1;
        parse_XSDDuration(p_PreAlarmTime->data, (int *)&p_req->PreAlarmTime);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_Door(XMLN * p_node, onvif_Door * p_req)
{
    XMLN * p_DoorType;
    XMLN * p_Timings;

    parse_DoorInfo(p_node, &p_req->DoorInfo);

    p_DoorType = _node_soap_get(p_node, "DoorType");
    if (p_DoorType && p_DoorType->data)
    {
        strncpy(p_req->DoorType, p_DoorType->data, sizeof(p_req->DoorType)-1);
    }

    p_Timings = _node_soap_get(p_node, "Timings");
    if (p_Timings)
    {
        parse_Timings(p_Timings, &p_req->Timings);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_GetAccessPointList(XMLN * p_node, tac_GetAccessPointList_REQ * p_req)
{
    XMLN * p_Limit;
	XMLN * p_StartReference;

	p_Limit = _node_soap_get(p_node, "Limit");
	if (p_Limit && p_Limit->data)
	{
		p_req->LimitFlag = 1;
		p_req->Limit = atoi(p_Limit->data);
	}

	p_StartReference = _node_soap_get(p_node, "StartReference");
	if (p_StartReference && p_StartReference->data)
	{
		p_req->StartReferenceFlag = 1;
		strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tac_GetAccessPoints(XMLN * p_node, tac_GetAccessPoints_REQ * p_req)
{
    int idx = 0;
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && p_Token->data && soap_strcmp(p_Token->name, "Token") == 0)
    {
        strncpy(p_req->token[idx], p_Token->data, sizeof(p_req->token[idx])-1);

        if (++idx >= ARRAY_SIZE(p_req->token))
        {
            return ONVIF_ERR_TooManyItems;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_CreateAccessPoint(XMLN * p_node, tac_CreateAccessPoint_REQ * p_req)
{
    XMLN * p_AccessPoint;
    XMLN * p_AuthenticationProfileToken;

    p_AccessPoint = _node_soap_get(p_node, "AccessPoint");
    if (p_AccessPoint)
    {
        parse_AccessPointInfo(p_AccessPoint, &p_req->AccessPoint);

        p_AuthenticationProfileToken = _node_soap_get(p_node, "AuthenticationProfileToken");
    	if (p_AuthenticationProfileToken && p_AuthenticationProfileToken->data)
    	{
    		strncpy(p_req->AuthenticationProfileToken, p_AuthenticationProfileToken->data, sizeof(p_req->AuthenticationProfileToken)-1);
    	}
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_SetAccessPoint(XMLN * p_node, tac_SetAccessPoint_REQ * p_req)
{
    XMLN * p_AccessPoint;
    XMLN * p_AuthenticationProfileToken;

    p_AccessPoint = _node_soap_get(p_node, "AccessPoint");
    if (p_AccessPoint)
    {
        parse_AccessPointInfo(p_AccessPoint, &p_req->AccessPoint);

        p_AuthenticationProfileToken = _node_soap_get(p_node, "AuthenticationProfileToken");
    	if (p_AuthenticationProfileToken && p_AuthenticationProfileToken->data)
    	{
    		strncpy(p_req->AuthenticationProfileToken, p_AuthenticationProfileToken->data, sizeof(p_req->AuthenticationProfileToken)-1);
    	}
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_ModifyAccessPoint(XMLN * p_node, tac_ModifyAccessPoint_REQ * p_req)
{
    XMLN * p_AccessPoint;
    XMLN * p_AuthenticationProfileToken;

    p_AccessPoint = _node_soap_get(p_node, "AccessPoint");
    if (p_AccessPoint)
    {
        parse_AccessPointInfo(p_AccessPoint, &p_req->AccessPoint);

        p_AuthenticationProfileToken = _node_soap_get(p_node, "AuthenticationProfileToken");
    	if (p_AuthenticationProfileToken && p_AuthenticationProfileToken->data)
    	{
    		strncpy(p_req->AuthenticationProfileToken, p_AuthenticationProfileToken->data, sizeof(p_req->AuthenticationProfileToken)-1);
    	}
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_DeleteAccessPoint(XMLN * p_node, tac_DeleteAccessPoint_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_GetAccessPointInfoList(XMLN * p_node, tac_GetAccessPointInfoList_REQ * p_req)
{
    XMLN * p_Limit;
	XMLN * p_StartReference;

	p_Limit = _node_soap_get(p_node, "Limit");
	if (p_Limit && p_Limit->data)
	{
		p_req->LimitFlag = 1;
		p_req->Limit = atoi(p_Limit->data);
	}

	p_StartReference = _node_soap_get(p_node, "StartReference");
	if (p_StartReference && p_StartReference->data)
	{
		p_req->StartReferenceFlag = 1;
		strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tac_GetAccessPointInfo(XMLN * p_node, tac_GetAccessPointInfo_REQ * p_req)
{
    int idx = 0;
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && p_Token->data && soap_strcmp(p_Token->name, "Token") == 0)
    {
        strncpy(p_req->token[idx], p_Token->data, sizeof(p_req->token[idx])-1);

        if (++idx >= ARRAY_SIZE(p_req->token))
        {
            return ONVIF_ERR_TooManyItems;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_GetAreaList(XMLN * p_node, tac_GetAreaList_REQ * p_req)
{
    XMLN * p_Limit;
	XMLN * p_StartReference;

	p_Limit = _node_soap_get(p_node, "Limit");
	if (p_Limit && p_Limit->data)
	{
		p_req->LimitFlag = 1;
		p_req->Limit = atoi(p_Limit->data);
	}

	p_StartReference = _node_soap_get(p_node, "StartReference");
	if (p_StartReference && p_StartReference->data)
	{
		p_req->StartReferenceFlag = 1;
		strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tac_GetAreas(XMLN * p_node, tac_GetAreas_REQ * p_req)
{
    int idx = 0;
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && p_Token->data && soap_strcmp(p_Token->name, "Token") == 0)
    {
        strncpy(p_req->token[idx], p_Token->data, sizeof(p_req->token[idx])-1);

        if (++idx >= ARRAY_SIZE(p_req->token))
        {
            return ONVIF_ERR_TooManyItems;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_CreateArea(XMLN * p_node, tac_CreateArea_REQ * p_req)
{
    XMLN * p_Area;

    p_Area = _node_soap_get(p_node, "Area");
    if (p_Area)
    {
        parse_AreaInfo(p_Area, &p_req->Area);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_SetArea(XMLN * p_node, tac_SetArea_REQ * p_req)
{
    XMLN * p_Area;

    p_Area = _node_soap_get(p_node, "Area");
    if (p_Area)
    {
        parse_AreaInfo(p_Area, &p_req->Area);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_ModifyArea(XMLN * p_node, tac_ModifyArea_REQ * p_req)
{
    XMLN * p_Area;

    p_Area = _node_soap_get(p_node, "Area");
    if (p_Area)
    {
        parse_AreaInfo(p_Area, &p_req->Area);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_DeleteArea(XMLN * p_node, tac_DeleteArea_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_GetAreaInfoList(XMLN * p_node, tac_GetAreaInfoList_REQ * p_req)
{
    XMLN * p_Limit;
	XMLN * p_StartReference;

	p_Limit = _node_soap_get(p_node, "Limit");
	if (p_Limit && p_Limit->data)
	{
		p_req->LimitFlag = 1;
		p_req->Limit = atoi(p_Limit->data);
	}

	p_StartReference = _node_soap_get(p_node, "StartReference");
	if (p_StartReference && p_StartReference->data)
	{
		p_req->StartReferenceFlag = 1;
		strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tac_GetAreaInfo(XMLN * p_node, tac_GetAreaInfo_REQ * p_req)
{
    int idx = 0;
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && p_Token->data && soap_strcmp(p_Token->name, "Token") == 0)
    {
        strncpy(p_req->token[idx], p_Token->data, sizeof(p_req->token[idx])-1);

        if (++idx >= ARRAY_SIZE(p_req->token))
        {
            return ONVIF_ERR_TooManyItems;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_GetAccessPointState(XMLN * p_node, tac_GetAccessPointState_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_EnableAccessPoint(XMLN * p_node, tac_EnableAccessPoint_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tac_DisableAccessPoint(XMLN * p_node, tac_DisableAccessPoint_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_GetDoorList(XMLN * p_node, tdc_GetDoorList_REQ * p_req)
{
    XMLN * p_Limit;
	XMLN * p_StartReference;

	p_Limit = _node_soap_get(p_node, "Limit");
	if (p_Limit && p_Limit->data)
	{
		p_req->LimitFlag = 1;
		p_req->Limit = atoi(p_Limit->data);
	}

	p_StartReference = _node_soap_get(p_node, "StartReference");
	if (p_StartReference && p_StartReference->data)
	{
		p_req->StartReferenceFlag = 1;
		strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tdc_GetDoors(XMLN * p_node, tdc_GetDoors_REQ * p_req)
{
    int idx = 0;
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && p_Token->data && soap_strcmp(p_Token->name, "Token") == 0)
    {
        strncpy(p_req->token[idx], p_Token->data, sizeof(p_req->token[idx])-1);

        if (++idx >= ARRAY_SIZE(p_req->token))
        {
            return ONVIF_ERR_TooManyItems;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_CreateDoor(XMLN * p_node, tdc_CreateDoor_REQ * p_req)
{
    XMLN * p_Door;

    p_Door = _node_soap_get(p_node, "Door");
    if (p_Door)
    {
        parse_Door(p_Door, &p_req->Door);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_SetDoor(XMLN * p_node, tdc_SetDoor_REQ * p_req)
{
    XMLN * p_Door;

    p_Door = _node_soap_get(p_node, "Door");
    if (p_Door)
    {
        parse_Door(p_Door, &p_req->Door);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_ModifyDoor(XMLN * p_node, tdc_ModifyDoor_REQ * p_req)
{
    XMLN * p_Door;

    p_Door = _node_soap_get(p_node, "Door");
    if (p_Door)
    {
        parse_Door(p_Door, &p_req->Door);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_DeleteDoor(XMLN * p_node, tdc_DeleteDoor_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_GetDoorInfoList(XMLN * p_node, tdc_GetDoorInfoList_REQ * p_req)
{
    XMLN * p_Limit;
	XMLN * p_StartReference;

	p_Limit = _node_soap_get(p_node, "Limit");
	if (p_Limit && p_Limit->data)
	{
		p_req->LimitFlag = 1;
		p_req->Limit = atoi(p_Limit->data);
	}

	p_StartReference = _node_soap_get(p_node, "StartReference");
	if (p_StartReference && p_StartReference->data)
	{
		p_req->StartReferenceFlag = 1;
		strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tdc_GetDoorInfo(XMLN * p_node, tdc_GetDoorInfo_REQ * p_req)
{
    int idx = 0;
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && p_Token->data && soap_strcmp(p_Token->name, "Token") == 0)
    {
        strncpy(p_req->token[idx], p_Token->data, sizeof(p_req->token[idx])-1);

        if (++idx >= ARRAY_SIZE(p_req->token))
        {
            return ONVIF_ERR_TooManyItems;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_GetDoorState(XMLN * p_node, tdc_GetDoorState_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_AccessDoor(XMLN * p_node, tdc_AccessDoor_REQ * p_req)
{
    XMLN * p_Token;
    XMLN * p_UseExtendedTime;
    XMLN * p_AccessTime;
    XMLN * p_OpenTooLongTime;
    XMLN * p_PreAlarmTime;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    p_UseExtendedTime = _node_soap_get(p_node, "UseExtendedTime");
    if (p_UseExtendedTime && p_UseExtendedTime->data)
    {
        p_req->UseExtendedTimeFlag = 1;
        p_req->UseExtendedTime = parse_Bool(p_UseExtendedTime->data);
    }

    p_AccessTime = _node_soap_get(p_node, "AccessTime");
    if (p_AccessTime && p_AccessTime->data)
    {
        p_req->AccessTimeFlag = 1;
        parse_XSDDuration(p_AccessTime->data, &p_req->AccessTime);
    }

    p_OpenTooLongTime = _node_soap_get(p_node, "AccessTime");
    if (p_OpenTooLongTime && p_OpenTooLongTime->data)
    {
        p_req->OpenTooLongTimeFlag = 1;
        parse_XSDDuration(p_OpenTooLongTime->data, &p_req->OpenTooLongTime);
    }

    p_PreAlarmTime = _node_soap_get(p_node, "PreAlarmTime");
    if (p_PreAlarmTime && p_PreAlarmTime->data)
    {
        p_req->PreAlarmTimeFlag = 1;
        parse_XSDDuration(p_PreAlarmTime->data, &p_req->PreAlarmTime);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_LockDoor(XMLN * p_node, tdc_LockDoor_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_UnlockDoor(XMLN * p_node, tdc_UnlockDoor_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_DoubleLockDoor(XMLN * p_node, tdc_DoubleLockDoor_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_BlockDoor(XMLN * p_node, tdc_BlockDoor_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_LockDownDoor(XMLN * p_node, tdc_LockDownDoor_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_LockDownReleaseDoor(XMLN * p_node, tdc_LockDownReleaseDoor_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_LockOpenDoor(XMLN * p_node, tdc_LockOpenDoor_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tdc_LockOpenReleaseDoor(XMLN * p_node, tdc_LockOpenReleaseDoor_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

#endif  // end of PROFILE_C_SUPPORT

#ifdef DEVICEIO_SUPPORT

ONVIF_RET parse_RelayOutputSettings(XMLN * p_node, onvif_RelayOutputSettings * p_req)
{
    XMLN * p_Mode;
    XMLN * p_DelayTime;
    XMLN * p_IdleState;

    p_Mode = _node_soap_get(p_node, "Mode");
    if (p_Mode && p_Mode->data)
    {
        p_req->Mode = onvif_StringToRelayMode(p_Mode->data);
    }

    p_DelayTime = _node_soap_get(p_node, "DelayTime");
    if (p_DelayTime && p_DelayTime->data)
    {
        parse_XSDDuration(p_DelayTime->data, (int*)&p_req->DelayTime);
    }

    p_IdleState = _node_soap_get(p_node, "IdleState");
    if (p_IdleState && p_IdleState->data)
    {
        p_req->IdleState = onvif_StringToRelayIdleState(p_IdleState->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_SetRelayOutputState(XMLN * p_node, tmd_SetRelayOutputState_REQ * p_req)
{
    XMLN * p_RelayOutputToken;
    XMLN * p_LogicalState;

    p_RelayOutputToken = _node_soap_get(p_node, "RelayOutputToken");
    if (p_RelayOutputToken && p_RelayOutputToken->data)
    {
        strncpy(p_req->RelayOutputToken, p_RelayOutputToken->data, sizeof(p_req->RelayOutputToken)-1);
    }

    p_LogicalState = _node_soap_get(p_node, "LogicalState");
    if (p_LogicalState && p_LogicalState->data)
    {
        p_req->LogicalState = onvif_StringToRelayLogicalState(p_LogicalState->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_GetVideoOutputConfiguration(XMLN * p_node, tmd_GetVideoOutputConfiguration_REQ * p_req)
{
    XMLN * p_VideoOutputToken;

    p_VideoOutputToken = _node_soap_get(p_node, "VideoOutputToken");
    if (p_VideoOutputToken && p_VideoOutputToken->data)
    {
        strncpy(p_req->VideoOutputToken, p_VideoOutputToken->data, sizeof(p_req->VideoOutputToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_SetVideoOutputConfiguration(XMLN * p_node, tmd_SetVideoOutputConfiguration_REQ * p_req)
{
    XMLN * p_Configuration;
    XMLN * p_ForcePersistence;

    p_Configuration = _node_soap_get(p_node, "Configuration");
    if (NULL == p_Configuration)
    {
        XMLN * p_Name;
        XMLN * p_UseCount;
        XMLN * p_OutputToken;
        const char * p_token;

        p_token = _attr_get(p_Configuration, "token");
        if (p_token)
        {
            strncpy(p_req->Configuration.token, p_token, sizeof(p_req->Configuration.token)-1);
        }

        p_Name = _node_soap_get(p_Configuration, "Name");
        if (p_Name && p_Name->data)
        {
            strncpy(p_req->Configuration.Name, p_Name->data, sizeof(p_req->Configuration.Name)-1);
        }

        p_UseCount = _node_soap_get(p_Configuration, "UseCount");
        if (p_UseCount && p_UseCount->data)
        {
            p_req->Configuration.UseCount = atoi(p_UseCount->data);
        }

        p_OutputToken = _node_soap_get(p_Configuration, "OutputToken");
        if (p_OutputToken && p_OutputToken->data)
        {
            strncpy(p_req->Configuration.OutputToken, p_OutputToken->data, sizeof(p_req->Configuration.OutputToken)-1);
        }
    }

    p_ForcePersistence = _node_soap_get(p_node, "ForcePersistence");
    if (p_ForcePersistence && p_ForcePersistence->data)
    {
        p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_GetVideoOutputConfigurationOptions(XMLN * p_node, tmd_GetVideoOutputConfigurationOptions_REQ * p_req)
{
    XMLN * p_VideoOutputToken;

    p_VideoOutputToken = _node_soap_get(p_node, "VideoOutputToken");
    if (p_VideoOutputToken && p_VideoOutputToken->data)
    {
        strncpy(p_req->VideoOutputToken, p_VideoOutputToken->data, sizeof(p_req->VideoOutputToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_GetAudioOutputConfiguration(XMLN * p_node, tmd_GetAudioOutputConfiguration_REQ * p_req)
{
    XMLN * p_AudioOutputToken;

    p_AudioOutputToken = _node_soap_get(p_node, "AudioOutputToken");
    if (p_AudioOutputToken && p_AudioOutputToken->data)
    {
        strncpy(p_req->AudioOutputToken, p_AudioOutputToken->data, sizeof(p_req->AudioOutputToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_GetAudioOutputConfigurationOptions(XMLN * p_node, tmd_GetAudioOutputConfigurationOptions_REQ * p_req)
{
    XMLN * p_AudioOutputToken;

    p_AudioOutputToken = _node_soap_get(p_node, "AudioOutputToken");
    if (p_AudioOutputToken && p_AudioOutputToken->data)
    {
        strncpy(p_req->AudioOutputToken, p_AudioOutputToken->data, sizeof(p_req->AudioOutputToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_GetRelayOutputOptions(XMLN * p_node, tmd_GetRelayOutputOptions_REQ * p_req)
{
    XMLN * p_RelayOutputToken;

    p_RelayOutputToken = _node_soap_get(p_node, "RelayOutputToken");
    if (p_RelayOutputToken && p_RelayOutputToken->data)
    {
        p_req->RelayOutputTokenFlag = 1;
        strncpy(p_req->RelayOutputToken, p_RelayOutputToken->data, sizeof(p_req->RelayOutputToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_SetRelayOutputSettings(XMLN * p_node, tmd_SetRelayOutputSettings_REQ * p_req)
{
    XMLN * p_RelayOutput;

    p_RelayOutput = _node_soap_get(p_node, "RelayOutput");
    if (p_RelayOutput)
    {
        XMLN * p_Properties;
        const char * p_token;

        p_token = _attr_get(p_RelayOutput, "token");
        if (p_token)
        {
            strncpy(p_req->RelayOutput.token, p_token, sizeof(p_req->RelayOutput.token)-1);
        }

        p_Properties = _node_soap_get(p_RelayOutput, "Properties");
        if (p_Properties)
        {
            parse_RelayOutputSettings(p_Properties, &p_req->RelayOutput.Properties);
        }
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_GetDigitalInputConfigurationOptions(XMLN * p_node, tmd_GetDigitalInputConfigurationOptions_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        p_req->TokenFlag = 1;
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_SetDigitalInputConfigurations(XMLN * p_node, tmd_SetDigitalInputConfigurations_REQ * p_req)
{
    XMLN * p_DigitalInputs;

    p_DigitalInputs = _node_soap_get(p_node, "DigitalInputs");
    while (p_DigitalInputs && soap_strcmp(p_DigitalInputs->name, "DigitalInputs") == 0)
    {
        DigitalInputList * p_input = onvif_add_DigitalInput(&p_req->DigitalInputs);
        if (p_input)
        {
            const char * p_token;
            const char * p_IdleState;

            p_token = _attr_get(p_DigitalInputs, "token");
            if (p_token)
            {
                strncpy(p_input->DigitalInput.token, p_token, sizeof(p_input->DigitalInput.token)-1);
            }

            p_IdleState = _attr_get(p_DigitalInputs, "IdleState");
            if (p_IdleState)
            {
                p_input->DigitalInput.IdleStateFlag = 1;
                p_input->DigitalInput.IdleState = onvif_StringToDigitalIdleState(p_IdleState);
            }
        }

        p_DigitalInputs = p_DigitalInputs->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_GetSerialPortConfiguration(XMLN * p_node, tmd_GetSerialPortConfiguration_REQ * p_req)
{
    XMLN * p_SerialPortToken;

    p_SerialPortToken = _node_soap_get(p_node, "SerialPortToken");
    if (p_SerialPortToken && p_SerialPortToken->data)
    {
        strncpy(p_req->SerialPortToken, p_SerialPortToken->data, sizeof(p_req->SerialPortToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_GetSerialPortConfigurationOptions(XMLN * p_node, tmd_GetSerialPortConfigurationOptions_REQ * p_req)
{
    XMLN * p_SerialPortToken;

    p_SerialPortToken = _node_soap_get(p_node, "SerialPortToken");
    if (p_SerialPortToken && p_SerialPortToken->data)
    {
        strncpy(p_req->SerialPortToken, p_SerialPortToken->data, sizeof(p_req->SerialPortToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_SetSerialPortConfiguration(XMLN * p_node, tmd_SetSerialPortConfiguration_REQ * p_req)
{
    XMLN * p_SerialPortConfiguration;
    XMLN * p_ForcePersistance;

    p_SerialPortConfiguration = _node_soap_get(p_node, "SerialPortConfiguration");
    if (p_SerialPortConfiguration)
    {
        XMLN * p_BaudRate;
        XMLN * p_ParityBit;
        XMLN * p_CharacterLength;
        XMLN * p_StopBit;
        const char * p_token;
        const char * pt;

        p_token = _attr_get(p_SerialPortConfiguration, "token");
        if (p_token)
        {
            strncpy(p_req->SerialPortConfiguration.token, p_token, sizeof(p_req->SerialPortConfiguration.token)-1);
        }

        pt = _attr_get(p_SerialPortConfiguration, "type");
        if (pt)
        {
            p_req->SerialPortConfiguration.type = onvif_StringToSerialPortType(pt);
        }

        p_BaudRate = _node_soap_get(p_SerialPortConfiguration, "BaudRate");
        if (p_BaudRate && p_BaudRate->data)
        {
            p_req->SerialPortConfiguration.BaudRate = atoi(p_BaudRate->data);
        }

        p_ParityBit = _node_soap_get(p_SerialPortConfiguration, "ParityBit");
        if (p_ParityBit && p_ParityBit->data)
        {
            p_req->SerialPortConfiguration.ParityBit = onvif_StringToParityBit(p_ParityBit->data);
        }

        p_CharacterLength = _node_soap_get(p_SerialPortConfiguration, "CharacterLength");
        if (p_CharacterLength && p_CharacterLength->data)
        {
            p_req->SerialPortConfiguration.CharacterLength = atoi(p_CharacterLength->data);
        }

        p_StopBit = _node_soap_get(p_SerialPortConfiguration, "StopBit");
        if (p_StopBit && p_StopBit->data)
        {
            p_req->SerialPortConfiguration.StopBit = (float)atof(p_StopBit->data);
        }
    }

    p_ForcePersistance = _node_soap_get(p_node, "ForcePersistance");
    if (p_ForcePersistance && p_ForcePersistance->data)
    {
        p_req->ForcePersistance = parse_Bool(p_ForcePersistance->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tmd_SendReceiveSerialCommand(XMLN * p_node, tmd_SendReceiveSerialCommand_REQ * p_req)
{
    XMLN * p_token;
    XMLN * p_SerialData;
    XMLN * p_TimeOut;
    XMLN * p_DataLength;
    XMLN * p_Delimiter;

    p_token = _node_soap_get(p_node, "token");
    if (p_token && p_token->data)
    {
        strncpy(p_req->token, p_token->data, sizeof(p_req->token)-1);
    }

    p_SerialData = _node_soap_get(p_node, "SerialData");
    if (p_SerialData)
    {
        XMLN * p_Binary;
        XMLN * p_String;

        p_Binary = _node_soap_get(p_SerialData, "Binary");
        if (p_Binary && p_Binary->data)
        {
            p_req->Command.SerialData._union_SerialData = 0;

            onvif_malloc_SerialData(&p_req->Command.SerialData, 0, (int)strlen(p_Binary->data)+1);
            strcpy(p_req->Command.SerialData.union_SerialData.Binary, p_Binary->data);
        }

        p_String = _node_soap_get(p_SerialData, "String");
        if (p_String && p_String->data)
        {
            p_req->Command.SerialData._union_SerialData = 1;

            onvif_malloc_SerialData(&p_req->Command.SerialData, 1, (int)strlen(p_String->data)+1);
            strcpy(p_req->Command.SerialData.union_SerialData.String, p_String->data);
        }

        p_req->Command.SerialDataFlag = 1;
    }

    p_TimeOut = _node_soap_get(p_node, "TimeOut");
    if (p_TimeOut && p_TimeOut->data)
    {
        p_req->Command.TimeOutFlag = 1;
        parse_XSDDuration(p_TimeOut->data, (int *)&p_req->Command.TimeOut);
    }

    p_DataLength = _node_soap_get(p_node, "DataLength");
    if (p_DataLength && p_DataLength->data)
    {
        p_req->Command.DataLengthFlag = 1;
        p_req->Command.DataLength = atoi(p_DataLength->data);
    }

    p_Delimiter = _node_soap_get(p_node, "Delimiter");
    if (p_Delimiter && p_Delimiter->data)
    {
        p_req->Command.DelimiterFlag = 1;
        strncpy(p_req->Command.Delimiter, p_Delimiter->data, sizeof(p_req->Command.Delimiter)-1);
    }

    return ONVIF_OK;
}

#endif // end of DEVICEIO_SUPPORT

#ifdef MEDIA2_SUPPORT

ONVIF_RET parse_ConfigurationRef(XMLN * p_node, onvif_ConfigurationRef * p_req)
{
    XMLN * p_Type;
    XMLN * p_Token;

    p_Type = _node_soap_get(p_node, "Type");
    if (p_Type && p_Type->data)
    {
        strncpy(p_req->Type, p_Type->data, sizeof(p_req->Type)-1);
    }

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        p_req->TokenFlag = 1;
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

BOOL parse_Polygon(XMLN * p_node, onvif_Polygon * p_req)
{
    XMLN * p_Point;

    p_Point = _node_soap_get(p_node, "Point");
    while (p_Point && soap_strcmp(p_Point->name, "Point") == 0)
    {
        int idx = p_req->sizePoint;

        parse_Vector(p_Point, &p_req->Point[idx]);

        p_req->sizePoint++;
        if (p_req->sizePoint >= ARRAY_SIZE(p_req->Point))
        {
            break;
        }

        p_Point = p_Point->next;
    }

    return TRUE;
}

BOOL parse_Color(XMLN * p_node, onvif_Color * p_req)
{
    const char * p_X;
    const char * p_Y;
    const char * p_Z;
    const char * p_Colorspace;

    p_X = _attr_get(p_node, "X");
    if (p_X)
    {
        p_req->X = (float)atof(p_X);
    }

    p_Y = _attr_get(p_node, "Y");
    if (p_Y)
    {
        p_req->Y = (float)atof(p_Y);
    }

    p_Z = _attr_get(p_node, "Z");
    if (p_Z)
    {
        p_req->Z = (float)atof(p_Z);
    }

    p_Colorspace = _attr_get(p_node, "Colorspace");
    if (p_Colorspace)
    {
        p_req->ColorspaceFlag = 1;
        strncpy(p_req->Colorspace, p_Colorspace, sizeof(p_req->Colorspace)-1);
    }

    return TRUE;
}

ONVIF_RET parse_Mask(XMLN * p_node, onvif_Mask * p_req)
{
    XMLN * p_ConfigurationToken;
    XMLN * p_Polygon;
    XMLN * p_Type;
    XMLN * p_Color;
    XMLN * p_Enabled;
    const char * p_token;

    p_token = _attr_get(p_node, "token");
    if (p_token)
    {
        strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
    }

    p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
    if (p_ConfigurationToken && p_ConfigurationToken->data)
    {
        strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
    }

    p_Polygon = _node_soap_get(p_node, "Polygon");
    if (p_Polygon)
    {
        parse_Polygon(p_Polygon, &p_req->Polygon);
    }

    p_Type = _node_soap_get(p_node, "Type");
    if (p_Type && p_Type->data)
    {
        strncpy(p_req->Type, p_Type->data, sizeof(p_req->Type)-1);
    }

    p_Color = _node_soap_get(p_node, "Color");
    if (p_Color)
    {
        p_req->ColorFlag = parse_Color(p_Color, &p_req->Color);
    }

    p_Enabled = _node_soap_get(p_node, "Enabled");
    if (p_Enabled && p_Enabled->data)
    {
        p_req->Enabled = parse_Bool(p_Enabled->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_GetConfiguration(XMLN * p_node, tr2_GetConfiguration * p_req)
{
    XMLN * p_ConfigurationToken;
    XMLN * p_ProfileToken;

    p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
    if (p_ConfigurationToken && p_ConfigurationToken->data)
    {
        p_req->ConfigurationTokenFlag = 1;
        strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
    }

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        p_req->ProfileTokenFlag = 1;
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_SetVideoEncoderConfiguration(XMLN * p_node, tr2_SetVideoEncoderConfiguration_REQ * p_req)
{
    XMLN * p_Configuration;

    p_Configuration = _node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
        XMLN * p_Name;
    	XMLN * p_UseCount;
    	XMLN * p_Encoding;
    	XMLN * p_Resolution;
    	XMLN * p_RateControl;
    	const char * p_token;
        const char * p_GovLength;
        const char * p_Profile;

        p_token = _attr_get(p_Configuration, "token");
        if (p_token)
        {
            strncpy(p_req->Configuration.token, p_token, sizeof(p_req->Configuration.token)-1);
        }

        p_GovLength = _attr_get(p_Configuration, "GovLength");
        if (p_GovLength)
        {
            p_req->Configuration.GovLengthFlag = 1;
            p_req->Configuration.GovLength = atoi(p_GovLength);
        }

        p_Profile = _attr_get(p_Configuration, "Profile");
        if (p_Profile)
        {
            p_req->Configuration.ProfileFlag = 1;
            strncpy(p_req->Configuration.Profile, p_Profile, sizeof(p_req->Configuration.Profile)-1);
        }

        p_Name = _node_soap_get(p_Configuration, "Name");
        if (p_Name && p_Name->data)
        {
            strncpy(p_req->Configuration.Name, p_Name->data, sizeof(p_req->Configuration.Name)-1);
        }

        p_UseCount = _node_soap_get(p_Configuration, "UseCount");
        if (p_UseCount && p_UseCount->data)
        {
            p_req->Configuration.UseCount = atoi(p_UseCount->data);
        }

        p_Encoding = _node_soap_get(p_Configuration, "Encoding");
        if (p_Encoding && p_Encoding->data)
        {
    		strncpy(p_req->Configuration.Encoding, p_Encoding->data, sizeof(p_req->Configuration.Encoding)-1);
        }

        p_Resolution = _node_soap_get(p_Configuration, "Resolution");
        if (p_Resolution)
        {
            XMLN * p_Width;
    		XMLN * p_Height;

    		p_Width = _node_soap_get(p_Resolution, "Width");
    	    if (p_Width && p_Width->data)
    	    {
    	        p_req->Configuration.Resolution.Width = atoi(p_Width->data);
    	    }

    	    p_Height = _node_soap_get(p_Resolution, "Height");
    	    if (p_Height && p_Height->data)
    	    {
    	        p_req->Configuration.Resolution.Height = atoi(p_Height->data);
    	    }
        }

        p_RateControl = _node_soap_get(p_Configuration, "RateControl");
        if (p_RateControl)
        {
        	XMLN * p_FrameRateLimit;
    		XMLN * p_BitrateLimit; 

            p_FrameRateLimit = _node_soap_get(p_RateControl, "FrameRateLimit");
    	    if (p_FrameRateLimit && p_FrameRateLimit->data)
    	    {
    	        p_req->Configuration.RateControl.FrameRateLimit = (float)atof(p_FrameRateLimit->data);
    	    }



    	    p_BitrateLimit = _node_soap_get(p_RateControl, "BitrateLimit");
    	    if (p_BitrateLimit && p_BitrateLimit->data)
    	    {
    	        p_req->Configuration.RateControl.BitrateLimit = atoi(p_BitrateLimit->data);
    	    }
        }

    	if (ONVIF_OK == parse_MulticastConfiguration(p_Configuration, &p_req->Configuration.Multicast))
    	{
    	    p_req->Configuration.MulticastFlag = 1;
    	}

    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_CreateProfile(XMLN * p_node, tr2_CreateProfile_REQ * p_req)
{
    XMLN * p_Name;
    XMLN * p_Configuration;

    p_Name = _node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
    }

    p_Configuration = _node_soap_get(p_node, "Configuration");
    while (p_Configuration && soap_strcmp(p_Configuration->name, "Configuration") == 0)
    {
        int idx = p_req->sizeConfiguration;

        parse_ConfigurationRef(p_Configuration, &p_req->Configuration[idx]);

        p_req->sizeConfiguration++;
        if (p_req->sizeConfiguration >= ARRAY_SIZE(p_req->Configuration))
        {
            break;
        }

        p_Configuration = p_Configuration->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_GetProfiles(XMLN * p_node, tr2_GetProfiles_REQ * p_req)
{
    XMLN * p_Token;
    XMLN * p_Type;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        p_req->TokenFlag = 1;
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    p_Type = _node_soap_get(p_node, "Type");
    while (p_Type && p_Type->data && soap_strcmp(p_Type->name, "Type") == 0)
    {
        int idx = p_req->sizeType;

        strncpy(p_req->Type[idx], p_Type->data, sizeof(p_req->Type[idx])-1);

        p_req->sizeType++;
        if (p_req->sizeType >= ARRAY_SIZE(p_req->Type))
        {
            break;
        }

        p_Type = p_Type->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_DeleteProfile(XMLN * p_node, tr2_DeleteProfile_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_AddConfiguration(XMLN * p_node, tr2_AddConfiguration_REQ * p_req)
{
    XMLN * p_ProfileToken;
    XMLN * p_Name;
    XMLN * p_Configuration;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    p_Name = _node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        p_req->NameFlag = 1;
        strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
    }

    p_Configuration = _node_soap_get(p_node, "Configuration");
    while (p_Configuration && soap_strcmp(p_Configuration->name, "Configuration") == 0)
    {
        int idx = p_req->sizeConfiguration;

        parse_ConfigurationRef(p_Configuration, &p_req->Configuration[idx]);

        p_req->sizeConfiguration++;
        if (p_req->sizeConfiguration >= ARRAY_SIZE(p_req->Configuration))
        {
            break;
        }

        p_Configuration = p_Configuration->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_RemoveConfiguration(XMLN * p_node, tr2_RemoveConfiguration_REQ * p_req)
{
    XMLN * p_ProfileToken;
    XMLN * p_Configuration;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    p_Configuration = _node_soap_get(p_node, "Configuration");
    while (p_Configuration && soap_strcmp(p_Configuration->name, "Configuration") == 0)
    {
        int idx = p_req->sizeConfiguration;

        parse_ConfigurationRef(p_Configuration, &p_req->Configuration[idx]);

        p_req->sizeConfiguration++;
        if (p_req->sizeConfiguration >= ARRAY_SIZE(p_req->Configuration))
        {
            break;
        }

        p_Configuration = p_Configuration->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_SetVideoSourceConfiguration(XMLN * p_node, tr2_SetVideoSourceConfiguration_REQ * p_req)
{
    return parse_VideoSourceConfiguration(p_node, &p_req->Configuration);
}

ONVIF_RET parse_tr2_SetAudioEncoderConfiguration(XMLN * p_node, tr2_SetAudioEncoderConfiguration_REQ * p_req)
{
    XMLN * p_Configuration;

    p_Configuration = _node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
        XMLN * p_Name;
    	XMLN * p_UseCount;
    	XMLN * p_Encoding;
    	XMLN * p_Bitrate;
    	XMLN * p_SampleRate;
    	const char * p_token;

        p_token = _attr_get(p_Configuration, "token");
        if (p_token)
        {
            strncpy(p_req->Configuration.token, p_token, sizeof(p_req->Configuration.token)-1);
        }

        p_Name = _node_soap_get(p_Configuration, "Name");
        if (p_Name && p_Name->data)
        {
            strncpy(p_req->Configuration.Name, p_Name->data, sizeof(p_req->Configuration.Name)-1);
        }

        p_UseCount = _node_soap_get(p_Configuration, "UseCount");
        if (p_UseCount && p_UseCount->data)
        {
            p_req->Configuration.UseCount = atoi(p_UseCount->data);
        }

        p_Encoding = _node_soap_get(p_Configuration, "Encoding");
        if (p_Encoding && p_Encoding->data)
        {
    		strncpy(p_req->Configuration.Encoding, p_Encoding->data, sizeof(p_req->Configuration.Encoding)-1);
        }

    	if (ONVIF_OK == parse_MulticastConfiguration(p_Configuration, &p_req->Configuration.Multicast))
    	{
    	    p_req->Configuration.MulticastFlag = 1;
    	}

    	p_Bitrate = _node_soap_get(p_Configuration, "Bitrate");
        if (p_Bitrate && p_Bitrate->data)
        {
            p_req->Configuration.Bitrate = atoi(p_Bitrate->data);
        }

        p_SampleRate = _node_soap_get(p_Configuration, "SampleRate");
        if (p_SampleRate && p_SampleRate->data)
        {
            p_req->Configuration.SampleRate = atoi(p_SampleRate->data);
        }
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_SetMetadataConfiguration(XMLN * p_node, tr2_SetMetadataConfiguration_REQ * p_req)
{
    return parse_MetadataConfiguration(p_node, &p_req->Configuration);
}

ONVIF_RET parse_tr2_GetVideoEncoderInstances(XMLN * p_node, tr2_GetVideoEncoderInstances_REQ * p_req)
{
    XMLN * p_ConfigurationToken;

    p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
    if (p_ConfigurationToken && p_ConfigurationToken->data)
    {
        strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_GetStreamUri(XMLN * p_node, tr2_GetStreamUri_REQ * p_req)
{
    XMLN * p_Protocol;
    XMLN * p_ProfileToken;

    p_Protocol = _node_soap_get(p_node, "Protocol");
    if (p_Protocol && p_Protocol->data)
    {
        strncpy(p_req->Protocol, p_Protocol->data, sizeof(p_req->Protocol)-1);
    }

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_SetSynchronizationPoint(XMLN * p_node, tr2_SetSynchronizationPoint_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_GetVideoSourceModes(XMLN * p_node, tr2_GetVideoSourceModes_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
		strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tr2_SetVideoSourceMode(XMLN * p_node, tr2_SetVideoSourceMode_REQ * p_req)
{
    XMLN * p_VideoSourceToken;
    XMLN * p_VideoSourceModeToken;

    p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
		strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	p_VideoSourceModeToken = _node_soap_get(p_node, "VideoSourceModeToken");
	if (p_VideoSourceModeToken && p_VideoSourceModeToken->data)
	{
		strncpy(p_req->VideoSourceModeToken, p_VideoSourceModeToken->data, sizeof(p_req->VideoSourceModeToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tr2_GetSnapshotUri(XMLN * p_node, tr2_GetSnapshotUri_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tr2_GetOSDs(XMLN * p_node, tr2_GetOSDs_REQ * p_req)
{
    XMLN * p_OSDToken;
    XMLN * p_ConfigurationToken;

    p_OSDToken = _node_soap_get(p_node, "OSDToken");
	if (p_OSDToken && p_OSDToken->data)
	{
	    p_req->OSDTokenFlag = 1;
	    strncpy(p_req->OSDToken, p_OSDToken->data, sizeof(p_req->OSDToken)-1);
	}

	p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
	    p_req->ConfigurationTokenFlag = 1;
	    strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tr2_CreateMask(XMLN * p_node, tr2_CreateMask_REQ * p_req)
{
    XMLN * p_Mask;

    p_Mask = _node_soap_get(p_node, "Mask");
    if (p_Mask)
    {
        parse_Mask(p_Mask, &p_req->Mask);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_DeleteMask(XMLN * p_node, tr2_DeleteMask_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
	if (p_Token && p_Token->data)
	{
	    strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
	}

	return ONVIF_OK;
}


ONVIF_RET parse_tr2_StartMulticastStreaming(XMLN * p_node, tr2_StartMulticastStreaming_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_StopMulticastStreaming(XMLN * p_node, tr2_StopMulticastStreaming_REQ * p_req)
{
    XMLN * p_ProfileToken;

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
    }

    return ONVIF_OK;
}

#ifdef DEVICEIO_SUPPORT

ONVIF_RET parse_tr2_GetAudioOutputConfigurations(XMLN * p_node, tr2_GetAudioOutputConfigurations_REQ * p_req)
{
    XMLN * p_ConfigurationToken;
    XMLN * p_ProfileToken;

    p_ConfigurationToken = _node_soap_get(p_node, "ConfigurationToken");
    if (p_ConfigurationToken && p_ConfigurationToken->data)
    {
        p_req->GetConfiguration.ConfigurationTokenFlag = 1;
        strncpy(p_req->GetConfiguration.ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->GetConfiguration.ConfigurationToken)-1);
    }

    p_ProfileToken = _node_soap_get(p_node, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        p_req->GetConfiguration.ProfileTokenFlag = 1;
        strncpy(p_req->GetConfiguration.ProfileToken, p_ProfileToken->data, sizeof(p_req->GetConfiguration.ProfileToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tr2_SetAudioOutputConfiguration(XMLN * p_node, tr2_SetAudioOutputConfiguration_REQ * p_req)
{
    XMLN * p_Configuration;

    p_Configuration = _node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
        parse_AudioOutputConfiguration(p_Configuration, &p_req->Configuration);
    }

    return ONVIF_OK;
}

#endif // end of DEVICEIO_SUPPORT

#endif // end of MEDIA2_SUPPORT

#ifdef THERMAL_SUPPORT

ONVIF_RET parse_ColorPalette(XMLN * p_node, onvif_ColorPalette * p_req)
{
    const char * p_token;
    const char * p_Type;
    XMLN * p_Name;

    p_token = _attr_get(p_node, "token");
    if (p_token)
    {
        strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
    }

    p_Type = _attr_get(p_node, "Type");
    if (p_Type)
    {
        strncpy(p_req->Type, p_Type, sizeof(p_req->Type)-1);
    }

    p_Name = _node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
	    strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_NUCTable(XMLN * p_node, onvif_NUCTable * p_req)
{
    const char * p_token;
    const char * p_LowTemperature;
    const char * p_HighTemperature;
    XMLN * p_Name;

    p_token = _attr_get(p_node, "token");
    if (p_token)
    {
        strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
    }

    p_LowTemperature = _attr_get(p_node, "LowTemperature");
    if (p_LowTemperature)
    {
        p_req->LowTemperatureFlag = 1;
        p_req->LowTemperature = (float) atof(p_LowTemperature);
    }

    p_HighTemperature = _attr_get(p_node, "HighTemperature");
    if (p_HighTemperature)
    {
        p_req->HighTemperatureFlag = 1;
        p_req->HighTemperature = (float) atof(p_HighTemperature);
    }

    p_Name = _node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
	    strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_Cooler(XMLN * p_node, onvif_Cooler * p_req)
{
    XMLN * p_Enabled;
    XMLN * p_RunTime;

    p_Enabled = _node_soap_get(p_node, "Enabled");
	if (p_Enabled && p_Enabled->data)
	{
	    p_req->Enabled = parse_Bool(p_Enabled->data);
	}

	p_RunTime = _node_soap_get(p_node, "RunTime");
	if (p_RunTime && p_RunTime->data)
	{
	    p_req->RunTime = (float) atof(p_RunTime->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_ThermalConfiguration(XMLN * p_node, onvif_ThermalConfiguration * p_req)
{
    XMLN * p_ColorPalette;
    XMLN * p_Polarity;
    XMLN * p_NUCTable;
    XMLN * p_Cooler;

    p_ColorPalette = _node_soap_get(p_node, "ColorPalette");
	if (p_ColorPalette)
	{
	    parse_ColorPalette(p_ColorPalette, &p_req->ColorPalette);
	}

	p_Polarity = _node_soap_get(p_node, "Polarity");
	if (p_Polarity && p_Polarity->data)
	{
	    p_req->Polarity = onvif_StringToPolarity(p_Polarity->data);
	}

	p_NUCTable = _node_soap_get(p_node, "NUCTable");
	if (p_NUCTable)
	{
	    p_req->NUCTableFlag = 1;
	    parse_NUCTable(p_NUCTable, &p_req->NUCTable);
	}

	p_Cooler = _node_soap_get(p_node, "Cooler");
	if (p_Cooler)
	{
	    p_req->CoolerFlag = 1;
	    parse_Cooler(p_Cooler, &p_req->Cooler);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_RadiometryGlobalParameters(XMLN * p_node, onvif_RadiometryGlobalParameters * p_req)
{
    XMLN * p_ReflectedAmbientTemperature;
    XMLN * p_Emissivity;
    XMLN * p_DistanceToObject;
    XMLN * p_RelativeHumidity;
    XMLN * p_AtmosphericTemperature;
    XMLN * p_AtmosphericTransmittance;
    XMLN * p_ExtOpticsTemperature;
    XMLN * p_ExtOpticsTransmittance;

    p_ReflectedAmbientTemperature = _node_soap_get(p_node, "ReflectedAmbientTemperature");
	if (p_ReflectedAmbientTemperature && p_ReflectedAmbientTemperature->data)
	{
	    p_req->ReflectedAmbientTemperature = (float) atof(p_ReflectedAmbientTemperature->data);
	}

	p_Emissivity = _node_soap_get(p_node, "Emissivity");
	if (p_Emissivity && p_Emissivity->data)
	{
	    p_req->Emissivity = (float) atof(p_Emissivity->data);
	}

	p_DistanceToObject = _node_soap_get(p_node, "DistanceToObject");
	if (p_DistanceToObject && p_DistanceToObject->data)
	{
	    p_req->DistanceToObject = (float) atof(p_DistanceToObject->data);
	}

	p_RelativeHumidity = _node_soap_get(p_node, "RelativeHumidity");
	if (p_RelativeHumidity && p_RelativeHumidity->data)
	{
	    p_req->RelativeHumidityFlag = 1;
	    p_req->RelativeHumidity = (float) atof(p_RelativeHumidity->data);
	}

    p_AtmosphericTemperature = _node_soap_get(p_node, "AtmosphericTemperature");
	if (p_AtmosphericTemperature && p_AtmosphericTemperature->data)
	{
	    p_req->AtmosphericTemperatureFlag = 1;
	    p_req->AtmosphericTemperature = (float) atof(p_AtmosphericTemperature->data);
	}

	p_AtmosphericTransmittance = _node_soap_get(p_node, "AtmosphericTransmittance");
	if (p_AtmosphericTransmittance && p_AtmosphericTransmittance->data)
	{
	    p_req->AtmosphericTransmittanceFlag = 1;
	    p_req->AtmosphericTransmittance = (float) atof(p_AtmosphericTransmittance->data);
	}

    p_ExtOpticsTemperature = _node_soap_get(p_node, "ExtOpticsTemperature");
	if (p_ExtOpticsTemperature && p_ExtOpticsTemperature->data)
	{
	    p_req->ExtOpticsTemperatureFlag = 1;
	    p_req->ExtOpticsTemperature = (float) atof(p_ExtOpticsTemperature->data);
	}

	p_ExtOpticsTransmittance = _node_soap_get(p_node, "ExtOpticsTransmittance");
	if (p_ExtOpticsTransmittance && p_ExtOpticsTransmittance->data)
	{
	    p_req->ExtOpticsTransmittanceFlag = 1;
	    p_req->ExtOpticsTransmittance = (float) atof(p_ExtOpticsTransmittance->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_RadiometryConfiguration(XMLN * p_node, onvif_RadiometryConfiguration * p_req)
{
    XMLN * p_RadiometryGlobalParameters;

    p_RadiometryGlobalParameters = _node_soap_get(p_node, "RadiometryGlobalParameters");
	if (p_RadiometryGlobalParameters)
	{
	    p_req->RadiometryGlobalParametersFlag = 1;

	    parse_RadiometryGlobalParameters(p_RadiometryGlobalParameters, &p_req->RadiometryGlobalParameters);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tth_GetConfiguration(XMLN * p_node, tth_GetConfiguration_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

	p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
	    strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tth_SetConfiguration(XMLN * p_node, tth_SetConfiguration_REQ * p_req)
{
    XMLN * p_VideoSourceToken;
    XMLN * p_Configuration;

	p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
	    strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	p_Configuration = _node_soap_get(p_node, "Configuration");
	if (p_Configuration)
	{
	    parse_ThermalConfiguration(p_Configuration, &p_req->Configuration);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tth_GetConfigurationOptions(XMLN * p_node, tth_GetConfigurationOptions_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

	p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
	    strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tth_GetRadiometryConfiguration(XMLN * p_node, tth_GetRadiometryConfiguration_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

	p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
	    strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tth_SetRadiometryConfiguration(XMLN * p_node, tth_SetRadiometryConfiguration_REQ * p_req)
{
    XMLN * p_VideoSourceToken;
    XMLN * p_Configuration;

	p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
	    strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	p_Configuration = _node_soap_get(p_node, "Configuration");
	if (p_Configuration)
	{
	    parse_RadiometryConfiguration(p_Configuration, &p_req->Configuration);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_tth_GetRadiometryConfigurationOptions(XMLN * p_node, tth_GetRadiometryConfigurationOptions_REQ * p_req)
{
    XMLN * p_VideoSourceToken;

	p_VideoSourceToken = _node_soap_get(p_node, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
	    strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
	}

	return ONVIF_OK;
}

#endif // end of THERMAL_SUPPORT
#if 0
void checkLoginStatus(XMLN *p_node) {
    tcr_GetCredentials_REQ req;
    req.sizeToken = 0;

    // 자격 증명 요청 파싱
    ONVIF_RET ret = parse_tcr_GetCredentials(p_node, &req);
    if (ret == ONVIF_OK && req.sizeToken > 0) {
        printf("login state:  found ---------------------------------------\n");
        printf("Token list:\n");
        for (int i = 0; i < req.sizeToken; i++) {
            printf("Token[%d]: %s\n", i, req.Token[i]);
        }
    } else {
        printf("login state: not found--------------------------------------- \n");
    }
}
int checkLoginStatus____w4000() {
    XMLN *someNode;  // 실제 XML 노드를 할당해야합니다.
    // someNode = ... (SOAP 요청을 위한 XML 노드 초기화 또는 생성)
	printf_function_soap_parser();

    checkLoginStatus(someNode);
    
    return 0;
}
#endif
#ifdef CREDENTIAL_SUPPORT

ONVIF_RET parse_CredentialIdentifierType(XMLN * p_node, onvif_CredentialIdentifierType * p_req)
{
    XMLN * p_Name;
    XMLN * p_FormatType;

    p_Name = _node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
    }

    p_FormatType = _node_soap_get(p_node, "FormatType");
    if (p_FormatType && p_FormatType->data)
    {
        strncpy(p_req->FormatType, p_FormatType->data, sizeof(p_req->FormatType)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_CredentialIdentifier(XMLN * p_node, onvif_CredentialIdentifier * p_req)
{
    XMLN * p_Type;
    XMLN * p_ExemptedFromAuthentication;
    XMLN * p_Value;

    p_Type = _node_soap_get(p_node, "Type");
    if (p_Type)
    {
        parse_CredentialIdentifierType(p_Type, &p_req->Type);
    }

    p_ExemptedFromAuthentication = _node_soap_get(p_node, "ExemptedFromAuthentication");
    if (p_ExemptedFromAuthentication && p_ExemptedFromAuthentication->data)
    {
        p_req->ExemptedFromAuthentication = parse_Bool(p_ExemptedFromAuthentication->data);
    }

    p_Value = _node_soap_get(p_node, "Value");
    if (p_Value && p_Value->data)
    {
        strncpy(p_req->Value, p_Value->data, sizeof(p_req->Value)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_CredentialIdentifierItem(XMLN * p_node, onvif_CredentialIdentifierItem * p_req)
{
    XMLN * p_Type;
    XMLN * p_Value;

    p_Type = _node_soap_get(p_node, "Type");
    if (p_Type)
    {
        parse_CredentialIdentifierType(p_Type, &p_req->Type);
    }

    p_Value = _node_soap_get(p_node, "Value");
    if (p_Value && p_Value->data)
    {
        strncpy(p_req->Value, p_Value->data, sizeof(p_req->Value)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_CredentialAccessProfile(XMLN * p_node, onvif_CredentialAccessProfile * p_req)
{
    XMLN * p_AccessProfileToken;
    XMLN * p_ValidFrom;
    XMLN * p_ValidTo;

    p_AccessProfileToken = _node_soap_get(p_node, "AccessProfileToken");
    if (p_AccessProfileToken && p_AccessProfileToken->data)
    {
        strncpy(p_req->AccessProfileToken, p_AccessProfileToken->data, sizeof(p_req->AccessProfileToken)-1);
    }

    p_ValidFrom = _node_soap_get(p_node, "ValidFrom");
    if (p_ValidFrom && p_ValidFrom->data)
    {
        p_req->ValidFromFlag = 1;
        strncpy(p_req->ValidFrom, p_ValidFrom->data, sizeof(p_req->ValidFrom)-1);
    }

    p_ValidTo = _node_soap_get(p_node, "ValidTo");
    if (p_ValidTo && p_ValidTo->data)
    {
        p_req->ValidToFlag = 1;
        strncpy(p_req->ValidTo, p_ValidTo->data, sizeof(p_req->ValidFrom)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_Attribute(XMLN * p_node, onvif_Attribute * p_req)
{
    const char * p_Name;
    const char * p_Value;

    p_Name = _attr_get(p_node, "Name");
    if (p_Name)
    {
        strncpy(p_req->Name, p_Name, sizeof(p_req->Name)-1);
    }

    p_Value = _attr_get(p_node, "Value");
    if (p_Value)
    {
        p_req->ValueFlag = 1;
        strncpy(p_req->Value, p_Value, sizeof(p_req->Value)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_Credential(XMLN * p_node, onvif_Credential * p_req)
{
    XMLN * p_Description;
    XMLN * p_CredentialHolderReference;
    XMLN * p_ValidFrom;
    XMLN * p_ValidTo;
    XMLN * p_CredentialIdentifier;
    XMLN * p_CredentialAccessProfile;
    XMLN * p_Attribute;

    p_Description = _node_soap_get(p_node, "Description");
    if (p_Description && p_Description->data)
    {
        p_req->DescriptionFlag = 1;
        strncpy(p_req->Description, p_Description->data, sizeof(p_req->Description)-1);
    }

    p_CredentialHolderReference = _node_soap_get(p_node, "CredentialHolderReference");
    if (p_CredentialHolderReference && p_CredentialHolderReference->data)
    {
        strncpy(p_req->CredentialHolderReference, p_CredentialHolderReference->data, sizeof(p_req->CredentialHolderReference)-1);
    }

    p_ValidFrom = _node_soap_get(p_node, "ValidFrom");
    if (p_ValidFrom && p_ValidFrom->data)
    {
        p_req->ValidFromFlag = 1;
        strncpy(p_req->ValidFrom, p_ValidFrom->data, sizeof(p_req->ValidFrom)-1);
    }

    p_ValidTo = _node_soap_get(p_node, "ValidTo");
    if (p_ValidTo && p_ValidTo->data)
    {
        p_req->ValidToFlag = 1;
        strncpy(p_req->ValidTo, p_ValidTo->data, sizeof(p_req->ValidTo)-1);
    }

    p_CredentialIdentifier = _node_soap_get(p_node, "CredentialIdentifier");
    while (p_CredentialIdentifier && soap_strcmp(p_CredentialIdentifier->name, "CredentialIdentifier") == 0)
    {
        int idx = p_req->sizeCredentialIdentifier;

        parse_CredentialIdentifier(p_CredentialIdentifier, &p_req->CredentialIdentifier[idx]);

        p_req->sizeCredentialIdentifier++;
        if (p_req->sizeCredentialIdentifier >= ARRAY_SIZE(p_req->CredentialIdentifier))
        {
            break;
        }

        p_CredentialIdentifier = p_CredentialIdentifier->next;
    }

    p_CredentialAccessProfile = _node_soap_get(p_node, "CredentialAccessProfile");
    while (p_CredentialAccessProfile && soap_strcmp(p_CredentialAccessProfile->name, "CredentialAccessProfile") == 0)
    {
        int idx = p_req->sizeCredentialAccessProfile;

        parse_CredentialAccessProfile(p_CredentialAccessProfile, &p_req->CredentialAccessProfile[idx]);

        p_req->sizeCredentialAccessProfile++;
        if (p_req->sizeCredentialAccessProfile >= ARRAY_SIZE(p_req->CredentialAccessProfile))
        {
            break;
        }

        p_CredentialAccessProfile = p_CredentialAccessProfile->next;
    }

    p_Attribute = _node_soap_get(p_node, "Attribute");
    while (p_Attribute && soap_strcmp(p_Attribute->name, "Attribute") == 0)
    {
        int idx = p_req->sizeAttribute;

        parse_Attribute(p_Attribute, &p_req->Attribute[idx]);

        p_req->sizeAttribute++;
        if (p_req->sizeAttribute >= ARRAY_SIZE(p_req->Attribute))
        {
            break;
        }

        p_Attribute = p_Attribute->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_CredentialState(XMLN * p_node, onvif_CredentialState * p_req)
{
    XMLN * p_Enabled;
    XMLN * p_Reason;
    XMLN * p_AntipassbackState;

    p_Enabled = _node_soap_get(p_node, "Enabled");
    if (p_Enabled && p_Enabled->data)
    {
        p_req->Enabled = parse_Bool(p_Enabled->data);
    }

    p_Reason = _node_soap_get(p_node, "Reason");
    if (p_Reason && p_Reason->data)
    {
        p_req->ReasonFlag = 1;
        strncpy(p_req->Reason, p_Reason->data, sizeof(p_req->Reason)-1);
    }

    p_AntipassbackState = _node_soap_get(p_node, "AntipassbackState");
    if (p_AntipassbackState)
    {
        XMLN * p_AntipassbackViolated;

        p_req->AntipassbackStateFlag = 1;

        p_AntipassbackViolated = _node_soap_get(p_AntipassbackState, "AntipassbackViolated");
        if (p_AntipassbackViolated && p_AntipassbackViolated->data)
        {
            p_req->AntipassbackState.AntipassbackViolated = parse_Bool(p_AntipassbackViolated->data);
        }
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetCredentialInfo(XMLN * p_node, tcr_GetCredentialInfo_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && soap_strcmp(p_Token->name, "Token") == 0)
    {
        int idx = p_req->sizeToken;

        strncpy(p_req->Token[idx], p_Token->data, sizeof(p_req->Token[idx])-1);

        p_req->sizeToken++;
        if (p_req->sizeToken >= ARRAY_SIZE(p_req->Token))
        {
            break;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetCredentialInfoList(XMLN * p_node, tcr_GetCredentialInfoList_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetCredentials(XMLN * p_node, tcr_GetCredentials_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && soap_strcmp(p_Token->name, "Token") == 0)
    {
        int idx = p_req->sizeToken;

        strncpy(p_req->Token[idx], p_Token->data, sizeof(p_req->Token[idx])-1);

        p_req->sizeToken++;
        if (p_req->sizeToken >= ARRAY_SIZE(p_req->Token))
        {
            break;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetCredentialList(XMLN * p_node, tcr_GetCredentialList_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_CreateCredential(XMLN * p_node, tcr_CreateCredential_REQ * p_req)
{
    XMLN * p_Credential;
    XMLN * p_State;

    p_Credential = _node_soap_get(p_node, "Credential");
    if (p_Credential)
    {
        const char * p_token;

        p_token = _attr_get(p_Credential, "token");
        if (p_token)
        {
            strncpy(p_req->Credential.token, p_token, sizeof(p_req->Credential.token)-1);
        }

        parse_Credential(p_Credential, &p_req->Credential);
    }

    p_State = _node_soap_get(p_node, "State");
    if (p_State)
    {
        parse_CredentialState(p_State, &p_req->State);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_ModifyCredential(XMLN * p_node, tcr_ModifyCredential_REQ * p_req)
{
    XMLN * p_Credential;

    p_Credential = _node_soap_get(p_node, "Credential");
    if (p_Credential)
    {
        const char * p_token;

        p_token = _attr_get(p_Credential, "token");
        if (p_token)
        {
            strncpy(p_req->Credential.token, p_token, sizeof(p_req->Credential.token)-1);
        }

        parse_Credential(p_Credential, &p_req->Credential);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_DeleteCredential(XMLN * p_node, tcr_DeleteCredential_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetCredentialState(XMLN * p_node, tcr_GetCredentialState_REQ * p_req)
{printf_function_soap_parser();
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_EnableCredential(XMLN * p_node, tcr_EnableCredential_REQ * p_req)
{
    XMLN * p_Token;
    XMLN * p_Reason;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    p_Reason = _node_soap_get(p_node, "Reason");
    if (p_Reason && p_Reason->data)
    {
        p_req->ReasonFlag = 1;
        strncpy(p_req->Reason, p_Reason->data, sizeof(p_req->Reason)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_DisableCredential(XMLN * p_node, tcr_DisableCredential_REQ * p_req)
{
    XMLN * p_Token;
    XMLN * p_Reason;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    p_Reason = _node_soap_get(p_node, "Reason");
    if (p_Reason && p_Reason->data)
    {
        p_req->ReasonFlag = 1;
        strncpy(p_req->Reason, p_Reason->data, sizeof(p_req->Reason)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_SetCredential(XMLN * p_node, tcr_SetCredential_REQ * p_req)
{
    XMLN * p_CredentialData;

    p_CredentialData = _node_soap_get(p_node, "CredentialData");
    if (p_CredentialData)
    {
        XMLN * p_Credential;
        XMLN * p_CredentialState;

        p_Credential = _node_soap_get(p_CredentialData, "Credential");
        if (p_Credential)
        {
            const char * p_token;

            p_token = _attr_get(p_Credential, "token");
            if (p_token)
            {
                strncpy(p_req->CredentialData.Credential.token, p_token, sizeof(p_req->CredentialData.Credential.token)-1);
            }

            parse_Credential(p_Credential, &p_req->CredentialData.Credential);
        }

        p_CredentialState = _node_soap_get(p_CredentialData, "CredentialState");
        if (p_CredentialState)
        {
            parse_CredentialState(p_CredentialState, &p_req->CredentialData.CredentialState);
        }
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_ResetAntipassbackViolation(XMLN * p_node, tcr_ResetAntipassbackViolation_REQ * p_req)
{
    XMLN * p_CredentialToken;

    p_CredentialToken = _node_soap_get(p_node, "CredentialToken");
    if (p_CredentialToken && p_CredentialToken->data)
    {
        strncpy(p_req->CredentialToken, p_CredentialToken->data, sizeof(p_req->CredentialToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetSupportedFormatTypes(XMLN * p_node, tcr_GetSupportedFormatTypes_REQ * p_req)
{
    XMLN * p_CredentialIdentifierTypeName;

    p_CredentialIdentifierTypeName = _node_soap_get(p_node, "CredentialIdentifierTypeName");
    if (p_CredentialIdentifierTypeName && p_CredentialIdentifierTypeName->data)
    {
        strncpy(p_req->CredentialIdentifierTypeName, p_CredentialIdentifierTypeName->data, sizeof(p_req->CredentialIdentifierTypeName)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetCredentialIdentifiers(XMLN * p_node, tcr_GetCredentialIdentifiers_REQ * p_req)
{
    XMLN * p_CredentialToken;

    p_CredentialToken = _node_soap_get(p_node, "CredentialToken");
    if (p_CredentialToken && p_CredentialToken->data)
    {
        strncpy(p_req->CredentialToken, p_CredentialToken->data, sizeof(p_req->CredentialToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_SetCredentialIdentifier(XMLN * p_node, tcr_SetCredentialIdentifier_REQ * p_req)
{
    XMLN * p_CredentialToken;
    XMLN * p_CredentialIdentifier;

    p_CredentialToken = _node_soap_get(p_node, "CredentialToken");
    if (p_CredentialToken && p_CredentialToken->data)
    {
        strncpy(p_req->CredentialToken, p_CredentialToken->data, sizeof(p_req->CredentialToken)-1);
    }

    p_CredentialIdentifier = _node_soap_get(p_node, "CredentialIdentifier");
    if (p_CredentialIdentifier)
    {
        parse_CredentialIdentifier(p_CredentialIdentifier, &p_req->CredentialIdentifier);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_DeleteCredentialIdentifier(XMLN * p_node, tcr_DeleteCredentialIdentifier_REQ * p_req)
{
    XMLN * p_CredentialToken;
    XMLN * p_CredentialIdentifierTypeName;

    p_CredentialToken = _node_soap_get(p_node, "CredentialToken");
    if (p_CredentialToken && p_CredentialToken->data)
    {
        strncpy(p_req->CredentialToken, p_CredentialToken->data, sizeof(p_req->CredentialToken)-1);
    }

    p_CredentialIdentifierTypeName = _node_soap_get(p_node, "CredentialIdentifierTypeName");
    if (p_CredentialIdentifierTypeName && p_CredentialIdentifierTypeName->data)
    {
        strncpy(p_req->CredentialIdentifierTypeName, p_CredentialIdentifierTypeName->data, sizeof(p_req->CredentialIdentifierTypeName)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetCredentialAccessProfiles(XMLN * p_node, tcr_GetCredentialAccessProfiles_REQ * p_req)
{
    XMLN * p_CredentialToken;

    p_CredentialToken = _node_soap_get(p_node, "CredentialToken");
    if (p_CredentialToken && p_CredentialToken->data)
    {
        strncpy(p_req->CredentialToken, p_CredentialToken->data, sizeof(p_req->CredentialToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_SetCredentialAccessProfiles(XMLN * p_node, tcr_SetCredentialAccessProfiles_REQ * p_req)
{
    XMLN * p_CredentialToken;
    XMLN * p_CredentialAccessProfile;

    p_CredentialToken = _node_soap_get(p_node, "CredentialToken");
    if (p_CredentialToken && p_CredentialToken->data)
    {
        strncpy(p_req->CredentialToken, p_CredentialToken->data, sizeof(p_req->CredentialToken)-1);
    }

    p_CredentialAccessProfile = _node_soap_get(p_node, "CredentialAccessProfile");
    while (p_CredentialAccessProfile && soap_strcmp(p_CredentialAccessProfile->name, "CredentialAccessProfile") == 0)
    {
        int idx = p_req->sizeCredentialAccessProfile;

        parse_CredentialAccessProfile(p_CredentialAccessProfile, &p_req->CredentialAccessProfile[idx]);

        p_req->sizeCredentialAccessProfile++;
        if (p_req->sizeCredentialAccessProfile >= ARRAY_SIZE(p_req->CredentialAccessProfile))
        {
            break;
        }

        p_CredentialAccessProfile = p_CredentialAccessProfile->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_DeleteCredentialAccessProfiles(XMLN * p_node, tcr_DeleteCredentialAccessProfiles_REQ * p_req)
{
    XMLN * p_CredentialToken;
    XMLN * p_AccessProfileToken;

    p_CredentialToken = _node_soap_get(p_node, "CredentialToken");
    if (p_CredentialToken && p_CredentialToken->data)
    {
        strncpy(p_req->CredentialToken, p_CredentialToken->data, sizeof(p_req->CredentialToken)-1);
    }

    p_AccessProfileToken = _node_soap_get(p_node, "AccessProfileToken");
    while (p_AccessProfileToken && p_AccessProfileToken->data && soap_strcmp(p_AccessProfileToken->name, "AccessProfileToken") == 0)
    {
        int idx = p_req->sizeAccessProfileToken;

        strncpy(p_req->AccessProfileToken[idx], p_AccessProfileToken->data, sizeof(p_req->AccessProfileToken[idx])-1);

        p_req->sizeAccessProfileToken++;
        if (p_req->sizeAccessProfileToken >= ARRAY_SIZE(p_req->AccessProfileToken))
        {
            break;
        }

        p_AccessProfileToken = p_AccessProfileToken->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetWhitelist(XMLN * p_node, tcr_GetWhitelist_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;
    XMLN * p_IdentifierType;
    XMLN * p_FormatType;
    XMLN * p_Value;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    p_IdentifierType = _node_soap_get(p_node, "IdentifierType");
    if (p_IdentifierType && p_IdentifierType->data)
    {
        p_req->IdentifierTypeFlag = 1;
        strncpy(p_req->IdentifierType, p_IdentifierType->data, sizeof(p_req->IdentifierType)-1);
    }

    p_FormatType = _node_soap_get(p_node, "FormatType");
    if (p_FormatType && p_FormatType->data)
    {
        p_req->FormatTypeFlag = 1;
        strncpy(p_req->FormatType, p_FormatType->data, sizeof(p_req->FormatType)-1);
    }

    p_Value = _node_soap_get(p_node, "Value");
    if (p_Value && p_Value->data)
    {
        p_req->ValueFlag = 1;
        strncpy(p_req->Value, p_Value->data, sizeof(p_req->Value)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_AddToWhitelist(XMLN * p_node, tcr_AddToWhitelist_REQ * p_req)
{
    XMLN * p_Identifier;

    p_Identifier = _node_soap_get(p_node, "Identifier");
    while (p_Identifier && soap_strcmp(p_Identifier->name, "Identifier") == 0)
    {
        CredentialIdentifierItemList * p_item = onvif_add_CredentialIdentifierItem(&p_req->Identifier);
        if (p_item)
        {
            parse_CredentialIdentifierItem(p_Identifier, &p_item->Item);
        }

        p_Identifier = p_Identifier->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_RemoveFromWhitelist(XMLN * p_node, tcr_RemoveFromWhitelist_REQ * p_req)
{
    XMLN * p_Identifier;

    p_Identifier = _node_soap_get(p_node, "Identifier");
    while (p_Identifier && soap_strcmp(p_Identifier->name, "Identifier") == 0)
    {
        CredentialIdentifierItemList * p_item = onvif_add_CredentialIdentifierItem(&p_req->Identifier);
        if (p_item)
        {
            parse_CredentialIdentifierItem(p_Identifier, &p_item->Item);
        }

        p_Identifier = p_Identifier->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_GetBlacklist(XMLN * p_node, tcr_GetBlacklist_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;
    XMLN * p_IdentifierType;
    XMLN * p_FormatType;
    XMLN * p_Value;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    p_IdentifierType = _node_soap_get(p_node, "IdentifierType");
    if (p_IdentifierType && p_IdentifierType->data)
    {
        p_req->IdentifierTypeFlag = 1;
        strncpy(p_req->IdentifierType, p_IdentifierType->data, sizeof(p_req->IdentifierType)-1);
    }

    p_FormatType = _node_soap_get(p_node, "FormatType");
    if (p_FormatType && p_FormatType->data)
    {
        p_req->FormatTypeFlag = 1;
        strncpy(p_req->FormatType, p_FormatType->data, sizeof(p_req->FormatType)-1);
    }

    p_Value = _node_soap_get(p_node, "Value");
    if (p_Value && p_Value->data)
    {
        p_req->ValueFlag = 1;
        strncpy(p_req->Value, p_Value->data, sizeof(p_req->Value)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_AddToBlacklist(XMLN * p_node, tcr_AddToBlacklist_REQ * p_req)
{
    XMLN * p_Identifier;

    p_Identifier = _node_soap_get(p_node, "Identifier");
    while (p_Identifier && soap_strcmp(p_Identifier->name, "Identifier") == 0)
    {
        CredentialIdentifierItemList * p_item = onvif_add_CredentialIdentifierItem(&p_req->Identifier);
        if (p_item)
        {
            parse_CredentialIdentifierItem(p_Identifier, &p_item->Item);
        }

        p_Identifier = p_Identifier->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tcr_RemoveFromBlacklist(XMLN * p_node, tcr_RemoveFromBlacklist_REQ * p_req)
{
    XMLN * p_Identifier;

    p_Identifier = _node_soap_get(p_node, "Identifier");
    while (p_Identifier && soap_strcmp(p_Identifier->name, "Identifier") == 0)
    {
        CredentialIdentifierItemList * p_item = onvif_add_CredentialIdentifierItem(&p_req->Identifier);
        if (p_item)
        {
            parse_CredentialIdentifierItem(p_Identifier, &p_item->Item);
        }

        p_Identifier = p_Identifier->next;
    }

    return ONVIF_OK;
}

#endif // end of CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES

ONVIF_RET parse_AccessPolicy(XMLN * p_node, onvif_AccessPolicy * p_req)
{
    XMLN * p_ScheduleToken;
    XMLN * p_Entity;
    XMLN * p_EntityType;

    p_ScheduleToken = _node_soap_get(p_node, "ScheduleToken");
    if (p_ScheduleToken && p_ScheduleToken->data)
    {
        strncpy(p_req->ScheduleToken, p_ScheduleToken->data, sizeof(p_req->ScheduleToken)-1);
    }

    p_Entity = _node_soap_get(p_node, "Entity");
    if (p_Entity && p_Entity->data)
    {
        strncpy(p_req->Entity, p_Entity->data, sizeof(p_req->Entity)-1);
    }

    p_EntityType = _node_soap_get(p_node, "EntityType");
    if (p_EntityType && p_EntityType->data)
    {
        p_req->EntityTypeFlag = 1;
        strncpy(p_req->EntityType, p_EntityType->data, sizeof(p_req->EntityType)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_AccessProfile(XMLN * p_node, onvif_AccessProfile * p_req)
{
    XMLN * p_Name;
    XMLN * p_Description;
    XMLN * p_AccessPolicy;

    p_Name = _node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
    }

    p_Description = _node_soap_get(p_node, "Description");
    if (p_Description && p_Description->data)
    {
        p_req->DescriptionFlag = 1;
        strncpy(p_req->Description, p_Description->data, sizeof(p_req->Description)-1);
    }

    p_AccessPolicy = _node_soap_get(p_node, "AccessPolicy");
    while (p_AccessPolicy && soap_strcmp(p_AccessPolicy->name, "AccessPolicy") == 0)
    {
        int idx = p_req->sizeAccessPolicy;

        parse_AccessPolicy(p_AccessPolicy, &p_req->AccessPolicy[idx]);

        p_req->sizeAccessPolicy++;
        if (p_req->sizeAccessPolicy >= ARRAY_SIZE(p_req->AccessPolicy))
        {
            break;
        }

        p_AccessPolicy = p_AccessPolicy->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tar_GetAccessProfileInfo(XMLN * p_node, tar_GetAccessProfileInfo_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && soap_strcmp(p_Token->name, "Token") == 0)
    {
        int idx = p_req->sizeToken;

        strncpy(p_req->Token[idx], p_Token->data, sizeof(p_req->Token[idx])-1);

        p_req->sizeToken++;
        if (p_req->sizeToken >= ARRAY_SIZE(p_req->Token))
        {
            break;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tar_GetAccessProfileInfoList(XMLN * p_node, tar_GetAccessProfileInfoList_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tar_GetAccessProfiles(XMLN * p_node, tar_GetAccessProfiles_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && soap_strcmp(p_Token->name, "Token") == 0)
    {
        int idx = p_req->sizeToken;

        strncpy(p_req->Token[idx], p_Token->data, sizeof(p_req->Token[idx])-1);

        p_req->sizeToken++;
        if (p_req->sizeToken >= ARRAY_SIZE(p_req->Token))
        {
            break;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tar_GetAccessProfileList(XMLN * p_node, tar_GetAccessProfileList_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tar_CreateAccessProfile(XMLN * p_node, tar_CreateAccessProfile_REQ * p_req)
{
    XMLN * p_AccessProfile;

    p_AccessProfile = _node_soap_get(p_node, "AccessProfile");
    if (p_AccessProfile)
    {
        const char * p_token;

        p_token = _attr_get(p_AccessProfile, "token");
        if (p_token)
        {
            strncpy(p_req->AccessProfile.token, p_token, sizeof(p_req->AccessProfile.token)-1);
        }

        parse_AccessProfile(p_AccessProfile, &p_req->AccessProfile);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tar_ModifyAccessProfile(XMLN * p_node, tar_ModifyAccessProfile_REQ * p_req)
{
    XMLN * p_AccessProfile;

    p_AccessProfile = _node_soap_get(p_node, "AccessProfile");
    if (p_AccessProfile)
    {
        const char * p_token;

        p_token = _attr_get(p_AccessProfile, "token");
        if (p_token)
        {
            strncpy(p_req->AccessProfile.token, p_token, sizeof(p_req->AccessProfile.token)-1);
        }

        parse_AccessProfile(p_AccessProfile, &p_req->AccessProfile);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tar_DeleteAccessProfile(XMLN * p_node, tar_DeleteAccessProfile_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tar_SetAccessProfile(XMLN * p_node, tar_SetAccessProfile_REQ * p_req)
{
    XMLN * p_AccessProfile;

    p_AccessProfile = _node_soap_get(p_node, "AccessProfile");
    if (p_AccessProfile)
    {
        const char * p_token;

        p_token = _attr_get(p_AccessProfile, "token");
        if (p_token)
        {
            strncpy(p_req->AccessProfile.token, p_token, sizeof(p_req->AccessProfile.token)-1);
        }

        parse_AccessProfile(p_AccessProfile, &p_req->AccessProfile);
    }

    return ONVIF_OK;
}

#endif // end of ACCESS_RULES

#ifdef SCHEDULE_SUPPORT

ONVIF_RET parse_TimePeriod(XMLN * p_node, onvif_TimePeriod * p_req)
{
    XMLN * p_From;
    XMLN * p_Until;

    p_From = _node_soap_get(p_node, "From");
    if (p_From && p_From->data)
    {
        strncpy(p_req->From, p_From->data, sizeof(p_req->From)-1);
    }

    p_Until = _node_soap_get(p_node, "Until");
    if (p_Until && p_Until->data)
    {
        p_req->UntilFlag = 1;
        strncpy(p_req->Until, p_Until->data, sizeof(p_req->Until)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_SpecialDaysSchedule(XMLN * p_node, onvif_SpecialDaysSchedule * p_req)
{
    XMLN * p_GroupToken;
    XMLN * p_TimeRange;

    p_GroupToken = _node_soap_get(p_node, "GroupToken");
    if (p_GroupToken && p_GroupToken->data)
    {
        strncpy(p_req->GroupToken, p_GroupToken->data, sizeof(p_req->GroupToken)-1);
    }

    p_TimeRange = _node_soap_get(p_node, "TimeRange");
    while (p_TimeRange && soap_strcmp(p_TimeRange->name, "TimeRange") == 0)
    {
        int idx = p_req->sizeTimeRange;

        parse_TimePeriod(p_TimeRange, &p_req->TimeRange[idx]);

        p_req->sizeTimeRange++;
        if (p_req->sizeTimeRange >= ARRAY_SIZE(p_req->TimeRange))
        {
            break;
        }

        p_TimeRange = p_TimeRange->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_Schedule(XMLN * p_node, onvif_Schedule * p_req)
{
    XMLN * p_Name;
    XMLN * p_Description;
    XMLN * p_Standard;
    XMLN * p_SpecialDays;

    p_Name = _node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
    }

    p_Description = _node_soap_get(p_node, "Description");
    if (p_Description && p_Description->data)
    {
        p_req->DescriptionFlag = 1;
        strncpy(p_req->Description, p_Description->data, sizeof(p_req->Description)-1);
    }

    p_Standard = _node_soap_get(p_node, "Standard");
    if (p_Standard && p_Standard->data)
    {
        strncpy(p_req->Standard, p_Standard->data, sizeof(p_req->Standard)-1);
    }

    p_SpecialDays = _node_soap_get(p_node, "SpecialDays");
    while (p_SpecialDays && soap_strcmp(p_SpecialDays->name, "SpecialDays") == 0)
    {
        int idx = p_req->sizeSpecialDays;

        parse_SpecialDaysSchedule(p_SpecialDays, &p_req->SpecialDays[idx]);

        p_req->sizeSpecialDays++;
        if (p_req->sizeSpecialDays >= ARRAY_SIZE(p_req->SpecialDays))
        {
            break;
        }

        p_SpecialDays = p_SpecialDays->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_SpecialDayGroup(XMLN * p_node, onvif_SpecialDayGroup * p_req)
{
    XMLN * p_Name;
    XMLN * p_Description;
    XMLN * p_Days;

    p_Name = _node_soap_get(p_node, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
    }

    p_Description = _node_soap_get(p_node, "Description");
    if (p_Description && p_Description->data)
    {
        p_req->DescriptionFlag = 1;
        strncpy(p_req->Description, p_Description->data, sizeof(p_req->Description)-1);
    }

    p_Days = _node_soap_get(p_node, "Days");
    if (p_Days && p_Days->data)
    {
        p_req->DaysFlag = 1;
        strncpy(p_req->Days, p_Days->data, sizeof(p_req->Days)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_GetScheduleInfo(XMLN * p_node, tsc_GetScheduleInfo_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && soap_strcmp(p_Token->name, "Token") == 0)
    {
        int idx = p_req->sizeToken;

        strncpy(p_req->Token[idx], p_Token->data, sizeof(p_req->Token[idx])-1);

        p_req->sizeToken++;
        if (p_req->sizeToken >= ARRAY_SIZE(p_req->Token))
        {
            break;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_GetScheduleInfoList(XMLN * p_node, tsc_GetScheduleInfoList_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_GetSchedules(XMLN * p_node, tsc_GetSchedules_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && p_Token->data && soap_strcmp(p_Token->name, "Token") == 0)
    {
        int idx = p_req->sizeToken;

        strncpy(p_req->Token[idx], p_Token->data, sizeof(p_req->Token[idx])-1);

        p_req->sizeToken++;
        if (p_req->sizeToken >= ARRAY_SIZE(p_req->Token))
        {
            break;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_GetScheduleList(XMLN * p_node, tsc_GetScheduleList_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_CreateSchedule(XMLN * p_node, tsc_CreateSchedule_REQ * p_req)
{
    XMLN * p_Schedule;

    p_Schedule = _node_soap_get(p_node, "Schedule");
    if (p_Schedule)
    {
        const char * p_token;

        p_token = _attr_get(p_Schedule, "token");
        if (p_token)
        {
            strncpy(p_req->Schedule.token, p_token, sizeof(p_req->Schedule.token)-1);
        }

        parse_Schedule(p_Schedule, &p_req->Schedule);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_ModifySchedule(XMLN * p_node, tsc_ModifySchedule_REQ * p_req)
{
    XMLN * p_Schedule;

    p_Schedule = _node_soap_get(p_node, "Schedule");
    if (p_Schedule)
    {
        const char * p_token;

        p_token = _attr_get(p_Schedule, "token");
        if (p_token)
        {
            strncpy(p_req->Schedule.token, p_token, sizeof(p_req->Schedule.token)-1);
        }

        parse_Schedule(p_Schedule, &p_req->Schedule);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_DeleteSchedule(XMLN * p_node, tsc_DeleteSchedule_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_GetSpecialDayGroupInfo(XMLN * p_node, tsc_GetSpecialDayGroupInfo_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && soap_strcmp(p_Token->name, "Token") == 0)
    {
        int idx = p_req->sizeToken;

        strncpy(p_req->Token[idx], p_Token->data, sizeof(p_req->Token[idx])-1);

        p_req->sizeToken++;
        if (p_req->sizeToken >= ARRAY_SIZE(p_req->Token))
        {
            break;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_GetSpecialDayGroupInfoList(XMLN * p_node, tsc_GetSpecialDayGroupInfoList_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_GetSpecialDayGroups(XMLN * p_node, tsc_GetSpecialDayGroups_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    while (p_Token && soap_strcmp(p_Token->name, "Token") == 0)
    {
        int idx = p_req->sizeToken;

        strncpy(p_req->Token[idx], p_Token->data, sizeof(p_req->Token[idx])-1);

        p_req->sizeToken++;
        if (p_req->sizeToken >= ARRAY_SIZE(p_req->Token))
        {
            break;
        }

        p_Token = p_Token->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_GetSpecialDayGroupList(XMLN * p_node, tsc_GetSpecialDayGroupList_REQ * p_req)
{
    XMLN * p_Limit;
    XMLN * p_StartReference;

    p_Limit = _node_soap_get(p_node, "Limit");
    if (p_Limit && p_Limit->data)
    {
        p_req->LimitFlag = 1;
        p_req->Limit = atoi(p_Limit->data);
    }

    p_StartReference = _node_soap_get(p_node, "StartReference");
    if (p_StartReference && p_StartReference->data)
    {
        p_req->StartReferenceFlag = 1;
        strncpy(p_req->StartReference, p_StartReference->data, sizeof(p_req->StartReference)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_CreateSpecialDayGroup(XMLN * p_node, tsc_CreateSpecialDayGroup_REQ * p_req)
{
    XMLN * p_SpecialDayGroup;

    p_SpecialDayGroup = _node_soap_get(p_node, "SpecialDayGroup");
    if (p_SpecialDayGroup)
    {
        const char * p_token;

        p_token = _attr_get(p_SpecialDayGroup, "token");
        if (p_token)
        {
            strncpy(p_req->SpecialDayGroup.token, p_token, sizeof(p_req->SpecialDayGroup.token)-1);
        }

        parse_SpecialDayGroup(p_SpecialDayGroup, &p_req->SpecialDayGroup);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_ModifySpecialDayGroup(XMLN * p_node, tsc_ModifySpecialDayGroup_REQ * p_req)
{
    XMLN * p_SpecialDayGroup;

    p_SpecialDayGroup = _node_soap_get(p_node, "SpecialDayGroup");
    if (p_SpecialDayGroup)
    {
        const char * p_token;

        p_token = _attr_get(p_SpecialDayGroup, "token");
        if (p_token)
        {
            strncpy(p_req->SpecialDayGroup.token, p_token, sizeof(p_req->SpecialDayGroup.token)-1);
        }

        parse_SpecialDayGroup(p_SpecialDayGroup, &p_req->SpecialDayGroup);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_DeleteSpecialDayGroup(XMLN * p_node, tsc_DeleteSpecialDayGroup_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_GetScheduleState(XMLN * p_node, tsc_GetScheduleState_REQ * p_req)
{
    XMLN * p_Token;

    p_Token = _node_soap_get(p_node, "Token");
    if (p_Token && p_Token->data)
    {
        strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_SetSchedule(XMLN * p_node, tsc_SetSchedule_REQ * p_req)
{
    XMLN * p_Schedule;

    p_Schedule = _node_soap_get(p_node, "Schedule");
    if (p_Schedule)
    {
        const char * p_token;

        p_token = _attr_get(p_Schedule, "token");
        if (p_token)
        {
            strncpy(p_req->Schedule.token, p_token, sizeof(p_req->Schedule.token)-1);
        }

        parse_Schedule(p_Schedule, &p_req->Schedule);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tsc_SetSpecialDayGroup(XMLN * p_node, tsc_SetSpecialDayGroup_REQ * p_req)
{
    XMLN * p_SpecialDayGroup;

    p_SpecialDayGroup = _node_soap_get(p_node, "SpecialDayGroup");
    if (p_SpecialDayGroup)
    {
        const char * p_token;

        p_token = _attr_get(p_SpecialDayGroup, "token");
        if (p_token)
        {
            strncpy(p_req->SpecialDayGroup.token, p_token, sizeof(p_req->SpecialDayGroup.token)-1);
        }

        parse_SpecialDayGroup(p_SpecialDayGroup, &p_req->SpecialDayGroup);
    }

    return ONVIF_OK;
}

#endif // end of SCHEDULE_SUPPORT

#ifdef RECEIVER_SUPPORT

ONVIF_RET parse_ReceiverConfiguration(XMLN * p_node, onvif_ReceiverConfiguration * p_req)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN * p_Mode;
    XMLN * p_MediaUri;
    XMLN * p_StreamSetup;

    p_Mode = _node_soap_get(p_node, "Mode");
    if (p_Mode && p_Mode->data)
    {
        p_req->Mode = onvif_StringToReceiverMode(p_Mode->data);
    }

    p_MediaUri = _node_soap_get(p_node, "MediaUri");
    if (p_MediaUri && p_MediaUri->data)
    {
        strncpy(p_req->MediaUri, p_MediaUri->data, sizeof(p_req->MediaUri)-1);
    }

    p_StreamSetup = _node_soap_get(p_node, "StreamSetup");
    if (p_StreamSetup)
    {
        ret = parse_StreamSetup(p_StreamSetup, &p_req->StreamSetup);
    }

    return ret;
}

ONVIF_RET parse_trv_GetReceiver(XMLN * p_node, trv_GetReceiver_REQ * p_req)
{
    XMLN * p_ReceiverToken;

    p_ReceiverToken = _node_soap_get(p_node, "ReceiverToken");
    if (p_ReceiverToken && p_ReceiverToken->data)
    {
        strncpy(p_req->ReceiverToken, p_ReceiverToken->data, sizeof(p_req->ReceiverToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_trv_CreateReceiver(XMLN * p_node, trv_CreateReceiver_REQ * p_req)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN * p_Configuration;

    p_Configuration = _node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
        ret = parse_ReceiverConfiguration(p_Configuration, &p_req->Configuration);
    }

    return ret;
}

ONVIF_RET parse_trv_DeleteReceiver(XMLN * p_node, trv_DeleteReceiver_REQ * p_req)
{
    XMLN * p_ReceiverToken;

    p_ReceiverToken = _node_soap_get(p_node, "ReceiverToken");
    if (p_ReceiverToken && p_ReceiverToken->data)
    {
        strncpy(p_req->ReceiverToken, p_ReceiverToken->data, sizeof(p_req->ReceiverToken)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_trv_ConfigureReceiver(XMLN * p_node, trv_ConfigureReceiver_REQ * p_req)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN * p_ReceiverToken;
    XMLN * p_Configuration;

    p_ReceiverToken = _node_soap_get(p_node, "ReceiverToken");
    if (p_ReceiverToken && p_ReceiverToken->data)
    {
        strncpy(p_req->ReceiverToken, p_ReceiverToken->data, sizeof(p_req->ReceiverToken)-1);
    }

    p_Configuration = _node_soap_get(p_node, "Configuration");
    if (p_Configuration)
    {
        ret = parse_ReceiverConfiguration(p_Configuration, &p_req->Configuration);
    }

    return ret;
}

ONVIF_RET parse_trv_SetReceiverMode(XMLN * p_node, trv_SetReceiverMode_REQ * p_req)
{
    XMLN * p_ReceiverToken;
    XMLN * p_Mode;

    p_ReceiverToken = _node_soap_get(p_node, "ReceiverToken");
    if (p_ReceiverToken && p_ReceiverToken->data)
    {
        strncpy(p_req->ReceiverToken, p_ReceiverToken->data, sizeof(p_req->ReceiverToken)-1);
    }

    p_Mode = _node_soap_get(p_node, "Mode");
    if (p_Mode && p_Mode->data)
    {
        p_req->Mode = onvif_StringToReceiverMode(p_Mode->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_trv_GetReceiverState(XMLN * p_node, trv_GetReceiverState_REQ * p_req)
{
    XMLN * p_ReceiverToken;

    p_ReceiverToken = _node_soap_get(p_node, "ReceiverToken");
    if (p_ReceiverToken && p_ReceiverToken->data)
    {
        strncpy(p_req->ReceiverToken, p_ReceiverToken->data, sizeof(p_req->ReceiverToken)-1);
    }

    return ONVIF_OK;
}

#endif // end of RECEIVER_SUPPORT

#ifdef PROVISIONING_SUPPORT

ONVIF_RET parse_tpv_PanMove(XMLN * p_node, tpv_PanMove_REQ * p_req)
{
    XMLN * p_VideoSource;
    XMLN * p_Direction;
    XMLN * p_Timeout;

    p_VideoSource = _node_soap_get(p_node, "VideoSource");
    if (p_VideoSource && p_VideoSource->data)
    {
        strncpy(p_req->VideoSource, p_VideoSource->data, sizeof(p_req->VideoSource)-1);
    }

    p_Direction = _node_soap_get(p_node, "Direction");
    if (p_Direction && p_Direction->data)
    {
        p_req->Direction = onvif_StringToPanDirection(p_Direction->data);
    }

    p_Timeout = _node_soap_get(p_node, "Timeout");
    if (p_Timeout && p_Timeout->data)
    {
        p_req->TimeoutFlag = parse_XSDDuration(p_Timeout->data, &p_req->Timeout);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tpv_TiltMove(XMLN * p_node, tpv_TiltMove_REQ * p_req)
{
    XMLN * p_VideoSource;
    XMLN * p_Direction;
    XMLN * p_Timeout;

    p_VideoSource = _node_soap_get(p_node, "VideoSource");
    if (p_VideoSource && p_VideoSource->data)
    {
        strncpy(p_req->VideoSource, p_VideoSource->data, sizeof(p_req->VideoSource)-1);
    }

    p_Direction = _node_soap_get(p_node, "Direction");
    if (p_Direction && p_Direction->data)
    {
        p_req->Direction = onvif_StringToTiltDirection(p_Direction->data);
    }

    p_Timeout = _node_soap_get(p_node, "Timeout");
    if (p_Timeout && p_Timeout->data)
    {
        p_req->TimeoutFlag = parse_XSDDuration(p_Timeout->data, &p_req->Timeout);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tpv_ZoomMove(XMLN * p_node, tpv_ZoomMove_REQ * p_req)
{
    XMLN * p_VideoSource;
    XMLN * p_Direction;
    XMLN * p_Timeout;

    p_VideoSource = _node_soap_get(p_node, "VideoSource");
    if (p_VideoSource && p_VideoSource->data)
    {
        strncpy(p_req->VideoSource, p_VideoSource->data, sizeof(p_req->VideoSource)-1);
    }

    p_Direction = _node_soap_get(p_node, "Direction");
    if (p_Direction && p_Direction->data)
    {
        p_req->Direction = onvif_StringToZoomDirection(p_Direction->data);
    }

    p_Timeout = _node_soap_get(p_node, "Timeout");
    if (p_Timeout && p_Timeout->data)
    {
        p_req->TimeoutFlag = parse_XSDDuration(p_Timeout->data, &p_req->Timeout);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tpv_RollMove(XMLN * p_node, tpv_RollMove_REQ * p_req)
{
    XMLN * p_VideoSource;
    XMLN * p_Direction;
    XMLN * p_Timeout;

    p_VideoSource = _node_soap_get(p_node, "VideoSource");
    if (p_VideoSource && p_VideoSource->data)
    {
        strncpy(p_req->VideoSource, p_VideoSource->data, sizeof(p_req->VideoSource)-1);
    }

    p_Direction = _node_soap_get(p_node, "Direction");
    if (p_Direction && p_Direction->data)
    {
        p_req->Direction = onvif_StringToRollDirection(p_Direction->data);
    }

    p_Timeout = _node_soap_get(p_node, "Timeout");
    if (p_Timeout && p_Timeout->data)
    {
        p_req->TimeoutFlag = parse_XSDDuration(p_Timeout->data, &p_req->Timeout);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tpv_FocusMove(XMLN * p_node, tpv_FocusMove_REQ * p_req)
{
    XMLN * p_VideoSource;
    XMLN * p_Direction;
    XMLN * p_Timeout;

    p_VideoSource = _node_soap_get(p_node, "VideoSource");
    if (p_VideoSource && p_VideoSource->data)
    {
        strncpy(p_req->VideoSource, p_VideoSource->data, sizeof(p_req->VideoSource)-1);
    }

    p_Direction = _node_soap_get(p_node, "Direction");
    if (p_Direction && p_Direction->data)
    {
        p_req->Direction = onvif_StringToFocusDirection(p_Direction->data);
    }

    p_Timeout = _node_soap_get(p_node, "Timeout");
    if (p_Timeout && p_Timeout->data)
    {
        p_req->TimeoutFlag = parse_XSDDuration(p_Timeout->data, &p_req->Timeout);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tpv_Stop(XMLN * p_node, tpv_Stop_REQ * p_req)
{
    XMLN * p_VideoSource;

    p_VideoSource = _node_soap_get(p_node, "VideoSource");
    if (p_VideoSource && p_VideoSource->data)
    {
        strncpy(p_req->VideoSource, p_VideoSource->data, sizeof(p_req->VideoSource)-1);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_tpv_GetUsage(XMLN * p_node, tpv_GetUsage_REQ * p_req)
{
    XMLN * p_VideoSource;

    p_VideoSource = _node_soap_get(p_node, "VideoSource");
    if (p_VideoSource && p_VideoSource->data)
    {
        strncpy(p_req->VideoSource, p_VideoSource->data, sizeof(p_req->VideoSource)-1);
    }

    return ONVIF_OK;
}

#endif // end of PROVISIONING_SUPPORT




