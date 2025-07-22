#ifndef __HEOPSDK_COMMON_H__
#define __HEOPSDK_COMMON_H__

/* data type */
typedef unsigned int UINT32;
typedef int INT32;
typedef unsigned char UINT8;
typedef char INT8;

/* Errno */
#define HEOPSDK_OPERATE_OK				0x0
#define HEOPSDK_OPERATE_REBOOT			0x1
#define HEOPSDK_OPERATE_FAIL			0x2
#define HEOPSDK_OPERATE_PARAM_ERROR		0x3
#define HEOPSDK_OPERATE_NOT_SUPPORT		0x4

#define ERROR -1
#define OK 0


#define FIRM_UPG_PERCENT			100

typedef enum
{
	FIRM_UPG_STEP_INIT				= 0x00,			
	FIRM_UPG_STEP_PRESET			= 0x10,			
	FIRM_UPG_STEP_SLAVE				= 0x20,			
	FIRM_UPG_STEP_BEGIN				= 0x30,			
	FIRM_UPG_STEP_READ				= 0x40,		
	FIRM_UPG_STEP_DECODE			= 0x50,		
	FIRM_UPG_STEP_WRITE				= 0x60,	
	FIRM_UPG_STEP_EXPORT			= 0x70,
	FIRM_UPG_STEP_OVER				= 0xff,
}SYS_UPG_STEP_E;


typedef struct
{
	UINT32							iMagicNum;		/* The state of the system structure of magic */
	UINT32							iCheckSum;		/* System state UINT8 sum check */
	UINT32							iStatSize;		/* System state structure data length */
	UINT32							iStatRes[1];	/* System state structure data alignment */
	UINT8							bUpgFlgStep;	/* System upgrade read, write, backup step identification */
	UINT8							bStpPercent;	/* System upgrade each step progress percentage */
	UINT8							bUpgPercent;	/* Total progress percentage of system upgrade */
	UINT8							bStatRes[45];	/* System upgrade flag reserved byte */
}SYS_UPG_STAT_T;

typedef struct
{
	SYS_UPG_STAT_T					tUpgStat;		/* System upgrade status information */
	struct timespec					tStatSpec;		/* System status update time */
	struct timespec					tUpgTmBeg;		/* System upgrade development time */
	struct timespec					tUpgTmEnd;		/* System upgrade end time */
	INT32							iHeartCnt;		/* System heartbeat disconnection statistics */
	UINT32							iResU32[8];
	UINT8							bPreRatio;		/* Total system pre upgrade schedule */
	UINT8							bPrePercent;	/* System pre update progress percentage */
	UINT8							bSlaveRatio;	/* Total proportion of progress from the system upgrade */
	UINT8							bSlavePercent;	/* Progress percentage from system upgrade */
}SYS_UPG_REPORT_T;

typedef enum {
	NONE_LED = 0,
	RED_LED = 1,
	GREEN_LED = 2,
	ALL_LED = 3,
}HEOPSDK_LED_MODE;


typedef enum {
	ICR_DAY_SDK = 0,
	ICR_NIGHT_SDK = 1,
}HEOPSDK_ICR_MODE;



typedef struct
{
	INT8	hik_firmware_version[32];		///< hikvision app version
	INT8	heopSdk_version[32];			///< hik libsdk version
	INT8	heopSdk_serialNo[10];			///< hik serial Number
	UINT32	devType;						///1:Cylinder machine,2:hemisphere;3:error	
	INT8	heopSdk_logis[20];				///< logistics codes
}HEOPSDK_VERSION;


typedef int (*pfun_upg_read)(int iFirmOff, void *pReadBuf, int iReadLen, void *pReadArg);

typedef int (*pfun_upg_stat)(SYS_UPG_REPORT_T *pUpgReport, void *pStatArg);




/**@ brief:  HEOP SDK libarbry initation
*@ return 
*/
void heopSdk_lib_init();


/* get version and serial Number baseline version description 
*@ libVersion: return hik version, sdk version ,serial Number 
* @return HEOPSDK_OPERATE_OK, success, other failed.
*/
INT32 heopSdk_sys_get_version(HEOPSDK_VERSION *libVersion);


/**@ brief: Control LED switch,  
*@ mode: RED_LED: turn to red   GREEN_LED: turn to green  ORANGE_LED: turn to orange  NONE_LED: turn to none
* @return HEOPSDK_OPERATE_OK, success, other failed.
*/
INT32 heopSdk_led_status_ctrl(HEOPSDK_LED_MODE led_mode);


/**@ brief: Get the status of the reset Button 
*@ p_status: return the status by this variable value, 0 for down, 1for up.
* @return HEOPSDK_OPERATE_OK, success, other failed.
*/
INT32 heopSdk_get_reset_button_status(INT32* p_status);


/**@ brief: put icr mode ,  
*@ mode: 0 for day mode, 1 for night mode
* @return HEOPSDK_OPERATE_OK, success, other failed.
*/
INT32 heopSdk_set_icr_mode(HEOPSDK_ICR_MODE icr_mode);

/**@ brief: put ir status ,  
*@ mode: 0 ~ 100 for ir_pwm
* @return HEOPSDK_OPERATE_OK, success, other failed.
*/
INT32 heopSdk_set_ir_status(INT32 ir_pwm);


/**@ brief: This interface for the firmware upgrade, you can refer to the demo program to achieve ftp, HTTP, cloud storage, local 
	storage (our demo program to achieve the way) and other ways to achieve the device firmware upgrade function.
*@ pFnUpgRead: Read data callback interface, demo program is to read the local data, you can change the file descriptor socket
	to read data from the network to read more extensive way to achieve data reading.Cannot be NULL.
*@ pArgFnRead: Read function callback interface to input, this parameter can be customized by the user, of course, need can guarantee the 
file descriptor or socket network socket into easy to read, can also be achieved in their own way, which can make reading data interface can get data can be.
*@ pFnUpgStat: Upgrade state feedback callback interface, the callback interface will upgrade the progress of information feedback to the interface by the 
caller, the caller interface to external processing, such as web or above display platform upgrade and so on the progress of.
*@ pArgFnStat:Upgrade state feedback callback interface requires the parameters, the user can customize the NULL.
*@ iFirmSize: To upgrade the firmware file size, file size required by the specific interface implementation program to obtain the caller.
*@ return: HEOPSDK_OPERATE_REBOOT, Success; other failed
 */
INT32 heopSdk_firmware_upgrade(pfun_upg_read pFnUpgRead, void* pArgFnRead, pfun_upg_stat pFnUpgStat, void *pArgFnStat,UINT32 iFirmSize);

/**@ brief: Get the status of the photosensitive 
*@ deglitch_bypass: 0 or 1 deglitch_bypass
*@ p_status: return the status by this variable value
* @return HEOPSDK_OPERATE_OK, success, other failed.
*/
INT32 heopSdk_get_adc_status(INT32* p_status, INT32 deglitch_bypass);

#endif
