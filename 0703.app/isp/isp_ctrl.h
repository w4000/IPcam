#ifndef __ISPCTRL_H__
#define __ISPCTRL_H__

#include "mpi_isp.h"
#include "setup/nsetup.h"

/*--------------------------------------------------------------------------------------------------
	Data types
--------------------------------------------------------------------------------------------------*/
typedef enum eISP_OP_MODE {
	OPMODE_AUTO = 0,
	OPMODE_MANUAL
} ISP_OP_MODE;

typedef enum eISP_VIDEO_MODE {
	VMODE_NTSC = 0,
	VMODE_PAL
} ISP_VIDEO_MODE;

typedef enum eISP_IRIS_MODE {
	IRIS_OPEN = 0,
	IRIS_AUTO
} ISP_IRIS_MODE;

typedef enum eISP_WB_MODE {
	WB_ATW = 0,
	WB_AWB,
	WB_PUSH,
	WB_MANUAL
} ISP_WB_MODE;

typedef enum eISP_TDN_MODE {
	TDN_AUTO = 0,		// cds w4000 gpio 8=L night, H= day
	TDN_DAY,
	TDN_NIGHT,
	TDN_AUTO_EXP,		// auto mode 
	TDN_UNKNOWN
} ISP_TDN_MODE;

typedef enum eISP_TDN_STATUS {
	TDN_STATUS_DAY = 0,
	TDN_STATUS_NIGHT,
	//TDN_STATUS_Auto,//w4000
	TDN_STATUS_UNKNOWN
} ISP_TDN_STATUS;

typedef enum eISP_GAMMA_CURVE_TYPE {
	GAMMA_CURVE_OFF = 0,
	GAMMA_CURVE_DEFAULT,
	GAMMA_CURVE_SRGB,
} ISP_GAMMA_CURVE_TYPE;

// NovaTek
typedef enum _ISP_AWB_MODE {
	ISP_AWB_MANUAL,
	ISP_AWB_AUTO,
	ISP_AWB_DAYLIGHT,
	ISP_AWB_CLOUDY,
	ISP_AWB_TUNGSTEN,
	ISP_AWB_SUNSET,
	ISP_AWB_COUNT
} ISP_AWB_MODE;

/*--------------------------------------------------------------------------------------------------
	Functions
--------------------------------------------------------------------------------------------------*/
int isp_update_iq();
int isp_update_ae();
int isp_update_blc();
int isp_update_awb();
int isp_update_dn();

extern int isp_init(void);
extern void isp_exit(void);
extern int is_isp_auto();
extern int is_run_isp();


extern int isp_wb_auto_mode(ISP_WB_MODE mode, unsigned char rstrength, unsigned char bstrength); 	//
extern int isp_wb_manual_mode(ISP_WB_MODE mode, char rgain, char grgain, char gbgain, char bgain);				//
extern int isp_wb_saturation(unsigned char value);

extern int isp_blc_weight(unsigned char *weight);
extern int isp_blc_wdr(int enable, int level);

extern int isp_iris_opmode(ISP_OP_MODE opmode);
extern int isp_exposure_brightness(unsigned char bright);
#ifdef HKARCH
extern int isp_exposure_shutter_speed(int shutter, int shutter_speed1, int shutter_speed2, int antiflicker_freq, ISP_VIDEO_MODE ntsc_pal);
#else
extern int isp_exposure_shutter_speed(int shutter, int shutter_speed1, int shutter_speed2, ISP_VIDEO_MODE ntsc_pal);
#endif
extern int isp_exposure_slow_shutter(unsigned char slow);					
extern int isp_exposure_gain(unsigned char gain);							

extern int isp_tdn_mode(ISP_TDN_MODE mode);
extern int isp_ircut_mode(char mode);	

extern int isp_dnr_2dnr(char on);
extern int isp_dnr_3dnr(char on);
extern int isp_antifog(char on);									
extern int isp_sharpness(ISP_OP_MODE mode, unsigned char value);
extern int isp_gamma(HI_BOOL bEnable, ISP_GAMMA_CURVE_TYPE gamma); 				
extern int isp_drc(HI_BOOL bEnable, ISP_OP_MODE mode, unsigned char value);

#endif /* __ISPCTRL_H__ */
