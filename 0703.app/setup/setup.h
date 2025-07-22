
#ifndef __SETUP_H
#define __SETUP_H


#include "device/tty.h"
#include "./sensor.h"
#include "./relay.h"
#include "device/flash.h"
#include "device/quaddev.h"
#include "device/codecdev.h"
#include "push/pushevt.h"
//#include "hi_comm_isp.h"
#include "nsetup.h"

#define SETUP_SZ_MAX		(128*K)

#if 1
#define CGI_UPFILE_PATH  		"/edvr"
#define CGI_SETUPFILE_LOCK		"/mnt/ipm/setup/lock"
#define CGI_SETUPFILE 			"/mnt/ipm/setup/CAMSETUP.DAT"
#define CGI_SETUPFILE_EXT		"/mnt/ipm/setup/CAMSETUP2.DAT"
#define CGI_SETUPFILE_BAK		"/mnt/ipm/setup/CAMSETUP.BAK"
#define CGI_SETUPFILE_EXT_BAK	"/mnt/ipm/setup/CAMSETUP2.BAK"
#define CGI_UPGRADE 			"/mnt/ipm/ipmupdate"
#define ACTION_INFO 			"/mnt/ipm/actioninfo"
#define FAIL_INFO 				"/mnt/ipm/failinfo"

#ifdef USE_NEW_ONVIF
#define ONVIF_SETUP_FILE		"/mnt/cam/setup/onvif_setup.xml"
#endif
#else 
#endif




#if defined(USE_ENCRYPT_CMD_LINE)
#define	DEC_TMP_SETUPFILE		"/tmp/_setup.dat"
#define	DEC_TMP_SETUPFILE_EXT	"/tmp/_setup_ext.dat"
#endif	/* USE_ENCRYPT_CMD_LINE */
#define __copyfile(src, dst)	{ char __cmd[256]; sprintf(__cmd, "cp %s %s", src, dst); system(__cmd); }




enum
{
	ACTIONINFO_NONE,
	ACTIONINFO_REBOOT = 5, //FIX
	ACTIONINFO_RESTART = 7,//FIX
	ACTIONINFO_UPGRADE_DOWNLOAD_DONE = 9,//FIX
	ACTIONINFO_UPGRADE_START,
	ACTIONINFO_UPGRADE_SUCCESS,
	ACTIONINFO_UPGRADE_FAIL,
	ACTIONINFO_UPGRADE_ONVIF,



	ACTIONINFO_MAX
};


/* boa web service config file */
#define WEBCONF_BASE_FILE		"/edvr/scripts/boa.conf.script"
#define WEBCONF_FILE			"/etc/boa/boa.conf"



#if 0
#define USRNAME_ADMIN			"admin"
#define DEFAULT_ADMIN_PWD		"11111111"
#else
#define USRNAME_ADMIN			"admin"	// Default admin user can not be deleted.
#define DEFAULT_ADMIN_PWD		"1234"
#endif





#define USRNAME_MAX				10
#define USRPWD_MAX				64
#define USRPWD_ENC_MAX			64
#define USRDESC_MAX 			10
#define RMDISK_MAX				32

#define WIFI_MAX				10

typedef struct _rmdisk_info
{
	boolean 	isthere;
	char 	name[RMDISK_MAX + 1];
	int		totsz;							// total disk size
	int		freesz;							// free disk size
	char		stat;
} ST_RMDISK_INFO;

typedef struct _usr {
	boolean		avail;

	char		usrname [USRNAME_MAX +1];
	char		usrpwd [USRPWD_MAX +1];
	char		usrdesc [USRDESC_MAX +1];

	struct _uperm {
		unsigned	live: 1;
		unsigned	ptz: 1;
		unsigned	setup: 1;
		unsigned	reset: 1;
		unsigned 	vod: 1;
	} uperm;

} ST_USR;


#define USR_AVAIL(usr)			((usr)->avail)
#define USR_NAME(usr)			((usr)->usrname)
#define USR_DESC(usr)			((usr)->usrdesc)
#define USR_PWD(usr)			((usr)->usrpwd)



#define VALID_PRESET(n)	((0 < n) && (n <= 255))

// properties
enum {
	BRIGHTNESS_CTRL = 0,
	CONTRAST_CTRL,
	SATURATION_CTRL,
	HUE_CTRL,
};


// Scheduling
#define WEEKDAYS		7
#define DAYHOURS		24

enum WEEK_DAY {
	W_SUN,
	W_MON,
	W_TUE,
	W_WED,
	W_THU,
	W_FRI,
	W_SAT,
	W_HOL,
};

enum SCHED {
	SCHED_OFF  = 0, 	// no recording.
	SCHED_CONT,			// continuous recording.
};



#define PRESET_TITLE_MAX 	32
#define CAMTITLE_MAX	16
#define HOLYDAYS_MAX	128

typedef	struct _caminfo {
	boolean			rec;					
	unsigned		codec;						
	unsigned 		res;						// Resolution
	unsigned        video;
	unsigned        video_mode;
	unsigned        video_ch;
	unsigned		fps;						// Recording fps
	unsigned 		qty;						// Recording quality.
	unsigned		gop;						// Recording GOP

	unsigned 		ratectrl;					// VBR or CBR
	unsigned 		bitrate;					// Target Bitrate

	boolean			sen_ints;				
	unsigned		sen_fps;

	boolean			mot_ints;					
	unsigned		mot_fps;
	unsigned 		mot_qty;

	unsigned		dwell_sensor_pre;			
	unsigned		dwell_mot_pre;				

	char			wsched [WEEKDAYS][24];		
	char			hsched [24];				
	int				cl_id[32];
	char			cl_id_name[32][1024];
	unsigned		datakeepdays;				
	int				mfz_cl_info[16][1024];
	char			title [CAMTITLE_MAX +1];	// Cammera title.
	struct _title_osd {
		unsigned	usage;
		unsigned 	position;
		unsigned 	color;
	} title_osd;


	unsigned 		reserved[100];
	char			_preset_title [8][PRESET_TITLE_MAX];	
	unsigned		sens;							
	__u8			area [15][16];	
	int				ptz_cl_info[16][1024];
} ST_CAMINFO;


typedef struct _event {
	boolean	onoff;						// 0: off, non-zero : on
	int		relay;						// Port number to relay out.
	unsigned dwell;						// Relay out dwell.
} ST_EVNT;



typedef	ST_EVNT ST_EVNT_MOT;

typedef	struct _net_notify {
	unsigned	poweron : 1;
	unsigned	shutdown : 1;
	unsigned	upgrade : 1;
	unsigned	vloss : 1;
	unsigned	relay : 1;
	unsigned	sensor : 1;
	unsigned	motion :1;
	unsigned	SMART : 1;
	unsigned	recfail : 1;
	unsigned	diskfull : 1;
	unsigned	logon_fail :1;
}	ST_NET_NOTIFY;

typedef struct	_lan_notify {
	boolean			onoff;
	struct in_addr	dest_ip [5]; 
	unsigned 		dest_port [5];
	ST_NET_NOTIFY	notify [5];
}	ST_LAN_NOTIFY;


#define SETUP_MAGIC		(0x7872777+OEM_ID)
#define SETUP_MAGIC2		(0x7307202)
#define SETUP_MAGIC3		(0x7375782)
#define SETUP_MAGIC4		(0x7677712)
#define SETUP_MAGIC5		(0x7577792)
#define SETUP_MAGIC6		(0x7477752)
#define SETUP_MAGIC7		(0x7877712)

#define SITEID_MAX			255
#define ID_MAX			32

#define HOLIDAYDESC_MAX		32

/* Date and Time display format. */
#define FORMAT_AMPM			0
#define FORMAT_24H			1
#define FORMAT_AMPM_FRONT	2

#define FORMAT_MMDDYYYY		0	// 05/01/2011
#define FORMAT_DDMMYYYY		1	// 01/05/2011
#define FORMAT_YYYYMMDD		2	// 2011/05/01
#define FORMAT_mmDDYYYY		3	// MAY 01, 2011
#define FORMAT_DDmmYYYY		4	// 01 MAY, 2011
#define FORMAT_YYYYmmDD		5	// 2011, MAY 01

/* tmsync mode */
#define TMS_NONE				0
// #define TMS_SVR					1
#define TMS_NTPSVR				2

/* Network connection type. */
#define	NETTYPE_NONE		0
#define NETTYPE_LAN			1
#define NETTYPE_DHCP		2
#define NETTYPE_MODEM		3

#define IPADDR_MAX			32
#define DDNSSVR_MAX			32
#define NTPSVR_MAX			32

// #define ADDR_DEFAULT		"0.0.0.0"
#define ADDR_DEFAULT		"192.168.10.100"
// #define NET_TYPE_DEFAULT	NETTYPE_LAN
#define NET_TYPE_DEFAULT	NETTYPE_DHCP

#if 0
#else 
#define IP_DEFAULT			"192.168.10.100"
#endif 
#define GW_DEFAULT			"192.168.10.1"
#define NM_DEFAULT			"255.255.255.0"
//w4000 #define CAM_NAME			"IP42"

#define USR_MAX 10
#define SETUP_USR_DESC(setup, i)			(USR_DESC (& (setup)->usr[i]))
#define SETUP_USR_PWD(setup, i)				(USR_PWD (& (setup)->usr[i]))

typedef struct _CAMSETUP 
{
	unsigned		magic;						
	unsigned		magic2;
	unsigned		magic3;
	unsigned		magic4;
	unsigned		magic5;
	unsigned		magic6;
	unsigned		magic7;
	__u8			sw_ver [3];					
	__u8			hw_ver [3];					
	unsigned		size;						

 	unsigned 		sysfps;						


	unsigned		lang;						
	char			mac_addr[32];				
	unsigned		videotype;					



	ST_CAMINFO		cam [CAM_MAX];

	struct holyday {
		int 		avail;
		int 		mon;						
		int 		day;						
		char		desc[HOLIDAYDESC_MAX +1];	
	} holyday [HOLYDAYS_MAX];

	unsigned		datefmt, timefmt;			
	unsigned		tmzone;						
	unsigned		dst;						

	struct tm		dst_stm, dst_etm;			
	int				dst_status;					

	struct tm		sv_tm;						
	boolean			update_tm;					

	struct _tmosd {
		int			usage;
		int			position;					
		int			color;						
	} tmosd;

	struct _status_osd {
		int			usage;
		int			color;
	} status_osd;


	struct	_tmsync {
		int				usage;					
		struct in_addr	ip;						
		char 			ntpsvr[NTPSVR_MAX +1];
		unsigned		syncycle;				
	}	tmsync;

	struct _stupgr {
		int				method;					
		struct in_addr	hostip;				
	} upgr;

	ST_USR				usr[USR_MAX];

	ST_EVNT_MOT			evnt_mot [CAM_MAX];

	unsigned			resol;

	struct _net {
		unsigned		type;
		struct in_addr	ip;
		struct in_addr	netmask;
		struct in_addr	gateway;
		struct in_addr	dns;
		struct in_addr	netip;					

		unsigned		port_tcp;				
		unsigned		port_web;				
#ifdef LIGHTTPD
		unsigned 		port_https;
#endif

		int				baud;				
		int				databit;
		int				stopbit;
		int				parbit;
		int				bandwidth;
		char			speed;				
		char			reserved_dummy[2];
		struct in_addr	dns_sub;
		unsigned int	port_pos;	
		unsigned int	port_analytics;
	} net;

	ST_LAN_NOTIFY		lan_notify;

	int					id[32];	

#ifdef NARCH
	NSETUP_T			nsetup;
#endif
	int					dummy[1024];	
} _CAMSETUP; //


typedef struct {
	unsigned			rtsp_port;
	unsigned			rtp_port;	

	int					use_rtsp;
	int					use_security;
	int					use_rtsp_security_select_set;
	char				reserved[504];

} ST_RTSP_INFO;



typedef struct _REMOTE_ADMIN_INFO {
	char				ip_addr[24];
} REMOTE_ADMIN_INFO;

typedef struct _REMOTE_ADMIN {
	int					use;
	REMOTE_ADMIN_INFO	info[USR_MAX];
} REMOTE_ADMIN;

enum {
	EXT_PORT_WEB	=	(0),
	EXT_PORT_HTTPS,
	EXT_PORT_BASE,
	EXT_PORT_RTSP,
	EXT_PORT_TIMESYNC,
	EXT_PORT_POS,
	EXT_PORT_MAX
};




#define SETUP_USR_AVAIL(setup, i)			(USR_AVAIL (& (setup)->usr[i]))
#define SETUP_USR_NAME(setup, i)			(USR_NAME (& (setup)->usr[i]))



typedef struct _setup_ext_isp
{
	char			mode;		
	struct _wb
	{
		char			mode;
		unsigned char	Saturation;
		unsigned char 	RGStrength;
		unsigned char 	BGStrength;
		char 			RGain;
		char 			GrGain;
		char 			GbGain;
		char 			BGain;
		char			reserved[30];
	} wb;

	struct _bl
	{
		char			mode;		
		char			wdr_level;
		char			reserved[30];
	} bl;

	struct _exp
	{
		char			lens_mode;
		char			shutter;
		unsigned char	brightness;	
		unsigned char	gain;
		unsigned char	shutter_speed[2];
		char			reserved[30];
	} exp;

	struct _tdn
	{
		char			mode;
		unsigned char 	delay;
		char			reserved[30];
	} tdn;

	struct _special
	{
		char			dnr;		
		unsigned char	strength;
		unsigned char	sharpness;
		char			gamma;
		char			reserved[30];
	} special;

} SETUP_EXT_ISP;

typedef struct _MD_PUSH {
	int use;
	char server_url[128];
	char server_url_name[16][128];
	char server_recv[1024];
	char server_recv_name[16][1024];
	int dwell_msec;
	int interval_msec;
} MD_PUSH;


#define SETUP_LANG(setup)					((setup)->lang)


enum {
	OSD_TITLE,
	OSD_DATETIME,
	OSD_ITEM_COUNT
};

typedef struct _OSD_ITEM {
	int enable;
	int pos_x;	
	int pos_y;	
	int color;	// 0:White, 1:Black, 2:Red, 3:Blue, 4:Green
	char text[128];
} OSD_ITEM;

typedef struct _SETUP_EXT_OSD {
	OSD_ITEM osd_item[OSD_ITEM_COUNT];
} SETUP_EXT_OSD;



enum {
	UPDATE_CGI_VIDEO,
	UPDATE_CGI_SYSTEM,
	UPDATE_CGI_TIME,
	UPDATE_CGI_USER,
	UPDATE_CGI_NET,
	UPDATE_CGI_MAX
};


enum {
	UPDATE_FCGI_ISP_IQ,
	UPDATE_FCGI_ISP_AE,
	UPDATE_FCGI_ISP_BLC,
	UPDATE_FCGI_ISP_AWB,
	UPDATE_FCGI_ISP_DN,
	UPDATE_FCGI_ISP_MAX
};


typedef struct _CAMSETUP_EXT {
	int					magic[4];		
	int 				encryption[1024];
	int					sizeOfStruct;
	int					reserved;

	int					use_mume;	

#ifdef SUPPORT_MD_PUSH
	MD_PUSH				md_push;
#endif

#ifdef USE_NEW_ONVIF
	int					onvif_port;
	int					onvif_use_https;
	int					onvif_use_discovery;
	int					_reserved[247];
#else
	int					_reserved[250];
#endif

	int					use_webgate;


	struct _pos {
		int				mode;
		int				on_flag;
		int				reserved[1024];
	} pos;


	ST_RTSP_INFO		rtsp_info;
	char				rtsp_uri[CAM_MAX][32];

	int					iframe_only_rec;
	int					onvif_security;
	int					factory_default_keep_ip;
	int					dyn_port_forward___;
	int					dyn_ip_mapping;
	int					dyn_external_ip;





	struct _serial_dev {
		int  			baudrate;		
		int  			databit;
		int  			stopbit;
		int  			parity;
		int  			flowcontrol;
		int				reserved[4];
		int				inbrk_len;
		char			inbrk[8];

	} serial_dev;


	struct _video_setup
	{
		int				r;
		int				f;
	} video_setup;

	SETUP_EXT_ISP isp;

	struct _caminfo_ext
	{
		int				Iref_mode;		
		int				encryption;		
		int				reserved[28];
	} caminfo_ext[30];

	int		motion_shutter_level;


	struct _mdin_vo_setup
	{
		unsigned int	resolution;
		unsigned int	brightness;
		unsigned int	hue;
		unsigned int	saturation;
		unsigned int	contrast;
		int				colorbar_status;
		int				dnr_status;
	} mdin_vo_setup;


	SETUP_EXT_OSD		osd;

	// update notify
	int cgi_update_notify[UPDATE_CGI_MAX];
	int fcgi_update_notify_isp[UPDATE_FCGI_ISP_MAX];
	int onvif_update_notify_isp[UPDATE_FCGI_ISP_MAX];

} _CAMSETUP_EXT; //



enum CGI_UPDATE_TYPE
{
	UPDATE_SETUP_CAM = 1,
	UPDATE_SYSTEM_UPGRADE,
	UPDATE_SYSTEM_DEFAULT,
	UPDATE_SYSTEM_LANG,
	UPDATE_SYSTEM_REBOOT,
	UPDATE_NETWORK_LINK_STATUS,

	UPDATE_ISP_IQ = 100,
	UPDATE_ISP_AE,
	UPDATE_ISP_BLC,
	UPDATE_ISP_AWB,
	UPDATE_ISP_DN,


	UPDATE_ISP,

	UPDATE_TYPE_MAX
};

// Channel encoding mode.
enum CHENCMODE	{
	ENCMODE_OFF = 0,					
	ENCMODE_SENSOR = 200,				
	ENCMODE_MOTION,					
};

enum RETECTRLMODE	{
	RATECTRL_VBR = 0,
	RATECTRL_CBR,
	RATECTRL_MAX,
};

enum BITRATEMODE	{
	BITRATE_200 = 0,
	BITRATE_300,
	BITRATE_400,
	BITRATE_500,
	BITRATE_600,
	BITRATE_700,
	BITRATE_750,
	BITRATE_900,
	BITRATE_1100,
	BITRATE_1300,
	BITRATE_1500,	// 10
	BITRATE_1800,
	BITRATE_2000,
	BITRATE_2500,
	BITRATE_3000,
	BITRATE_3500,
	BITRATE_4000,
	BITRATE_5000,
	BITRATE_6000,
	BITRATE_7000,
	BITRATE_8000,
	BITRATE_9000,
	BITRATE_10000,
	BITRATE_12000,
	BITRATE_14000,
	BITRATE_16000,
	BITRATE_MAX,
};


enum QTY {
	QTY_LOW = 0,
	QTY_STD,
	QTY_HIGH,
	QTY_HIGHEST,
	QTY_MAX
};


extern _CAMSETUP g_setup;
extern _CAMSETUP g_tmpset;
extern _CAMSETUP_EXT	g_setup_ext;
extern _CAMSETUP_EXT	g_tmpset_ext;

extern int setup_save (ST_FLASH *, _CAMSETUP *, _CAMSETUP_EXT*);
extern int setup_check_valid (_CAMSETUP *);
extern int setup_check_no_encryption(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext);
extern int setup_load (ST_FLASH *, _CAMSETUP *, _CAMSETUP_EXT*);
extern void __setup_default_cam (_CAMSETUP *);
extern void setup_ext_default (_CAMSETUP_EXT *setup_ext);
extern void set_isp_default(_CAMSETUP_EXT *setup_ext);
extern void setup_load_default (_CAMSETUP *, _CAMSETUP_EXT*, int, int);
extern void setup_load_default_netskip (_CAMSETUP *, _CAMSETUP_EXT*, int);
extern void setup_datetime_apply (_CAMSETUP *, struct tm *, int);
extern void setup_device_cam_apply (_CAMSETUP *);
extern void setup_apply (_CAMSETUP *, _CAMSETUP_EXT *);


extern void setup_network_webport(_CAMSETUP *, _CAMSETUP_EXT *);
extern void restart_network_related_app(_CAMSETUP *setup);

extern int setup_find_usrid_by_name (_CAMSETUP *set, char *name);
extern ST_USR	* setup_find_usr_by_name (_CAMSETUP *, char *);
extern int setup_network_dhcp_apply (_CAMSETUP *);
extern int setup_mac_apply (_CAMSETUP *);
extern int setup_network_final_apply (_CAMSETUP * , _CAMSETUP_EXT *, int wifi_update);
extern int setup_network_apply (_CAMSETUP *, _CAMSETUP_EXT *, int wifi_update, int bForce, int user);
extern int setup_export (_CAMSETUP *, _CAMSETUP_EXT*);
extern void setup_lock();
extern void setup_unlock();



#endif
