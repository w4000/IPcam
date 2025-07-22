#ifndef __NSETUP_H__
#define __NSETUP_H__
#ifdef NARCH

#define DISP_VAL_MAX 16
#define LIST_VAL_MAX 64

typedef struct _NSETUP_RANGE_VALUE {
	int val;
	int min;
	int max;
	int init_val;
} NSETUP_RANGE_VALUE;

typedef struct _NSETUP_DISP_VAL {
	int val;
	char disp[DISP_VAL_MAX];
} NSETUP_DISP_VAL;

typedef struct _NSETUP_LIST_VALUE {
	int val;
	int init_val;
	NSETUP_DISP_VAL disp_val[LIST_VAL_MAX];
} NSETUP_LIST_VALUE;

/** IQ **/
typedef struct _NSETUP_ISP_IQ_COLOR_T {
	NSETUP_RANGE_VALUE brightness;		// 1~200
	NSETUP_RANGE_VALUE contrast;		// 1~200
	NSETUP_RANGE_VALUE saturation;		// 1~200
	NSETUP_RANGE_VALUE hue;				// 0~360
} NSETUP_ISP_IQ_COLOR_T;

typedef struct _NSETUP_ISP_IQ_NR_T {
	NSETUP_RANGE_VALUE denoise;			// 0~200
	NSETUP_RANGE_VALUE nr3d;			// 0~200
} NSETUP_ISP_IQ_NR_T;

typedef struct _NSETUP_ISP_IQ_T {
	NSETUP_ISP_IQ_COLOR_T color;
	NSETUP_ISP_IQ_NR_T nr;
	NSETUP_RANGE_VALUE sharpeness;		// 0~200
	NSETUP_RANGE_VALUE shdr_tone;		// 0~100
	NSETUP_LIST_VALUE defog;
	NSETUP_LIST_VALUE gamma;
} NSETUP_ISP_IQ_T;

/** AE **/
typedef struct _NSETUP_ISP_AE_SHUTTER_T {
	NSETUP_RANGE_VALUE min;			// 1~33333
	NSETUP_RANGE_VALUE max;			// 1~33333
} NSETUP_ISP_AE_SHUTTER_T;

typedef struct _NSETUP_ISP_AE_GAIN_T {
	NSETUP_RANGE_VALUE min;			// 100~204800
	NSETUP_RANGE_VALUE max;			// 100_204800
} NSETUP_ISP_AE_GAIN_T;

typedef struct _NSETUP_ISP_AE_T {
	NSETUP_LIST_VALUE dciris;			// Off, On
	NSETUP_LIST_VALUE ev_offset;		// -16~16
	NSETUP_LIST_VALUE iso;
	NSETUP_ISP_AE_SHUTTER_T shutter;
	NSETUP_ISP_AE_GAIN_T gain;
	NSETUP_LIST_VALUE dss;
	NSETUP_LIST_VALUE antiflicker;
	NSETUP_RANGE_VALUE balance;
} NSETUP_ISP_AE_T;

/** BLC **/
typedef struct _NSETUP_ISP_BLC_BLC_T {
	NSETUP_LIST_VALUE mode;			// Off, On
	NSETUP_RANGE_VALUE level;
	unsigned int grid[8*8];
} NSETUP_ISP_BLC_BLC_T;

typedef struct _NSETUP_ISP_BLC_WDR_T {
	NSETUP_LIST_VALUE mode;			// Off, Auto, Manual
	NSETUP_RANGE_VALUE strength;	// 0~255
} NSETUP_ISP_BLC_WDR_T;

typedef struct _NSETUP_ISP_BLC_SHDR_T {
	NSETUP_LIST_VALUE mode;			// Off, On
	NSETUP_RANGE_VALUE tone_level;	// 0~100
} NSETUP_ISP_BLC_SHDR_T;

typedef struct _NSETUP_ISP_BLC_T {
	NSETUP_ISP_BLC_BLC_T blc;
	NSETUP_ISP_BLC_WDR_T wdr;
	NSETUP_ISP_BLC_SHDR_T shdr;
} NSETUP_ISP_BLC_T;

/** AWB **/
typedef struct _NSETUP_ISP_AWB_AUTO_T {
	NSETUP_RANGE_VALUE r_ratio;			// 1~400
	NSETUP_RANGE_VALUE b_ratio;			// 1~400
} NSETUP_ISP_AWB_AUTO_T;

typedef struct _NSETUP_ISP_AWB_MANUAL_T {
	NSETUP_RANGE_VALUE r_gain;			// 1~2047
	NSETUP_RANGE_VALUE g_gain;			// 1~2047
	NSETUP_RANGE_VALUE b_gain;			// 1~2047
} NSETUP_ISP_AWB_MANUAL_T;

typedef struct _NSETUP_ISP_AWB_T {
	NSETUP_LIST_VALUE mode;
	NSETUP_ISP_AWB_AUTO_T auto_ratio;
	NSETUP_ISP_AWB_MANUAL_T manual_gain;
} NSETUP_ISP_AWB_T;

/** DAY&NIGHT **/
typedef struct _NSETUP_ISP_DN_SCHDULE_T {
	NSETUP_RANGE_VALUE start_min;		// 0~1410
	NSETUP_RANGE_VALUE end_min;			// 30~1440
} NSETUP_ISP_DN_SCHDULE_T;

typedef struct _NSETUP_ISP_DN_T {
	NSETUP_LIST_VALUE mode;
	NSETUP_RANGE_VALUE dwell;			// 0~30
	NSETUP_RANGE_VALUE d2n_threshold;	// 0~255
	NSETUP_RANGE_VALUE n2d_threshold;	// 0~255
	NSETUP_ISP_DN_SCHDULE_T schedule;
} NSETUP_ISP_DN_T;

/** LPR **/
enum {
	LPR_NIGHT,
	LPR_DAY,
	LPR_COUNT
};

typedef struct _NSETUP_ISP_LPR_T {
	NSETUP_LIST_VALUE mode;				// Off, On
	NSETUP_ISP_AE_T ae[LPR_COUNT];
} NSETUP_ISP_LPR_T;

typedef struct _NSETUP_ISP_T {
	NSETUP_ISP_IQ_T iq;
	NSETUP_ISP_AE_T ae;
	NSETUP_ISP_BLC_T blc;
	NSETUP_ISP_AWB_T awb;
	NSETUP_ISP_DN_T dn;
#ifdef SUPPORT_LPR_EXP
	NSETUP_ISP_LPR_T lpr;
#endif
} NSETUP_ISP_T;

typedef struct _NSETUP_T {
	NSETUP_ISP_T isp;
} NSETUP_T;

int setup_default_nsetup(NSETUP_T *nsetup);
int setup_range_nsetup(NSETUP_T *nsetup);

#endif /*NARCH*/
#endif /*__NSETUP_H__*/

