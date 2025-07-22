#include "common.h"
#include <fcntl.h>
#include <unistd.h>

#include "lib/threadsafe.h"
#include "lib/net.h"

#include "setup/setup_ext.h"
#include "setup_ini.h"

_CAMSETUP	g_setup;	
_CAMSETUP g_tmpset;	
_CAMSETUP_EXT	g_setup_ext;
_CAMSETUP_EXT	g_tmpset_ext;


#define SETUP_USR(setup, i)					(& (setup)->usr[i])

static pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;

__u16 enc_fps_max[2] = {30, 25};
static void __write_actioninfo(int command, int param)
{
	int fd;
	char str[4];

	remove_file(ACTION_INFO, 0);

	if((fd = open(ACTION_INFO, O_RDWR |O_CREAT, 0644)) == -1) {
		perror ("open");
		return;
	}

	sprintf(str, "%03d", command);
	write(fd, str, sizeof(str));


	char cmd[128];
	sprintf (cmd, "echo \"file:%s, func:%s, line: %d > str? %d \" | cat >> /root/uplog.txt", __FILE__, __func__, __LINE__, atoi(str));
	system (cmd);

	close (fd);
}



int setup_export (_CAMSETUP * setup, _CAMSETUP_EXT* setup_ext)
{
#ifdef USE_SETUP_ENCRYPTION
#else
	FILE *fp = NULL, *fp_ext = NULL;
	assert (setup);
#if defined(USE_ENCRYPT_CMD_LINE)
	const char* setup_bak_file = DEC_TMP_SETUPFILE;
	const char* setup_ext_bak_file = DEC_TMP_SETUPFILE_EXT;
#else
	const char* setup_bak_file = CGI_SETUPFILE_BAK;
	const char* setup_ext_bak_file = CGI_SETUPFILE_EXT_BAK;
#endif	/* USE_ENCRYPT_CMD_LINE */

	
	if (file_exist (setup_bak_file))
		remove_file (setup_bak_file, 0);
		
	if ((fp = fopen (setup_bak_file, "w+")) == NULL) {
		perror ("fopen");
		return -1;
	}
	
	fwrite ((void *) setup, 1, sizeof (_CAMSETUP), fp);
	fsync (fp);
	fclose (fp);

#if defined(USE_ENCRYPT_CMD_LINE)
	int ret = encrypt_file(CGI_SETUPFILE_BAK, setup_bak_file);
	if(ret < 0) {
		unlink(setup_bak_file);
	}
	else {
	}
#endif	/* USE_ENCRYPT_CMD_LINE */
	
	__copyfile(CGI_SETUPFILE_BAK, CGI_SETUPFILE);
	
	if (file_exist(setup_ext_bak_file))
		fp_ext = fopen(setup_ext_bak_file, "r+b");
	else
		fp_ext = fopen(setup_ext_bak_file, "wb");
	
	if (fp_ext)
	{
		
		fseek(fp_ext, 0L, SEEK_SET);
		fwrite ((void *) setup_ext, 1, sizeof (_CAMSETUP_EXT), fp_ext);
		fsync(fp_ext);
		fclose(fp_ext);
		
#if defined(USE_ENCRYPT_CMD_LINE)
	ret = encrypt_file(CGI_SETUPFILE_EXT_BAK, setup_ext_bak_file);
	if(ret < 0) {
		unlink(setup_ext_bak_file);
	}
	else {
	}
#endif	/* USE_ENCRYPT_CMD_LINE */
		
		__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
		
	}
#endif
	return 0;
}

int setup_save (ST_FLASH * dev, _CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
	
	assert (dev && setup);

	setup->magic = SETUP_MAGIC;
	setup->magic2 = SETUP_MAGIC2;
	setup->magic3 = SETUP_MAGIC3;
	setup->magic4 = SETUP_MAGIC4;
	setup->magic5 = SETUP_MAGIC5;
	setup->magic6 = SETUP_MAGIC6;
	setup->magic7 = SETUP_MAGIC7;
	
	setup->size = sizeof (_CAMSETUP);
	
	setup_ext->magic[0] = MAGIC_NUM_1;
	setup_ext->magic[1] = 0x00710225;
	setup_ext->magic[2] = 0x00951220;
	setup_ext->magic[3] = 0x00981029;
	setup_ext->sizeOfStruct = sizeof(_CAMSETUP_EXT);
	
	pthread_mutex_lock (& mutex);

	if (dev->write_fs (dev, setup, setup_ext) < 0) {
		pthread_mutex_unlock (& mutex);
		return -1;
	}

	pthread_mutex_unlock (& mutex);

	setup_export (setup, setup_ext);

	return 0;
}

int setup_check_valid (_CAMSETUP * setup)
{
	assert (setup);

#if 1
	if ((setup->magic != SETUP_MAGIC || setup->magic2 != SETUP_MAGIC2 || setup->magic3 != SETUP_MAGIC3 || setup->magic4 != SETUP_MAGIC4 || setup->magic5 != SETUP_MAGIC5 || setup->magic6 != SETUP_MAGIC6 || setup->magic7 != SETUP_MAGIC7) || (setup->size != sizeof (_CAMSETUP))) {
		
		return 0;
	}
	else
	{
	}
#endif

	if(setup->net.ip.s_addr == 0) {
		printf("ip is 0!!!");
		return 0;
	}

	return 1;
}

static int __read_old_setup(void *data, void *data_ext)
{
	_CAMSETUP *setup = (_CAMSETUP*)data;
	_CAMSETUP_EXT *setup_ext = (_CAMSETUP_EXT*)data_ext;

	FILE *fp = NULL, *fp_ext = NULL;
	int sz = -1;
	int bIsBackupFile = 0, bIsBackupExtFile = 0;
#if defined(USE_ENCRYPT_CMD_LINE)
	int ret = -1;
	const char* setup_file = DEC_TMP_SETUPFILE;
	const char* setup_ext_file = DEC_TMP_SETUPFILE_EXT;
#else
	const char* setup_file = CGI_SETUPFILE;
	const char* setup_ext_file = CGI_SETUPFILE_EXT;
#endif

#if defined(USE_ENCRYPT_CMD_LINE)
	ret = decrypt_file(setup_file, CGI_SETUPFILE);
	if(ret < 0) {
		__copyfile(CGI_SETUPFILE_BAK, CGI_SETUPFILE);
	}
	else {
	}
#endif	/* USE_ENCRYPT_CMD_LINE */

retry:
	fp = fopen(setup_file, "rb");
	if (!fp)
	{
		if (bIsBackupFile)
		{
			perror ("fopen");
			return -1;
		}
		else
		{
			__copyfile(CGI_SETUPFILE_BAK, CGI_SETUPFILE);
			bIsBackupFile = 1;
			goto retry;
		}
	}

	sz = fread((void *)setup ,1 ,sizeof(_CAMSETUP), fp);

	if (sz != sizeof(_CAMSETUP))
	{
		if (bIsBackupFile)
		{
			perror ("fread");
			sz = -1;
			goto err;
		}
		else
		{
			fclose(fp);
			__copyfile(CGI_SETUPFILE_BAK, CGI_SETUPFILE);
			bIsBackupFile = 1;
			goto retry;
		}
	}

#if defined(USE_ENCRYPT_CMD_LINE)
	ret = decrypt_file(setup_ext_file, CGI_SETUPFILE_EXT);
	if(ret < 0) {
		__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
	}
	else {
	}
#endif	/* USE_ENCRYPT_CMD_LINE */

retry_ext:

	fp_ext = fopen(setup_ext_file, "rb");
	if (!fp_ext)
	{
		if (!bIsBackupExtFile)
		{
			__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
			bIsBackupExtFile = 1;
			goto retry_ext;
		}
	}
	else
	{
		int bValidCheck = 0;
		int magic2[4], size2 = 0;
		memset(magic2, 0, sizeof(magic2));
		fread(magic2, 1, sizeof(magic2), fp_ext);
		if (magic2[0] == MAGIC_NUM_1 && magic2[1] == 0x00710225 && magic2[2] == 0x00951220 && magic2[3] == 0x00981029) {
			fread(&size2, 1, sizeof(int), fp_ext);
			if (size2 > 0) {
				int sz2, len2 = min(size2, sizeof(_CAMSETUP_EXT)) - sizeof(magic2) - sizeof(int);
				if (len2 > 0)
				{
					sz2 = fread(&(setup_ext->reserved), 1, len2, fp_ext);
					if (sz2 == len2) {
						memcpy(setup_ext->magic, magic2, sizeof(magic2));
						setup_ext->sizeOfStruct = size2;

						bValidCheck = 1;
					}
				}
			}
		}

		if (!bValidCheck)
		{
			if (!bIsBackupExtFile)
			{
				fclose(fp_ext);
				__copyfile(CGI_SETUPFILE_EXT_BAK, CGI_SETUPFILE_EXT);
				bIsBackupExtFile = 1;
				goto retry_ext;
			}
		}
	}

err:
	if (fp)
		fclose(fp);
	if (fp_ext)
		fclose(fp_ext);

	if(setup_check_valid (setup) == 0)
		sz = -1;


	return sz;
}


int setup_check_no_encryption(_CAMSETUP * setup, _CAMSETUP_EXT * setup_ext)
{
	if(__read_old_setup(setup, setup_ext) < 0) {
		return -1;
	}

	return 0;
}
int setup_load (ST_FLASH * dev, _CAMSETUP * setup, _CAMSETUP_EXT * setup_ext)
{
	
	assert (dev);
	assert (setup);
#if 1
	
	setup_ext_default(setup_ext);
	

	
	if (dev->read_fs (dev, setup, setup_ext) < 0)
	{
		return -1;
	}
#else
#endif

	int i;
	
	for (i = 0; i < 3; i++) {
		setup->evnt_mot[i].onoff = TRUE;	

		SIZE_S size;
		def_get_picsize(1, \
				setup->cam[i].res, &size);
		
		int max_width = 1920;	
		int max_height = 1080;
			

		if(size.u32Width > max_width || size.u32Width < 2UL
				|| size.u32Height > max_height || size.u32Height < 2UL) {
			return -1;
		}

	};

	
	if (setup_check_valid (setup))
		return 0;
	else
		return -1;
}

void setup_default_datetime (_CAMSETUP *setup) {


#if 0
#else
#ifdef USE_DF2_SUPPORT
	
	setup->datefmt = FORMAT_YYYYMMDD;
	setup->timefmt = FORMAT_24H;
	setup->tmzone = 0;
#else
	setup->datefmt = FORMAT_YYYYMMDD;
	setup->timefmt = FORMAT_24H;
	setup->tmzone = 0;
#endif
#endif

#if (DST == 1)
	setup->dst = 0;
	setup->dst_stm.tm_year = 2008 - 1900;
	setup->dst_stm.tm_mon = 0;
	setup->dst_stm.tm_mday = 1;
	setup->dst_stm.tm_hour = 0;
	setup->dst_stm.tm_min = 0;
	setup->dst_stm.tm_sec = 0;

	setup->dst_etm.tm_year = 2008 - 1900;
	setup->dst_etm.tm_mon = 0;
	setup->dst_etm.tm_mday = 1;
	setup->dst_etm.tm_hour = 0;
	setup->dst_etm.tm_min = 0;
	setup->dst_etm.tm_sec = 0;

	setup->dst_status = 0;
#endif

	setup->update_tm = FALSE;

	setup->tmosd.usage = 0;
	setup->tmosd.position = 0;
	setup->tmosd.color = 0;
}


void setup_default_system (_CAMSETUP *setup, int keep_account) {


	int i, j;

	assert (setup);
	setup->sysfps = FPS_MAX;


	setup->lang = 0;
	setup->videotype = 0;

	sprintf (setup->mac_addr, "00:00:00:00:00:00");	// default mac address


#ifdef USE_DF2_SUPPORT
	setup->tmsync.usage = TMS_NONE;
	setup->tmsync.syncycle = 24;

#else
#endif


	if(keep_account == 0) {
		setup->usr[0].avail = TRUE;
		sprintf (setup->usr[0].usrname, setup_ini_get_str("cam_setup", "SYSTEM_USER_NAME", "admin"));
#ifdef USE_DF2_SUPPORT
		sprintf (setup->usr[0].usrpwd, setup_ini_get_str("cam_setup", "SYSTEM_USER_PASSWD", "1234"));
#else
		sprintf (setup->usr[0].usrpwd, setup_ini_get_str("cam_setup", "SYSTEM_USER_PASSWD", "1234"));
#endif
		sprintf (setup->usr[0].usrdesc, "administrator");

		setup->usr[0].uperm.live = 1;
		setup->usr[0].uperm.ptz = 1;
		setup->usr[0].uperm.setup = 1;
		setup->usr[0].uperm.reset = 1;
		setup->usr[0].uperm.vod = 1;
	}

	for (i = 1; i < 10; i++)
		setup->usr[i].avail = FALSE;


	for (i = 0; i < 3; i++) {
		setup->evnt_mot[i].onoff = TRUE;	
		setup->evnt_mot[i].dwell = 5;
	};

}

void __setup_default_cam (_CAMSETUP *setup) {
	int i;


	for (i = 0; i < 3; i ++) {
		sprintf (setup->cam [i].title, "STREAM_%d", i + 1);

		setup->cam [i].title_osd.usage = 0;
		setup->cam [i].title_osd.position = 1;	// bottom-left
		setup->cam [i].title_osd.color = 0;
#ifdef USE_DF2_SUPPORT
		setup->cam [i].sens = 50;
#else
		setup->cam [i].sens = DEF_SENSITIVITY;
#endif
		memset (setup->cam [i].area, 1, sizeof (setup->cam [i].area));
	}


	setup->cam [0].bitrate = BITRATE_5000;
	setup->cam [1].bitrate = BITRATE_750;
	setup->cam [2].bitrate = BITRATE_400;

}


void setup_default_cam (_CAMSETUP *setup) {
	int i, j;

	assert (setup);
	foreach_cam_ch (i)
	static const PIC_SIZE_E __default_resol[] = { PIC_HD1080, PIC_VGA, PIC_QVGA };

#ifdef USE_DF2_SUPPORT
		setup->cam [i].rec = TRUE;
		setup->cam [i].codec = 0;
		// if (i==2)
		// 	setup->cam [i].codec = 2;

		int resol_idx = __default_resol[i];
		
		setup->cam [i].res = resol_idx;
		setup->cam [i].ratectrl = RATECTRL_VBR;
		setup->cam [i].fps = 30;		
		setup->cam [i].gop = 30;		
		setup->cam [i].sen_fps = 30;
		setup->cam [i].mot_fps = 30;
		setup->cam [i].mot_qty = 1;

		setup->cam [i].datakeepdays = 0;
		for (j = 0; j < 8; j++) {
			sprintf (setup->cam [i]._preset_title [j], "PRESET%d", j + 1);
		}
#else
		setup->cam [i].rec = TRUE;//(i==0 ? TRUE : FALSE);								// on/off
		setup->cam [i].codec = 0; //0:H264, 1:H265
		setup->cam [i].res = __default_resol[i];
		setup->cam [i].ratectrl = RATECTRL_VBR;
		setup->cam [i].fps = 30;			// frame per sec
		setup->cam [i].gop = 30;
		setup->cam [i].sen_fps = 30;
		setup->cam [i].mot_fps = 30;
		setup->cam [i].mot_qty = 1;


		setup->cam [i].datakeepdays = 0;

		for (j = 0; j < 8; j++) {
			sprintf (setup->cam [i]._preset_title [j], "PRESET%d", j + 1);
		}


#endif

	setup->cam [i].ratectrl = 1;


	end_foreach

	__setup_default_cam (setup);
}

void setup_default_net (_CAMSETUP *setup, int keep_ip) {
	int i;
	assert (setup);


#ifdef USE_DF2_SUPPORT
	if (!keep_ip)
	{

#if 0
#else
		if (medel_spec_value_comp("NET_DHCP", "TRUE"))
			setup->net.type = NETTYPE_DHCP;
		else 
			setup->net.type = NETTYPE_LAN;

		// setup->net.type = NETTYPE_LAN;
		// printf("ip? %s\n", setup_ini_get_str("cam_setup", "NET_IPADDR", "NULL"));
		// printf("mask? %s\n", setup_ini_get_str("cam_setup", "NET_SUSBNET", "NULL"));
		// printf("gw? %s\n", setup_ini_get_str("cam_setup", "NET_GW", "NULL"));

		inet_aton(setup_ini_get_str("cam_setup", "NET_IPADDR", "192.168.1.100"), & setup->net.ip);
		inet_aton(setup_ini_get_str("cam_setup", "NET_SUSBNET", "255.255.255.0"), & setup->net.netmask);
		inet_aton(setup_ini_get_str("cam_setup", "NET_GW", "192.168.1.1"), & setup->net.gateway);
		inet_aton(setup_ini_get_str("cam_setup", "NET_GW", "192.168.1.1"), & setup->net.gateway);


		if (strcmp(setup_ini_get_str("cam_setup", "NET_DHCP", "FALSE"), "TRUE") == 0)
			setup->net.type = NETTYPE_DHCP;

#endif

		setup->net.port_tcp = 8001;
		setup->net.port_web =  (int)atoi(setup_ini_get_str("cam_setup", "WEB_PORT", "90"));
		printf("set default web port: %d \n", setup->net.port_web);
	}

	setup->net.speed = 0;
#else
	
	
	if (!keep_ip)
	{
		setup->net.type = NET_TYPE_DEFAULT;

		inet_aton(IP_DEFAULT, & setup->net.ip);
		inet_aton(NM_DEFAULT, & setup->net.netmask);
		inet_aton(GW_DEFAULT, & setup->net.gateway);

		setup->net.port_tcp = 8001;
		setup->net.port_web = 80;
	}

	setup->net.speed = 0;
#endif
	// printf("net type? %d, ini str? %s \n", setup->net.type, setup_ini_get_str("cam_setup", "NET_DHCP", "NULL"));
}

void setup_load_default (_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext, int vsig, int keep_ip) {//w4000_default

	assert (setup);
	
	if (keep_ip)
	{
		
		_CAMSETUP save;
		memcpy(&save, setup, sizeof(_CAMSETUP));

		memset (setup, 0, sizeof (_CAMSETUP));

		if (keep_ip & 1) {
			setup->net.type = save.net.type;
			memcpy(& setup->net.ip, &save.net.ip, sizeof(struct in_addr));
			memcpy(& setup->net.netmask, &save.net.netmask, sizeof(struct in_addr));
			memcpy(& setup->net.gateway, &save.net.gateway, sizeof(struct in_addr));
			setup->net.port_tcp = save.net.port_tcp;
			setup->net.port_web = save.net.port_web;
#ifdef LIGHTTPD
#endif
		}

		if (keep_ip & (1<<1)) {
			int i;
			for(i = 0; i < 10; i++) {
				memcpy(&setup->usr[i], &save.usr[i], sizeof(ST_USR));
			}
		}
	}
	else
		memset (setup, 0, sizeof (_CAMSETUP));

	
	setup_default_system (setup, (keep_ip & (1<<1))?1:0);
	
	
	setup_default_cam (setup);
	
	setup_default_net (setup, (keep_ip & 1)?1:0);
	
	setup_default_datetime (setup);
	
	setup_ext_default (setup_ext);
	
	setup_default_nsetup(&setup->nsetup);
	

	// make setup folder
	mkdir (FLASH_SETUPPATH, 0755);

#if defined(USE_ENCRYPT_CMD_LINE)
	// Decrypt setup file
	{
		const char* cmdRemoveSetup = "rm /tmp/_setup*";
		system(cmdRemoveSetup);
	}
#endif



#ifdef USE_NEW_ONVIF
	remove(ONVIF_SETUP_FILE);
	scope_file_remove();
#endif

}

void setup_load_default_netskip (_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext, int vsig) {

	setup_load_default(setup, setup_ext, vsig, 1);

}


void setup_apply (_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext) {
	assert (setup);

	setup_network_apply (setup, setup_ext, 1, 1, 0);

	if (setup->net.type == NETTYPE_DHCP) {
		setup_save (& DEV_MTD, setup, setup_ext);
	}

	setup_mac_apply(setup);

	g_tmpset = *setup;	
}



int setup_find_usrid_by_name (_CAMSETUP *set, char *name) {
	int ret;
	int i;

	assert (set);
	assert (name && strlen(name) < USRNAME_MAX);

	ret = -1;
	for (i = 0; i < 10; i++) {
		if (! USR_AVAIL (& set->usr [i]))
			continue;

		if (strncmp (USR_NAME (& set->usr [i]), name, USRNAME_MAX) == 0) 
			ret = i;

	}

	return ret;
}

ST_USR	* setup_find_usr_by_name (_CAMSETUP *set, char *name) {
	ST_USR * ret;
	int i;

	assert (set);
	assert (name && strlen(name) < USRNAME_MAX);

	ret = NULL;
	for (i = 0; i < 10; i++) {
		if (! USR_AVAIL (& set->usr [i]))
			continue;

		if (strncmp (USR_NAME (& set->usr [i]), name, USRNAME_MAX) == 0) 
			ret = & set->usr [i];
	}

	return ret;
}


void setup_datetime_apply (_CAMSETUP * set, struct tm * tm, int byset) {

	//time_t 	t1;
	assert (set);
	assert (tm);
	printf("%s \r\n",__func__);//w4000

	if (tm) {
		if (byset == 0)	
			datetime_tzset (set->tmzone);

		system("date");

		do {

			struct tm tm_ptr;
			time_t m_time;

			tm_ptr.tm_year  	= tm->tm_year;
			tm_ptr.tm_mon   	= tm->tm_mon;
			tm_ptr.tm_mday  	= tm->tm_mday;
			tm_ptr.tm_hour  	= tm->tm_hour;
			tm_ptr.tm_min   	= tm->tm_min;
			tm_ptr.tm_sec   	= tm->tm_sec;
			tm_ptr.tm_isdst 	= -1;//tm->tm_isdst;

			//printf("TIME HOUR: %d, %d\n", tm_ptr.tm_hour, tm->tm_hour);
			m_time = mktime(&tm_ptr);
			//printf("TIME HOUR: %d\n", tm_ptr.tm_hour);


			system("date");

			if (stime(&m_time) < 0) {
				break;
			}

			datetime_systohc();

			system("date");

		} while(0);

		send_msg(NETSVR, TIMER, _CAMMSG_TM_RESTSRT, 1, 0, 0);	
	}
}

void setup_device_cam_apply (_CAMSETUP * set) {

	assert (set);
}


void setup_device_cam_motdet_apply (_CAMSETUP * set) {
	int i;

	assert (set);

	for (i = 0; i < 3; i ++) {
		// TODO
	}
}

// Boa web service port
void setup_network_webport(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext)
{
#ifndef LIGHTTPD
	char cmd [256];

	sprintf (cmd, "/bin/rm -fR %s", WEBCONF_FILE);
	__system (cmd);

	sprintf (cmd, "cat %s >> %s", WEBCONF_BASE_FILE, WEBCONF_FILE);
	__system (cmd);

	sprintf (cmd, "echo \"SinglePostLimit %lu\" >> %s", (unsigned long)(1024*1024*30), WEBCONF_FILE);
	__system (cmd);

	sprintf (cmd, "echo \"Port %d\" >> %s", setup.net.port_web, WEBCONF_FILE);
	__system (cmd);

	int fd, param = 0;
	char str[4];
	if (file_exist(ACTION_INFO)) {
		fd = open(ACTION_INFO, O_RDWR);
		if (read(fd, str, 4) != 0)
			param = atoi(str);
	}
	if (param != ACTIONINFO_UPGRADE_START)
		__system ("killall -9 boa");

	__system ("boa -c /etc/boa");

	debug("Starting boa webserver : Port(%d)\n", setup.net.port_web);
#endif

#ifdef USE_NEW_ONVIF
#if 0
	printf("!!!!!!!!!!!!!!!!!!		not start onvif >>> test \n");	
#else 
	onvifserver_init(setup, setup_ext);
#endif 
#endif

#ifdef LIGHTTPD
	int http_port = setup->net.port_web;
	
#if 1
	int ret = start_webserver(http_port);
	if(ret < 0) {
		printf("Failed to restart webserver");
	}
#else 
	printf("!!!!!!!!!!!!!!!!!!		not start web server >>> test \n");	
#endif	
#else
#endif

	restart_network_related_app(setup);
}

void restart_network_related_app(_CAMSETUP *setup)
{

#ifndef USE_NEW_ONVIF
	system("killall -SIGINT discovery");
	system("killall -9 discovery");
	system("/edvr/discovery &");
#endif


#ifdef SUPPORT_SIP
	// TODO: 설정 적용
	start_sip();
#endif
}


int setup_network_static_apply(_CAMSETUP * set)
{
	int retry = 0;
	int ret = 0;
	char buf [256];
	FILE * fp = NULL;
	char ip [32], netmask [32], gw [32];
	char get_ip[32];
	struct in_addr ip_s;

	// Set interface.
	ip [31] = '\0';
	netmask [31] = '\0';
	gw [31] = '\0';

	strncpy (ip, inet_ntoa (set->net.ip), 31);
	strncpy (netmask, inet_ntoa (set->net.netmask), 31);
	strncpy (gw, inet_ntoa (set->net.gateway), 31);


	if_set_net (ETHERNET_DEV, ip, netmask, gw);

	// Retry
	retry = 0;
	do {
		ret = if_set_net (ETHERNET_DEV, ip, netmask, gw);
		retry++;
		usleep(100*1000);

		if(if_get_ip(ETHERNET_DEV, &ip_s) == 0) {
			sprintf(get_ip, "%s", inet_ntoa(ip_s));
			if(strncmp(ip, get_ip, 32) != 0) {
				ret = -1;
			}
		}
	} while(ret < 0 && retry < 5);

	if(retry > 1) {
	}



	return ret;
}

// Apply IP, netmask, gateway to system by DHCP
int setup_network_dhcp_apply (_CAMSETUP * set) {

	FILE * fp = NULL;
	char cmd [1024], buf [1024];

	char str[32];
	char * p1, * p2 = NULL;

	if (!file_exist("/usr/share/udhcpc/netinfo"))
	{
		{
			char ip[32];
			ip[31] = '\0';
			strncpy (ip, inet_ntoa (set->net.ip), 31);
		}

		return setup_network_static_apply(set);
	}

	sprintf (cmd, "cat /usr/share/udhcpc/netinfo");	// leases info for dhcp

	int dns_check = 0;

	BEGIN_SYSTEM (fp, cmd, buf)

		memset(str, 0x00, 32);

		if (strstr(buf, "IP[") != NULL) {
			p1 = strstr(buf, "IP[") + strlen("IP[");
			p2 = strstr(buf, "]");
			strncpy (str, p1, p2 -p1);

			inet_aton (str, &set->net.ip);

		} else if (strstr(buf, "MASK[") != NULL) {
			p1 = strstr(buf, "MASK[") + strlen("MASK[");
			p2 = strstr(buf, "]");
			strncpy (str, p1, p2 -p1);

			inet_aton (str, &set->net.netmask);

		} else if (strstr(buf, "GW[") != NULL) {
			p1 = strstr(buf, "GW[") + strlen("GW[");
			p2 = strstr(buf, "]");
			strncpy (str, p1, p2 -p1);


			inet_aton (str, &set->net.gateway);

		} 
		else
			continue;


	END_SYSTEM

//out:
	if (fp) {
		fclose (fp);
		fp = NULL;
	}

	{
		char ip[32];
		char gw[32];
		strncpy(ip, inet_ntoa(set->net.ip), 31);
		strncpy(gw, inet_ntoa(set->net.gateway), 31);
		if(if_set_gateway(ETHERNET_DEV, gw) < 0) {
			if(if_set_gateway(ETHERNET_DEV, ip) < 0) {
				return 0;
			}
		}
	}
	return 0;
}


int setup_mac_apply (_CAMSETUP *set) {
	sprintf(set->mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X",
    			 IPCAM_THREAD_MAIN->mac [0],
    			 IPCAM_THREAD_MAIN->mac [1],
    			 IPCAM_THREAD_MAIN->mac [2],
    			 IPCAM_THREAD_MAIN->mac [3],
    			 IPCAM_THREAD_MAIN->mac [4],
    			 IPCAM_THREAD_MAIN->mac [5]);

	return 0;
}


int setup_network_final_apply (_CAMSETUP * set, _CAMSETUP_EXT * set_ext, int wifi_update)
{

	assert(set);
	assert(set_ext);

	
	setup_network_webport(set, set_ext);
	

	return 0;
}

int setup_network_apply (_CAMSETUP * set, _CAMSETUP_EXT * set_ext, int wifi_update, int bForce, int user)
{
	
	assert (set);

	int ret = 0;
#ifdef USE_NEW_ONVIF
	onvifserver_deinit();
#endif


	


	if(set->net.speed == 0 && get_net_auto_negotiation() == 0) {
		set_net_speed(set->net.speed);
		ret = 0;

		// goto End;
	}

	if(set->net.speed == 1 && get_net_speed() != 10) {
		set_net_speed(set->net.speed);
	}

	
	if (set->net.type == NETTYPE_LAN)
	{
		stop_dhcp_client();
		
		ret = setup_network_static_apply(set);
		if (ret < 0)
			goto End;
		setup_network_final_apply(set, set_ext, wifi_update);
		
		// if(user) {
		// 	char ip[32], gw[32], netmask[32];
		// 	ip [31] = '\0';
		// 	netmask [31] = '\0';
		// 	gw [31] = '\0';

		// 	strncpy (ip, inet_ntoa (set->net.ip), 31);
		// 	strncpy (netmask, inet_ntoa (set->net.netmask), 31);
		// 	strncpy (gw, inet_ntoa (set->net.gateway), 31);

			
		// 	// printf("! %s:%d > set->net.gateway? %s\n", __func__, __LINE__, gw);

	}
	else if (set->net.type == NETTYPE_DHCP)
	{
		g_main.isDHCP = 1;
		run_dhcp_client(&set->net.ip, 5);
		g_main.isDHCP = 0;

		_CAMSETUP	tmpset = *set;
		setup_network_dhcp_apply (&tmpset);

		if (bForce || tmpset.net.ip.s_addr != set->net.ip.s_addr)
		{
			if(!bForce) {
				char old_ip[32];
				char new_ip[32];
				old_ip [31] = '\0';
				new_ip [31] = '\0';
				strncpy (new_ip, inet_ntoa (tmpset.net.ip), 31);
				strncpy (old_ip, inet_ntoa (set->net.ip), 31);
			}

			*set = tmpset;
			setup_network_final_apply(set, set_ext, wifi_update);
		}
		else {
			ret = -1;
			goto End;
		}

		if(user) {
		}
	}




End:
	return ret;
}


static int fd_lock(int fd)
{
	struct flock lock;

	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	return fcntl(fd, F_SETLKW, &lock);
}

static int fd_unlock(int fd)
{
	struct flock lock;

	lock.l_type = F_UNLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	return fcntl(fd, F_SETLK, &lock);
}

static int g_fd = -1;
void setup_lock()
{
	g_fd = open(CGI_SETUPFILE_LOCK, O_CREAT|O_RDWR, 644);
	fd_lock(g_fd);
}

void setup_unlock()
{
	if(g_fd < 0) {
		return;
	}
	fd_unlock(g_fd);
	close(g_fd);
	g_fd = -1;
}

#define AES_KEY_BIT 128
#define AES_KEY_BYTE 16
#define AES_BLOCK_SIZE 16

static unsigned char aes_key[AES_KEY_BYTE] = "ecam!!!";
#define ENCSETSIZE(sz) ((((sz)/AES_BLOCK_SIZE)+1)*AES_BLOCK_SIZE)

typedef struct _SETUP_AES_HEADER_S {
	unsigned int magic;
	unsigned int src_size;
	unsigned int enc_size;
	unsigned int opt;
} SETUP_AES_HEADER_S;

int setup_enc(void *src, void *dst, int src_size)
{
	AES_KEY key;
	unsigned char iv_aes[AES_BLOCK_SIZE] = {0};

	SETUP_AES_HEADER_S *hdr = (SETUP_AES_HEADER_S *)dst;
	void *enc = dst + sizeof(SETUP_AES_HEADER_S);
	int enc_size = 0;
	int dst_size = 0;

	enc_size = ENCSETSIZE(src_size) + 256;
	dst_size = enc_size + sizeof(SETUP_AES_HEADER_S);
	memset(dst, 0, dst_size);

	hdr->magic = 0x79761109;
	hdr->src_size = src_size;
	hdr->enc_size = enc_size;
	hdr->opt = 0;

	memset(iv_aes, 0, sizeof(iv_aes));
	AES_set_encrypt_key(aes_key, AES_KEY_BIT, &key);
	AES_cbc_encrypt(src, enc, enc_size, &key, iv_aes, AES_ENCRYPT);

	return dst_size;
}

int setup_dec(void *src, void *dst, int dst_size)
{
	AES_KEY key;
	unsigned char iv_aes[AES_BLOCK_SIZE] = {0};

	SETUP_AES_HEADER_S *hdr;
	void *enc;

	hdr = (SETUP_AES_HEADER_S *)src;
	enc = src + sizeof(SETUP_AES_HEADER_S);

	if(hdr->magic != 0x79761109) {
		return -1;
	}

	memset(iv_aes, 0, sizeof(iv_aes));
	AES_set_decrypt_key(aes_key, AES_KEY_BIT, &key);
	AES_cbc_encrypt(enc, dst, hdr->enc_size, &key, iv_aes, AES_DECRYPT);

	return hdr->src_size;
}

#if defined(USE_ENCRYPT_CMD_LINE)
int generate_decrypted_file(_CAMSETUP * setup, _CAMSETUP_EXT * setup_ext)
{
	FILE *fp = fopen(DEC_TMP_SETUPFILE, "wb");
	int ret = -1;
	if (fp == NULL) {
		perror ("fopen");
		goto err;
	}

	int sz = fwrite((void *)setup , 1, sizeof(_CAMSETUP), fp);
	if (sz != sizeof(_CAMSETUP)) {
		perror ("fwrite");
		goto err;
	}

	FILE *fp_ext = fopen(DEC_TMP_SETUPFILE_EXT, "wb");
	if (fp_ext == NULL) {
		perror ("fopen");
		goto err;
	}

	sz = fwrite((void *)setup , 1, sizeof(_CAMSETUP_EXT), fp_ext);
	if (sz != sizeof(_CAMSETUP_EXT)) {
		perror ("fwrite");
		goto err;
	}

	ret = 0;
err:
	if(fp != NULL) {
		fclose(fp);
	}
	fp = NULL;

	if(fp_ext != NULL) {
		fclose(fp_ext);
	}
	fp = NULL;

	return ret;
}
#endif	/* USE_ENCRYPT_CMD_LINE */
