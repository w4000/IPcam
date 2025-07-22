//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2010  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:	APP325A.C
// Description 		:
// Ref. Docment		: 
// Revision History 	:

//--------------------------------------------------------------------------------------------------------------------------
// Note for Host Clock Interface
//--------------------------------------------------------------------------------------------------------------------------
// TEST_MODE1() is GPIO(OUT) pin of CPU. This is connected to TEST_MODE1 of MDIN3xx.
// TEST_MODE2() is GPIO(OUT) pin of CPU. This is connected to TEST_MODE2 of MDIN3xx.

//--------------------------------------------------------------------------------------------------------------------------
// You must make drive functions for I2C read & I2C write.
//--------------------------------------------------------------------------------------------------------------------------
// static BYTE MDINI2C_Write(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes)
// static BYTE MDINI2C_Read(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes)

//--------------------------------------------------------------------------------------------------------------------------
// You must make drive functions for delay (usec and msec).
//--------------------------------------------------------------------------------------------------------------------------
// MDIN_ERROR_t MDINDLY_10uSec(WORD delay) -- 10usec unit
// MDIN_ERROR_t MDINDLY_mSec(WORD delay) -- 1msec unit

//--------------------------------------------------------------------------------------------------------------------------
// You must make drive functions for debug (ex. printf).
//--------------------------------------------------------------------------------------------------------------------------
// void UARTprintf(const char *pcString, ...)

// ----------------------------------------------------------------------
// Include files
// ----------------------------------------------------------------------
#ifndef		__MDIN3xx_H__
#include	 "mdin3xx.h"
#endif

// ----------------------------------------------------------------------
// Static Global Data section variables
// ----------------------------------------------------------------------
static MDIN_VIDEO_INFO		stVideo;
static MDIN_INTER_WINDOW	stInterWND;

// ----------------------------------------------------------------------
// External Variable 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Prototype Functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Exported functions
// ----------------------------------------------------------------------

#if	defined(SYSTEM_USE_MDIN325A)&&defined(SYSTEM_USE_MCLK202)
//--------------------------------------------------------------------------------------------------------------------------
static void MDIN3xx_SetHCLKMode(MDIN_HOST_CLK_MODE_t mode)
{
#if 0	// not connected between TEST_MODE2 pin and any GPIO
	switch (mode) {
		case MDIN_HCLK_CRYSTAL:	TEST_MODE2( LOW); TEST_MODE1( LOW); break;
		case MDIN_HCLK_MEM_DIV: TEST_MODE2(HIGH); TEST_MODE1(HIGH); break;
	}
#endif	/* defined(FOCUS_MODEL) */
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
	BYTE gain_value = 32;
	BYTE offset_value = 128;
	if (stVideo.macro_mode == 1) {
		gain_value = 128;
		offset_value = 128;
	}
	stVideo.stOUT_m.r_gain = gain_value;				// set main gain/offset
	stVideo.stOUT_m.g_gain = gain_value;
	stVideo.stOUT_m.b_gain = gain_value;
	stVideo.stOUT_x.r_offset = offset_value;
	stVideo.stOUT_x.g_offset = offset_value;
	stVideo.stOUT_x.b_offset = offset_value;
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

/**
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 */
DWORD hex2int(char *hex) {
	DWORD val = 0;
	while (*hex) {
		// get current character then increment
		BYTE byte = *hex++; 
		// transform hex character to the 4bit equivalent number, using the ascii table indexes
		if (byte >= '0' && byte <= '9') byte = byte - '0';
		else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
		// shift 4 to make space for new digit, and add the 4 bits of the new digit 
		val = (val << 4) | (byte & 0xF);
	}
	return val;
}

enum {
	READ_WRITE_OFFSET = 1,
	I2C_NUM_OFFSET,
	DEV_ADDR_OFFSET,
	REG_ADDR_OFFSET,
	REG_VALUE_OFFSET,
	I2C_COMMAND_OFFSET_MAX,
};

enum {
	READ_OPER_HOST = 0,
	READ_OPER_LOCAL,
	WRITE_OPER_HOST,
	WRITE_OPER_LOCAL,
	GET_CHIP_ID,
	WRITE_SAMPLE,
	VIDEO_INIT,
	VIDEO_TEST_PATTERN,
	VIDEO_TEST_PATTERN_TEST,
	VIDEO_ONLY_PATTERN,
	SET_VOUT_RESOLUTION,
	SET_VOUT_QUALITY,
	SET_VOUT_3DNR,
	I2C_COMMAND_MAX,
};

const char* i2c_command[] = {
	"rdhost",
	"rdlocal",
	"wrhost",
	"wrlocal",
	"chid",
	"wrsamp",
	"vinit",
	"vpatt",
	"vpatt_test",
	"vponly",
	"vout_set_res",
	"vout_set_qual",
	"vout_set_3dnr",
};

WORD register_addr = 0x0085;	// chip id MDIN325A

extern BYTE i2c_id;
extern BYTE device_id;
extern WORD register_value;
extern WORD register_offset;
extern BYTE show_console;
extern BYTE addr_width;

#define	READ_BUF_SIZE	256
#define	I2C_TIME_OUT	10

void print_help(int index, int argc)
{
	printf("[%s:%d][READ] There are short arguments(%d)\n", __FILE__, __LINE__, argc);
	switch(index) {
		case GET_CHIP_ID:
		case WRITE_SAMPLE:
		case VIDEO_INIT:
		case VIDEO_TEST_PATTERN:
		case VIDEO_TEST_PATTERN_TEST:
		case VIDEO_ONLY_PATTERN:
		{
			printf("format : ./${APP_NAME} [COMMAND] [I2C Channel Number] [Device ID Number]\n");
			printf("example : ./app325a [%s or %d] [2] [0 == 0xdc(0x68)] [0...3ffH]\n", i2c_command[index], index);
		}
		break;
		case READ_OPER_HOST:
		case READ_OPER_LOCAL:
		{
			printf("format : ./${APP_NAME} [COMMAND] [I2C Channel Number] [Device ID Number] [Register Start Address] [Register End Address]\n");
			printf("example : ./app325a [%s or %d] [2] [0 == 0xdc(0x68)] [0...3ffH]\n", i2c_command[index], index);
		}
		break;
		case WRITE_OPER_HOST:
		case WRITE_OPER_LOCAL:
		{
			printf("format : ./${APP_NAME} [COMMAND] [I2C Channel Number] [Device ID Number] [Register Start Address] [Register End Address] [Register Value]\n");
			printf("example : ./app325a [%s or %d] [2] [0 == 0xdc(0x68)] [0...3ffH] [0...ffffH]\n", i2c_command[index], index);
		}
		break;
		case SET_VOUT_RESOLUTION:
		{
			printf("format : ./${APP_NAME} [COMMAND] [I2C Channel Number] [Device ID Number] [Resolution ID]\n");
			printf("example : ./app325a [%s or %d] [2] [0 == 0xdc(0x68)] [0...13H]\n", i2c_command[index], index);
		}
		break;
		case SET_VOUT_QUALITY:
		{
			printf("format : ./${APP_NAME} [COMMAND] [I2C Channel Number] [Device ID Number] [QUALITY MODE] [VALUE]\n");
			printf("example : ./app325a [%s or %d] [2] [0 == 0xdc(0x68)] [h(HUE), c(CONTRAST), b(BRIGHTNESS), s(SATURATION)] [0...ffH]\n", i2c_command[index], index);
		}
		break;
		case SET_VOUT_3DNR:
		{
			printf("format : ./${APP_NAME} [COMMAND] [I2C Channel Number] [Device ID Number] [3DNR ON/OFF]\n");
			printf("example : ./app325a [%s or %d] [2] [0 == 0xdc(0x68)] [0...1H]\n", i2c_command[index], index);
		}
		break;
		default:
		{
		}
		break;
	}
}


int apply3DNR(int isEnable)
{
	if (isEnable != 0) {
		isEnable = 1;
	}

	MDIN3xx_HardReset();
	printf("[%s:%d][SET_VOUT_3DNR]\n", __FILE__, __LINE__);

	CreateMDIN325VideoInstance();				// initialize MDIN-325
	printf("[%s:%d][Create Instance]\n", __FILE__, __LINE__);

	printf("[%s:%d][fine = %d(%d)]\n", __FILE__, __LINE__, stVideo.stIPC_m.fine, (stVideo.stIPC_m.fine & MDIN_DEINT_3DNR_ON));
	stVideo.stIPC_m.fine &= (isEnable ^ ~MDIN_DEINT_3DNR_ON);
	stVideo.stIPC_m.fine |= (isEnable & MDIN_DEINT_3DNR_ON);
	printf("[%s:%d][fine = %d(%d)]\n", __FILE__, __LINE__, stVideo.stIPC_m.fine, (stVideo.stIPC_m.fine & MDIN_DEINT_3DNR_ON));

	MDIN3xx_VideoProcess(&stVideo);		// mdin3xx main video process
	printf("[%s:%d][Process]\n", __FILE__, __LINE__);

	MDIN3xx_EnableMainDisplay(ON);
	mdin_csc_set();
}


int mdin_csc_set(void)
{
	WORD csc_value;
	csc_value |= 0x1;
	MDINHIF_MultiWrite(MDIN_LOCAL_ID, 0x019, (PBYTE)&csc_value, 2);
}


int get_quality_value(PMDIN_VIDEO_INFO pINFO)
{
	PMDIN_OUTVIDEO_INFO pOUT = (PMDIN_OUTVIDEO_INFO)&pINFO->stOUT_m;

#if 1
	stVideo.stOUT_m.brightness = 128;			// set main picture factor
	stVideo.stOUT_m.contrast = 128;
	stVideo.stOUT_m.saturation = 128;
	stVideo.stOUT_m.hue = 128;
#endif

	pOUT->saturation = stVideo.stOUT_m.saturation;
	pOUT->contrast = stVideo.stOUT_m.contrast;
	pOUT->brightness = stVideo.stOUT_m.brightness;
	pOUT->hue = stVideo.stOUT_m.hue;

	return 0;
}


//--------------------------------------------------------------------------------------------------------------------------
void main(int argc, char* argv[])
{
	BYTE q_mode = 0;
	BYTE q_value = 0;
	register_offset = 0x2;
	register_addr = 0x0;
	show_console = ON;
	WORD nID = 0;
	WORD read_buf[READ_BUF_SIZE];
	int loop = 0;
	int read_write_flag = READ_OPER_HOST;
	int index = 0;
	WORD test_value = 0;
	BYTE dnr_status = 0x7f;
	addr_width = 0x2;

	stVideo.macro_mode = 1;

	if (argc < 2) {
		printf("[%s:%d] There is no argument\n", __FILE__, __LINE__);
		return;
	}


	for (index = 0;index < I2C_COMMAND_MAX;index++) {
		if (strcmp(argv[READ_WRITE_OFFSET], i2c_command[index]) == 0) {
			read_write_flag = index;
			break;
		}
		else {
			// This is number and not string
			read_write_flag = atoi(argv[READ_WRITE_OFFSET]);
		}
	}

/* GET_CHIP_ID ./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] */
	if (read_write_flag == GET_CHIP_ID) {
		if (argc < REG_ADDR_OFFSET) {
			print_help(read_write_flag, argc);
			return;
		}
	}
/* VIDEO_INIT 				./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] */
/* VIDEO_TEST_PATTERN 		./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] */
/* VIDEO_TEST_PATTERN_TEST	./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] */
/* VIDEO_ONLY_PATTERN 		./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] */
/* WRITE_SAMPLE 			./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] */
	else if (read_write_flag == WRITE_SAMPLE || VIDEO_INIT == read_write_flag || VIDEO_TEST_PATTERN == read_write_flag || VIDEO_ONLY_PATTERN == read_write_flag || VIDEO_TEST_PATTERN_TEST == read_write_flag || read_write_flag == WRITE_SAMPLE) {
		if (argc < REG_ADDR_OFFSET) {
			print_help(read_write_flag, argc);
			return;
		}
	}
/* SET_VOUT_RESOLUTION		./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] [RESOLUTION_INDEX] */
	else if (read_write_flag == SET_VOUT_RESOLUTION) {
		if (argc < REG_VALUE_OFFSET) {
			print_help(read_write_flag, argc);
			return;
		}

		register_offset = hex2int(argv[REG_ADDR_OFFSET]);
	}
/* READ_OPER_HOST			./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] [REG_ADDR_OFFSET] [REG_VALUE_OFFSET] */
/* READ_OPER_LOCAL			./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] [REG_ADDR_OFFSET] [REG_VALUE_OFFSET] */
	else if (read_write_flag == READ_OPER_HOST || read_write_flag == READ_OPER_LOCAL) {
		if (argc < REG_VALUE_OFFSET) {
			print_help(read_write_flag, argc);
			return;
		}
		else if (argc > REG_VALUE_OFFSET) {
			register_offset = atoi(argv[REG_VALUE_OFFSET]);
		}
		register_addr = hex2int(argv[REG_ADDR_OFFSET]);
	}
/* SET_VOUT_QUALITY			./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] [QUALITY_MODE(REG_ADDR_OFFSET)] [VALUE(REG_VALUE_OFFSET)] */
	else if (read_write_flag == SET_VOUT_QUALITY) {
		if (argc < REG_VALUE_OFFSET) {
			print_help(read_write_flag, argc);
			return;
		}
		q_mode = argv[REG_ADDR_OFFSET][0];
		q_value = atoi(argv[REG_VALUE_OFFSET]);
	}
/* SET_VOUT_3DNR			./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] [3DNR ON/OFF(REG_ADDR_OFFSET)] */
	else if (read_write_flag == SET_VOUT_3DNR) {
		if (argc < REG_ADDR_OFFSET) {
			print_help(read_write_flag, argc);
			return;
		}
		dnr_status = hex2int(argv[REG_ADDR_OFFSET]);
	}
/* WRITE_OPER_HOST			./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] [REG_ADDR_OFFSET] [REG_VALUE_OFFSET] */
/* WRITE_OPER_LOCAL			./app325a [READ_WRITE_OFFSET] [I2C_NUM_OFFSET] [DEV_ADDR_OFFSET] [REG_ADDR_OFFSET] [REG_VALUE_OFFSET] */
	else {
		if (argc < I2C_COMMAND_OFFSET_MAX) {
			print_help(read_write_flag, argc);
			return;
		}
		register_addr = hex2int(argv[REG_ADDR_OFFSET]);
		register_value = hex2int(argv[REG_VALUE_OFFSET]);
	}

	i2c_id = atoi(argv[I2C_NUM_OFFSET]);
	device_id = atoi(argv[DEV_ADDR_OFFSET]);
	printf("[%s:%d][%s] i2c = 0x%x; dev = 0x%x; reg_addr = 0x%x; reg_val = 0x%x; reg_off = 0x%x\n", __FILE__, __LINE__, i2c_command[read_write_flag], i2c_id, device_id, register_addr, register_value, register_offset);
	switch (read_write_flag)
	{
		case SET_VOUT_RESOLUTION:
		case VIDEO_TEST_PATTERN_TEST:
		case VIDEO_ONLY_PATTERN:
		case VIDEO_TEST_PATTERN:
			MDIN3xx_HardReset();
			break;
	}

	switch (read_write_flag) {
		case READ_OPER_HOST:
		{
			show_console = ON;
			printf("[%s:%d][READ_OPER_HOST]\n", __FILE__, __LINE__);
			MDINHIF_MultiRead(MDIN_HOST_ID, register_addr, (PBYTE)read_buf, register_offset);
		}
		break;
		case READ_OPER_LOCAL:
		{
			show_console = ON;
			printf("[%s:%d][READ_OPER_LOCAL]\n", __FILE__, __LINE__);
			MDINHIF_MultiRead(MDIN_LOCAL_ID, register_addr, (PBYTE)read_buf, register_offset);
		}
		break;
		case WRITE_OPER_HOST:
		{
			show_console = ON;
			printf("[%s:%d][WRITE_OPER_HOST]\n", __FILE__, __LINE__);
			MDINHIF_MultiWrite(MDIN_HOST_ID, register_addr, (PBYTE)(&register_value), register_offset);
		}
		break;
		case WRITE_OPER_LOCAL:
		{
			show_console = ON;
			printf("[%s:%d][WRITE_OPER_LOCAL]\n", __FILE__, __LINE__);
			MDINHIF_MultiWrite(MDIN_LOCAL_ID, register_addr, (PBYTE)(&register_value), register_offset);
		}
		break;
		case GET_CHIP_ID:
		{
			show_console = ON;
			printf("[%s:%d][GET_CHIP_ID]\n", __FILE__, __LINE__);
			while (nID!=0x85) {
				MDIN3xx_GetChipID(&nID);	// get chip-id
				printf("[%s:%d] nID = 0x%x\n", __FILE__, __LINE__, nID);
				if (loop > I2C_TIME_OUT) break;
				loop++;
			}
		}
		break;
		case WRITE_SAMPLE:
		{
			show_console = ON;
			printf("[%s:%d][WRITE_SAMPLE]\n", __FILE__, __LINE__);
			MDINHIF_MultiWrite(MDIN_LOCAL_ID, 0x200, (PBYTE)MDIN_Deinter_Default, 0x70 * register_offset);
			MDINHIF_MultiRead(MDIN_LOCAL_ID, 0x200, (PBYTE)read_buf, 0x70 * register_offset);
		}
		break;
		case VIDEO_INIT:
		{
			show_console = ON;
			printf("[%s:%d][VIDEO_INIT]\n", __FILE__, __LINE__);
			CreateMDIN325VideoInstance();				// initialize MDIN-325
		}
		break;
		case VIDEO_ONLY_PATTERN:
		{
			show_console = OFF;
			printf("[%s:%d][!!!!!VIDEO_ONLY_PATTERN]\n", __FILE__, __LINE__);
			CreateMDIN325VideoInstance();				// initialize MDIN-325
			MDIN3xx_VideoProcess(&stVideo);		// mdin3xx main video process
			MDIN3xx_SetOutTestPattern(MDIN_OUT_TEST_COLOR);
			MDIN3xx_EnableMainDisplay(ON);
		}
		break;
		case VIDEO_TEST_PATTERN:
		{
			show_console = OFF;
			printf("[%s:%d][VIDEO_TEST_PATTERN]\n", __FILE__, __LINE__);
			CreateMDIN325VideoInstance();				// initialize MDIN-325
			printf("[%s:%d][Create Instance]\n", __FILE__, __LINE__);
			MDIN3xx_VideoProcess(&stVideo);		// mdin3xx main video process
			printf("[%s:%d][Process]\n", __FILE__, __LINE__);

			MDIN3xx_EnableMainDisplay(ON);
			mdin_csc_set();
		}
		break;
		case VIDEO_TEST_PATTERN_TEST:
		{
			show_console = OFF;
			printf("[%s:%d][VIDEO_TEST_PATTERN_TEST]\n", __FILE__, __LINE__);
			CreateMDIN325VideoInstance();				// initialize MDIN-325
			printf("[%s:%d][Create Instance]\n", __FILE__, __LINE__);
			stVideo.macro_mode = 0;
			MDIN3xx_VideoProcess(&stVideo);		// mdin3xx main video process
			printf("[%s:%d][Process]\n", __FILE__, __LINE__);

			MDIN3xx_EnableMainDisplay(ON);
			mdin_csc_set();
		}
		break;
		case SET_VOUT_RESOLUTION:
		{
			printf("[%s:%d][SET_VOUT_RESOLUTION]\n", __FILE__, __LINE__);
			show_console = OFF;
			CreateMDIN325VideoInstance();				// initialize MDIN-325
			stVideo.stOUT_m.frmt = register_offset;
			MDIN3xx_VideoProcess(&stVideo);		// mdin3xx main video process

			MDIN3xx_EnableMainDisplay(ON);
			mdin_csc_set();
		}
		break;
		case SET_VOUT_QUALITY:
		{
			show_console = OFF;
			get_quality_value(&stVideo);
			printf("[%s:%d][SET_VOUT_QUALITY] [mode = %c] [value = %d]\n", __FILE__, __LINE__, q_mode, q_value);
			if (q_mode == 'b' || q_mode == 'B') {
				MDIN3xx_SetPictureBrightness(&stVideo, q_value);
			}
			else if (q_mode == 'c' || q_mode == 'C') {
				MDIN3xx_SetPictureContrast(&stVideo, q_value);
			}
			else if (q_mode == 's' || q_mode == 'S') {
				MDIN3xx_SetPictureSaturation(&stVideo, q_value);
			}
			else if (q_mode == 'h' || q_mode == 'H') {
				MDIN3xx_SetPictureHue(&stVideo, q_value);
			}
			mdin_csc_set();
		}
		break;
		case SET_VOUT_3DNR:
		{
			show_console = OFF;
			apply3DNR(dnr_status);
		}
		break;
	}
}

/*  FILE_END_HERE */
