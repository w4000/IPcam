#ifndef __ENCODER_DEF_H__
#define __ENCODER_DEF_H__

#include "soc_common.h"
#include "common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

const int is_support_h265();

const int def_max_bitrate(int ch);
const int def_bitrate_index_to_value(int index);
const int def_bitrate_value_to_index(const int value);


const PIC_SIZE_E def_max_resol(int ch);

// return: enum PIC_SIZE_E in setup/hi_resolution.h
const char *def_resol_index_to_string(const PIC_SIZE_E index);
const int def_resol_index_to_width(int ispal, const PIC_SIZE_E index);
const int def_resol_index_to_height(int ispal, const PIC_SIZE_E index);
const int def_get_picsize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S* pstSize);
const int def_resol_size_to_index(int ispal, const char *size);

// return; PIC_SIZE_E array, endmark PIC_BUTT
const PIC_SIZE_E *def_support_resol_array(const int ch);
PIC_SIZE_E *def_all_resol_array();


// return: ex) HD1080
const char *def_resol_string();

typedef enum _SHUTTER_SPEED_E {
	SS_AUTO = 0,
	SS_ANTIFLICKER,
	SS_30,
	SS_60,
	SS_125,
	SS_250,
	SS_500,
	SS_750,
	SS_1000,
	SS_1500,
	SS_2000,
	SS_2500,
	SS_3000,
	SS_4000,
	SS_5000,
	SS_6000,
	SS_7000,
	SS_8000,
	SS_9000,
	SS_10000,
	SS_15000,
	SS_30000
} SHUTTER_SPEED_E;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __ENCODER_DEF_H__ */
