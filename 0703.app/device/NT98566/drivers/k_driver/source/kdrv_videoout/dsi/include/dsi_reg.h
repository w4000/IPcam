#ifndef __DSI_REG_H__
#define __DSI_REG_H__



#ifdef __KERNEL__
#include <mach/rcw_macro.h>
#include "kwrap/type.h"

#define REGISTER_ADDR_START 0xf0840000
#define REGISTER_ADDR_END   0xf0840900
#define IRQ_ID              1
#else
#if defined(__FREERTOS)
#include "rcw_macro.h"
#include "kwrap/type.h"
#else
#include "DrvCommon.h"
#endif
#endif



// 0x00 MIPI DSI Enable Register
#define DSI_EN_REG_OFS                      0x00
REGDEF_BEGIN(DSI_EN_REG)
REGDEF_BIT(TX_EN, 1)
REGDEF_BIT(PHY_EN, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(DAT0_ESC_TRIG, 1)
REGDEF_BIT(DAT1_ESC_TRIG, 1)
REGDEF_BIT(DAT2_ESC_TRIG, 1)
REGDEF_BIT(DAT3_ESC_TRIG, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(CLK_ULP_TRIG, 1)
REGDEF_BIT(, 22)
REGDEF_END(DSI_EN_REG)

// 0x04 MIPI DSI Control Register 0
#define DSI_CTRL0_REG_OFS                   0x04
REGDEF_BEGIN(DSI_CTRL0_REG)
REGDEF_BIT(MODE, 3)
REGDEF_BIT(, 1)
REGDEF_BIT(PIXEL_FMT, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(PIXPKT_MODE, 1)
REGDEF_BIT(VDOPKT_TYPE, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(RX_ECC_CHK_EN, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(FRM_END_BTA_EN, 1)
REGDEF_BIT(EOT_PKT_EN, 1)
REGDEF_BIT(BLANK_CTRL, 1)
REGDEF_BIT(INTER_PKT_LP, 1)
REGDEF_BIT(CLK_LP_CTRL, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(SYNC_EN, 1)
REGDEF_BIT(SYNC_SRC, 1)
REGDEF_BIT(SYNC_WITH_SETTEON, 1)
REGDEF_BIT(SYNC_WITH_SETTEON_RTY, 1)
REGDEF_BIT(RGB_SWAP, 1)
REGDEF_BIT(BRG_BIT_SWAP, 1)
REGDEF_BIT(SYNC_WITH_SETTEON_RTY_TWICEBTA, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(DATA_LANE_NO, 2)
REGDEF_BIT(, 1)
REGDEF_BIT(DSI_SRC_SEL, 1)
REGDEF_END(DSI_CTRL0_REG)

// 0x08 MIPI DSI PIXEL Packet Data Type Register
#define DSI_PIXTYPE_REG_OFS                 0x08
REGDEF_BEGIN(DSI_PIXTYPE_REG)
REGDEF_BIT(PIXPKT_PH_DT, 6)
REGDEF_BIT(PIXPKT_PH_VC, 2)
REGDEF_BIT(, 8)
REGDEF_BIT(DCS_CT0, 8)
REGDEF_BIT(DCS_CT1, 8)
REGDEF_END(DSI_PIXTYPE_REG)

// 0x0C MIPI DSI VDO Sync Event Control Register
#define DSI_SYNCEVT_CTRL_REG_OFS            0x0C
REGDEF_BEGIN(DSI_SYNCEVT_CTRL_REG)
REGDEF_BIT(SLICE_NO, 4)
REGDEF_BIT(, 4)
REGDEF_BIT(NULL_LEN, 13)
REGDEF_BIT(, 11)
REGDEF_END(DSI_SYNCEVT_CTRL_REG)

// 0x10 MIPI DSI Verical Timing Control Register 0
#define DSI_VERT_CTRL0_REG_OFS              0x10
REGDEF_BEGIN(DSI_VERT_CTRL0_REG)
REGDEF_BIT(VSA, 9)
REGDEF_BIT(, 7)
REGDEF_BIT(VTOTAL, 11)
REGDEF_BIT(, 5)
REGDEF_END(DSI_VERT_CTRL0_REG)

// 0x14 MIPI DSI Vertical Timing Control Register 1
#define DSI_VERT_CTRL1_REG_OFS              0x14
REGDEF_BEGIN(DSI_VERT_CTRL1_REG)
REGDEF_BIT(VVALID_START, 10)
REGDEF_BIT(, 6)
REGDEF_BIT(VVALID_END, 11)
REGDEF_BIT(, 5)
REGDEF_END(DSI_VERT_CTRL1_REG)

// 0x18 MIPI DSI Horizontal Timing Control Register 0
#define DSI_HORI_CTRL0_REG_OFS              0x18
REGDEF_BEGIN(DSI_HORI_CTRL0_REG)
REGDEF_BIT(HSA, 11)
REGDEF_BIT(, 5)
REGDEF_BIT(BLLP, 13)
REGDEF_BIT(, 3)
REGDEF_END(DSI_HORI_CTRL0_REG)

// 0x1C MIPI DSI Horizontal Timing Control Register 1
#define DSI_HORI_CTRL1_REG_OFS              0x1C
REGDEF_BEGIN(DSI_HORI_CTRL1_REG)
REGDEF_BIT(HBP, 11)
REGDEF_BIT(, 5)
REGDEF_BIT(HFP, 11)
REGDEF_BIT(, 5)
REGDEF_END(DSI_HORI_CTRL1_REG)

// 0x20 MIPI DSI Horizontal Timing Control Register 2
#define DSI_HORI_CTRL2_REG_OFS              0x20
REGDEF_BEGIN(DSI_HORI_CTRL2_REG)
REGDEF_BIT(HACT, 13)
REGDEF_BIT(, 19)
REGDEF_END(DSI_HORI_CTRL2_REG)

// 0x24 MIPI DSI Escape Control Register 0
#define DSI_ESCCTRL0_REG_OFS                0x24
REGDEF_BEGIN(DSI_ESCCTRL0_REG)
REGDEF_BIT(DAT0_ESC_START, 1)
REGDEF_BIT(DAT1_ESC_START, 1)
REGDEF_BIT(DAT2_ESC_START, 1)
REGDEF_BIT(DAT3_ESC_START, 1)
REGDEF_BIT(DAT0_ESC_STOP, 2)
REGDEF_BIT(DAT1_ESC_STOP, 2)
REGDEF_BIT(DAT2_ESC_STOP, 2)
REGDEF_BIT(DAT3_ESC_STOP, 2)
REGDEF_BIT(CLK_ULP_SEL, 1)
REGDEF_BIT(, 19)
REGDEF_END(DSI_ESCCTRL0_REG)

// 0x28 MIPI DSI Escape Control Register 1
#define DSI_ESCCTRL1_REG_OFS                0x28
REGDEF_BEGIN(DSI_ESCCTRL1_REG)
REGDEF_BIT(DAT0_ESC_CMD, 8)
REGDEF_BIT(DAT1_ESC_CMD, 8)
REGDEF_BIT(DAT2_ESC_CMD, 8)
REGDEF_BIT(DAT3_ESC_CMD, 8)
REGDEF_END(DSI_ESCCTRL1_REG)


// 0x2C MIPI DSI Command RW Control Register
#define DSI_CMDCTRL_REG_OFS                 0x2C
REGDEF_BEGIN(DSI_CMDCTRL_REG)
REGDEF_BIT(CMD_NO, 3)
REGDEF_BIT(, 5)
REGDEF_BIT(CMD_BTA_EN, 1)
REGDEF_BIT(CMD_BTA_ONLY, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(SRAM_RD_OFS, 8)
REGDEF_BIT(SRAM_RD_CNT, 9)
REGDEF_BIT(, 3)
REGDEF_END(DSI_CMDCTRL_REG)

// 0x30 MIPI DSI Command Register 0
#define DSI_CMD0_REG_OFS                    0x30
REGDEF_BEGIN(DSI_CMD0_REG)
REGDEF_BIT(CMD0_DT, 6)
REGDEF_BIT(CMD0_VC, 2)
REGDEF_BIT(CMD0_WC, 16)
REGDEF_BIT(, 4)
REGDEF_BIT(CMD0_LPKT, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_CMD0_REG)

// 0x34 MIPI DSI Command Register 1
#define DSI_CMD1_REG_OFS                    0x34
REGDEF_BEGIN(DSI_CMD1_REG)
REGDEF_BIT(CMD1_DT, 6)
REGDEF_BIT(CMD1_VC, 2)
REGDEF_BIT(CMD1_WC, 16)
REGDEF_BIT(, 4)
REGDEF_BIT(CMD1_LPKT, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_CMD1_REG)

// 0x38 MIPI DSI Command Register 2
#define DSI_CMD2_REG_OFS                    0x38
REGDEF_BEGIN(DSI_CMD2_REG)
REGDEF_BIT(CMD2_DT, 6)
REGDEF_BIT(CMD2_VC, 2)
REGDEF_BIT(CMD2_WC, 16)
REGDEF_BIT(, 4)
REGDEF_BIT(CMD2_LPKT, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_CMD2_REG)

// 0x3C MIPI DSI Command Register 3
#define DSI_CMD3_REG_OFS                    0x3C
REGDEF_BEGIN(DSI_CMD2_REG)
REGDEF_BIT(CMD3_DT, 6)
REGDEF_BIT(CMD3_VC, 2)
REGDEF_BIT(CMD3_WC, 16)
REGDEF_BIT(, 4)
REGDEF_BIT(CMD3_LPKT, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_CMD3_REG)

// 0x40 MIPI DSI Command Register 4
#define DSI_CMD4_REG_OFS                    0x40
REGDEF_BEGIN(DSI_CMD4_REG)
REGDEF_BIT(CMD4_DT, 6)
REGDEF_BIT(CMD4_VC, 2)
REGDEF_BIT(CMD4_WC, 16)
REGDEF_BIT(, 4)
REGDEF_BIT(CMD4_LPKT, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_CMD4_REG)

// 0x44 MIPI DSI Command Register 5
#define DSI_CMD5_REG_OFS                    0x44
REGDEF_BEGIN(DSI_CMD5_REG)
REGDEF_BIT(CMD5_DT, 6)
REGDEF_BIT(CMD5_VC, 2)
REGDEF_BIT(CMD5_WC, 16)
REGDEF_BIT(, 4)
REGDEF_BIT(CMD5_LPKT, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_CMD5_REG)

// 0x48 MIPI DSI Command Register 6
#define DSI_CMD6_REG_OFS                    0x48
REGDEF_BEGIN(DSI_CMD6_REG)
REGDEF_BIT(CMD6_DT, 6)
REGDEF_BIT(CMD6_VC, 2)
REGDEF_BIT(CMD6_WC, 16)
REGDEF_BIT(, 4)
REGDEF_BIT(CMD6_LPKT, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_CMD6_REG)

// 0x4C MIPI DSI Command Register 7
#define DSI_CMD7_REG_OFS                    0x4C
REGDEF_BEGIN(DSI_CMD7_REG)
REGDEF_BIT(CMD7_DT, 6)
REGDEF_BIT(CMD7_VC, 2)
REGDEF_BIT(CMD7_WC, 16)
REGDEF_BIT(, 4)
REGDEF_BIT(CMD7_LPKT, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_CMD7_REG)

// 0x50 MIPI DSI IDE Horizontal Timing Control Register 0
#define DSI_IDEHORT_CTRL0_REG_OFS           0x50
REGDEF_BEGIN(DSI_IDEHORT_CTRL0_REG)
REGDEF_BIT(HVLD, 12)
REGDEF_BIT(, 20)
REGDEF_END(DSI_IDEHORT_CTRL0_REG)



// 0x80 MIPI DSI Interrupt Enable Register
#define DSI_INTEN_REG_OFS                   0x80
REGDEF_BEGIN(DSI_INTEN_REG)
REGDEF_BIT(TX_DIS, 1)
REGDEF_BIT(FRM_END, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(SRAM_OV, 1)
REGDEF_BIT(BTA_TIMEOUT, 1)
REGDEF_BIT(BTA_HANDSK_TIMEOUT, 1)
REGDEF_BIT(CONTENTION, 1)
REGDEF_BIT(RX_RDY, 1)
REGDEF_BIT(ERR_REPORT, 1)
REGDEF_BIT(RX_ECC_ERR, 1)
REGDEF_BIT(RX_CRC_ERR, 1)
REGDEF_BIT(RX_STATE_ERR, 1)
REGDEF_BIT(RX_INSUF_PL, 1)
REGDEF_BIT(RX_ALIGN_ERR, 1)
REGDEF_BIT(FIFO_UNDER, 1)
REGDEF_BIT(SYNC_ERR, 1)
REGDEF_BIT(RX_UNKNOWN_PKT, 1)
REGDEF_BIT(RX_INVALID_ESCCMD, 1)
REGDEF_BIT(HS_FIFO_UNDER, 1)
REGDEF_BIT(, 4)
REGDEF_BIT(CLK_ULPS_DONE, 1)
REGDEF_BIT(DAT0_ESC_DONE, 1)
REGDEF_BIT(DAT1_ESC_DONE, 1)
REGDEF_BIT(DAT2_ESC_DONE, 1)
REGDEF_BIT(DAT3_ESC_DONE, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_INTEN_REG)

// 0x84 MIPI DSI Interrupt Status Register
#define DSI_INTSTS_REG_OFS                  0x84
REGDEF_BEGIN(DSI_INTSTS_REG)
REGDEF_BIT(TX_DIS, 1)
REGDEF_BIT(FRM_END, 1)
REGDEF_BIT(, 2)

REGDEF_BIT(SRAM_OV, 1)
REGDEF_BIT(BTA_TIMEOUT, 1)
REGDEF_BIT(BTA_HANDSK_TIMEOUT, 1)
REGDEF_BIT(CONTENTION, 1)

REGDEF_BIT(RX_RDY, 1)
REGDEF_BIT(ERR_REPORT, 1)
REGDEF_BIT(RX_ECC_ERR, 1)
REGDEF_BIT(RX_CRC_ERR, 1)

REGDEF_BIT(RX_STATE_ERR, 1)
REGDEF_BIT(RX_INSUF_PL, 1)
REGDEF_BIT(RX_ALIGN_ERR, 1)
REGDEF_BIT(FIFO_UNDER, 1)

REGDEF_BIT(SYNC_ERR, 1)
REGDEF_BIT(RX_UNKNOWN_PKT, 1)
REGDEF_BIT(RX_INVALID_ESCCMD, 1)
REGDEF_BIT(HS_FIFO_UNDER, 1)
REGDEF_BIT(, 4)

REGDEF_BIT(CLK_ULPS_DONE, 1)
REGDEF_BIT(DAT0_ESC_DONE, 1)
REGDEF_BIT(DAT1_ESC_DONE, 1)
REGDEF_BIT(DAT2_ESC_DONE, 1)
REGDEF_BIT(DAT3_ESC_DONE, 1)
REGDEF_BIT(, 3)
REGDEF_END(DSI_INTSTS_REG)

// 0x88 MIPI DSI Error Report Register
#define DSI_ERREPORT_REG_OFS                0x88
REGDEF_BEGIN(DSI__ERREPORT_REG)
REGDEF_BIT(SOT_ERR, 1)
REGDEF_BIT(SOT_SYNC_ERR, 1)
REGDEF_BIT(EOT_SYNC_ERR, 1)
REGDEF_BIT(ESC_ENTRY_CMD_ERR, 1)
REGDEF_BIT(LP_TX_SYNC_ERR, 1)
REGDEF_BIT(HS_RX_TIMEOUT, 1)
REGDEF_BIT(FALSE_CTRL_ERR, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(ECC_SINGLE_BIT, 1)
REGDEF_BIT(ECC_MULTI_BIT, 1)
REGDEF_BIT(CRC_ERR, 1)
REGDEF_BIT(DT_NOT_RECOGNIZED, 1)
REGDEF_BIT(VC_ID_INVALID, 1)
REGDEF_BIT(INVALID_TX_LENGTH, 1)
REGDEF_BIT(, 1)
REGDEF_BIT(PROTOCOL_VIALATION, 1)
REGDEF_BIT(, 16)
REGDEF_END(DSI__ERREPORT_REG)

// 0x8C MIPI DSI Bus Timing Control Register 0
#define DSI_BUS_TIME0_REG_OFS               0x8C
REGDEF_BEGIN(DSI_BUS_TIME0_REG)
REGDEF_BIT(TLPX, 4)
REGDEF_BIT(BTA_TA_GO, 6)
REGDEF_BIT(, 2)
REGDEF_BIT(BTA_TA_SURE, 5)
REGDEF_BIT(, 3)
REGDEF_BIT(BTA_TA_GET, 7)
REGDEF_BIT(, 5)
REGDEF_END(DSI_BUS_TIME0_REG)

// 0x90 MIPI DSI Bus Timing Control Register 1
#define DSI_BUS_TIME1_REG_OFS               0x90
REGDEF_BEGIN(DSI_BUS_TIME1_REG)
REGDEF_BIT(THS_PREPARE, 4)
REGDEF_BIT(THS_ZERO, 4)
REGDEF_BIT(THS_TRAIL, 4)
REGDEF_BIT(THS_EXIT, 6)
REGDEF_BIT(, 2)
REGDEF_BIT(TWAKEUP, 9)
REGDEF_BIT(, 3)
REGDEF_END(DSI_BUS_TIME1_REG)

// 0x94 MIPI DSI Bus Timing Control Register 2
#define DSI_BUS_TIME2_REG_OFS               0x94
REGDEF_BEGIN(DSI_BUS_TIME2_REG)
REGDEF_BIT(TCLK_PREPARE, 4)
REGDEF_BIT(TCLK_ZERO, 6)
REGDEF_BIT(, 2)
REGDEF_BIT(TCLK_POST, 6)
REGDEF_BIT(, 2)
REGDEF_BIT(TCLK_PRE, 4)
REGDEF_BIT(TCLK_TRAIL, 5)
REGDEF_BIT(, 3)
REGDEF_END(DSI_BUS_TIME2_REG)

// 0x98 MIPI DSI Bus Turn Around Control Register
#define DSI_BTA_CTRL_REG_OFS                0x98
REGDEF_BEGIN(DSI_BTA_CTRL_REG)
REGDEF_BIT(BTA_TIMEOUT_VAL, 16)
REGDEF_BIT(BTA_HANDSK_TIMEOUT, 8)
REGDEF_BIT(, 8)
REGDEF_END(DSI_BTA_CTRL_REG)

// 0xA0 MIPI DSI SYNC Control Register
#define DSI_SYNC_CTRL_REG_OFS               0xA0
REGDEF_BEGIN(DSI_SYNC_CTRL_REG)
REGDEF_BIT(SYNC_POL, 1)
REGDEF_BIT(SYNC_SEL, 1)
REGDEF_BIT(, 6)
REGDEF_BIT(SYNC_DLY_CLK, 10)
REGDEF_BIT(, 6)
REGDEF_BIT(TE_BTA_INTERVAL, 7)
REGDEF_BIT(, 1)
REGDEF_END(DSI_SYNC_CTRL_REG)

// 0xC0 MIPI DSI Debug 0 Register
#define DSI_DEBUG0_REG_OFS                  0xC0
REGDEF_BEGIN(DSI_DEBUG0_REG)
REGDEF_BIT(CONTENTION_CASE, 1)
REGDEF_BIT(SRAM_RD_RDY, 1)
REGDEF_BIT(, 6)
REGDEF_BIT(BLANK_DATA, 8)
REGDEF_BIT(NULL_DATA, 8)
REGDEF_BIT(, 8)
REGDEF_END(DSI_DEBUG0_REG)

// 0xC4 MIPI DSI Debug 1 Register
#define DSI_DEBUG1_REG_OFS                  0xC4
REGDEF_BEGIN(DSI_DEBUG1_REG)
REGDEF_BIT(TE_SYNC_DATA, 8)
REGDEF_BIT(, 8)
REGDEF_BIT(RX_UNKNOWN_DT, 6)
REGDEF_BIT(, 2)
REGDEF_BIT(RX_INVALID_ESCCMD, 8)
REGDEF_END(DSI_DEBUG1_REG)

// 0xD0 MIPI DSI Phy Register
#define DSI_DEBUG2_REG_OFS                  0xD0
REGDEF_BEGIN(DSI_DEBUG2_REG)
REGDEF_BIT(CONTENT_HS_SEL, 1)
REGDEF_BIT(, 3)
REGDEF_BIT(CONTENT_LP_HF_D0P, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(CONTENT_LP_HF_D0N, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(CONTENT_LP_LF_D0P, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(CONTENT_LP_LF_D0N, 2)
REGDEF_BIT(, 2)
REGDEF_BIT(PHY_LP_SLEW_RATE, 1)
REGDEF_BIT(PHY_DRVING, 2)
REGDEF_BIT(PHY_TRANSITION_OVLP, 1)
REGDEF_BIT(, 8)
REGDEF_END(DSI_DEBUG2_REG)

// 0xD4 MIPI DSI Phy 1 Register
#define DSI_DEBUG3_REG_OFS                  0xD4
REGDEF_BEGIN(DSI_DEBUG3_REG)
REGDEF_BIT(CLK_PHASE_OFS, 3)
REGDEF_BIT(, 1)
REGDEF_BIT(DAT0_PHASE_OFS, 3)
REGDEF_BIT(, 1)
REGDEF_BIT(DAT1_PHASE_OFS, 3)
REGDEF_BIT(, 1)
REGDEF_BIT(DAT2_PHASE_OFS, 3)
REGDEF_BIT(, 1)
REGDEF_BIT(DAT3_PHASE_OFS, 3)
REGDEF_BIT(, 1)
REGDEF_BIT(DAT0_LAND_SEL, 2)
REGDEF_BIT(DAT1_LAND_SEL, 2)
REGDEF_BIT(DAT2_LAND_SEL, 2)
REGDEF_BIT(DAT3_LAND_SEL, 2)
REGDEF_BIT(PHY_HS_CLK_INV, 1)
REGDEF_BIT(, 2)
REGDEF_BIT(PHY_DELAY_EN, 1)
REGDEF_END(DSI_DEBUG3_REG)

// 0xD8 MIPI DSI Phy 2 Register
#define DSI_DEBUG4_REG_OFS                  0xD8
REGDEF_BEGIN(DSI_DEBUG4_REG)
REGDEF_BIT(PHY_OPSEL, 1)
REGDEF_BIT(PHY_ISEL, 1)
REGDEF_BIT(PHY_VSEL, 1)
REGDEF_BIT(PHY_PDO, 1)
REGDEF_BIT(PHY_EN_SLEW, 1)
REGDEF_BIT(PHY_EN_PEM, 1)
REGDEF_BIT(PHY_SR, 2)
REGDEF_BIT(PHY_SF, 2)
REGDEF_BIT(, 18)
REGDEF_BIT(PHY_LP_RX_DAT0_EN, 1)
REGDEF_BIT(PHY_LP_RX_DAT1_EN, 1)
REGDEF_BIT(PHY_LP_RX_DAT2_EN, 1)
REGDEF_BIT(PHY_LP_RX_DAT3_EN, 1)
REGDEF_END(DSI_DEBUG4_REG)

// 0x100-0x1FC MIPI DSI SRAM Register Bank
#define DSI_SRAM_REG_OFS                    0x100
#endif
