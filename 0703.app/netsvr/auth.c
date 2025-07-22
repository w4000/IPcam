#include "common.h"
#include "../setup/setup.h"
#define SETUP_CAM_PRESET_TITLE(setup, ch, i) ((setup)->cam [ch]._preset_title [i])
#define SETUP_USR(setup, i)					(& (setup)->usr[i])


static int handle_usrauth(struct st_stream *strsvr, int sd, struct phdr *ph) {

	struct pusrauth req;	
	struct pusrauth_resp repl;	
	int result = RETSUCCESS;	

	{ /* Get request data. */
		char rcvbuf[4*K];
		char *tmp, *tmp_first;
		int i, ret=0;

		tmp = tmp_first = rcvbuf;

#if 1
		if ((ret=recvmsgfrom(strsvr, sd, rcvbuf, ph->len)) < 0) {
			result=RETERRNET;
			goto reply;
		}
#else
		{
			int retry;
			for(retry = 0; retry < 10; retry++) {
				ret = recvmsgfrom(strsvr, sd, rcvbuf, ph->len);
				if(ret > 0) {
					break;
				}
			}
			if(ret
		}
#endif
		setfield(string, req.uid, tmp);
		setfield(string, req.upwd, tmp);
		setfield(char, req.oemcode, tmp);

		if (ph->len > tmp - tmp_first)
			setfield(char, req.smscode, tmp);
		else
			req.smscode = 0;
	}

	{
		strsvr->usr = setup_find_usr_by_name (& g_setup, req.uid);

		if (! strsvr->usr) { 
			result=RETNOUSR;
			goto reply;
		}

		if (strncmp (USR_PWD (strsvr->usr), req.upwd, USRPWD_MAX)) { 
			result=RETBADPWD;
			goto reply;
		}
	}

	{ 
		int mask = 0;			
		int recmask = 0;		
		int covert = 0;		
		int perm = 0xFFFFFFFF;
		int i, j;

		memset (& repl, 0, sizeof (struct pusrauth_resp));
		repl.cammax = CAM_MAX;

		repl.devtype = PLATFORM_MODEL;

		for(i = 0, mask = 0 ; i < MAX_CAM; i ++)
		{
			if (i < CAM_MAX)
			{
				if (CAMERA_EXIST (i))
					mask |= (0x01 << (31-i));

				if (check_recording_stat (i) == 0)
					recmask |= (0x01 << (31-i));
				

			}
		}

		repl.cammask = mask;
		repl.recmask = recmask;
		repl.covertmask = covert;
		repl.permmask = perm;


		for(i = 0; i < MAX_CAM; i ++)
		{
			repl.ptz[i].ptzact = 0xFFFFFFFF;
			repl.ptz[i].ptzactext = 0xFFFFFFFF;
			repl.ptz[i].maxsensor = 0;
			repl.ptz[i].maxalarm = 0;
		}

		for(i = 0; i < MAX_CAM; i ++) {
			if (VALIDCH (i) && CAMERA_EXIST (i)) {
				repl.pset [i].num = 8;

				for (j = 0; j < repl.pset [i].num; j ++)
					strcpy (repl.pset [i].title [j], SETUP_CAM_PRESET_TITLE (& g_setup, i, j));
			}
			else
				repl.pset [i].num = 0;
		}

		repl.twoway = 1;
	}

reply:
	{
		char buf [16*K];
		char *tmp= buf + sizeof(struct phdr);
		int msgsz=0, i, j;

		if (result==RETSUCCESS) {
			setnetbuf(tmp, short, repl.cammax);
			setnetbuf(tmp, char, repl.devtype);
			setnetbuf(tmp, int, repl.cammask);
			setnetbuf(tmp, int, repl.recmask);
			setnetbuf(tmp, int, repl.covertmask);
			setnetbuf(tmp, int, repl.permmask);

			for (i=0; i < MAX_CAM; i++)
			{
				setnetbuf(tmp, int, repl.ptz[i].ptzact);
				setnetbuf(tmp, int, repl.ptz[i].ptzactext);
				setnetbuf(tmp, char, repl.ptz[i].maxsensor);
				setnetbuf(tmp, char, repl.ptz[i].maxalarm);
			}

			for (i=0; i < MAX_CAM; i++) {
				setnetbuf(tmp, char, repl.pset[i].num);

				for (j = 0; j < repl.pset[i].num; j ++) {
					setnetbuf(tmp, string, repl.pset[i].title [j]);
				}
			}

			setnetbuf (tmp, char, repl.twoway);

			msgsz = tmp - (buf + sizeof (struct phdr));

			SS_SETFLAG(strsvr, 0, __SS_AUTHED);
			return reply2client(strsvr, sd, buf, REPLYID(ph->id), result, msgsz);
		}

		reply2client(strsvr, sd, buf, REPLYID(ph->id), result, msgsz);
		return -1;	// to close socket
	}
}
#if 1
static int auth_verify_sha256 (struct st_stream *strsvr, char *usrname, char *passwd)
{
	ST_USR	*usr = NULL;
	int		usrid = -1;
	char	sha256_id [128];
	char	sha256_pwd [128];
	int		i;
	int		len = 0;

	 {
		len = strlen (usrname);

		if (len >= 64) { 
			for (i = 0; i < 10; i++) {
				if (! USR_AVAIL (& g_setup.usr [i]))
					continue;
				SHA256_digest_string (SETUP_USR_NAME (& g_setup, i), strlen (SETUP_USR_NAME (& g_setup, i)), sha256_id);
				if (strcasecmp (sha256_id, usrname) == 0) {
					usr = & g_setup.usr [i];
					usrid = i;
					break;
				}
			}

			if (usrid < 0) // Could not find user.
				return -1;

			if (! passwd)
				return -2;

			SHA256_digest_string (USR_PWD (usr), strlen (USR_PWD (usr)), sha256_pwd);
			if (strcasecmp (sha256_pwd, passwd)) // Password does not match.
				return -2;

		}
		else {
			// Find user ID in user list and check password.
			usrid = setup_find_usrid_by_name (& g_setup, usrname);
			if (usrid < 0)
				return -1;
			if (! passwd)
				return -2;

			if (strcmp (SETUP_USR (& g_setup, usrid)->usrpwd, passwd)) { // Password does not match.
				if(strlen(passwd) > 32) {
					char __encrypted_pwd[USRPWD_ENC_MAX];
					ST_USR* usr = &(g_setup.usr[usrid]);
					cm_ci_enc2txt(__encrypted_pwd, USR_PWD (usr), strlen(USR_PWD (usr)) +1);

					if (strncmp (__encrypted_pwd, passwd, USRPWD_ENC_MAX)) { // Password does not match.
						return -2;
					}
				}
				else {
					return -2;
				}
			}

		}
	}
	return usrid;
}
#endif

#if 1

static int handle_usrauth_sha256(struct st_stream *strsvr, int sd, struct phdr *ph)
{

	struct pusrauth_enc req;	
	struct pusrauth_resp repl;	
	int result = RETSUCCESS;	
	int	usrid = -1;

	{ /* Get request data. */
		char rcvbuf[4*K];
		char *tmp = rcvbuf;
		int ret=0;

		if ((ret = recvmsgfrom(strsvr, sd, rcvbuf, ph->len)) < 0) {
			//return ret;
			result=RETERRNET;
			goto reply;
		}

		setfield(string, req.uid, tmp);
		setfield(string, req.upwd, tmp);
		setfield(char, req.oemcode, tmp);
	}

	{ 
		usrid = auth_verify_sha256 (strsvr, req.uid, req.upwd);
		if (usrid < 0) {
			if (usrid == -1) {
				result = RETNOUSR;
			}
			else if(usrid == -2) {
				result = RETBADPWD;
			}
			else if(usrid == -4) {
				result = RETBADPROT;
			}
			else {
				result = RETDEFAULT;
			}

			goto reply;
		}
	}

	{
		int mask = 0;	 
		int recmask = 0; 
		int covert = 0;	 
		int perm = 0xFFFFFFFF;
		int i, j;

		memset(&repl, 0, sizeof(struct pusrauth_resp));
		repl.cammax = CAM_MAX;
		repl.devtype = PLATFORM_MODEL;

		for (i = 0, mask = 0; i < MAX_CAM; i++)
		{
			if (i < CAM_MAX)
			{
				if (CAMERA_EXIST(i))
					mask |= (0x01 << (31 - i));

				if (check_recording_stat(i) == 0)
					recmask |= (0x01 << (31 - i));
			}
		}

		repl.cammask = mask;
		repl.recmask = recmask;
		repl.covertmask = covert;
		repl.permmask = perm;


		for (i = 0; i < MAX_CAM; i++)
		{
			repl.ptz[i].ptzact = 0xFFFFFFFF;
			repl.ptz[i].ptzactext = 0xFFFFFFFF;
			repl.ptz[i].maxsensor = 0;
			repl.ptz[i].maxalarm = 0;
		}

		for (i = 0; i < MAX_CAM; i++)
		{
			if (VALIDCH(i) && CAMERA_EXIST(i))
			{
				repl.pset[i].num = 8;

				for (j = 0; j < repl.pset[i].num; j++)
					strcpy(repl.pset[i].title[j], SETUP_CAM_PRESET_TITLE(&g_setup, i, j));
			}
			else
				repl.pset[i].num = 0;
		}

		repl.twoway = 1;
	}

reply:
	{
		char buf [16*K];
		char *tmp= buf + sizeof(struct phdr);
		int msgsz=0, i, j;

		if (result==RETSUCCESS) {
			setnetbuf(tmp, short, repl.cammax);
			setnetbuf(tmp, char, repl.devtype);
			setnetbuf(tmp, int, repl.cammask);
			setnetbuf(tmp, int, repl.recmask);
			setnetbuf(tmp, int, repl.covertmask);
			setnetbuf(tmp, int, repl.permmask);

			for (i=0; i < MAX_CAM; i++)
			{
				setnetbuf(tmp, int, repl.ptz[i].ptzact);
				setnetbuf(tmp, int, repl.ptz[i].ptzactext);
				setnetbuf(tmp, char, repl.ptz[i].maxsensor);
				setnetbuf(tmp, char, repl.ptz[i].maxalarm);
			}

			for (i=0; i < MAX_CAM; i++) {
				setnetbuf(tmp, char, repl.pset[i].num);

				for (j = 0; j < repl.pset[i].num; j ++) {
					setnetbuf(tmp, string, repl.pset[i].title [j]);
				}
			}

			setnetbuf (tmp, char, repl.twoway);

			msgsz = tmp - (buf + sizeof (struct phdr));
			SS_SETFLAG(strsvr, 0, __SS_AUTHED);
			return reply2client(strsvr, sd, buf, REPLYID(ph->id), result, msgsz);
		}

		reply2client(strsvr, sd, buf, REPLYID(ph->id), result, msgsz);
		return -1;	// to close socket
	}
}
#endif

int handle_auth_request(struct st_stream *strsvr, ST_PHDR phdr) {
	int sd;
	int ret=0;

	sd = strsvr->sock;
	assert (sd >= 0);

	switch (phdr.id) {
		case PUSRENCAUTH:
		case PUSRENCAUTH_REINF:
			break;

		case PUSRAUTH:
			{
				ret = handle_usrauth_sha256(strsvr, sd, &phdr);
				if (ret < 0)
					return ret;
			}
			break;

		default:
			return 1;
	}
	return 0;
}
