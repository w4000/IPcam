#include "common.h"
#ifndef	__MDIN3xx_H__
#include	 "mdin3xx.h"
#endif

static MDIN_VIDEO_INFO		stVideo;
static MDIN_INTER_WINDOW	stInterWND;

#if	defined(SYSTEM_USE_MDIN325A)&&defined(SYSTEM_USE_MCLK202)
//--------------------------------------------------------------------------------------------------------------------------
static void MDIN3xx_SetHCLKMode(MDIN_HOST_CLK_MODE_t mode)
{
#if 0
	switch (mode) {
		case MDIN_HCLK_CRYSTAL:	TEST_MODE2( LOW); TEST_MODE1( LOW); break;
		case MDIN_HCLK_MEM_DIV: TEST_MODE2(HIGH); TEST_MODE1(HIGH); break;
	}
#endif
}
#endif

//--------------------------------------------------------------------------------------------------------------------------
static void CreateMDIN325VideoInstance(void)
{
	WORD nID = 0;
#if	defined(SYSTEM_USE_MDIN325A)&&defined(SYSTEM_USE_MCLK202)
	MDIN3xx_SetHCLKMode(MDIN_HCLK_CRYSTAL);	// set HCLK to XTAL
	MDINDLY_mSec(10);	// delay 10ms
#endif

	while (nID!=0x85) MDIN3xx_GetChipID(&nID);	// get chip-id
	MDIN3xx_EnableMainDisplay(OFF);		// set main display off
	MDIN3xx_SetMemoryConfig();			// initialize DDR memory

#if	defined(SYSTEM_USE_MDIN325A)&&defined(SYSTEM_USE_MCLK202)
	MDIN3xx_SetHCLKMode(MDIN_HCLK_MEM_DIV);	// set HCLK to MCLK/2
	MDINDLY_mSec(10);	// delay 10ms
#endif

	MDIN3xx_SetVCLKPLLSource(MDIN_PLL_SOURCE_XTAL);	// set PLL source
	MDIN3xx_EnableClockDrive(MDIN_CLK_DRV_ALL, ON);

	MDIN3xx_SetInDataMapMode(MDIN_IN_DATA24_MAP3);	// set in-data map
	MDIN3xx_SetDIGOutMapMode(MDIN_DIG_OUT_M_MAP5);	// main digital out

	// setup enhancement
	MDIN3xx_SetFrontNRFilterCoef(NULL);		// set default frontNR filter coef
	MDINAUX_SetFrontNRFilterCoef(NULL);		// set default frontNR filter coef
	MDIN3xx_SetPeakingFilterCoef(NULL);		// set default peaking filter coef
	MDIN3xx_SetColorEnFilterCoef(NULL);		// set default color enhancer coef
	MDIN3xx_SetBlockNRFilterCoef(NULL);		// set default blockNR filter coef
	MDIN3xx_SetMosquitFilterCoef(NULL);		// set default mosquit filter coef
	MDIN3xx_SetColorTonFilterCoef(NULL);		// set default colorton filter coef

	MDIN3xx_EnableLTI(OFF);					// set LTI off
	MDIN3xx_EnableCTI(OFF);					// set CTI off
	MDIN3xx_SetPeakingFilterLevel(0);		// set peaking gain
	MDIN3xx_EnablePeakingFilter(ON);		// set peaking on
	MDIN3xx_EnableColorEnFilter(ON);		// set color enhancement on

	MDIN3xx_EnableFrontNRFilter(OFF);		// set frontNR off
	MDIN3xx_EnableBWExtension(OFF);			// set B/W extension off

	MDIN3xx_SetIPCBlock();		// initialize IPC block (3DNR gain is 34)

	memset((PBYTE)&stVideo, 0, sizeof(MDIN_VIDEO_INFO));

	MDIN3xx_SetMFCHYFilterCoef(&stVideo, NULL);	// set default MFC filters
	MDIN3xx_SetMFCHCFilterCoef(&stVideo, NULL);
	MDIN3xx_SetMFCVYFilterCoef(&stVideo, NULL);
	MDIN3xx_SetMFCVCFilterCoef(&stVideo, NULL);

	// set aux display ON
	stVideo.dspFLAG = MDIN_AUX_DISPLAY_ON | MDIN_AUX_FREEZE_OFF;

	// set video path (main/aux/dac/enc)
	stVideo.srcPATH = PATH_MAIN_A_AUX_A;	// set main is A, aux is A
	stVideo.dacPATH = DAC_PATH_MAIN_OUT;	// set main only
	stVideo.encPATH = VENC_PATH_PORT_X;		// set venc is aux

	// if you need to front format conversion then set size.
//	stVideo.ffcH_m  = 1440;

	// define video format of PORTA-INPUT
	stVideo.stSRC_a.frmt = VIDSRC_1920x1080p60;
	//stVideo.stSRC_a.mode = MDIN_SRC_SEP422_8;
	stVideo.stSRC_a.mode = MDIN_SRC_EMB422_8;
	stVideo.stSRC_a.fine = MDIN_FIELDID_INPUT | MDIN_LOW_IS_TOPFLD;
	stVideo.stSRC_a.offH = 0;
	stVideo.stSRC_a.offV = 0;

	// define video format of MAIN-OUTPUT
	stVideo.stOUT_m.frmt = VIDOUT_1920x1080p60;
	//stVideo.stOUT_m.frmt = VIDOUT_1920x1080p30;
	//stVideo.stOUT_m.frmt = VIDOUT_1280x720p60;
	//stVideo.stOUT_m.mode = MDIN_OUT_RGB444_8;
	stVideo.stOUT_m.mode = MDIN_OUT_YUV444_8;

	stVideo.stOUT_m.fine = MDIN_SYNC_FREERUN;	// set main outsync free-run

	stVideo.stOUT_m.brightness = 128;			// set main picture factor
	stVideo.stOUT_m.contrast = 128;
	stVideo.stOUT_m.saturation = 128;
	stVideo.stOUT_m.hue = 128;

#if RGB_GAIN_OFFSET_TUNE == 1
	stVideo.stOUT_m.r_gain = 128;				// set main gain/offset
	stVideo.stOUT_m.g_gain = 128;
	stVideo.stOUT_m.b_gain = 128;
	stVideo.stOUT_m.r_offset = 128;
	stVideo.stOUT_m.g_offset = 128;
	stVideo.stOUT_m.b_offset = 128;
#endif

	// define video format of IPC-block
	stVideo.stIPC_m.mode = MDIN_DEINT_ADAPTIVE;
	stVideo.stIPC_m.film = MDIN_DEINT_FILM_OFF;
	stVideo.stIPC_m.gain = 34;
	stVideo.stIPC_m.fine = MDIN_DEINT_3DNR_ON | MDIN_DEINT_CCS_ON;

	// define map of frame buffer
	stVideo.stMAP_m.frmt = MDIN_MAP_AUX_ON_NR_ON;	// when MDIN_DEINT_3DNR_ON
//	stVideo.stMAP_m.frmt = MDIN_MAP_AUX_ON_NR_OFF;	// when MDIN_DEINT_3DNR_OFF

	// define video format of PORTB-INPUT
	stVideo.stSRC_b.frmt = VIDSRC_720x480i60;
	//stVideo.stSRC_b.mode = MDIN_SRC_MUX656_8;
	stVideo.stSRC_b.mode = MDIN_SRC_YUV444_8;
	stVideo.stSRC_b.fine = MDIN_FIELDID_INPUT | MDIN_LOW_IS_TOPFLD;

	// define video format of AUX-OUTPUT
	stVideo.stOUT_x.frmt = VIDOUT_720x480i60;
	//stVideo.stOUT_x.frmt = VIDOUT_1280x720p30;
	stVideo.stOUT_x.mode = MDIN_OUT_MUX656_8;
	//stVideo.stOUT_x.mode = MDIN_OUT_YUV444_8;
	stVideo.stOUT_x.fine = MDIN_SYNC_FREERUN;	// set aux outsync free-run

	stVideo.stOUT_x.brightness = 128;			// set aux picture factor
	stVideo.stOUT_x.contrast = 128;
	stVideo.stOUT_x.saturation = 128;
	stVideo.stOUT_x.hue = 128;

#if RGB_GAIN_OFFSET_TUNE == 1
	stVideo.stOUT_x.r_gain = 128;				// set aux gain/offset
	stVideo.stOUT_x.g_gain = 128;
	stVideo.stOUT_x.b_gain = 128;
	stVideo.stOUT_x.r_offset = 128;
	stVideo.stOUT_x.g_offset = 128;
	stVideo.stOUT_x.b_offset = 128;
#endif

	// define video format of video encoder
	stVideo.encFRMT = VID_VENC_NTSC_M;

	// define window for inter-area
	stInterWND.lx = 315;	stInterWND.rx = 405;
	stInterWND.ly = 90;		stInterWND.ry = 150;
	MDIN3xx_SetDeintInterWND(&stInterWND, MDIN_INTER_BLOCK0);
	MDIN3xx_EnableDeintInterWND(MDIN_INTER_BLOCK0, OFF);

	stVideo.exeFLAG = MDIN_UPDATE_MAINFMT;	// execution of video process
}

int mdin_csc_set(void)
{
	WORD csc_value;
	csc_value |= 0x1;
	MDINHIF_MultiWrite(MDIN_LOCAL_ID, 0x019, (PBYTE)&csc_value, 2);
}

int turn_colorbar(int isOn)
{
	static int pattern_idx = MDIN_OUT_TEST_DISABLE;

	if (isOn == 1) {
		pattern_idx++;
		if (pattern_idx > MDIN_OUT_TEST_BLUE) {
			pattern_idx = MDIN_OUT_TEST_WHITE;
		}
	}
	else {
		pattern_idx = MDIN_OUT_TEST_DISABLE;
	}

	MDIN3xx_SetOutTestPattern(pattern_idx);
}

int colorbar_for_test(int isOn)
{
	MDIN3xx_HardReset();
	CreateMDIN325VideoInstance();				// initialize MDIN-325
	MDIN3xx_VideoProcess(&stVideo);		// mdin3xx main video process
	turn_colorbar(isOn);
	MDIN3xx_EnableMainDisplay(ON);
	mdin_csc_set();
	g_setup_ext.mdin_vo_setup.colorbar_status = isOn;

	return 0;
}


int convert_idx_to_res_id(int index)
{
	switch (index)
	{
		case 0 :
			return VIDOUT_720x480i60; 		// 720x480i 60Hz
		case 1 : 
			return VIDOUT_720x576i50;			// 720x576i 50Hz
		case 2 : 	
			return VIDOUT_1280x720p60;		// 1280x720p 60Hz
		case 3 :
			return VIDOUT_1280x720p30;		// 1280x720p 30Hz
		case 4 :
			return VIDOUT_1920x1080p60;		// 1920x1080p 60Hz
		case 5 :
			return VIDOUT_1920x1080p50;		// 1920x1080p 50Hz
	}
	return VIDOUT_1920x1080p30;
}

int change_resolution(int idx)
{
	MDIN3xx_HardReset();
	CreateMDIN325VideoInstance();				// initialize MDIN-325
	stVideo.stOUT_m.frmt = convert_idx_to_res_id(idx);
	g_setup_ext.mdin_vo_setup.resolution = idx;
	//printf("[%s:%d] resolution = %d\n", __FILE__, __LINE__, g_setup_ext.mdin_vo_setup.resolution);

	MDIN3xx_VideoProcess(&stVideo);		// mdin3xx main video process
	MDIN3xx_EnableMainDisplay(ON);
	mdin_csc_set();
}



int set_vo_quality(void)
{
	struct _mdin_vo_setup* pvo_setup = &g_setup_ext.mdin_vo_setup;

	MDIN3xx_SetPictureBrightness(&stVideo, pvo_setup->brightness);
	MDIN3xx_SetPictureContrast(&stVideo, pvo_setup->contrast);
	MDIN3xx_SetPictureSaturation(&stVideo, pvo_setup->saturation);
	MDIN3xx_SetPictureHue(&stVideo, pvo_setup->hue);
	//printf("[%s:%d][hue:%d][brightness=%d][contrast=%d][saturation=%d]\n", __FILE__, __LINE__, pvo_setup->hue, pvo_setup->brightness, pvo_setup->contrast, pvo_setup->saturation);
	return 0;
}


int apply3DNR(int isEnable)
{
	if (isEnable != 0) {
		isEnable = 1;
	}

	MDIN3xx_HardReset();

	CreateMDIN325VideoInstance();				// initialize MDIN-325

	//printf("[%s:%d][fine = %d(%d)]\n", __FILE__, __LINE__, stVideo.stIPC_m.fine, (stVideo.stIPC_m.fine & MDIN_DEINT_3DNR_ON));
	stVideo.stIPC_m.fine &= (isEnable ^ ~MDIN_DEINT_3DNR_ON);
	stVideo.stIPC_m.fine |= (isEnable & MDIN_DEINT_3DNR_ON);
	//printf("[%s:%d][fine = %d(%d)]\n", __FILE__, __LINE__, stVideo.stIPC_m.fine, (stVideo.stIPC_m.fine & MDIN_DEINT_3DNR_ON));

	MDIN3xx_VideoProcess(&stVideo);		// mdin3xx main video process

	MDIN3xx_EnableMainDisplay(ON);
	mdin_csc_set();
}


