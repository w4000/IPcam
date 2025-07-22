
#ifndef MAIN_H_
#define MAIN_H_

#include "setup/setup.h"
#include "setup/setup_encryption.h"
#include "setup/setup_ini.h"
#include "device/tty.h"
#include "device/flash.h"
#include "device/quaddev.h"
#include "device/codecdev.h"
#include "device/vo.h"

#define DEV_MCU				(IPCAM_THREAD_MAIN->tty1)   	
#define DEV_MTD				(IPCAM_THREAD_MAIN->flash)	
#define DEV_QUAD			(IPCAM_THREAD_MAIN->quad)
#define DEV_CODEC			(IPCAM_THREAD_MAIN->codec)

#define CAMERA_EXIST(ch)	(g_setup.cam[ch].rec)
#define MAX_SHM_NUM			(2) // CAM_MAX


typedef struct _MY_SHM_INFO
{
	char			shm_name[32];
	int				shm_size;
	int				shm_fd;
	unsigned char*  shm_ptr;

} MY_SHM_INFO;

enum
{
	THREAD_ALIVE_CHECK_MAIN = 0,
	THREAD_ALIVE_CHECK_NETSVR,
	THREAD_ALIVE_CHECK_TIMER,
	THREAD_ALIVE_CHECK_FILE,
	THREAD_ALIVE_CHECK_MAX
};

typedef struct _DVRMAIN {

	_CAM_THREAD_COMMON	th;					// This has to be on top.

	int bQuit;

	char			usrname [USRNAME_MAX +1];	// Login user name.

	/*----------------------- Events & notification --------------------------*/
	unsigned 		m_sensor;						// Bitwise sensor state. 0 : normal stat   1: Alarm stat.

	unsigned 		mot;						// Bitwise motion state for dwell

	unsigned 		vloss;
	boolean			diskfull;
	boolean			recfail;

	unsigned 		alarm;						// Bitmask of current sensor alarmed cammera.

	char			relayforce [RELAYOUT_MAX];	// Relay out by force (by remote action)
	__u64			relayend [RELAYOUT_MAX];	// Relay out end time.

	/*----------------- Recording on/off status -----------*/
	boolean			rec;
	boolean			emg_rec;
	boolean			rec_force;					
	unsigned 		on_rec;						


	/*-------------------- System info. -------------------*/
	__u8			mac [6];					// Network device MAC address.
	__u8			sw_ver[3];					// [0]-Relase [1]-Major [2]-Minor.
	__u8			MCU_ver[3];


	/*-------------------- system flag -------------------*/
	__u8			fixed_modified;				// modified flag (fixed system information) for CMS
	__u8			on_config;					// show status of menu configuration panel
	__u8			usage_cpu;					// usage cpu (percent)
	int				free_mem;					// free memory size (bytes)

	unsigned		system_fps;					// System default FPS

	ST_TTYDEV		tty1;				
	ST_RELAY		relay;					
	ST_FLASH		flash;					
	ST_QUADDEV		quad;					
	ST_CODECDEV		codec;						
	struct tm		dst_start;
	struct tm		dst_end;

	pthread_mutex_t rtsp_mutex;

	MY_SHM_INFO		alive_info;
	MY_SHM_INFO		pos_info_1;
	MY_SHM_INFO		pos_info_2;

	int				isUpgrade;
	int				isNetUpgrade;

	int				isDHCP;
	int				thread_alive_check[THREAD_ALIVE_CHECK_MAX];	
	int				alive_check_ok;

	int				thread_work_check[THREAD_ALIVE_CHECK_MAX];

} _CAMMAIN;

extern _CAMMAIN	g_main;


#endif /* MAIN_H_ */
