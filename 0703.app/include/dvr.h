#ifndef __DVR_H
#define __DVR_H

//#ifndef CAM_MAX
//#define CAM_MAX					2
//#endif

#ifndef FPS_MAX
#define FPS_MAX					120
#endif

#ifndef AUDIN_MAX
#define AUDIN_MAX				1
#endif

#ifndef DST
#define DST						1
#endif

#define SENSOR_MAX				1				// Number of sensor input.
#define RELAYOUT_MAX			1

#define SENSOR_TEMPER_BIT		1	// 1 << (SENSOR_MAX)
#define SENSOR_TEMPER_CAM		0


#define SENSOR_PRESET_MAX		16

#define GDTOUR_MAX				10				// Max number of guard tour points.
#define GDTOUR_SEC_MAX			30

#define RDXHDR_MINSZ			240 *16


#define MAX_SUB_FPS				MAX_CH_FPS
#define DEFAULT_SUB_FPS			15

#define _SUPPORT_SENSOR_SWITCH_ 1

#define __VALIDRANGE(var,min,max)	((min) <= (var) && (var) < (max))
#define VALIDCH(ch)					__VALIDRANGE(ch, 0, CAM_MAX)
#define VALIDSENSORCH(ch)			__VALIDRANGE(ch, 0, SENSOR_MAX)

#define _USE_SET_SOCKET_BUFFER_SZ_			1
#if _USE_SET_SOCKET_BUFFER_SZ_
	#define _USE_SET_SOCKET_SND_BUFFER_SZ_		1
	#define _USE_SET_SOCKET_RCV_BUFFER_SZ_		1
#endif

/* Device driver */
#define DEV_SD_MEM_BASE	"/dev/mmcblk0"
#define DEV_SD_MEM		"/dev/mmcblk0p1"
#define DEV_WIFI_DRIVER	"/edvr/rt5370sta.ko"
#define DEVICE_RS232_0	"/dev/ttyUSB0"
#define DEVICE_RS232_1	"/dev/ttyUSB1"

#define DEVICE_RS485		"/dev/tts/1"
#define DEVICE_SENSOR		"/dev/dio"
#define ETHERNET_DEV		"eth0"


#define UPNPC_PATH			"/edvr/upnpc"

/* Video input signal. */
#define SYS_NTSC				0x00
#define SYS_PAL					0x01



#define foreach_cam_ch(ch)			for ((ch) = 0; (ch) < CAM_MAX; (ch)++) {
#define foreach_sensor_ch(ch)		for ((ch) = 0; (ch) < SENSOR_MAX; (ch) ++) {
#define end_foreach					}


#define AUDIO_SAMP_RATE			812
#define AUDIO_BITS_RATE			16

// for hour table
#define DM_RESOL 				6					// Time in minutes a data mark representing.
#define DM_TBLSZ 				(24 * (60/DM_RESOL))


#define MAGIC_NUM_1				0x77782877

// ========== Netsvr ===========
// Maximum number of clients can connect same time.
#define	FRAMES_PER_GOP			16						// Number of frames in a GOP.

#define	MAX_VOD_FRAME_IN_LIST	(5)	//FRAMES_PER_GOP 	 //Maximum number of frame buffer can be linked to a stream server.
#define MAX_JPG_FRAME_IN_LIST	(5)


#ifndef USE_NEW_RTSP
#define __USE_RTSP_SERVER__
#endif

#define	__POS__		0





// #define _USE_WEBSOCKET_SERVER_ 				1
// #ifdef _USE_WEBSOCKET_SERVER_
	#define __USE_WEBSOCKET_VIEWER__		1	
	#define __USE_WS_PROXY_SERVER__			1	
	#define __USE_WEBSOCKET_BY_TCP__		1	
	#define _USE_WEB_SERVICE_BY_TCP_PORT_		1
// #else
	// #define __USE_VMS_VIEWER__ 				0 	// mini-uvms
// #endif


enum { REBOOT_TIME = 70 };

#if (AI == 2)
#define AI_BOARD
#endif

#endif
