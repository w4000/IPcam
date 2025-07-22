#include "common.h"
#include "lib/upgr.h"
#include "lib/net.h"
#include "file/ssfsearch.h"
#include "file/log.h"
#include "setup/setup.h"
#include "push/pushevt.h"

// Video Codec Type
enum {
	MP1 = 0,
	MP2,
	MP4SP,
	MP4ASP,
	H264 = 4,
	H265
};

// Audio Codec Type
enum {
	NONE = 0,
	ADPCM,
	MP3,
	PCM,
	G711,
	G723,
	AAC
};




#if 0
#else
static void get_bypass_event_notify(struct st_stream * svr, char* event_notify, int event_notify_sz, int bForce)
{
	memset(event_notify, 0, event_notify_sz);

	int gotya = 0;
	char tmp[128];

	event_notify[0] = '\0';
	if(svr->transbypass == 0) {
		event_notify[0] = '\0';
		return;
	}

	// Motion & Sensor
	// {
	// 	char szEvent[64];

	// 	if (bForce || svr->bm_motion != g_main.mot || svr->by_bmsensor != g_main.m_sensor)
	// 	{
	// 		if(svr->bm_motion != g_main.mot) {
	// 			svr->bm_motion = g_main.mot;
	// 		}

	// 		if(svr->by_bmsensor != g_main.m_sensor) {
	// 			svr->by_bmsensor = g_main.m_sensor;
	// 		}

	// 		if (!gotya) {
	// 			strsafecat(event_notify, "EVENT_NOTIFY\n", event_notify_sz);
	// 			gotya = 1;
	// 		}

	// 		if(svr->bm_motion && svr->by_bmsensor) {
	// 			sprintf(szEvent, "MS,MS");
	// 		}
	// 		else if(svr->bm_motion && svr->by_bmsensor == 0) {
	// 			sprintf(szEvent, "M,M");
	// 		}
	// 		else if(svr->bm_motion == 0 && svr->by_bmsensor) {
	// 			sprintf(szEvent, "S,S");
	// 		}
	// 		else {
	// 			sprintf(szEvent, "U,U");
	// 		}

	// 		snprintf(tmp, sizeof(tmp), "RECMODE_CAM=%s\n", szEvent);
	// 		strsafecat(event_notify, tmp, event_notify_sz);
	// 	}
	// }

	// Emergency mode
	// {
	// 	if (bForce)
	// 	{

	// 		if (!gotya) {
	// 			strsafecat(event_notify, "EVENT_NOTIFY\n", event_notify_sz);
	// 			gotya = 1;
	// 		}
	// 		snprintf(tmp, sizeof(tmp), "EMG_MODE=%u\n", 0);
	// 		strsafecat(event_notify, tmp, event_notify_sz);
	// 	}
	// }

	// Disk fail
	// {
	// 	if (bForce || svr->by_recfail != g_main.recfail)
	// 	{
	// 		svr->by_recfail = g_main.recfail;

	// 		if (!gotya) {
	// 			strsafecat(event_notify, "EVENT_NOTIFY\n", event_notify_sz);
	// 			gotya = 1;
	// 		}
	// 		snprintf(tmp, sizeof(tmp), "DISK_FAIL=%u\n", (g_main.recfail) ? 1 : 0);
	// 		strsafecat(event_notify, tmp, event_notify_sz);
	// 	}
	// }

	// Video loss
	// {
	// 	char szVideoLoss2[128];
	// 	int ch;

	// 	int vloss[CAM_MAX] = { 0, };
	// 	for (ch = 0; ch < CAM_MAX; ch++)
	// 	{
	// 		vloss[ch] = 0;
	// 	}

	// 	if (bForce || memcmp(svr->by_vloss, vloss, sizeof(vloss)) != 0)
	// 	{
	// 		memset(szVideoLoss2, 0, sizeof(szVideoLoss2));

	// 		char szVidLoss2[32];
	// 		for (ch = 0; ch < CAM_MAX; ch++)
	// 		{
	// 			snprintf(szVidLoss2, sizeof(szVidLoss2), "%d", vloss[ch]);	
	// 			strsafecat(szVideoLoss2, szVidLoss2, sizeof(szVideoLoss2));
	// 			if (ch != CAM_MAX - 1)
	// 				strsafecat(szVideoLoss2, ",", sizeof(szVideoLoss2));
	// 		}

	// 		memcpy(svr->by_vloss, vloss, sizeof(vloss));

	// 		if (!gotya) {
	// 			strsafecat(event_notify, "EVENT_NOTIFY\n", event_notify_sz);
	// 			gotya = 1;
	// 		}

	// 		snprintf(tmp, sizeof(tmp), "VLOSS_CAM=%s\n", szVideoLoss2);
	// 		strsafecat(event_notify, tmp, event_notify_sz);
	// 	}
	// }



}
#endif


// Handle system info request like log file.
//	@fd: Socket to reply with info.
//
// Return:
//	0, on success.
//	-1, on error.
int handle_sysinfo_request (struct st_stream * svr, ST_PHDR phdr)
{
	char buf [32 * K], * tmp = buf;
	int fd;
	int msgsz = 0, ret = 0;

	fd = svr->sock;


	assert (fd >= 0);

	switch (phdr.id)
	{
	case PREQDEVINFO:
	case PREQFIXINFO:
	case PREQUPGRADE:
	case PREQSCHEDBKUP:
	case PRECORDDATE:
	case PREQSWVER:
	case PLOGFIRST:
	case PREQPUBLICINFO:
		{
			if (svr->usr)
			{
				if (!svr->usr->uperm.setup)
				{
					printf("NO USER PERMISSION..(SETUP) : %08x\n", phdr.id);
					char buf [1];
					reply2client(svr, fd, buf, REPLYID(phdr.id), RETNOPERM, 0);
					return -99;
				}
			}
		}
		break;
	}


	switch (phdr.id)
	{
	case PREQBYPASSINFO:
		{
			if (recvmsgfrom (svr, fd, buf, phdr.len) < 0)
			{
				tmp = buf + sizeof (struct phdr);
				setnetbuf (tmp, short, 0);
				msgsz = tmp - (buf + sizeof (struct phdr));
				ret = reply2client (svr, fd, buf, REPLYID (phdr.id), RETERRNET, msgsz);
			}

			char	szData[20480];

			memset(szData, 0, sizeof(szData));
			memcpy(szData, tmp, phdr.len);

			if (strncmp(szData, "[DEVICE_INFO]", 13) == 0) {

#if 1
				if (strcasestr(szData, "[NOTIFY_CTRL]") != NULL) {
					char *p = strcasestr(szData, "NOTIFY_ONOFF");
					if(p != NULL) {
						char *on = strcasestr(p+12, "ON");
						if(on == NULL) {
							svr->transbypass = 0;
						}
						else {
							svr->transbypass = 1;
						}
					}
				}
#endif
				char szEventNotify[20480];
				get_bypass_event_notify(svr, szEventNotify, sizeof(szEventNotify), 1);

				snprintf(szData, sizeof(szData),
						"[DEVICE_INFO]\n"
						"CAM_MAX=%d\n"
						"DUAL_STREAM=FALSE\n"
						"VOD_DUAL_STREAM=FALSE\n"
						"VOD_EX_CTL=TRUE\n"
						"ANALITICS=%s\n"
						"\n"
						"%s",
						CAM_MAX,
						"N",
						szEventNotify);

				tmp = buf + sizeof (struct phdr);
				memcpy(tmp, szData, strlen(szData) + 1);
				tmp += (strlen(szData) + 1);

				msgsz = tmp - (buf + sizeof (struct phdr));
				return ret = reply2client (svr, fd, buf, REPLYID (phdr.id), RETSUCCESS, msgsz);
			}
			
#if 1
			if (strcasestr(szData, "[NOTIFY_CTRL]") != NULL) {
				char *p = strcasestr(szData, "NOTIFY_ONOFF");
				if(p != NULL) {
					char *on = strcasestr(p+12, "ON");
					if(on == NULL) {
						svr->transbypass = 0;
					}
					else {
						svr->transbypass = 1;
					}
				}
				return reply2client (svr, fd, buf, REPLYID(phdr.id), RETSUCCESS, 0);
			}
#endif
			else { // Unknown ID.
				return reply2client (svr, fd, buf, REPLYID(phdr.id), RETFAIL, 0);
			}
		}
		break;

	case PREQDEVINFO:
		{
		}
		break;



	case PREQSCHEDBKUP:
		break;

	case PREQSWVER:
		{
		}
		break;

	case PLOGFIRST:
		{
		}
		break;


	default:
		return 1;
	}

	return 0;
}




