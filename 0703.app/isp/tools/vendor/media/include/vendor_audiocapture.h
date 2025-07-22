/**
	@brief Header file of vendor audiocapture module.\n
	This file contains the functions which is related to vendor audiocapture.

	@file vendor_audiocapture.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _VENDOR_AUDIOCAP_H_
#define _VENDOR_AUDIOCAP_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hdal.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDOR_AUDIOCAP_NAME_LEN 32

/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
	TYPE DEFINITION
********************************************************************/
typedef enum _VENDOR_AUDIOCAP_ITEM {
	VENDOR_AUDIOCAP_ITEM_EXT,
	VENDOR_AUDIOCAP_ITEM_DEFAULT_SETTING,
	VENDOR_AUDIOCAP_ITEM_NOISEGATE_THRESHOLD,
	VENDOR_AUDIOCAP_ITEM_AGC_CONFIG,
	VENDOR_AUDIOCAP_ITEM_ALC_ENABLE,
	VENDOR_AUDIOCAP_ITEM_VOLUME,
	VENDOR_AUDIOCAP_ITEM_REC_SRC,
	VENDOR_AUDIOCAP_ITEM_PREPWR_ENABLE,
	VENDOR_AUDIOCAP_ITEM_LOOPBACK_CONFIG,
	VENDOR_AUDIOCAP_ITEM_AUDIO_FRAME,
	VENDOR_AUDIOCAP_ITEM_DMA_ABORT,
	VENDOR_AUDIOCAP_ITEM_GAIN_LEVEL,
	VENDOR_AUDIOCAP_ITEM_TDM_CH,
	VENDOR_AUDIOCAP_ITEM_ALC_CONFIG,
	VENDOR_AUDIOCAP_ITEM_MONO_EXPAND,
	ENUM_DUMMY4WORD(VENDOR_AUDIOCAP_ITEM)
} VENDOR_AUDIOCAP_ITEM;

typedef enum _VENDOR_AUDIOCAP_DEFAULT_SETTING {
	VENDOR_AUDIOCAP_DEFAULT_SETTING_10DB = 0x0,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_10DB_HP_8K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_10DB_HP_16K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_10DB_HP_32K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_10DB_HP_48K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB = 0x5,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB_HP_8K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB_HP_16K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB_HP_32K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_20DB_HP_48K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_30DB = 0xA,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_30DB_HP_8K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_30DB_HP_16K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_30DB_HP_32K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_30DB_HP_48K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_0DB,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_0DB_HP_8K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_0DB_HP_16K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_0DB_HP_32K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_0DB_HP_48K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_ALCOFF,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_ALCOFF_HP_8K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_ALCOFF_HP_16K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_ALCOFF_HP_32K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_ALCOFF_HP_48K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_DMIC,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_DMIC_LP_8K,           ///< not available
	VENDOR_AUDIOCAP_DEFAULT_SETTING_DMIC_LP_16K,
	VENDOR_AUDIOCAP_DEFAULT_SETTING_DMIC_LP_32K,          ///< not available
	VENDOR_AUDIOCAP_DEFAULT_SETTING_DMIC_LP_48K,          ///< not available
	ENUM_DUMMY4WORD(VENDOR_AUDIOCAP_DEFAULT_SETTING)
} VENDOR_AUDIOCAP_DEFAULT_SETTING;

typedef enum _VENDOR_AUDIOCAP_REC_SRC {
	VENDOR_AUDIOCAP_REC_SRC_AMIC,
	VENDOR_AUDIOCAP_REC_SRC_DMIC,
	ENUM_DUMMY4WORD(VENDOR_AUDIOCAP_REC_SRC)
} VENDOR_AUDIOCAP_REC_SRC;

/**
    Total Record Gain Level
*/
typedef enum {
	VENDOR_AUDIOCAP_GAIN_LEVEL8  =  8, ///< total  8 recordgain level
	VENDOR_AUDIOCAP_GAIN_LEVEL16 = 16, ///< total 16 recordgain level
	VENDOR_AUDIOCAP_GAIN_LEVEL32 = 32, ///< total 32 recordgain level

	ENUM_DUMMY4WORD(KDRV_AUDIO_CAP_GAIN_LEVEL)
} VENDOR_AUDIOCAP_GAIN_LEVEL;

typedef enum _VENDOR_AUDIOCAP_ALC_GAIN {
	VENDOR_AUDIOCAP_ALC_GAIN_MIN = 0x00,
	VENDOR_AUDIOCAP_ALC_GAIN_N21P0_DB = 0x00,       ///< ALC controllable Gain as -21.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N19P5_DB,              ///< ALC controllable Gain as -19.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N18P0_DB,              ///< ALC controllable Gain as -18.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N16P5_DB,              ///< ALC controllable Gain as -16.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N15P0_DB,              ///< ALC controllable Gain as -15.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N13P5_DB,              ///< ALC controllable Gain as -13.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N12P0_DB,              ///< ALC controllable Gain as -12.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N10P5_DB,              ///< ALC controllable Gain as -10.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N9P0_DB,               ///< ALC controllable Gain as  -9.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N7P5_DB,               ///< ALC controllable Gain as  -7.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N6P0_DB,               ///< ALC controllable Gain as  -6.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N4P5_DB,               ///< ALC controllable Gain as  -4.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N3P0_DB,               ///< ALC controllable Gain as  -3.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_N1P5_DB,               ///< ALC controllable Gain as  -1.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_0_DB,                  ///< ALC controllable Gain as     0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P1P5_DB,               ///< ALC controllable Gain as  +1.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P3P0_DB,               ///< ALC controllable Gain as  +3.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P4P5_DB,               ///< ALC controllable Gain as  +4.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P6P0_DB,               ///< ALC controllable Gain as  +6.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P7P5_DB,               ///< ALC controllable Gain as  +7.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P9P0_DB,               ///< ALC controllable Gain as  +9.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P10P5_DB,              ///< ALC controllable Gain as +10.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P12P0_DB,              ///< ALC controllable Gain as +12.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P13P5_DB,              ///< ALC controllable Gain as +13.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P15P0_DB,              ///< ALC controllable Gain as +15.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P16P5_DB,              ///< ALC controllable Gain as +16.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P18P0_DB,              ///< ALC controllable Gain as +18.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P19P5_DB,              ///< ALC controllable Gain as +19.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P21P0_DB,              ///< ALC controllable Gain as +21.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P22P5_DB,              ///< ALC controllable Gain as +22.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P24P0_DB,              ///< ALC controllable Gain as +24.0dB.
	VENDOR_AUDIOCAP_ALC_GAIN_P25P5_DB,              ///< ALC controllable Gain as +25.5dB.
	VENDOR_AUDIOCAP_ALC_GAIN_MAX = 0x1F,
	ENUM_DUMMY4WORD(VENDOR_AUDIOCAP_ALC_GAIN)
} VENDOR_AUDIOCAP_ALC_GAIN;

typedef struct _VENDOR_AUDIOCAP_PINMUX_CFG {
	UINT32 audio_pinmux;
	UINT32 cmd_if_pinmux;
} VENDOR_AUDIOCAP_PINMUX_CFG;

typedef struct _VENDOR_AUDIOCAP_INIT_PIN_CFG {
	VENDOR_AUDIOCAP_PINMUX_CFG pinmux;
} VENDOR_AUDIOCAP_INIT_PIN_CFG;

typedef struct _VENDOR_AUDIOCAP_INIT_I2S_CFG {
	UINT32 bit_width;
	UINT32 bit_clk_ratio;
	UINT32 op_mode;
	UINT32 tdm_ch;
} VENDOR_AUDIOCAP_INIT_I2S_CFG;

typedef struct _VENDOR_AUDIOCAP_INIT_CFG_OBJ {
	VENDOR_AUDIOCAP_INIT_PIN_CFG pin_cfg;
	VENDOR_AUDIOCAP_INIT_I2S_CFG i2s_cfg;
} VENDOR_AUDIOCAP_INIT_CFG_OBJ;

typedef struct _VENDOR_AUDIOCAP_INIT_CFG {
	CHAR driver_name[VENDOR_AUDIOCAP_NAME_LEN];
	VENDOR_AUDIOCAP_INIT_CFG_OBJ aud_init_cfg;
} VENDOR_AUDIOCAP_INIT_CFG;

typedef struct _VENDOR_AUDIOCAP_AGC_CONFIG {
	BOOL   enable;
	INT32  target_lvl;
	INT32  ng_threshold;
} VENDOR_AUDIOCAP_AGC_CONFIG;

typedef struct _VENDOR_AUDIOCAP_VOLUME {
	UINT32                 volume;       ///< audio input volume
} VENDOR_AUDIOCAP_VOLUME;

typedef struct _VENDOR_AUDIOCAP_AUDIO_FRAME {
    HD_AUDIO_FRAME audio_frame;          ///< audiocap frame
    HD_AUDIO_FRAME audio_aec_ref_frame;  ///< aec reference frame
    INT32 wait_ms;                       ///< timeout
} VENDOR_AUDIOCAP_AUDIO_FRAME;

typedef struct _VENDOR_AUDIOCAP_LOOPBACK_CONFIG {
    BOOL              enabled;
    HD_AUDIOCAP_LB_CH lb_channel;        ///< loopback channel for AEC
} VENDOR_AUDIOCAP_LOOPBACK_CONFIG;

typedef struct _VENDOR_AUDIOCAP_ALC_CONFIG {
	UINT32 decay_time;                  ///< Embedded codec ALC Decay Time length. Valid setting range is 0~10.
	UINT32 attack_time;                 ///< Embedded codec ALC Attack Time length.  Valid setting range is 0~10.
	INT32  max_gain;                    ///< Embedded codec ALC maximum PGA gain. Valid setting range is -21 ~ +25.5 (dB).
	INT32  min_gain;                    ///< Embedded codec ALC minimum PGA gain. Valid setting range is -21 ~ +25.5 (dB).
} VENDOR_AUDIOCAP_ALC_CONFIG;

/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_audiocap_set(UINT32 id, VENDOR_AUDIOCAP_ITEM item, VOID *p_param);
HD_RESULT vendor_audiocap_get(UINT32 id, VENDOR_AUDIOCAP_ITEM item, VOID *p_param);
#endif

