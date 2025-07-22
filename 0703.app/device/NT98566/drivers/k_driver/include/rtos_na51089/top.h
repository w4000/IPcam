/*
	TOP controller header

	Sets the pinmux of each module.

    @file       top.h
    @ingroup    mIDrvSys_TOP
    @note       Refer NA51089 data sheet for PIN/PAD naming

	Copyright Novatek Microelectronics Corp. 2020.  All rights reserved.
*/

#ifndef __ASM_ARCH_NA51089_TOP_H
#define __ASM_ARCH_NA51089_TOP_H

#include <kwrap/nvt_type.h>
#if !defined(__LINUX)
#include <stdio.h>
#endif

/**
    @addtogroup mIDrvSys_TOP
*/
//@{
#define PIN_GROUP_CONFIG_VER 0x21102501

/**
    Debug port select ID

    Debug port select value for pinmux_select_debugport().
*/
typedef enum {
	PINMUX_DEBUGPORT_CKG =          0x0000,     ///< CKGen
	PINMUX_DEBUGPORT_GPIO =         0x0001,     ///< GPIO
	PINMUX_DEBUGPORT_ARB =          0x0002,     ///< Arbiter
//	PINMUX_DEBUGPORT_ARB2 =         0x0003,     ///< Arbiter2
	PINMUX_DEBUGPORT_DDR =          0x0004,     ///< DDR1
//	PINMUX_DEBUGPORT_DDR2 =         0x0005,     ///< DDR2
	PINMUX_DEBUGPORT_DSI =          0x0006,     ///< DSI
	PINMUX_DEBUGPORT_IDE =          0x0007,     ///< IDE
	PINMUX_DEBUGPORT_TSE =          0x0008,     ///< TSE
	PINMUX_DEBUGPORT_ETH =          0x0009,     ///< ETH
//	PINMUX_DEBUGPORT_AHBC_ETH =     0x000A,     ///< AHBC ETH
	PINMUX_DEBUGPORT_GRAPHIC =      0x000A,     ///< GRAPHIC
	PINMUX_DEBUGPORT_GRAPHIC2 =     0x000B,     ///< GRAPHIC2
	PINMUX_DEBUGPORT_IFE =        0x000C,     ///< IFE
	PINMUX_DEBUGPORT_AXIBRG =       0x000D,     ///< AXIBRG
	PINMUX_DEBUGPORT_IPBRG =        0x000E,     ///< IPBRG

	PINMUX_DEBUGPORT_TIMER =        0x0010,     ///< Timer
	PINMUX_DEBUGPORT_WDT =          0x0011,     ///< WDT
	PINMUX_DEBUGPORT_DAI =          0x0012,     ///< DAI
	PINMUX_DEBUGPORT_AUDIO =        0x0013,     ///< AUDIO
	PINMUX_DEBUGPORT_SDIO1 =        0x0014,     ///< SDIO1
	PINMUX_DEBUGPORT_SDIO2 =        0x0015,     ///< SDIO2
	PINMUX_DEBUGPORT_SDIO3 =        0x0016,     ///< SDIO3
	PINMUX_DEBUGPORT_SMMC =         0x0017,     ///< XD/NAND/SMC
	PINMUX_DEBUGPORT_USB =          0x0018,     ///< USB
	PINMUX_DEBUGPORT_I2C =     	0x0019,     ///< I2C
	PINMUX_DEBUGPORT_I2C2 =          0x001A,     ///< I2C2
	PINMUX_DEBUGPORT_I2C3 =         0x001B,     ///< I2C3
	PINMUX_DEBUGPORT_SIF =         0x001C,     ///< SIF
	PINMUX_DEBUGPORT_SPI =          0x001D,     ///< SPI
	PINMUX_DEBUGPORT_SPI2 =          0x001E,     ///< SPI2
	PINMUX_DEBUGPORT_SPI3 =         0x001F,     ///< SPI3

	PINMUX_DEBUGPORT_HASH =         0x0020,     ///< HASH
	PINMUX_DEBUGPORT_PWM =         0x0021,     ///< PWM
	PINMUX_DEBUGPORT_ADM =          0x0022,     ///< ADC

	PINMUX_DEBUGPORT_REMOTE =      0x0023,     ///< Remote
	PINMUX_DEBUGPORT_CRYPTO =       0x0024,     ///< Crypto
	PINMUX_DEBUGPORT_TRNG =       0x0025,     ///< TRNG
	PINMUX_DEBUGPORT_DRTC =         0x0026,     ///< DRTC
	PINMUX_DEBUGPORT_UART2 =         0x0027,     ///< UART2
	PINMUX_DEBUGPORT_UART3 =        0x0028,     ///< UART3
	PINMUX_DEBUGPORT_SDP =        0x0029,     ///< SDP
	PINMUX_DEBUGPORT_EFUSE =          0x002A,     ///< EFUSE
	PINMUX_DEBUGPORT_RSA =        0x002B,     ///< RSA
	PINMUX_DEBUGPORT_UVCP =          0x002C,     ///< UVCP

	PINMUX_DEBUGPORT_CSI =          0x0030,     ///< MIPI CSI
	PINMUX_DEBUGPORT_CSI2 =         0x0031,     ///< MIPI CSI2
	PINMUX_DEBUGPORT_SIE =          0x0032,     ///< SIE
	PINMUX_DEBUGPORT_SIE2 =         0x0033,     ///< SIE2
	PINMUX_DEBUGPORT_SIE3 =         0x0034,     ///< SIE3
	PINMUX_DEBUGPORT_TGE =          0x0035,     ///< TGE

	PINMUX_DEBUGPORT_ISE =          0x0036,     ///< ISE
	PINMUX_DEBUGPORT_IPE =         0x0037,     ///< IPE
	PINMUX_DEBUGPORT_IME =          0x0038,     ///< IPE

	PINMUX_DEBUGPORT_NUE =          0x003A,     ///< NUE

	//PINMUX_DEBUGPORT_Reserved =     0x003F,     ///< Reserved

	PINMUX_DEBUGPORT_JPEG =         0x0040,     ///< JPEG
	PINMUX_DEBUGPORT_NUE2 =        0x0041,     ///< NUE2
	PINMUX_DEBUGPORT_CNN =          0x0050,     ///< CNN
	PINMUX_DEBUGPORT_DCE =          0x0051,     ///< DCE
	PINMUX_DEBUGPORT_DIS =           0x0052,     ///< DIS
	PINMUX_DEBUGPORT_IVE =           0x0053,     ///< IVE

	PINMUX_DEBUGPORT_GROUP_NONE =   0x0000,     ///< No debug port is output
	PINMUX_DEBUGPORT_GROUP1 =       0x0100,     ///< Output debug port to MC[18..0]
	PINMUX_DEBUGPORT_GROUP2 =       0x0200,     ///< Output debug port to LCD[18..0]

	ENUM_DUMMY4WORD(PINMUX_DEBUGPORT)
} PINMUX_DEBUGPORT;


/**
    Function group

    @note For pinmux_init()
*/
typedef enum {
	PIN_FUNC_SDIO,      ///< SDIO. Configuration refers to PIN_SDIO_CFG.
	PIN_FUNC_SDIO2,     ///< SDIO2. Configuration refers to PIN_SDIO_CFG.
	PIN_FUNC_SDIO3,     ///< SDIO3. Configuration refers to PIN_SDIO_CFG.
	PIN_FUNC_NAND,      ///< NAND. Configuration refers to PIN_NAND_CFG.
	PIN_FUNC_SENSOR,    ///< sensor interface. Configuration refers to PIN_SENSOR_CFG.
	PIN_FUNC_SENSOR2,   ///< sensor2 interface. Configuration refers to PIN_SENSOR2_CFG.
	PIN_FUNC_MIPI_LVDS, ///< MIPI/LVDS interface configuration. Configuration refers to PIN_MIPI_LVDS_CFG.
	PIN_FUNC_I2C,       ///< I2C. Configuration refers to PIN_I2C_CFG.
	PIN_FUNC_SIF,       ///< SIF. Configuration refers to PIN_SIF_CFG.
	PIN_FUNC_UART,      ///< UART. Configuration refers to PIN_UART_CFG.
	PIN_FUNC_SPI,       ///< SPI. Configuration refers to PIN_SPI_CFG.
	PIN_FUNC_SDP,       ///< SDP. Configuration refers to PIN_SDP_CFG.
	PIN_FUNC_REMOTE,    ///< REMOTE. Configuration refers to PIN_REMOTE_CFG.
	PIN_FUNC_PWM,       ///< PWM. Configuration refers to PIN_PWM_CFG.
	PIN_FUNC_PWM2,      ///< PWM2. Configuration refers to PIN_PWM_CFG2.
	PIN_FUNC_CCNT,      ///< CCNT. Configuration refers to PIN_CCNT_CFG.
	PIN_FUNC_AUDIO,     ///< AUDIO. Configuration refers to PIN_AUDIO_CFG.
	PIN_FUNC_LCD,       ///< LCD interface. Configuration refers to PINMUX_LCDINIT, PINMUX_PMI_CFG, PINMUX_DISPMUX_SEL.
	PIN_FUNC_TV,        ///< TV interface. Configuration refers to PINMUX_TV_HDMI_CFG.
	PIN_FUNC_ETH,       ///< ETH. Configuration refers to PINMUX_ETH_CFG
	PIN_FUNC_MISC,      ///< MISC. Configuration refers to PINMUX_MISC_CFG
	PIN_FUNC_MAX,

	ENUM_DUMMY4WORD(PIN_FUNC)
} PIN_FUNC;

/**
    PIN config for SDIO

    @note For pinmux_init() with PIN_FUNC_SDIO or PIN_FUNC_SDIO2 or PIN_FUNC_SDIO3.\n
            For example, you can use {PIN_FUNC_SDIO, PIN_SDIO_CFG_1ST_PINMUX|PIN_SDIO_CFG_4BITS}\n
            to declare SDIO is 4 bits and located in 1st pinmux location.
*/
typedef enum {
	PIN_SDIO_CFG_NONE,
	PIN_SDIO_CFG_4BITS = 0x01,          ///< 4 bits wide
	PIN_SDIO_CFG_8BITS = 0x02,          ///< 8 bits wide

	PIN_SDIO_CFG_1ST_PINMUX = 0x04,///< 1st pinmux location
						///< For SDIO: enable SDIO_CLK/SDIO_CMD/SDIO_D[0..3] on MC[11..16] (C_GPIO[11..16])
						///< For SDIO2: enable SDIO2_CLK/SDIO2_CMD/SDIO2_D[0..3] on MC[17..22] (C_GPIO[17..22])
						///< For SDIO3: enable SDIO3_D[0..7]/SDIO3_CLK/SDIO3_CMD on MC[0..9] (C_GPIO[0...9])
	PIN_SDIO_CFG_2ND_PINMUX = 0x10, ///< 2nd pinmux location
						///< For SDIO2: enable SDIO2_2_CLK/SDIO2_2_CMD/SDIO2_2_D[0..3] on S_GPIO[0..3] / S_GPIO[7..8]
	PIN_SDIO_CFG_3RD_PINMUX = 0x20, ///< 3rd pinmux location
						///< For SDIO2: enable SDIO2_3_CLK/SDIO2_3_CMD/SDIO2_3_D[0..3] on DSI_GPIO7/DSI_GPIO6 /DSI_GPIO[2..5]
	ENUM_DUMMY4WORD(PIN_SDIO_CFG)
} PIN_SDIO_CFG;

/**
    PIN config for NAND

    @note For pinmux_init() with PIN_FUNC_NAND.\n
            For example, you can use {PIN_FUNC_NAND, PIN_NAND_CFG_1CS} to declare NAND with 1 CS.
*/
typedef enum {
	PIN_NAND_CFG_NONE,
	PIN_NAND_CFG_1CS = 0x01,        ///< 1 chip select. Enable FSPI_DO/FSPI_DI/FSPI_WP/FSPI_HOLD/FSPI_CLK/FSPI_CS on MC[0..3]/MC[8]/MC[10] (C_GPIO[0..3], C_GPIO[8], C_GPIO[10])
	PIN_NAND_CFG_2CS = 0x02,        ///< Not support 2 chip select. (Backward compatible)
	PIN_NAND_CFG_SPI_NAND = 0x4,    ///< Virtual enum for project layer configuration
	PIN_NAND_CFG_SPI_NOR = 0x8,     ///< Virtual enum for project layer configuration
	ENUM_DUMMY4WORD(PIN_NAND_CFG)
} PIN_NAND_CFG;

/**
    PIN config for Sensor

    @note For pinmux_init() with PIN_FUNC_SENSOR.\n
            For example, you can use {PIN_FUNC_SENSOR, PIN_SENSOR_CFG_12BITS|PIN_SENSOR_CFG_SHUTTER0|PIN_SENSOR_CFG_MCLK}\n
            to declare sensor interface is 12 bits sensor, has SHUTTER 0 and MCLK.
*/
typedef enum {
	PIN_SENSOR_CFG_NONE,
	PIN_SENSOR_CFG_8BITS = 0x01,        ///< Not support. (Backward compatible)
	PIN_SENSOR_CFG_10BITS = 0x02,       ///< Not support. (Backward compatible)
	PIN_SENSOR_CFG_12BITS = 0x04,       ///< 12 bits sensor. Enable SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD on CLK1_N, CLK1_P,D0_N, D0_P, D1_N, D1_P, CLK0_N, CLK0_P, D2_N, D2_P, D3_N, D3_P/SN_PXCLK/SN_VD/SN_HD (HSI_GPI[0..11]/S_GPIO[1..3])
	PIN_SENSOR_CFG_CCIR8BITS = 0x08,    ///< CCIR 8 bits sensor. Enable CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_FIELD on D1_N, D1_P, CLK0_N, CLK0_P, D2_N, D2_P, D3_N, D3_P/SN_PXCLK/SN_VD/SN_HD/S_GPIO7 (HSI_GPI[2..9]/S_GPIO[1..3]/S_GPIO[7])
	PIN_SENSOR_CFG_CCIR16BITS = 0x10,   ///< CCIR 16 bits sensor. Enable CCIR_Y[0..7]/CCIR_C[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_FIELD on D1_N, D1_P, CLK0_N, CLK0_P, D2_N, D2_P, D3_N, D3_P/PWM[0..7]/SN_PXCLK/SN_VD/SN_HD/SGPIO4 (HSI_GPI[2..9]/P_GPIO[0..7]/S_GPIO[1..4]).
	PIN_SENSOR_CFG_MIPI = 0x20,         ///< MIPI sensor. PIN/PAD is configured by PIN_FUNC_MIPI_LVDS group.
	PIN_SENSOR_CFG_LVDS = 0x40,         ///< LVDS sensor PIN/PAD is configured by PIN_FUNC_MIPI_LVDS group.
	PIN_SENSOR_CFG_LVDS_VDHD = 0x80,    ///< Enable SN_XVS/SN_XHS on SN_VD/SN_HD (SN_SGPIO[2..3])

	PIN_SENSOR_CFG_SHUTTER0 = 0x100,    ///< Enable SN_SHUTTER0 on SGPIO12 (S_GPIO[12])
	PIN_SENSOR_CFG_MCLK = 0x200,        ///< Enable SN_MCLK on SN_MCLK (S_GPIO[0])
	PIN_SENSOR_CFG_MCLK2 = 0x400,       ///< Enable SN_MCLK2 on SN_PXCLK (S_GPIO[1])
	PIN_SENSOR_CFG_MES0 = 0x800,        ///< Not support. (Backward compatible)
	PIN_SENSOR_CFG_MES0_2ND = 0x1000,   ///< Not support. (Backward compatible)
	PIN_SENSOR_CFG_MES1 = 0x2000,       ///< Not support. (Backward compatible)
	PIN_SENSOR_CFG_MES1_2ND = 0x4000,   ///< Not support. (Backward compatible)
	PIN_SENSOR_CFG_FLCTR = 0x8000,      ///< Not support. (Backward compatible)
	PIN_SENSOR_CFG_STROBE = 0x10000,    ///< Not support. (Backward compatible)
	PIN_SENSOR_CFG_SPCLK = 0x20000,     ///< Enable SP_CLK on SP_CLK_1 (P_GPIO[17])
	PIN_SENSOR_CFG_SPCLK_2ND = 0x40000, ///< Enable SP_CLK on SP_CLK_2 (L_GPIO[23])
	PIN_SENSOR_CFG_SPCLK_3RD = 0x80000,    ///< Enable SP_CLK on SP_CLK_3 (D_GPIO[3])
	PIN_SENSOR_CFG_SP2CLK  = 0x100000,  ///< Enable SP_CLK2 on SP_CLK2_1 (P_GPIO[18])
	PIN_SENSOR_CFG_SP2CLK_2ND = 0x200000,      ///< Enable SP_CLK2 on SP_CLK2_2 (C_GPIO[9])
	PIN_SENSOR_CFG_SP2CLK_3RD = 0x400000,  ///< Enable SP_CLK2 on SP_CLK2_3 (D_GPIO[4])
	PIN_SENSOR_CFG_MCLK_2ND     = 0x800000,  ///Not support (Backward compatible)
	PIN_SENSOR_CFG_MCLK_3RD = 0x1000000,    ///Not support (Backward compatible)
	PIN_SENSOR_CFG_MES2 = 0x2000000,         ///Not support (Backward compatible)
	PIN_SENSOR_CFG_MES2_2ND     = 0x4000000, ///Not support (Backward compatible)
	PIN_SENSOR_CFG_MES3 = 0x8000000, ///< Not support (Backward compatible)
	PIN_SENSOR_CFG_CCIR_2 = 0x10000000, ///< CCIR 8 bits sensor. Enable CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_FIELD on D0_N/D0_P/CLK1_N/CLK1_P (HSI_GPIO[0..1]/HSI_GPIO[10..11])

	PIN_SENSOR_CFG_LVDS_VDHD2 = 0x20000000,     ///< Enable SN_XVS/SN_XHS on SN_VD/SN_HD (SN_SGPIO[2..3])
	PIN_SENSOR_CFG_12BITS_2ND = 0x40000000,    ///< 12 bits sensor 2nd. Enable SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD on PWM8~PWM11, PWM0~PWM7,PGPIO12/SN_VD/SN_HD (P_GPIO[0..12]/S_GPIO[2..3])

	ENUM_DUMMY4WORD(PIN_SENSOR_CFG)
} PIN_SENSOR_CFG;

/**
    PIN config for Sensor2

    @note For pinmux_init() with PIN_FUNC_SENSOR2.\n
            For example, you can use {PIN_FUNC_SENSOR2, PIN_SENSOR2_CFG_CCIR8BITS}\n
            to declare sensor2 interface connect a CCIR 8 bits sensor.
*/
typedef enum {
	PIN_SENSOR2_CFG_NONE,
	PIN_SENSOR2_CFG_CCIR8BITS = 0x08,   ///< CCIR 8 bits sensor. Enable CCIR_YC[0..7]/CCIR_VD/CCIR_HD/CCIR_CLK on HSI_D[5..9]N/HSI_D[5..9]P/SN_DGPIO[4] (S_GPI[12..21]/SN_DGPIO[4])
	PIN_SENSOR2_CFG_CCIR16BITS = 0x10,  ///< Not support. (Backward compatible)
	PIN_SENSOR2_CFG_MIPI = 0x20,        ///< MIPI sensor. PIN/PAD is configured by PIN_FUNC_MIPI_LVDS group.
	PIN_SENSOR2_CFG_LVDS = 0x40,        ///< LVDS sensor. PIN/PAD is configured by PIN_FUNC_MIPI_LVDS group.
	PIN_SENSOR2_CFG_LVDS_VDHD = 0x80,   ///< Enable SN2_XVS/SN2_XHS on SN2_VD/SN2_HD (SN_SGPIO[4..5])
	PIN_SENSOR2_CFG_12BITS = 0x100,     ///< Support 12 bits sensor.
	PIN_SENSOR2_CFG_MCLK = 0x200,       ///< Enable SN2_MCLK on SN_PXCLK (S_GPIO[1])
	PIN_SENSOR2_CFG_MCLK_2ND = 0x400,   ///< Enable SN2_MCLK on S_GPIO12 (S_GPIO[12])
	PIN_SENSOR2_CFG_SN3_MCLK = 0x800,   ///< Enable SN3_MCLK on P_GPIO12 (P_GPIO[12])

	PIN_SENSOR2_CFG_SN_MCLK = 0x1000,      ///< Enable SN_MCLK on S_GPIO[0]
	PIN_SENSOR2_CFG_MCLK_SRC = 0x2000,    ///< SN2_MCLK_SRC selected by SIE_MCLK (from CG)
	PIN_SENSOR2_CFG_SN_VSHS = 0x4000,      ///< SN2_XVSHS_SRC selected by SN_XVS/SN_XHS(TG)
	PIN_SENSOR2_CFG_CCIR8BITS_FIELD = 0x8000,   ///< Enable CCIR8bit mode field

	ENUM_DUMMY4WORD(PIN_SENSOR2_CFG)
} PIN_SENSOR2_CFG;


/**
    PIN config for MIPI/LVDS

    @note For pinmux_init() with PIN_FUNC_MIPI_LVDS.\n
            For example, you can use {PIN_FUNC_MIPI_LVDS, PIN_MIPI_LVDS_CFG_CLK0|PIN_MIPI_LVDS_CFG_DAT0|PIN_MIPI_LVDS_CFG_DAT1}\n
            to declare LVDS/MIPI connect sensor with CLK0/D0/D1 lanes.
*/
typedef enum {
	PIN_MIPI_LVDS_CFG_NONE,

	PIN_MIPI_LVDS_CFG_CLK0 = 0x1,       ///< CLK lane 0. Enable HSI_CK0N/HSI_CK0P for LVDS/CSI/CSI2.
	PIN_MIPI_LVDS_CFG_CLK1 = 0x2,       ///< CLK lane 1. Enable HSI_CK1N/HSI_CK1P for LVDS/CSI/CSI2.

	PIN_MIPI_LVDS_CFG_DAT0 = 0x100,     ///< DATA lane 0. Enable HSI_D0N/HSI_D0P for LVDS/CSI/CSI2.
	PIN_MIPI_LVDS_CFG_DAT1 = 0x200,     ///< DATA lane 1. Enable HSI_D1N/HSI_D1P for LVDS/CSI/CSI2.
	PIN_MIPI_LVDS_CFG_DAT2 = 0x400,     ///< DATA lane 2. Enable HSI_D2N/HSI_D2P for LVDS/CSI/CSI2.
	PIN_MIPI_LVDS_CFG_DAT3 = 0x800,     ///< DATA lane 3. Enable HSI_D3N/HSI_D3P for LVDS/CSI/CSI2.
	PIN_MIPI_LVDS_CFG_DAT4 = 0x1000,    ///< No DATA lane 4.  Only support 2C4D.
	PIN_MIPI_LVDS_CFG_DAT5 = 0x2000,    ///< No DATA lane 5.  Only support 2C4D.
	PIN_MIPI_LVDS_CFG_DAT6 = 0x4000,    ///< No DATA lane 6.  Only support 2C4D..
	PIN_MIPI_LVDS_CFG_DAT7 = 0x8000,    ///< No DATA lane 7.  Only support 2C4D..
	PIN_MIPI_LVDS_CFG_DAT8 = 0x10000,   ///< No DATA lane 8.  Only support 2C4D..
	PIN_MIPI_LVDS_CFG_DAT9 = 0x20000,   ///< No DATA lane 9.  Only support 2C4D..

	ENUM_DUMMY4WORD(PIN_MIPI_LVDS_CFG)
} PIN_MIPI_LVDS_CFG;

/**
    PIN config for I2C

    @note For pinmux_init() with PIN_FUNC_I2C.\n
            For example, you can use {PIN_FUNC_I2C, PIN_I2C_CFG_CH1|PIN_I2C_CFG_CH2}\n
            to declare I2C channel1 and channel2 pinmux both enabled.
*/
typedef enum {
	PIN_I2C_CFG_NONE,
	PIN_I2C_CFG_CH1 = 0x01,             ///< Enable I2C_1. Enable I2C_SCL/I2C_SDA @S_GPIO7[I2C_SCL_1]|S_GPIO8[I2C_SDA_1]
	PIN_I2C_CFG_CH1_2ND_PINMUX = 0x02,  /// < Enable I2C_2. Enable I2C_SCL/I2C_SDA @HSI_GPIO9[I2C_2_SCL]|HSI_GPIO10[I2C_2_SDA]
	PIN_I2C_CFG_CH2 = 0x10,             ///< Enable I2C2_1. Enable I2C2_SCL/I2C2_SDA @S_GPIO4[I2C2_1_SCL]|S_GPIO5[I2C2_1_SDA]
	PIN_I2C_CFG_CH2_2ND_PINMUX = 0x20,  ///< Enable I2C2_2. Enable I2C2_SCL/I2C2_SDA @MC17[I2C2_2_SCL]|MC18[I2C2_2_SDA]
	PIN_I2C_CFG_CH3 = 0x40,             ///< Enable I2C3_1. Enable I2C3_SCL/I2C3_SDA @P_GPIO22[I2C3_1_SCL]|P_GPIO21[I2C3_1_SDA]
	PIN_I2C_CFG_CH3_2ND_PINMUX = 0x80,  ///< Enable I2C3_2. Enable I2C3_SCL/I2C3_SDA @MC11[I2C3_2_SCL]|MC12[I2C3_2_SDA]
	PIN_I2C_CFG_CH3_3RD_PINMUX = 0x100,  ///< Enable I2C3_3. Enable I2C3_SCL/I2C3_SDA @DSI_GPIO9[I2C3_3_SCL]|DSI_GPIO8[I2C3_3_SDA]

	ENUM_DUMMY4WORD(PIN_I2C_CFG)
} PIN_I2C_CFG;

/**
    PIN config for SIF

    *note For pinmux_init() with PIN_FUNC_SIF.\n
            For example, you can use {PIN_FUNC_SIF, PIN_SIF_CFG_CH0|PIN_SIF_CFG_CH2}\n
            to declare SIF channel0 and channel2 pinmux are enabled.
*/
typedef enum {
	PIN_SIF_CFG_NONE,

	PIN_SIF_CFG_CH0 = 0x01,             ///< Enable SIF channel 0. Enable @S_GPIO6[SB_CS]|S_GPIO7[SB_CK]|S_GPIO8[SB_DAT]

	PIN_SIF_CFG_CH1 = 0x02,             ///< Enable SIF channel 1. Enable @S_GPIO3[SB2_CS]|S_GPIO4[SB2_CK]|S_GPIO5[SB2_DAT]
	PIN_SIF_CFG_CH1_2ND_PINMUX = 0x04,  ///< Backward compatible. Not supported

	PIN_SIF_CFG_CH2 = 0x08,             ///< Enable SIF channel 2_1. Enable @P_GPIO10[SB3_1_CS]|P_GPIO11[SB3_1_CK]|P_GPIO12[SB3_1_DAT]

	PIN_SIF_CFG_CH3 = 0x10,             ///< Enable SIF channel 3. Enable @P_GPIO7[SB4_CS]|P_GPIO8[SB4_CK]|P_GPIO9[SB4_DAT]

	PIN_SIF_CFG_CH4 = 0x20,             ///< Not support SIF channel 4.

	PIN_SIF_CFG_CH5 = 0x40,             ///< Not support SIF channel 5.

	PIN_SIF_CFG_CH2_2ND_PINMUX = 0x80,    ///< Enable SIF channel 2_2. Enable @P_GPIO17[SB3_2_CS]|P_GPIO18[SB3_2_CK]|P_GPIO19[SB3_2_DAT]
	PIN_SIF_CFG_CH2_3RD_PINMUX = 0x100,    ///< Enable SIF channel 2_3. Enable @D_GPIO3[SB3_3_CS]|D_GPIO4[SB3_3_CK]|D_GPIO5[SB3_3_DAT]

	ENUM_DUMMY4WORD(PIN_SIF_CFG)
} PIN_SIF_CFG;

/**
    PIN config for UART

    @note For pinmux_init() with PIN_FUNC_UART.\n
            For example, you can use {PIN_FUNC_UART, PIN_UART_CFG_CH1}\n
            to declare UART1 pinmux is enabled.
*/
typedef enum {
	PIN_UART_CFG_NONE,

	PIN_UART_CFG_CH1 = 0x01,            ///< Enable channel 1. Enable UART TX and RX. Enable TX/RX @P_GPIO23[Uart_TX]|P_GPIO24[Uart_RX]
	PIN_UART_CFG_CH1_TX = 0x02,         ///< Enalbe channel 1. Enable only UART_TX. Enable TX @P_GPIO23[Uart_TX]
	PIN_UART_CFG_CH2 = 0x04,            ///< Enable UART2 TX and RX. Enable TX/RX  @P_GPIO17[Uart2_1_TX]|P_GPIO18[Uart2_1_RX]. Enable RTS/CTS &&P_GPIO19[Uart2_1_RTS]|P_GPIO20[Uart2_1_CTS]
	PIN_UART_CFG_CH2_CTSRTS = 0x08,     ///< Enable channel 2 HW handshake. Enable UART2 CTS and RTS
	PIN_UART_CFG_CH2_2ND = 0x10,        ///< Enable channel 2 to 2ND pinmux (UART2_2). Enable TX/RX @MC15[Uart2_2_TX]|MC16[Uart2_2_RX]. Enable RTS/CTS &&MC14[Uart2_2_RTS]|MC13[Uart2_2_CTS]
	PIN_UART_CFG_CH3 = 0x20,            ///< Enable channel 3. Enable UART3 TX and RX @DSI_GPIO0[Uart3_1_TX]|DSI_GPIO1[Uart3_1_RX]. Enable RTS/CTS &&DSI_GPIO2[Uart3_1_RTS]|DSI_GPIO3[Uart3_1_CTS]
	PIN_UART_CFG_CH3_CTSRTS = 0x40,     ///< Enable channel 3 HW handshake. Enable UART3 CTS and RTS
	PIN_UART_CFG_CH3_2ND = 0x80,        ///< Enable channel 3 to 2ND pinmux (UART3_2). Enable TX/RX @MC21[Uart3_2_TX]|MC22[Uart3_2_RX]. Enable RTS/CTS &&MC20[Uart3_2_RTS]|MC19[Uart3_2_CTS]
	//CH4 Only support @ 528  -----------------------------------------------------------------------------------------------------------------------------------------------------
	PIN_UART_CFG_CH4 = 0x100,           ///< Not support (Backward compatible)
	PIN_UART_CFG_CH4_CTSRTS = 0x200,    ///< Not support (Backward compatible)
	PIN_UART_CFG_CH4_2ND = 0x400,       ///< Not support (Backward compatible)
	//CH4 Only support @ 528  -----------------------------------------------------------------------------------------------------------------------------------------------------
	PIN_UART_CFG_CH2_3RD = 0x800,       ///< Enable channel 2 to 3RD pinmux (UART2_3). Enable TX/RX @P_GPIO0[Uart2_3_TX]|P_GPIO1[Uart2_3_RX]. Enable RTS/CTS &&P_GPIO2[Uart2_3_RTS]|P_GPIO3[Uart2_3_CTS]
	PIN_UART_CFG_CH3_3RD = 0x1000,      ///< Enable channel 3 to 3RD pinmux (UART3_3). Enable TX/RX @P_GPIO4[Uart3_3_TX]|P_GPIO5[Uart3_3_RX]. Enable RTS/CTS &&P_GPIO6[Uart3_3_RTS]|P_GPIO7[Uart3_3_CTS]
	PIN_UART_CFG_CH3_4TH = 0x2000,      ///< Enable channel 3 to 4TH pinmux (UART3_4). Enable TX/RX @S_GPIO0[Uart3_4_TX]|S_GPIO1[Uart3_4_RX].
	PIN_UART_CFG_CH3_5TH = 0x4000,      ///< Enable channel 3 to 5TH pinmux (UART3_5). Enable TX/RX @S_GPIO4[Uart3_5_TX]|S_GPIO5[Uart3_5_RX].

	PIN_UART_CFG_CH2_DIROE = 0x8000,     ///< Enable channel 2 HW handshake. Enable UART2 DIR/OE
	PIN_UART_CFG_CH3_DIROE = 0x10000,    ///< Enable channel 3 HW handshake. Enable UART3 DIR/OE
	//////////////////////////////////
	//Only support @ 528 below  -----------------------------------------------------------------------------------------------------------------------------------------------------
	PIN_UART_CFG_CH5 = 0x20000,            ///< Not support (Backward compatible)
	PIN_UART_CFG_CH5_CTSRTS = 0x40000,     ///< Not support (Backward compatible)
	PIN_UART_CFG_CH5_2ND = 0x80000,        ///< Not support (Backward compatible)

	PIN_UART_CFG_CH6 = 0x100000,            ///< Not support (Backward compatible)
	PIN_UART_CFG_CH6_CTSRTS = 0x200000,     ///< Not support (Backward compatible)
	PIN_UART_CFG_CH6_2ND = 0x400000,        ///< Not support (Backward compatible)

	PIN_UART_CFG_CH4_DIROE = 0x800000,      ///< Not support (Backward compatible)
	PIN_UART_CFG_CH5_DIROE = 0x1000000,     ///< Not support (Backward compatible)
	PIN_UART_CFG_CH6_DIROE = 0x2000000,     ///< Not support (Backward compatible)

	ENUM_DUMMY4WORD(PIN_UART_CFG)
} PIN_UART_CFG;

/**
    PIN config for SPI

    @note For pinmux_init() with PIN_FUNC_SPI.\n
            For example, you can use {PIN_FUNC_SPI, PIN_SPI_CFG_CH1|PIN_SPI_CFG_CH3}\n
            to declare SPI channel1 and channel3 are enabled.
*/
typedef enum {
	PIN_SPI_CFG_NONE,

	PIN_SPI_CFG_CH1            = 0x01,   ///< Enable channel 1. Enable @MC17[SPI_1_CLK]|MC18[SPI_1_DO]|MC19[SPI_1_CS]. Enable 2BITs && MC20[SPI_1_DI]
	PIN_SPI_CFG_CH1_2BITS      = 0x02,   ///< Enable channel 1 with 2 bits mode. Enable SPI_1_DI on MC[20]  or SPI_2_DI on S_GPIO[5]
	PIN_SPI_CFG_CH1_4BITS      = 0x04,   ///< Not support SPI1 4-bit mode
	PIN_SPI_CFG_CH1_2ND_PINMUX = 0x08,   ///< Enable 2nd pinmux of channel 1. Enable @S_GPIO6[SPI_2_CS]|S_GPIO7[SPI_2_CLK]|S_GPIO8[SPI_2_DO]. Enable 2BITs &&S_GPIO5[SPI_2_DI]

	PIN_SPI_CFG_CH2            = 0x10,   ///< Enable channel 2. Eanble @DSI_GPIO4[SPI2_1_CS]|DSI_GPIO5[SPI2_1_CLK]|DSI_GPIO6[SPI2_1_DO]. Enable 2BITs && DSI_GPIO7[SPI2_1_DI])
	PIN_SPI_CFG_CH2_2BITS      = 0x20,   ///< Enable channel 2. Enable SPI2_1_DI on DSI_GPIO[7]  or SPI2_2_DI on P_GPIO[3]
	PIN_SPI_CFG_CH2_2ND_PINMUX = 0x40,   ///< Enable 2nd pinmux of channel 2. Eanble @P_GPIO0[SPI2_2_CS]|P_GPIO1[SPI2_2_CLK]|P_GPIO2[SPI2_2_DO]. Enable 2BITs && P_GPIO3[SPI2_2_DI])

	PIN_SPI_CFG_CH3            = 0x100,   ///< Enable channel 3. Eanble @P_GPIO17[SPI3_1_CS]|P_GPIO18[SPI3_1_CLK]|P_GPIO19[SPI3_1_DO]. Enable 2BITs && P_GPIO20[SPI3_1_DI])
	PIN_SPI_CFG_CH3_2BITS      = 0x200,   ///< Enable channel 3. Eanble SPI3_1_DI on P_GPIO[20] or  SPI3_2_DI on MC14 or SPI3_3_DI on P_GPIO[7]
	PIN_SPI_CFG_CH3_2ND_PINMUX = 0x400,   ///< Enable 2nd pinmux of channel 3. Enable @MC11[SPI3_2_CLK]|MC12[SPI3_2_DO]|MC13[SPI3_2_CS]. Enable 2BITs && MC14[SPI3_2_DI]
	PIN_SPI_CFG_CH3_RDY        = 0x800,   ///< Enable channel 3 RDY. If enable SPI3_1_RDY (P_GPIO[21]). If enable SPI3_2_RDY (MC15).    SPI3_3_RDY (P_GPIO[8]) 

	//Only support @ 528 below  (CH4/CH5)  ---------------------------------------------------------------------------------------------------------------------
	PIN_SPI_CFG_CH4            = 0x1000,    ///< Not support
	PIN_SPI_CFG_CH4_2BITS      = 0x2000,    ///< Not support
	PIN_SPI_CFG_CH4_2ND_PINMUX = 0x4000,    ///< Not support
	PIN_SPI_CFG_CH4_RDY        = 0x8000,    ///< Not support

	PIN_SPI_CFG_CH5            = 0x10000,   ///< Not support
	PIN_SPI_CFG_CH5_2BITS      = 0x20000,   ///< Not support
	PIN_SPI_CFG_CH5_2ND_PINMUX = 0x40000,   ///< Not support

	//Only support @ 520  (CH3_3RD)  ---------------------------------------------------------------------------------------------------------------------
	PIN_SPI_CFG_CH3_3RD_PINMUX = 0x80000,   ///< Enable 3rd pinmux of channel 3. Enable @P_GPIO4[SPI3_3_CS]|P_GPIO5[SPI3_3_CLK]|P_GPIO6[SPI3_3_DO]. Enable 2BITs && P_GPIO7[SPI3_3_DI])
	//Only support @ 520  (CH3_3RD)  ---------------------------------------------------------------------------------------------------------------------

	//Only support @ 528 below  (CH1_3RD)  ---------------------------------------------------------------------------------------------------------------------
	PIN_SPI_CFG_CH1_3RD_PINMUX = 0x100000,   ///< Not support
    //Special mode for application
	PIN_SPI_CFG_CH1_DI_ONLY = 0x200000,   ///< Enable channel 1. Only enable @MC20[SPI_1_DI]

	ENUM_DUMMY4WORD(PIN_SPI_CFG)
} PIN_SPI_CFG;

/**
    PIN config for SDP

    @note For pinmux_init() with PIN_FUNC_SDP.\n
            For example, you can use {PIN_FUNC_SDP, PIN_SDP_CFG_CH1}\n
            to declare SDP pinmux is enabled.
*/
typedef enum {
	PIN_SDP_CFG_NONE,
	PIN_SDP_CFG_CH1,                 ///< Enable SDP channel 1. Enable @MC11[SDP_1_RDY]|MC12[SDP_1_CLK]|MC13[SDP_1_DO]|MC14[SDP_1_CS]|MC15[SDP_1_DI]
	PIN_SDP_CFG_CH2,                 ///< Enable SDP channel 2. Enable @P_GPIO4[SDP_2_CS]|P_GPIO5[SDP_2_CLK]|P_GPIO6[SDP_2_DO]|P_GPIO7[SDP_2_DI]|P_GPIO8[SDP_2_RDY]

	ENUM_DUMMY4WORD(PIN_SDP_CFG)
} PIN_SDP_CFG;


/**
    PIN config for REMOTE

    @note For pinmux_init() with PIN_FUNC_REMOTE.\n
            For example, you can use {PIN_FUNC_REMOTE, PIN_REMOTE_CFG_CH1}\n
            to declare remote pinmux is enabled.
*/
typedef enum {
	PIN_REMOTE_CFG_NONE,
	PIN_REMOTE_CFG_CH1 = 0x01,                 ///< Enable remote rx channel 1. Enable REMOTE_RX @P_GPIO17[remote_in_1]
	PIN_REMOTE_CFG_CH2 = 0x02,                 ///< Enable remote rx channel 2. Enable REMOTE_RX @MC14[remote_in_2]
	PIN_REMOTE_CFG_CH3 = 0x04,                 ///< Reserved
	PIN_REMOTE_CFG_EXT = 0x10,                 ///< Enable remote in ext

	ENUM_DUMMY4WORD(PIN_REMOTE_CFG)
} PIN_REMOTE_CFG;

/**
    PIN config for PWM

    @note For pinmux_init() with PIN_FUNC_PWM.\n
            For example, you can use {PIN_FUNC_PWM, PIN_PWM_CFG_PWM0_1|PIN_PWM_CFG_PWM1_1|PIN_PWM_CFG_PWM2_1|PIN_PWM_CFG_PWM3_1}\n
            to declare your system need PWM channel0/1/2/3.
*/
typedef enum {
    PIN_PWM_CFG_NONE,
    PIN_PWM_CFG_PWM0_1 = 0x01,            ///< Enable PWM0 @P_GPIO0[PWM0_1]
    PIN_PWM_CFG_PWM0_2 = 0x02,            ///< Enable PWM0 @MC13[PWM0_2]
    PIN_PWM_CFG_PWM0_3 = 0x04,            ///< Enable PWM0 @S_GPIO1[PWM0_3]
    PIN_PWM_CFG_PWM0_4 = 0x08,            ///< Enable PWM0 @MC4[PWM0_4]
    PIN_PWM_CFG_PWM0_5 = 0x10,            ///< Enable PWM0 @DSI_GPIO0[PWM0_5]
    PIN_PWM_CFG_PWM1_1 = 0x20,            ///< Enable PWM1 @P_GPIO1[PWM1_1]
    PIN_PWM_CFG_PWM1_2 = 0x40,            ///< Enable PWM1 @MC14[PWM1_2]
    PIN_PWM_CFG_PWM1_3 = 0x80,            ///< Enable PWM1 @S_GPIO2[PWM1_3]
    PIN_PWM_CFG_PWM1_4 = 0x100,           ///< Enable PWM1 @MC5[PWM1_4]
    PIN_PWM_CFG_PWM1_5 = 0x200,           ///< Enable PWM1 @DSI_GPIO1[PWM1_5]
    PIN_PWM_CFG_PWM2_1 = 0x400,           ///< Enable PWM2 @P_GPIO2[PWM2_1]
    PIN_PWM_CFG_PWM2_2 = 0x800,           ///< Enable PWM2 @MC15[PWM2_2]
    PIN_PWM_CFG_PWM2_3 = 0x1000,          ///< Enable PWM2 @S_GPIO3[PWM2_3]
    PIN_PWM_CFG_PWM2_4 = 0x2000,          ///< Enable PWM2 @MC6[PWM2_4]
    PIN_PWM_CFG_PWM2_5 = 0x4000,          ///< Enable PWM2 @DSI_GPIO2[PWM2_5]
    PIN_PWM_CFG_PWM3_1 = 0x8000,          ///< Enable PWM3 @P_GPIO3[PWM3_1]
    PIN_PWM_CFG_PWM3_2 = 0x10000,         ///< Enable PWM3 @MC16[PWM3_2]
    PIN_PWM_CFG_PWM3_3 = 0x20000,         ///< Enable PWM3 @S_GPIO4[PWM3_3]
    PIN_PWM_CFG_PWM3_4 = 0x40000,         ///< Enable PWM3 @MC7[PWM3_4]
    PIN_PWM_CFG_PWM3_5 = 0x80000,         ///< Enable PWM3 @DSI_GPIO3[PWM3_5]

    PIN_PWM_CFG_PWM8_1 = 0x100000,        ///< Enable PWM8 @P_GPIO8[PWM8_1]
    PIN_PWM_CFG_PWM8_2 = 0x200000,        ///< Enable PWM8 @MC19[PWM8_2]
    PIN_PWM_CFG_PWM8_3 = 0x400000,        ///< Enable PWM8 @HSI_GPIO6[PWM8_3]
    PIN_PWM_CFG_PWM8_4 = 0x800000,        ///< Enable PWM8 @D_GPIO8[PWM8_4]
    PIN_PWM_CFG_PWM8_5 = 0x1000000,       ///< Enable PWM8 @DSI_GPIO8[PWM8_5]
    PIN_PWM_CFG_PWM9_1 = 0x2000000,       ///< Enable PWM9 @P_GPIO9[PWM9_1]
    PIN_PWM_CFG_PWM9_2 = 0x4000000,       ///< Enable PWM9 @MC20[PWM9_2]
    PIN_PWM_CFG_PWM9_3 = 0x8000000,       ///< Enable PWM9 @HSI_GPIO7[PWM9_3]
    PIN_PWM_CFG_PWM9_4 = 0x10000000,      ///< Enable PWM9 @D_GPIO9[PWM9_4]
    PIN_PWM_CFG_PWM9_5 = 0x20000000,      ///< Enable PWM9 @DSI_GPIO9[PWM9_5]

    ENUM_DUMMY4WORD(PIN_PWM_CFG)
} PIN_PWM_CFG;

/**
    PIN config2 for PWM

    @note For pinmux_init() with PIN_FUNC_PWM2.\n
            For example, you can use {PIN_FUNC_PWM2, PIN_PWM_CFG2_PWM4_1|PIN_PWM_CFG2_PWM5_1|PIN_PWM_CFG_PWM6_1|PIN_PWM_CFG_PWM7_1}\n
            to declare your system need PWM channel4/5/6/7.
*/
typedef enum {
    PIN_PWM_CFG2_NONE,

    PIN_PWM_CFG2_PWM4_1 = 0x01,            ///< Enable PWM4 @P_GPIO4[PWM4_1]
    PIN_PWM_CFG2_PWM4_2 = 0x02,            ///< Enable PWM4 @MC11[PWM4_2]
    PIN_PWM_CFG2_PWM4_3 = 0x04,            ///< Enable PWM4 @S_GPIO5[PWM4_3]
    PIN_PWM_CFG2_PWM4_4 = 0x08,            ///< Enable PWM4 @D_GPIO3[PWM4_4]
    PIN_PWM_CFG2_PWM4_5 = 0x10,            ///< Enable PWM4 @DSI_GPIO4[PWM4_5]
    PIN_PWM_CFG2_PWM5_1 = 0x20,            ///< Enable PWM5 @P_GPIO5[PWM5_1]
    PIN_PWM_CFG2_PWM5_2 = 0x40,            ///< Enable PWM5 @MC12[PWM5_2]
    PIN_PWM_CFG2_PWM5_3 = 0x80,            ///< Enable PWM5 @S_GPIO6[PWM5_3]
    PIN_PWM_CFG2_PWM5_4 = 0x100,           ///< Enable PWM5 @D_GPIO4[PWM5_4]
    PIN_PWM_CFG2_PWM5_5 = 0x200,           ///< Enable PWM5 @DSI_GPIO5[PWM5_5]
    PIN_PWM_CFG2_PWM6_1 = 0x400,           ///< Enable PWM6 @P_GPIO6[PWM6_1]
    PIN_PWM_CFG2_PWM6_2 = 0x800,           ///< Enable PWM6 @MC17[PWM6_2]
    PIN_PWM_CFG2_PWM6_3 = 0x1000,          ///< Enable PWM6 @S_GPIO7[PWM6_3]
    PIN_PWM_CFG2_PWM6_4 = 0x2000,          ///< Enable PWM6 @D_GPIO5[PWM6_4]
    PIN_PWM_CFG2_PWM6_5 = 0x4000,          ///< Enable PWM6 @DSI_GPIO6[PWM6_5]
    PIN_PWM_CFG2_PWM7_1 = 0x8000,          ///< Enable PWM7 @P_GPIO7[PWM7_1]
    PIN_PWM_CFG2_PWM7_2 = 0x10000,         ///< Enable PWM7 @MC18[PWM7_2]
    PIN_PWM_CFG2_PWM7_3 = 0x20000,         ///< Enable PWM7 @S_GPIO8[PWM7_3]
    PIN_PWM_CFG2_PWM7_4 = 0x40000,         ///< Enable PWM7 @D_GPIO6[PWM7_4]
    PIN_PWM_CFG2_PWM7_5 = 0x80000,         ///< Enable PWM7 @DSI_GPIO7[PWM7_5]

    PIN_PWM_CFG2_PWM10_1 = 0x100000,       ///< Enable PWM10 @P_GPIO10[PWM10_1]
    PIN_PWM_CFG2_PWM10_2 = 0x200000,       ///< Enable PWM10 @MC21[PWM10_2]
    PIN_PWM_CFG2_PWM10_3 = 0x400000,       ///< Enable PWM10 @HSI_GPIO8[PWM10_3]
    PIN_PWM_CFG2_PWM10_4 = 0x800000,       ///< Enable PWM10 @D_GPIO10[PWM10_4]
    PIN_PWM_CFG2_PWM10_5 = 0x1000000,      ///< Enable PWM10 @DSI_GPIO10[PWM10_5]
    PIN_PWM_CFG2_PWM11_1 = 0x2000000,      ///< Enable PWM11 @P_GPIO11[PWM11_1]
    PIN_PWM_CFG2_PWM11_2 = 0x4000000,      ///< Enable PWM11 @MC22[PWM11_2]
    PIN_PWM_CFG2_PWM11_3 = 0x8000000,      ///< Enable PWM11 @HSI_GPIO9[PWM11_3]
    PIN_PWM_CFG2_PWM11_4 = 0x10000000,     ///< Enable PWM11 @D_GPIO7[PWM11_4]
    PIN_PWM_CFG2_PWM11_5 = 0x20000000,     ///< Enable PWM11 @L_GPIO0[PWM11_5]

    ENUM_DUMMY4WORD(PIN_PWM_CFG2)
} PIN_PWM_CFG2;

typedef enum {
	PIN_PWM_CFG_CCNT_NONE,
	PIN_PWM_CFG_CCNT      = 0x1,      ///< Enable PICNT_1 @P_GPIO12[PICNT_1]
	PIN_PWM_CFG_CCNT_2ND  = 0x2,      ///< Enable PICNT_2 @P_GPIO18[PICNT_2]
	PIN_PWM_CFG_CCNT2     = 0x4,      ///< Enable PICNT2_1 @P_GPIO14[PICNT2_1]
	PIN_PWM_CFG_CCNT2_2ND = 0x8,      ///< Enable PICNT2_2 @P_GPIO19[PICNT2_2]
	PIN_PWM_CFG_CCNT3     = 0x10,     ///< Enable PICNT3_1 @P_GPIO17[PICNT3_1]
	PIN_PWM_CFG_CCNT3_2ND = 0x20,     ///< Enable PICNT3_2 @P_GPIO20[PICNT3_2]

	ENUM_DUMMY4WORD(PIN_CCNT_CFG)
} PIN_CCNT_CFG;


/**
    PIN config for AUDIO

    @note For pinmux_init() with PIN_FUNC_AUDIO.\n
            For example, you can use {PIN_FUNC_AUDIO, PIN_AUDIO_CFG_I2S|PIN_AUDIO_CFG_MCLK}\n
            to declare pinmux of I2S and I2S MCLK are enabled.
*/
typedef enum {
	PIN_AUDIO_CFG_NONE,
	PIN_AUDIO_CFG_I2S = 0x01,              ///< Enable I2S channel 1. Enable @P_GPIO9[I2S_BCLK_1]|P_GPIO10[I2S_SYNC_1]|P_GPIO11[I2S_SDATAO_1]|P_GPIO12[I2S_SDATAI_1]
	PIN_AUDIO_CFG_I2S_2ND_PINMUX = 0x02,///< Enable I2S channel 2. Enable @MC12[I2S_BCLK_2]|MC13[I2S_SYNC_2]|MC14[I2S_SDATAO_2]|MC15[I2S_SDATAI_2]

	PIN_AUDIO_CFG_MCLK = 0x10,             ///< Enable audio I2S_MCLK_1. Enable I2S_MCLK_1@P_GPIO8[I2S_MCLK_1])
	PIN_AUDIO_CFG_MCLK_2ND_PINMUX = 0x20,///< Enable audio I2S_MCLK_2. Enable I2S_MCLK_2@MC11[I2S_MCLK_2])

	PIN_AUDIO_CFG_DMIC = 0x100,           ///< Enable Digital Mic channel 1. Enable @P_GPIO19[DMCLK_1]|P_GPIO17[DMDAT0_1]|P_GPIO18[DMDAT1_1]
	PIN_AUDIO_CFG_DMIC_2ND = 0x200,       ///< Enable Digital Mic channel 2. Enable @DSI_GPIO10[DMCLK_2]|DSI_GPIO8[DMDAT0_2]|DSI_GPIO9[DMDAT1_2]
	PIN_AUDIO_CFG_DMIC_3RD = 0x400,       ///< Enable Digital Mic channel 3. Enable @MC11[DMCLK_3]|MC12[DMDAT0_3]|MC13[DMDAT1_3]

	ENUM_DUMMY4WORD(PIN_AUDIO_CFG)
} PIN_AUDIO_CFG;

/**
    PIN config of LCD modes

    @note For pinmux_init() with PIN_FUNC_LCD \n
            For example, you can use {PIN_FUNC_LCD, PINMUX_DISPMUX_SEL_LCD | PINMUX_LCDMODE_RGB_SERIAL}\n
            to tell display object that PIN_FUNC_LCD is located on primary LCD pinmux,\n
            and it's LCD mode is RGB serial.
*/
typedef enum {
	PINMUX_LCDMODE_RGB_SERIAL           =    0,     ///< LCD MODE is RGB Serial or UPS051
	PINMUX_LCDMODE_RGB_PARALL           =    1,     ///< LCD MODE is RGB Parallel (888)
	PINMUX_LCDMODE_YUV640               =    2,     ///< LCD MODE is YUV640
	PINMUX_LCDMODE_YUV720               =    3,     ///< LCD MODE is YUV720
	PINMUX_LCDMODE_RGBD360              =    4,     ///< LCD MODE is RGB Dummy 360
	PINMUX_LCDMODE_RGBD320              =    5,     ///< LCD MODE is RGB Dummy 320
	PINMUX_LCDMODE_RGB_THROUGH          =    6,     ///< LCD MODE is RGB through mode
	PINMUX_LCDMODE_CCIR601              =    7,     ///< LCD MODE is CCIR601
	PINMUX_LCDMODE_CCIR656              =    8,     ///< LCD MODE is CCIR656
	PINMUX_LCDMODE_RGB_PARALL666        =    9,     ///< LCD MODE is RGB Parallel 666
	PINMUX_LCDMODE_RGB_PARALL565        =   10,     ///< LCD MODE is RGB Parallel 565
	PINMUX_LCDMODE_RGB_PARALL_DELTA     =   11,     ///< LCD MODE is RGB Parallel Delta
	PINMUX_LCDMODE_MIPI                 =   12,     ///< LCD MODE is MIPI Display

	PINMUX_LCDMODE_MI_OFFSET            = 32,       ///< Memory LCD MODE offset (not support MI. Below is backward compatible)
	PINMUX_LCDMODE_MI_FMT0              = 32 + 0,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT1              = 32 + 1,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT2              = 32 + 2,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT3              = 32 + 3,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT4              = 32 + 4,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT5              = 32 + 5,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT6              = 32 + 6,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT7              = 32 + 7,   ///< LCD MODE is Memory(Parallel Interface) 9bits
	PINMUX_LCDMODE_MI_FMT8              = 32 + 8,   ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT9              = 32 + 9,   ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT10             = 32 + 10,  ///< LCD MODE is Memory(Parallel Interface) 18bits
	PINMUX_LCDMODE_MI_FMT11             = 32 + 11,  ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT12             = 32 + 12,  ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT13             = 32 + 13,  ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT14             = 32 + 14,  ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_SERIAL_BI         = 32 + 20,  ///< LCD MODE is Serial Interface bi-direction
	PINMUX_LCDMODE_MI_SERIAL_SEP        = 32 + 21,  ///< LCD MODE is Serial Interface separation

	PINMUX_LCDMODE_AUTO_PINMUX          = 0x01 << 22, ///< Set display device to GPIO mode when display device is closed. Select this filed will inform display object to switch to GPIO when display device is closed.


	PINMUX_HDMIMODE_OFFSET              = 64,       ///< HDMI MODE offset (not support HDMI. Below is backward compatible)
	PINMUX_HDMIMODE_640X480P60          = 64 + 1,   ///< HDMI Video format is 640x480 & Progressive 60fps
	PINMUX_HDMIMODE_720X480P60          = 64 + 2,   ///< HDMI Video format is 720x480 & Progressive 60fps & 4:3
	PINMUX_HDMIMODE_720X480P60_16X9     = 64 + 3,   ///< HDMI Video format is 720x480 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_1280X720P60         = 64 + 4,   ///< HDMI Video format is 1280x720 & Progressive 60fps
	PINMUX_HDMIMODE_1920X1080I60        = 64 + 5,   ///< HDMI Video format is 1920x1080 & Interlaced 60fps
	PINMUX_HDMIMODE_720X480I60          = 64 + 6,   ///< HDMI Video format is 720x480 & Interlaced 60fps
	PINMUX_HDMIMODE_720X480I60_16X9     = 64 + 7,   ///< HDMI Video format is 720x480 & Interlaced 60fps & 16:9
	PINMUX_HDMIMODE_720X240P60          = 64 + 8,   ///< HDMI Video format is 720x240 & Progressive 60fps
	PINMUX_HDMIMODE_720X240P60_16X9     = 64 + 9,   ///< HDMI Video format is 720x240 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_1440X480I60         = 64 + 10,  ///< HDMI Video format is 1440x480 & Interlaced 60fps
	PINMUX_HDMIMODE_1440X480I60_16X9    = 64 + 11,  ///< HDMI Video format is 1440x480 & Interlaced 60fps & 16:9
	PINMUX_HDMIMODE_1440X240P60         = 64 + 12,  ///< HDMI Video format is 1440x240 & Progressive 60fps
	PINMUX_HDMIMODE_1440X240P60_16X9    = 64 + 13,  ///< HDMI Video format is 1440x240 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_1440X480P60         = 64 + 14,  ///< HDMI Video format is 1440x480 & Progressive 60fps
	PINMUX_HDMIMODE_1440X480P60_16X9    = 64 + 15,  ///< HDMI Video format is 1440x480 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_720X576P50          = 64 + 17,  ///< HDMI Video format is 720x576 & Progressive 50fps
	PINMUX_HDMIMODE_720X576P50_16X9     = 64 + 18,  ///< HDMI Video format is 720x576 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1280X720P50         = 64 + 19,  ///< HDMI Video format is 1280x720 & Progressive 50fps
	PINMUX_HDMIMODE_1920X1080I50        = 64 + 20,  ///< HDMI Video format is 1920x1080 & Interlaced 50fps
	PINMUX_HDMIMODE_720X576I50          = 64 + 21,  ///< HDMI Video format is 720x576 & Interlaced 50fps
	PINMUX_HDMIMODE_720X576I50_16X9     = 64 + 22,  ///< HDMI Video format is 720x576 & Interlaced 50fps & 16:9
	PINMUX_HDMIMODE_720X288P50          = 64 + 23,  ///< HDMI Video format is 720x288 & Progressive 50fps
	PINMUX_HDMIMODE_720X288P50_16X9     = 64 + 24,  ///< HDMI Video format is 720x288 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1440X576I50         = 64 + 25,  ///< HDMI Video format is 1440x576 & Interlaced 50fps
	PINMUX_HDMIMODE_1440X576I50_16X9    = 64 + 26,  ///< HDMI Video format is 1440x576 & Interlaced 50fps & 16:9
	PINMUX_HDMIMODE_1440X288P50         = 64 + 27,  ///< HDMI Video format is 1440x288 & Progressive 50fps
	PINMUX_HDMIMODE_1440X288P50_16X9    = 64 + 28,  ///< HDMI Video format is 1440x288 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1440X576P50         = 64 + 29,  ///< HDMI Video format is 1440x576 & Progressive 50fps
	PINMUX_HDMIMODE_1440X576P50_16X9    = 64 + 30,  ///< HDMI Video format is 1440x576 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1920X1080P50        = 64 + 31,  ///< HDMI Video format is 1920x1080 & Progressive 50fps
	PINMUX_HDMIMODE_1920X1080P24        = 64 + 32,  ///< HDMI Video format is 1920x1080 & Progressive 24fps
	PINMUX_HDMIMODE_1920X1080P25        = 64 + 33,  ///< HDMI Video format is 1920x1080 & Progressive 25fps
	PINMUX_HDMIMODE_1920X1080P30        = 64 + 34,  ///< HDMI Video format is 1920x1080 & Progressive 30fps
	PINMUX_HDMIMODE_1920X1080I50_VT1250 = 64 + 39,  ///< HDMI Video format is 1920x1080 & Interlaced  50fps & V-total is 1250 lines
	PINMUX_HDMIMODE_1920X1080I100       = 64 + 40,  ///< HDMI Video format is 1920x1080 & Interlaced  100fps
	PINMUX_HDMIMODE_1280X720P100        = 64 + 41,  ///< HDMI Video format is 1280X720  & Progressive 100fps
	PINMUX_HDMIMODE_720X576P100         = 64 + 42,  ///< HDMI Video format is 720X576   & Progressive 100fps
	PINMUX_HDMIMODE_720X576P100_16X9    = 64 + 43,  ///< HDMI Video format is 720X576   & Progressive 100fps & 16:9
	PINMUX_HDMIMODE_720X576I100         = 64 + 44,  ///< HDMI Video format is 720X576  & Interlaced 100fps
	PINMUX_HDMIMODE_720X576I100_16X9    = 64 + 45,  ///< HDMI Video format is 720X576  & Interlaced 100fps & 16:9
	PINMUX_HDMIMODE_1920X1080I120       = 64 + 46,  ///< HDMI Video format is 1920X1080 & Interlaced 120fps
	PINMUX_HDMIMODE_1280X720P120        = 64 + 47,  ///< HDMI Video format is 1280X720  & Progressive 120fps
	PINMUX_HDMIMODE_720X480P120         = 64 + 48,  ///< HDMI Video format is 720X480   & Progressive 120fps
	PINMUX_HDMIMODE_720X480P120_16X9    = 64 + 49,  ///< HDMI Video format is 720X480   & Progressive 120fps & 16:9
	PINMUX_HDMIMODE_720X480I120         = 64 + 50,  ///< HDMI Video format is 720X480  & Interlaced 120fps
	PINMUX_HDMIMODE_720X480I120_16X9    = 64 + 51,  ///< HDMI Video format is 720X480  & Interlaced 120fps & 16:9
	PINMUX_HDMIMODE_720X576P200         = 64 + 52,  ///< HDMI Video format is 720X576  & Progressive 200fps
	PINMUX_HDMIMODE_720X576P200_16X9    = 64 + 53,  ///< HDMI Video format is 720X576  & Progressive 200fps & 16:9
	PINMUX_HDMIMODE_720X576I200         = 64 + 54,  ///< HDMI Video format is 720X576  & Interlaced 200fps
	PINMUX_HDMIMODE_720X576I200_16X9    = 64 + 55,  ///< HDMI Video format is 720X576  & Interlaced 200fps & 16:9
	PINMUX_HDMIMODE_720X480P240         = 64 + 56,  ///< HDMI Video format is 720X480  & Progressive 240fps
	PINMUX_HDMIMODE_720X480P240_16X9    = 64 + 57,  ///< HDMI Video format is 720X480  & Progressive 240fps & 16:9
	PINMUX_HDMIMODE_720X480I240         = 64 + 58,  ///< HDMI Video format is 720X480  & Interlaced 240fps
	PINMUX_HDMIMODE_720X480I240_16X9    = 64 + 59,  ///< HDMI Video format is 720X480  & Interlaced 240fps & 16:9

	PINMUX_DSI_1_LANE_CMD_MODE_RGB565           = 128 + 0, ///< DSI command mode with RGB565 format   (not support DSI. Below is backward compatible)
	PINMUX_DSI_1_LANE_CMD_MODE_RGB666P          = 128 + 1, ///< DSI command mode with RGB666 packed
	PINMUX_DSI_1_LANE_CMD_MODE_RGB666L          = 128 + 2, ///< DSI command mode with RGB666 loosely
	PINMUX_DSI_1_LANE_CMD_MODE_RGB888           = 128 + 3, ///< DSI command mode with RGB888

	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB565     = 128 + 4, ///< DSI video sync pulse mode with RGB565 format
	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB666P    = 128 + 5, ///< DSI video sync pulse mode with RGB666 packed
	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB666L    = 128 + 6, ///< DSI video sync pulse mode with RGB666 loosely
	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB888     = 128 + 7, ///< DSI video sync pulse mode with RGB888

	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB565     = 128 + 8, ///< DSI video sync event burst mode with RGB565 format
	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB666P    = 128 + 9, ///< DSI video sync event burst mode with RGB666 packed
	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB666L    = 128 + 10, ///< DSI video sync event burst mode with RGB666 loosely
	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB888     = 128 + 11, ///< DSI video sync event burst mode with RGB888

	PINMUX_DSI_2_LANE_CMD_MODE_RGB565           = 128 + 12, ///< DSI command mode with RGB565 format
	PINMUX_DSI_2_LANE_CMD_MODE_RGB666P          = 128 + 13, ///< DSI command mode with RGB666 packed
	PINMUX_DSI_2_LANE_CMD_MODE_RGB666L          = 128 + 14, ///< DSI command mode with RGB666 loosely
	PINMUX_DSI_2_LANE_CMD_MODE_RGB888           = 128 + 15, ///< DSI command mode with RGB888

	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB565     = 128 + 16, ///< DSI video sync pulse mode with RGB565 format
	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB666P    = 128 + 17, ///< DSI video sync pulse mode with RGB666 packed
	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB666L    = 128 + 18, ///< DSI video sync pulse mode with RGB666 loosely
	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB888     = 128 + 19, ///< DSI video sync pulse mode with RGB888

	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB565     = 128 + 20, ///< DSI video sync event burst mode with RGB565 format
	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB666P    = 128 + 21, ///< DSI video sync event burst mode with RGB666 packed
	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB666L    = 128 + 22, ///< DSI video sync event burst mode with RGB666 loosely
	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB888     = 128 + 23, ///< DSI video sync event burst mode with RGB888

	PINMUX_DSI_4_LANE_CMD_MODE_RGB565           = 128 + 24, ///< DSI command mode with RGB565 format
	PINMUX_DSI_4_LANE_CMD_MODE_RGB666P          = 128 + 25, ///< DSI command mode with RGB666 packed
	PINMUX_DSI_4_LANE_CMD_MODE_RGB666L          = 128 + 26, ///< DSI command mode with RGB666 loosely
	PINMUX_DSI_4_LANE_CMD_MODE_RGB888           = 128 + 27, ///< DSI command mode with RGB888

	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB565     = 128 + 28, ///< DSI video sync pulse mode with RGB565 format
	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB666P    = 128 + 29, ///< DSI video sync pulse mode with RGB666 packed
	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB666L    = 128 + 30, ///< DSI video sync pulse mode with RGB666 loosely
	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB888     = 128 + 31, ///< DSI video sync pulse mode with RGB888

	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB565     = 128 + 32, ///< DSI video sync event burst mode with RGB565 format
	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB666P    = 128 + 33, ///< DSI video sync event burst mode with RGB666 packed
	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB666L    = 128 + 34, ///< DSI video sync event burst mode with RGB666 loosely
	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB888     = 128 + 35, ///< DSI video sync event burst mode with RGB888



	ENUM_DUMMY4WORD(PINMUX_LCDINIT)
} PINMUX_LCDINIT;

/**
    PIN config for Parallel MI, not support (Backward compatible)

    @note For pinmux_init() with PIN_FUNC_LCD or PIN_FUNC_LCD2.\n
            For example, you can use {PIN_FUNC_LCD, PINMUX_DISPMUX_SEL_LCD|PINMUX_PMI_CFG_NORMAL|PINMUX_LCDMODE_MI_FMT0}\n
            to tell display object that PIN_FUNC_LCD is bound to MI and format is FMT0,\n
            and MI is located at primary location.
*/
typedef enum {
	PINMUX_PMI_CFG_NORMAL = 0x00,                   ///< Normal Parallel MI location (at LCD)
	PINMUX_PMI_CFG_2ND_PINMUX = 0x01 << 26,         ///< Secondary Parallel MI location

	PINMUX_PMI_CFG_MASK = 0x03 << 26,
	ENUM_DUMMY4WORD(PINMUX_PMI_CFG)
} PINMUX_PMI_CFG;

/**
    PIN config for TV/HDMI

    @note For pinmux_init() for PIN_FUNC_TV or PIN_FUNC_HDMI.\n
            For example, you can use {PIN_FUNC_HDMI, PINMUX_TV_HDMI_CFG_NORMAL|PINMUX_HDMIMODE_1280X720P60}\n
            to tell display object that HDMI activation will disable PANEL,\n
            and HDMI mode is 1280x720 P60.
*/
typedef enum {
	PINMUX_TV_HDMI_CFG_GPIO = 0x00,                 ///< TV activation will disable PINMUX to GPIO
	PINMUX_TV_HDMI_CFG_NORMAL = 0x00,               ///< TV activation will disable PANEL which shared the same IDE
	PINMUX_TV_HDMI_CFG_PINMUX_ON = 0x01 << 28,      ///< TV activation will keep PINMUX setting

	PINMUX_TV_HDMI_CFG_MASK = 0x03 << 28,
	ENUM_DUMMY4WORD(PINMUX_TV_HDMI_CFG)
} PINMUX_TV_HDMI_CFG;

/**
    PIN config for HDMI, not support (Backward compatible)

    @note For pinmux_init() for PIN_FUNC_HDMI.\n
            For example, you can use {PIN_FUNC_HDMI, PINMUX_HDMI_CFG_CEC|PINMUX_TV_HDMI_CFG_NORMAL}\n
            to declare HDMI CEC pinmux is enabled.
*/
typedef enum {
	PINMUX_HDMI_CFG_GPIO = 0x00,                    ///< HDMI specific PIN to GPIO
	PINMUX_HDMI_CFG_HOTPLUG = 0x01 << 26,           ///< HDMI HOTPLUG.
	PINMUX_HDMI_CFG_CEC = 0x02 << 26,               ///< HDMI CEC.

	PINMUX_HDMI_CFG_MASK = 0x03 << 26,
	ENUM_DUMMY4WORD(PINMUX_HDMI_CFG)
} PINMUX_HDMI_CFG;

/**
    PIN location of LCD

    @note For pinmux_init() with PIN_FUNC_LCD \n
            For example, you can use {PIN_FUNC_LCD, PINMUX_DISPMUX_SEL_LCD|PINMUX_LCDMODE_XXX}\n
            to tell display object that PIN_FUNC_LCD is located on primary LCD pinmux.
*/
typedef enum {
	PINMUX_DISPMUX_SEL_NONE = 0x00 << 28,           ///< PINMUX none
	PINMUX_DISPMUX_SEL_LCD = 0x01 << 28,            ///< PINMUX at LCD interface
	PINMUX_DISPMUX_SEL_LCD2 = 0x02 << 28,           ///< PINMUX at LCD2 interface (Not supported)

	PINMUX_DISPMUX_SEL_MASK = 0x03 << 28,
	ENUM_DUMMY4WORD(PINMUX_DISPMUX_SEL)
} PINMUX_DISPMUX_SEL;

/**
    PIN config for USB

    @note For pinmux_init() for PIN_FUNC_USB.\n
            For example, you can use {PIN_FUNC_USB, PINMUX_USB_CFG_DEVICE}\n
            to select USB as device.
*/
typedef enum {
	PINMUX_USB_CFG_NONE = 0x00,                     ///< USB as device
	PINMUX_USB_CFG_DEVICE = 0x00,                   ///< USB as device
	PINMUX_USB_CFG_HOST = 0x01,                     ///< USB as host

	ENUM_DUMMY4WORD(PINMUX_USB_CFG)
} PINMUX_USB_CFG;

/**
    PIN config for ETH

    @note For pinmux_init() for PIN_FUNC_ETH.\n
            For example, you can use {PIN_FUNC_ETH, PINMUX_ETH_CFG_RMII}\n
            to select ethernet interface.
*/
typedef enum {
	PIN_ETH_CFG_NONE   = 0x00,                      ///< PINMUX none
	PIN_ETH_CFG_RMII   = 0x01,                      ///< ETH RMII.  Enable RMII on @ LCD0~LCD10
	PIN_ETH_CFG_INTERANL = 0x02,                    ///< ETH INTERNAL MII
	PIN_ETH_CFG_EXTPHYCLK = 0x04,                   ///< ETH EXT_PHY_CLK

	PIN_ETH_CFG_LED1   = 0x10,                      ///< ETH INTERNAL PHY LED1
	PIN_ETH_CFG_LED2   = 0x20,                      ///< ETH INTERNAL PHY LED2

	PIN_ETH_CFG_RMII_2   = 0x40,                    ///< ETH RMII_2.  Enable RMII_2
	PIN_ETH_CFG_RGMII    = 0x80,                    ///< Reserved , compatible here
	PIN_ETH_CFG_MDIO    = 0x100,                    ///< ETH MDIO
	PIN_ETH_CFG_MDIO_2    = 0x200,                 ///< ETH MDIO_2

	PIN_ETH_CFG_LINKLED_ONLY   = 0x400,       ///< ETH INTERNAL PHY LINK LED only
	PIN_ETH_CFG_ACTLED_ONLY   = 0x800,        ///< ETH INTERNAL PHY ACT LED only

	ENUM_DUMMY4WORD(PINMUX_ETH_CFG)
} PINMUX_ETH_CFG;

/**
    PIN config for MISC

    @note For pinmux_init() for PIN_FUNC_MISC.\n
            For example, you can use {PIN_FUNC_MISC, PIN_MISC_CFG_RTCLK}\n
            to select related application.
*/
typedef enum {
	PIN_MISC_CFG_NONE           = 0x00,                     ///< PINMUX none
	PIN_MISC_CFG_RTCLK          = 0x01,                     ///< Enable RTCCLK. Enable @ P_GPIO19[RTC_CLK]
	PIN_MISC_CFG_SPCLK_NONE     = 0x02,                     ///< SP CLK none
	PIN_MISC_CFG_SPCLK          = 0x04,                     ///< Enable SP CLK. Enable SP_CLK_1@ P_GPIO20[SP_CLK_1]
	PIN_MISC_CFG_SPCLK_2ND      = 0x08,                     ///< Enable SP_2 CLK. Enable @ L_GPIO2[SP_CLK_2]
	PIN_MISC_CFG_SPCLK_3RD      = 0x10,                     ///< Enable SP_3 CLK. Enable SP_CLK_3 @ D_GPIO3[SP_CLK_3]
	PIN_MISC_CFG_SP2CLK_NONE    = 0x20,                     ///< SP2 CLK none
	PIN_MISC_CFG_SP2CLK         = 0x40,                     ///< Enable SP2 CLK. Enable SP_CLK2_1@ P_GPIO21[SP_CLK2_1]
	PIN_MISC_CFG_SP2CLK_2ND     = 0x80,                     ///< Enable SP2_2 CLK. Enable SP_CLK2_2 @ MC9[SP_CLK2_2]
	PIN_MISC_CFG_SP2CLK_3RD     = 0x100,                    ///< Enable SP2_3 CLK. Enable SP_CLK2_2 @ D_GPIO4[SP_CLK2_3]

	ENUM_DUMMY4WORD(PINMUX_MISC_CFG)
} PINMUX_MISC_CFG;


/**
    Pinmux direct mode channel

    @note For pinmux_set_direct_channel()
*/
typedef enum {
	PINMUX_DIRECTMODE_CH_SDIO = 0x00,                         ///< Directmode channel is SDIO
	PINMUX_DIRECTMODE_CH_NAND = 0x01,                ///< Directmode channel is NAND
	ENUM_DUMMY4WORD(PINMUX_DIRECTMODE_CH)
} PINMUX_DIRECTMODE_CH;


/**
    Pinmux power domain isolated function enable/disable

    @note For pinmux_set_direct_channel()
*/
typedef enum {
	PINMUX_ISOFUNC_DISABLE = 0x00,               ///< DISABLE power domain isolated function
	PINMUX_ISOFUNC_ENABLE = 0x01,                ///< ENABLE  power domain isolated function
	ENUM_DUMMY4WORD(PINMUX_POWER_ISOFUNC)
} PINMUX_POWER_ISOFUNC;

/**
    Pinmux Function identifier

    @note For pinmux_getDispMode(), pinmux_setPinmux().
*/
typedef enum {
	PINMUX_FUNC_ID_LCD,                             ///< 1st Panel (LCD), pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	///< - @b PINMUX_LCD_SEL_CCIR656
	///< - @b PINMUX_LCD_SEL_CCIR601
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_6BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_8BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_9BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDIO
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO
	///< ORed with
	///< - @b PINMUX_LCD_SEL_DE_ENABLE
	///< - @b PINMUX_LCD_SEL_TE_ENABLE
	PINMUX_FUNC_ID_LCD2,                            ///< 2nd Panel (LCD), not support (Backward compatible)
	///< - @b PINMUX_LCD_SEL_GPIO
	///< - @b PINMUX_LCD_SEL_CCIR656
	///< - @b PINMUX_LCD_SEL_CCIR601
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_6BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_8BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_9BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDIO
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO
	///< ORed with
	///< - @b PINMUX_LCD_SEL_DE_ENABLE
	PINMUX_FUNC_ID_TV,                              ///< TV, pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	PINMUX_FUNC_ID_HDMI,                            ///< HDMI, not support (Backward compatible)
	///< - @b PINMUX_LCD_SEL_GPIO
	PINMUX_FUNC_ID_SN_MES0,                         ///< Sensor MES0, pinmux can be:
	///< - @b PINMUX_SENSOR_SEL_INACTIVE:
	///< - @b PINMUX_SENSOR_SEL_ACTIVE:
	PINMUX_FUNC_ID_SN_MES1,                         ///< Sensor MES1, pinmux can be:
	///< - @b PINMUX_SENSOR_SEL_INACTIVE:
	///< - @b PINMUX_SENSOR_SEL_ACTIVE:
	PINMUX_FUNC_ID_SN_MES2,                         ///< Sensor MES2, pinmux can be:
	///< - @b PINMUX_SENSOR_SEL_INACTIVE:
	///< - @b PINMUX_SENSOR_SEL_ACTIVE:
	PINMUX_FUNC_ID_SN_MES3,                         ///< Sensor MES3, pinmux can be:
	///< - @b PINMUX_SENSOR_SEL_INACTIVE:
	///< - @b PINMUX_SENSOR_SEL_ACTIVE:
	PINMUX_FUNC_ID_SN_FLCTR,                        ///< Sensor Flash Control, pinmux can be:
	///< - @b PINMUX_SENSOR_SEL_INACTIVE:
	///< - @b PINMUX_SENSOR_SEL_ACTIVE:
	PINMUX_FUNC_ID_SN_MCLK,                         ///< Sensor MCLK, pinmux can be:
	///< - @b PINMUX_SENSOR_SEL_INACTIVE:
	///< - @b PINMUX_SENSOR_SEL_ACTIVE:
	PINMUX_FUNC_ID_SPI2,                            ///< SPI channel2 pinmux switch, pinmux can be:
	///< - @b PINMUX_SPI_SEL_INACTIVE
	///< - @b PINMUX_SPI_SEL_ACTIVE
	PINMUX_FUNC_ID_SPI3,                            ///< SPI channel3 pinmux switch, pinmux can be:
	///< - @b PINMUX_SPI_SEL_INACTIVE
	///< - @b PINMUX_SPI_SEL_ACTIVE
	PINMUX_FUNC_ID_SPI4,                            ///< SPI channel4 pinmux switch, not support (Backward compatible)
	///< - @b PINMUX_SPI_SEL_INACTIVE
	///< - @b PINMUX_SPI_SEL_ACTIVE
	PINMUX_FUNC_ID_SPI5,                            ///< SPI channel5 pinmux switch, not support (Backward compatible)
	///< - @b PINMUX_SPI_SEL_INACTIVE
	///< - @b PINMUX_SPI_SEL_ACTIVE
	PINMUX_FUNC_ID_COUNT,                           //< Total function count

	ENUM_DUMMY4WORD(PINMUX_FUNC_ID)
} PINMUX_FUNC_ID;

/**
    Pinmux selection for LCD

    @note For PINMUX_FUNC_ID_LCD, PINMUX_FUNC_ID_LCD2
*/
typedef enum {
	PINMUX_LCD_SEL_GPIO,                            ///< GPIO
	PINMUX_LCD_SEL_CCIR656,                         ///< CCIR-656 8 bits. Enable CCIR_YC[0..7]/CCIR_CLK on LCD[0..8] (L_GPIO[0..8])
	PINMUX_LCD_SEL_CCIR656_16BITS,                  ///< CCIR-656 16 bits. Enable CCIR_Y[0..7]/CCIR_CLK/CCIR_C[0..7] on LCD[0..8]/LCD[12..19] (L_GPIO[0..8]/L_GPIO[12..19])
	PINMUX_LCD_SEL_CCIR601,                         ///< CCIR-601 8 bits. Enable CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD on LCD[0..10] (L_GPIO[0..10])
	PINMUX_LCD_SEL_CCIR601_16BITS,                  ///< CCIR-601 16 bits. Enable CCIR_Y[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_C[0..7] on LCD[0..19] (L_GPIO[0..19])
	PINMUX_LCD_SEL_SERIAL_RGB_6BITS,                ///< Serial RGB 6 bits. Enable RGB_D[2..7]/RGB_CLK/RGB_VD/RGB_HD on LCD[2..10] (L_GPIO[2..10])
	PINMUX_LCD_SEL_SERIAL_RGB_8BITS,                ///< Serial RGB 8 bits. Enable RGB_D[0..7]/RGB_CLK/RGB_VD/RGB_HD on LCD[0..10] (L_GPIO[0..10])
	PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS,              ///< Serial YCbCr 8 bits. Enable CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD on LCD[0..10] (L_GPIO[0..10])
	PINMUX_LCD_SEL_PARALLE_RGB565,                  ///< Parallel RGB565. Enable RGB_C0_[0..4]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..5]/RGB_C2_[0..4] on LCD[0..19](L_GPIO[0..19])
	PINMUX_LCD_SEL_PARALLE_RGB666,                  ///< Parallel RGB666. (Not support)
	PINMUX_LCD_SEL_PARALLE_RGB888,                  ///< Parallel RGB888. (Not support)
	PINMUX_LCD_SEL_RGB_16BITS,                      ///< RGB 16 bits. Enable CCIR_Y[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_C[0..7] on LCD[0..19] (L_GPIO[0..19])
	PINMUX_LCD_SEL_PARALLE_MI_8BITS,                ///< Parallel MI 8 bits. (Not support)
	PINMUX_LCD_SEL_PARALLE_MI_9BITS,                ///< Parallel MI 9 bits.(Not support)
	PINMUX_LCD_SEL_PARALLE_MI_16BITS,               ///< Parallel MI 16 bits. (Not support)
	PINMUX_LCD_SEL_PARALLE_MI_18BITS,               ///< Parallel MI 18 bits. (Not support)
	PINMUX_LCD_SEL_SERIAL_MI_SDIO,                  ///< Serial MI SDIO bi-direction. (Not support)
	PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO,               ///< Serial MI SDI/SDO seperate. (Not support)
	PINMUX_LCD_SEL_MIPI,                            ///< MIPI DSI

	PINMUX_LCD_SEL_TE_ENABLE = 0x01 << 23,          ///< TE Enable (For Parallel/Serial MI) (Not support)
	PINMUX_LCD_SEL_DE_ENABLE = 0x01 << 24,          ///< DE Enable (For CCIR656 8/16bits, CCIR601 16bits, Serial RGB 6/8bits, Serial YCbCr 8 bits, RGB 16 bits)
	///< When corresponding item in pinmux_init() is set with PINMUX_DISPMUX_SEL_LCD, enable CCIR_DE on LCD11 (L_GPIO[11]).
	PINMUX_LCD_SEL_HVLD_VVLD = 0x01 << 25,          ///< HVLD/VVLD Enable (For CCIR-601 8 bits). Enable CCIR_HVLD/CCIR_VVLD on LCD[12..13] (L_GPIO[12..13])
	PINMUX_LCD_SEL_FIELD = 0x01 << 26,              ///< FIELD Enable (For CCIR-601 8 bits). Enable CCIR_FIELD on LCD14 (L_GPIO[14])
	PINMUX_LCD_SEL_NO_HVSYNC = 0x1 << 27,           ///< No HSYNC/VSYNC
	PINMUX_LCD_SEL_FEATURE_MSK = 0x1F << 23,

	ENUM_DUMMY4WORD(PINMUX_LCD_SEL)
} PINMUX_LCD_SEL;

/**
    Pinmux selection for Storage

    @note For PINMUX_FUNC_ID_SDIO, PINMUX_FUNC_ID_SPI, PINMUX_FUNC_ID_NAND
*/
typedef enum {
	PINMUX_STORAGE_SEL_INACTIVE,                    ///< Inactive storage
	PINMUX_STORAGE_SEL_ACTIVE,                      ///< Active storage
	PINMUX_STORAGE_SEL_INEXIST,                     ///< Inexist storage
	PINMUX_STORAGE_SEL_EXIST,                       ///< Exist storage
	ENUM_DUMMY4WORD(PINMUX_STORAGE_SEL)
} PINMUX_STORAGE_SEL;

/**
    Pinmux selection for sensor

    @note For PINMUX_FUNC_ID_SN_MES0, PINMUX_FUNC_ID_SN_MES1, PINMUX_FUNC_ID_SN_FLCTR
*/
typedef enum {
	PINMUX_SENSOR_SEL_INACTIVE,                     ///< Inactive
	PINMUX_SENSOR_SEL_ACTIVE,                       ///< Active
	ENUM_DUMMY4WORD(PINMUX_SENSOR_SEL)
} PINMUX_SENSOR_SEL;

/**
    Pinmux selection for usb

    @note For PINMUX_FUNC_ID_USB_VBUSI
*/
typedef enum {
	PINMUX_USB_SEL_INACTIVE,                        ///< Inactive
	PINMUX_USB_SEL_ACTIVE,                          ///< Active
	ENUM_DUMMY4WORD(PINMUX_USB_SEL)
} PINMUX_USB_SEL;

/**
    Pinmux selection for SPI

    @note For PINMUX_FUNC_ID_SPI2, PINMUX_FUNC_ID_SPI3
*/
typedef enum {
	PINMUX_SPI_SEL_INACTIVE,                        ///< Inactive
	PINMUX_SPI_SEL_ACTIVE,                          ///< Active
	ENUM_DUMMY4WORD(PINMUX_SPI_SEL)
} PINMUX_SPI_SEL;

/*
    Pinmux Function identifier for driver only

    @note For pinmux_setPinmux().
*/
typedef enum {
	PINMUX_FUNC_ID_SDIO = 0x8000000,                ///< SDIO, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_SDIO2,                           ///< SDIO2, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_SDIO3,                           ///< SDIO3, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_SPI,                             ///< SPI, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_NAND,                            ///< NAND, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_BMC,                             ///< BMC, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_USB_VBUSI,                       ///< USB VBUSI, pinmux can be:
	///< - @b PINMUX_USB_SEL_INACTIVE
	///< - @b PINMUX_USB_SEL_ACTIVE
	PINMUX_FUNC_ID_USB2_VBUSI,                      ///< USB2 VBUSI, pinmux can be:
	///< - @b PINMUX_USB_SEL_INACTIVE
	///< - @b PINMUX_USB_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C1_1ST,                        ///< I2C channel1 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C1_2ND,                        ///< I2C channel1 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C2_1ST,                        ///< I2C channel2 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C2_2ND,                        ///< I2C channel2 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C3_1ST,                        ///< I2C channel3 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C3_2ND,                        ///< I2C channel3 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C3_3RD,                        ///< I2C channel3 3rd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_1ST,                        ///< I2C channel4 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_2ND,                        ///< I2C channel4 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_3RD,                        ///< I2C channel4 3rd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_4TH,                        ///< I2C channel4 4th pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C5_1ST,                        ///< I2C channel5 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C5_2ND,                        ///< I2C channel5 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_SN_VDHD,                         ///< SN VD/HD, pinmux can be:
	///< - @b PINMUX_SENSOR_SEL_INACTIVE
	///< - @b PINMUX_SENSOR_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_0,                           ///< PWM channel 0
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_1,                           ///< PWM channel 1
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_2,                           ///< PWM channel 2
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_3,                           ///< PWM channel 3
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_4,                           ///< PWM channel 4
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_5,                           ///< PWM channel 5
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_6,                           ///< PWM channel 6
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_7,                           ///< PWM channel 7
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_8,                           ///< PWM channel 8
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_9,                           ///< PWM channel 9
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_10,                          ///< PWM channel 10
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_11,                          ///< PWM channel 11
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_12,                          ///< PWM channel 12
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_13,                          ///< PWM channel 13
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_14,                          ///< PWM channel 14
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_15,                          ///< PWM channel 15
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_16,                          ///< PWM channel 16
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_17,                          ///< PWM channel 17
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_18,                          ///< PWM channel 18
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_19,                          ///< PWM channel 19
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_0,                           ///< SIF channel 0
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_1,                           ///< SIF channel 1
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_2,                           ///< SIF channel 2
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_3,                           ///< SIF channel 3
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_4,                           ///< SIF channel 4
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_5,                           ///< SIF channel 5
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_6,                           ///< SIF channel 6
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_7,                           ///< SIF channel 7
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_1,                          ///< UART channel 1
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_2,                          ///< UART channel 2
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_3,                          ///< UART channel 3
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_4,                          ///< UART channel 4
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_I2S,                             ///< I2S
	///< - @b PINMUX_I2S_SEL_INACTIVE
	///< - @b PINMUX_I2S_SEL_ACTIVE
	PINMUX_FUNC_ID_I2S_MCLK,                        ///< I2S MCLK
	///< - @b PINMUX_I2S_SEL_INACTIVE
	///< - @b PINMUX_I2S_SEL_ACTIVE
	PINMUX_FUNC_ID_DSP_PERI_CH_EN,                  ///< DSP PERI channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE
	PINMUX_FUNC_ID_DSP_IOP_CH_EN,                   ///< DSP IOP channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE
	PINMUX_FUNC_ID_DSP2_PERI_CH_EN,                 ///< DSP2 PERI channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE
	PINMUX_FUNC_ID_DSP2_IOP_CH_EN,                  ///< DSP2 IOP channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE

	ENUM_DUMMY4WORD(PINMUX_FUNC_ID_DRV)
} PINMUX_FUNC_ID_DRV;

/**
    Pinmux selection for boot source

    @note For BOOT_SRC_ENUM
*/
typedef enum
{
	BOOT_SRC_SPINOR,                    //< Boot from SPI NOR
	BOOT_SRC_CARD,                      //< Boot from CARD
	BOOT_SRC_SPINAND,                   //< Boot from SPI NAND with on die ECC (2K page)
	BOOT_SRC_SPINAND_RS,                //< Boot from SPI NAND (Reed Solomon ECC) (2K page)
	BOOT_SRC_ETH,                       //< Boot from Ethernet
	BOOT_SRC_USB_HS,                    //< Boot from USB (High Speed)
	BOOT_SRC_SPINAND_4K,                //< Boot from SPI NAND with on die ECC (4K page)
	BOOT_SRC_RESERVED,                  //< RESERVED
	BOOT_SRC_EMMC_4BIT,                 //< Boot from eMMC 4-bit
	BOOT_SRC_EMMC_8BIT,                 //< Boot from eMMC 8-bit
	BOOT_SRC_SPINAND_RS_4K,             //< Boot from SPI NAND (Reed Solomon ECC) (4K page)
	BOOT_SRC_USB_FS,                    //< Boot from USB (Full Speed)
	ENUM_DUMMY4WORD(BOOT_SRC_ENUM)
} BOOT_SRC_ENUM;


/**
    Pinmux group

    @note For pinmux_init()
*/
typedef struct {
	PIN_FUNC    pin_function;            ///< PIN Function group
	UINT32      config;                 ///< Configuration for pinFunction
} PIN_GROUP_CONFIG;


enum CHIP_ID {
	CHIP_NA51055 = 0x4821,
	CHIP_NA51084 = 0x5021,
	CHIP_NA51089 = 0x7021
};

//@}    //addtogroup mIHALSysCfg

int nvt_pinmux_capture(PIN_GROUP_CONFIG *pinmux_config, int count);
int nvt_pinmux_update(PIN_GROUP_CONFIG *pinmux_config, int count);
int pinmux_set_config(PINMUX_FUNC_ID id, uint32_t pinmux);
UINT32 nvt_get_chip_id(void);
PINMUX_LCDINIT pinmux_get_dispmode(PINMUX_FUNC_ID id);
int nvt_pinmux_init(void);

int pinmux_select_debugport(PINMUX_DEBUGPORT uiDebug);
UINT32 top_get_bs(void);

void pinmux_enable_pwr_isofunc(PINMUX_POWER_ISOFUNC en);
void pinmux_set_direct_channel(PINMUX_DIRECTMODE_CH ch);

#endif

