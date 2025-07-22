//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2010  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:	MDINI2C.C
// Description 		:
// Ref. Docment		: 
// Revision History 	:

// ----------------------------------------------------------------------
// Include files
// ----------------------------------------------------------------------
#include	"mdin3xx.h"
//#define	__DEBUG__
// ----------------------------------------------------------------------
// Struct/Union Types and define
// ----------------------------------------------------------------------
#define		I2C_OK				0
#define		I2C_NOT_FREE		1
#define		I2C_HOST_NACK		2
#define		I2C_TIME_OUT		3

// ----------------------------------------------------------------------
// Static Global Data section variables
// ----------------------------------------------------------------------
static WORD PageID = 0;

// ----------------------------------------------------------------------
// External Variable 
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static Prototype Functions
// ----------------------------------------------------------------------

// user i2c functions
static BYTE MDINI2C_Write(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes);
static BYTE MDINI2C_Read(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes);

// ----------------------------------------------------------------------
// Static functions
// ----------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MDINI2C_SetPage(BYTE nID, WORD page)
{
#if	defined(SYSTEM_USE_MDIN380)&&defined(SYSTEM_USE_BUS_HIF)
	MDINBUS_SetPageID(page);	// set pageID to BUS-IF
#endif

#if !defined(FOR_TEST)
	if (page==PageID) return I2C_OK;	PageID = page;
#endif	/* defined(FOR_TEST) */
	return MDINI2C_Write(nID, 0x400, (PBYTE)&page, 2);	// write page
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MHOST_I2CWrite(WORD rAddr, PBYTE pBuff, WORD bytes)
{
	BYTE err = I2C_OK;

	err = MDINI2C_SetPage(MDIN_HOST_ID, 0x0000);	// write host page
	if (err) return err;

	err = MDINI2C_Write(MDIN_HOST_ID, rAddr, pBuff, bytes);
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MHOST_I2CRead(WORD rAddr, PBYTE pBuff, WORD bytes)
{
	BYTE err = I2C_OK;

	err = MDINI2C_SetPage(MDIN_HOST_ID, 0x0000);	// write host page
	if (err) return err;

	err = MDINI2C_Read(MDIN_HOST_ID, rAddr, pBuff, bytes);
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE LOCAL_I2CWrite(WORD rAddr, PBYTE pBuff, WORD bytes)
{
	BYTE err = I2C_OK;

	err = MDINI2C_SetPage(MDIN_LOCAL_ID, 0x0101);	// write local page
	if (err) return err;

	err = MDINI2C_Write(MDIN_LOCAL_ID, rAddr, pBuff, bytes);
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE LOCAL_I2CRead(WORD rAddr, PBYTE pBuff, WORD bytes)
{
	WORD RegOEN, err = I2C_OK;
	err = MDINI2C_SetPage(MDIN_LOCAL_ID, 0x0101);	// write local page
	if (err) return err;

	if		(rAddr>=0x030&&rAddr<0x036)	RegOEN = 0x04;	// mfc-size
	else if (rAddr>=0x043&&rAddr<0x045)	RegOEN = 0x09;	// out-ptrn
	else if (rAddr>=0x062&&rAddr<0x083)	RegOEN = 0x09;	// enhance
	else if (rAddr>=0x088&&rAddr<0x092)	RegOEN = 0x09;	// out-sync
	else if (rAddr>=0x094&&rAddr<0x097)	RegOEN = 0x09;	// out-sync
	else if (rAddr>=0x09a&&rAddr<0x09c)	RegOEN = 0x09;	// bg-color
	else if (rAddr>=0x0a0&&rAddr<0x0d0)	RegOEN = 0x09;	// out-ctrl
	else if (              rAddr<0x100)	RegOEN = 0x01;	// in-sync
	else if (rAddr>=0x100&&rAddr<0x140)	RegOEN = 0x05;	// main-fc
	else if (rAddr>=0x140&&rAddr<0x1a0)	RegOEN = 0x07;	// aux
	else if (rAddr>=0x1a0&&rAddr<0x1c0)	RegOEN = 0x03;	// arbiter
	else if (rAddr>=0x1c0&&rAddr<0x1e0)	RegOEN = 0x02;	// fc-mc
	else if (rAddr>=0x1e0&&rAddr<0x1f8)	RegOEN = 0x08;	// encoder
	else if (rAddr>=0x1f8&&rAddr<0x200)	RegOEN = 0x0a;	// audio
	else if (rAddr>=0x200&&rAddr<0x280)	RegOEN = 0x04;	// ipc
	else if (rAddr>=0x2a0&&rAddr<0x300)	RegOEN = 0x07;	// aux-osd
	else if (rAddr>=0x300&&rAddr<0x380)	RegOEN = 0x06;	// osd
	else if (rAddr>=0x3c0&&rAddr<0x3f8)	RegOEN = 0x09;	// enhance
	else								RegOEN = 0x00;	// host state

	err = LOCAL_I2CWrite(0x3ff, (PBYTE)&RegOEN, 2);	// write reg_oen
	if (err) return err;

	err = MDINI2C_Read(MDIN_LOCAL_ID, rAddr, pBuff, bytes);
	return err;
}

#if defined(SYSTEM_USE_MDIN340)||defined(SYSTEM_USE_MDIN380)
//--------------------------------------------------------------------------------------------------------------------------
static BYTE MHDMI_I2CWrite(WORD rAddr, PBYTE pBuff, WORD bytes)
{
	BYTE err = I2C_OK;

	err = MDINI2C_SetPage(MDIN_HDMI_ID, 0x0202);	// write hdmi page
	if (err) return err;

	err = MDINI2C_Write(MDIN_HDMI_ID, rAddr/2, (PBYTE)pBuff, bytes);
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MHDMI_GetWriteDone(void)
{
	WORD rVal = 0, count = 100, err = I2C_OK;

	while (count&&(rVal==0)) {
		err = MDINI2C_Read(MDIN_HDMI_ID, 0x027, (PBYTE)&rVal, 2);
		if (err) return err;	rVal &= 0x04;	count--;
	}
	return (count)? I2C_OK : I2C_TIME_OUT;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MHDMI_HOSTRead(WORD rAddr, PBYTE pBuff)
{
	WORD rData, err = I2C_OK;

	err = MDINI2C_SetPage(MDIN_HOST_ID, 0x0000);	// write host page
	if (err) return err;

	err = MDINI2C_Write(MDIN_HOST_ID, 0x025, (PBYTE)&rAddr, 2);
	if (err) return err;	rData = 0x0003;
	err = MDINI2C_Write(MDIN_HOST_ID, 0x027, (PBYTE)&rData, 2);
	if (err) return err;	rData = 0x0002;
	err = MDINI2C_Write(MDIN_HOST_ID, 0x027, (PBYTE)&rData, 2);
	if (err) return err;

	// check done flag
	err = MHDMI_GetWriteDone(); if (err) {mdinERR = 4; return err;}
	
	err = MDINI2C_Read(MDIN_HOST_ID, 0x026, (PBYTE)pBuff, 2);
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MHDMI_I2CRead(WORD rAddr, PBYTE pBuff, WORD bytes)
{
	BYTE err = I2C_OK;

	// DDC_STATUS, DDC_FIFOCNT
	if (rAddr==0x0f2||rAddr==0x0f5) return MHDMI_HOSTRead(rAddr, pBuff);

	err = MDINI2C_SetPage(MDIN_HDMI_ID, 0x0202);	// write hdmi page
	if (err) return err;

	err = MDINI2C_Read(MDIN_HDMI_ID, rAddr/2, (PBYTE)pBuff, bytes);
	return err;
}
#endif	/* defined(SYSTEM_USE_MDIN340)||defined(SYSTEM_USE_MDIN380) */

//--------------------------------------------------------------------------------------------------------------------------
static BYTE SDRAM_I2CWrite(DWORD rAddr, PBYTE pBuff, DWORD bytes)
{
	WORD row, len, idx, unit, err = I2C_OK;

	err = MDINI2C_RegRead(MDIN_HOST_ID, 0x005, &unit);	if (err) return err;
	unit = (unit&0x0100)? 4096 : 2048;

	while (bytes>0) {
		row = ADDR2ROW(rAddr, unit);	// get row
		idx = ADDR2COL(rAddr, unit);	// get col
		len = MIN((unit/2)-(rAddr%(unit/2)), bytes);

		err = MDINI2C_RegWrite(MDIN_HOST_ID, 0x003, row); if (err) return err;	// host access
		err = MDINI2C_SetPage(MDIN_HOST_ID, 0x0303); if (err) return err;	// write sdram page
		err = MDINI2C_Write(MDIN_SDRAM_ID, idx/2, (PBYTE)pBuff, len); if (err) return err;
		bytes-=len; rAddr+=len; pBuff+=len;
	}
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE SDRAM_I2CRead(DWORD rAddr, PBYTE pBuff, DWORD bytes)
{
	WORD row, len, idx, unit, err = I2C_OK;

	err = MDINI2C_RegRead(MDIN_HOST_ID, 0x005, &unit);	if (err) return err;
	unit = (unit&0x0100)? 4096 : 2048;

	while (bytes>0) {
		row = ADDR2ROW(rAddr, unit);	// get row
		idx = ADDR2COL(rAddr, unit);	// get col
		len = MIN((unit/2)-(rAddr%(unit/2)), bytes);

		err = MDINI2C_RegWrite(MDIN_HOST_ID, 0x003, row); if (err) return err;	// host access
		err = MDINI2C_SetPage(MDIN_HOST_ID, 0x0303); if (err) return err;	// write sdram page
		err = MDINI2C_Read(MDIN_SDRAM_ID, idx/2, (PBYTE)pBuff, len); if (err) return err;
		bytes-=len; rAddr+=len; pBuff+=len;
	}
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE I2C_WriteByID(BYTE nID, DWORD rAddr, PBYTE pBuff, DWORD bytes)
{
	BYTE err = I2C_OK;

	switch (nID&0xfe) {
		case MDIN_HOST_ID:	err = MHOST_I2CWrite(rAddr, (PBYTE)pBuff, bytes); break;
		case MDIN_LOCAL_ID:	err = LOCAL_I2CWrite(rAddr, (PBYTE)pBuff, bytes); break;
		case MDIN_SDRAM_ID:	err = SDRAM_I2CWrite(rAddr, (PBYTE)pBuff, bytes); break;

	#if defined(SYSTEM_USE_MDIN340)||defined(SYSTEM_USE_MDIN380)
		case MDIN_HDMI_ID:	err = MHDMI_I2CWrite(rAddr, (PBYTE)pBuff, bytes); break;
	#endif

	}
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE I2C_ReadByID(BYTE nID, DWORD rAddr, PBYTE pBuff, DWORD bytes)
{
	BYTE err = I2C_OK;

	switch (nID&0xfe) {
		case MDIN_HOST_ID:	err = MHOST_I2CRead(rAddr, (PBYTE)pBuff, bytes); break;
		case MDIN_LOCAL_ID:	err = LOCAL_I2CRead(rAddr, (PBYTE)pBuff, bytes); break;
		case MDIN_SDRAM_ID:	err = SDRAM_I2CRead(rAddr, (PBYTE)pBuff, bytes); break;

	#if defined(SYSTEM_USE_MDIN340)||defined(SYSTEM_USE_MDIN380)
		case MDIN_HDMI_ID:	err = MHDMI_I2CRead(rAddr, (PBYTE)pBuff, bytes); break;
	#endif

	}
	return err;
}

//--------------------------------------------------------------------------------------------------------------------------
#if	defined(SYSTEM_USE_MDIN380)&&defined(SYSTEM_USE_BUS_HIF)
MDIN_ERROR_t MDINI2C_SetPageID(WORD nID)
{
	PageID = nID;
	return MDIN_NO_ERROR;
}
#endif

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINI2C_MultiWrite(BYTE nID, DWORD rAddr, PBYTE pBuff, DWORD bytes)
{
	return (I2C_WriteByID(nID, rAddr, (PBYTE)pBuff, bytes))? MDIN_I2C_ERROR : MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINI2C_RegWrite(BYTE nID, DWORD rAddr, WORD wData)
{
	return (MDINI2C_MultiWrite(nID, rAddr, (PBYTE)&wData, 2))? MDIN_I2C_ERROR : MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINI2C_MultiRead(BYTE nID, DWORD rAddr, PBYTE pBuff, DWORD bytes)
{
	return (I2C_ReadByID(nID, rAddr, (PBYTE)pBuff, bytes))? MDIN_I2C_ERROR : MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINI2C_RegRead(BYTE nID, DWORD rAddr, PWORD rData)
{
	return (MDINI2C_MultiRead(nID, rAddr, (PBYTE)rData, 2))? MDIN_I2C_ERROR : MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
MDIN_ERROR_t MDINI2C_RegField(BYTE nID, DWORD rAddr, WORD bPos, WORD bCnt, WORD bData)
{
	WORD temp;

	if (bPos>15||bCnt==0||bCnt>16||(bPos+bCnt)>16) return MDIN_INVALID_PARAM;
	if (MDINI2C_RegRead(nID, rAddr, &temp)) return MDIN_I2C_ERROR;
	bCnt = ~(0xffff<<bCnt);
	temp &= ~(bCnt<<bPos);
	temp |= ((bData&bCnt)<<bPos);
	return (MDINI2C_RegWrite(nID, rAddr, temp))? MDIN_I2C_ERROR : MDIN_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------------------------------
// Drive Function for I2C read & I2C write
// User must make functions which is defined below.
//--------------------------------------------------------------------------------------------------------------------------
#if 0 // for TI cortex-m3 cpu i2c function
static BYTE MDINI2C_GetError(BOOL bRX)
{
	if (bRX) bRX = I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP;
	else	 bRX = I2C_MASTER_CMD_BURST_SEND_ERROR_STOP;

	if (I2CMasterErr(I2C0_MASTER_BASE)==0) return I2C_OK;
	I2CMasterControl(I2C0_MASTER_BASE, bRX);
	return I2C_HOST_NACK;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MDINI2C_SendAddr(BYTE sAddr, WORD rAddr)
{
	I2CMasterDataPut(I2C0_MASTER_BASE, HIBYTE(rAddr));	// start+slave+addr(msb)
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while (I2CMasterBusy(I2C0_MASTER_BASE));
	if (MDINI2C_GetError(false)) return I2C_HOST_NACK;

	I2CMasterDataPut(I2C0_MASTER_BASE, LOBYTE(rAddr));	// addr(lsb)
	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
	while (I2CMasterBusy(I2C0_MASTER_BASE));
	return MDINI2C_GetError(false);
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MDINI2C_SendByte(BYTE data, BOOL last)
{
	if (last) last = I2C_MASTER_CMD_BURST_SEND_FINISH;
	else	  last = I2C_MASTER_CMD_BURST_SEND_CONT;

	I2CMasterDataPut(I2C0_MASTER_BASE, data);
	I2CMasterControl(I2C0_MASTER_BASE, last);
	while (I2CMasterBusy(I2C0_MASTER_BASE));
	return MDINI2C_GetError(false);
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MDINI2C_GetByte(BOOL first, BOOL last)	// last == 1 for last byte
{
	BYTE read, cmd = 0;

	if (first==1&&last== ACK) cmd = I2C_MASTER_CMD_BURST_RECEIVE_START;
	if (first==1&&last==NACK) cmd = I2C_MASTER_CMD_SINGLE_RECEIVE;
	if (first==0&&last== ACK) cmd = I2C_MASTER_CMD_BURST_RECEIVE_CONT;
	if (first==0&&last==NACK) cmd = I2C_MASTER_CMD_BURST_RECEIVE_FINISH;

	I2CMasterControl(I2C0_MASTER_BASE, cmd);
	while (I2CMasterBusy(I2C0_MASTER_BASE));
	read = I2CMasterDataGet(I2C0_MASTER_BASE);
	MDINI2C_GetError(true);
	return read;
}
#endif

#if 0
//--------------------------------------------------------------------------------------------------------------------------
static BYTE MDINI2C_Write(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes)
{
	WORD i;	BYTE sAddr = I2C_MDIN3xx_ADDR;

#if 1	// add user i2c function

	// 0. check i2c  line is ready for use and it is not corrupted
	// 1. start condition (S)
	// 2. i2c slave address byte (R/W bit = low)
	// 3. i2c host address byte 1 (MSB) A(10:8)
	// 4. i2c host address byte 0 (LSB) A(7:0)
	// 5. re-start condition (Sr)   (* optional)
	// 6. i2c slave address byte (R/W bit = high) (* optional)
	
	for (i=0; i<bytes/2; i++) {	// for multi register write
		// 7. i2c write data byte 1 (MSB) D(15:8)
		// 8. i2c write data byte 0 (MSB) D(7:0)
	}
	// 9. stop condition (P)

#else	// for TI cortex-m3 cpu i2c function

	// check i2c  line is ready for use and it is not corrupted
	if (I2CMasterBusBusy(I2C0_MASTER_BASE)) return I2C_NOT_FREE;

	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, sAddr>>1, false);
	if (MDINI2C_SendAddr(sAddr, rAddr*1)) return I2C_HOST_NACK;	// start+slave+addr

	for (i=0; i<bytes/2-1; i++) {
		if (MDINI2C_SendByte(HIBYTE(((PWORD)pBuff)[i]), ACK)) return I2C_HOST_NACK;	// Transmit a buffer data
		if (MDINI2C_SendByte(LOBYTE(((PWORD)pBuff)[i]), ACK)) return I2C_HOST_NACK;	// Transmit a buffer data
	}
	
#endif

	return I2C_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
static BYTE MDINI2C_Read(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes)
{
	WORD i;	BYTE sAddr = I2C_MDIN3xx_ADDR;

#if 1	// add user i2c function

	// 0. check i2c  line is ready for use and it is not corrupted
	// 1. start condition (S)
	// 2. i2c slave address byte (R/W bit = low)
	// 3. i2c host address byte 1 (MSB) A(10:8)
	// 4. i2c host address byte 0 (LSB) A(7:0)
	// 5. re-start condition (Sr)
	// 6. i2c slave address byte (R/W bit = high)
	
	for (i=0; i<bytes/2; i++) { // for multi register read
		// 7. i2c read data byte 1 (MSB) D(15:8)
		// 8. i2c read data byte 0 (MSB) D(7:0)
	}
	// 9. stop condition (P)
	
#else   // for TI cortex-m3 cpu i2c function

	// check i2c  line is ready for use and it is not corrupted
	if (I2CMasterBusBusy(I2C0_MASTER_BASE)) return I2C_NOT_FREE;

	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, sAddr>>1, false);
	if (MDINI2C_SendAddr(sAddr, rAddr*1)) return I2C_HOST_NACK;	// start+slave+addr
	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, sAddr>>1, true);	// slave|read

	for (i=0; i<bytes/2-1; i++) {
		((PWORD)pBuff)[i]  = ((WORD)MDINI2C_GetByte((i)? 0:1, ACK))<<8;		// Receive a buffer data
		((PWORD)pBuff)[i] |= ((WORD)MDINI2C_GetByte(       0, ACK))<<0;		// Receive a buffer data
	}
	
#endif

	return I2C_OK;
}
#else	/* defined(FOCUS_MODEL) */

#if !defined(FOR_TEST)
#include "common.h"
#include "i2c-dev.h"
#else	/* defined(FOR_TEST) */
#include "stdio.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

typedef unsigned long long  __u64;
typedef unsigned int        __u32;
typedef unsigned short      __u16;
typedef unsigned char       __u8;

/*
 * I2C Message - used for pure i2c transaction, also from /dev interface
 */
struct i2c_msg {
	__u16 addr;	/* slave address			*/
	unsigned short flags;		
#define I2C_M_TEN	0x10	/* we have a ten bit chip address	*/
#define I2C_M_RD	0x01
#define I2C_M_NOSTART	0x4000
#define I2C_M_REV_DIR_ADDR	0x2000
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_NO_RD_ACK		0x0800
	short len;		/* msg length				*/
	char *buf;		/* pointer to msg data			*/
};


/* This is the structure as used in the I2C_RDWR ioctl call */
struct i2c_rdwr_ioctl_data {
	struct i2c_msg *msgs;	/* pointers to i2c_msgs */
	int nmsgs;		/* number of i2c_msgs */
};


/* /dev/i2c-X ioctl commands.  The ioctl's parameter is always an
 * unsigned long, except for:
 *      - I2C_FUNCS, takes pointer to an unsigned long
 *      - I2C_RDWR, takes pointer to struct i2c_rdwr_ioctl_data
 *      - I2C_SMBUS, takes pointer to struct i2c_smbus_ioctl_data
 */
#define I2C_RETRIES     0x0701  /* number of times a device address should be polled when not acknowledging */
#define I2C_TIMEOUT     0x0702  /* set timeout in units of 10 ms */

				   /* NOTE: Slave address is 7 or 10 bits, but 10-bit addresses
					* are NOT supported! (due to code brokenness)
					*/
#define I2C_SLAVE       0x0703  /* Use this slave address */
#define I2C_SLAVE_FORCE 0x0706  /* Use this slave address, even if it
				   				   is already in use by a driver! */
#define I2C_TENBIT      0x0704  /* 0 for 7 bit addrs, != 0 for 10 bit */

#define I2C_FUNCS       0x0705  /* Get the adapter functionality mask */

#define I2C_RDWR        0x0707  /* Combined R/W transfer (one STOP only) */

#define I2C_PEC         0x0708  /* != 0 to use PEC with SMBus */
#define I2C_16BIT_REG   0x0709  /* 16BIT REG WIDTH */
#define I2C_16BIT_DATA  0x070a  /* 16BIT DATA WIDTH */

BYTE i2c_id = 2;				// i2c_2
BYTE device_id = 0xDC;			// 0x6e
WORD register_value = 0x0;
WORD register_offset = 0x2;
BYTE show_console = ON;
BYTE addr_width = 0x2;

#endif	/* defined(FOCUS_MODEL) */


#define	FILE_NAME_SIZE		0x10
#define	RX_BUF_SIZE			0x4
#define	TX_BUF_SIZE			0x100
#define	MSG_SIZE			0X2
#define	_16BIT				0x2
//--------------------------------------------------------------------------------------------------------------------------

static BYTE MDINI2C_Write(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes)
{
#if !defined(FOR_TEST)
	BYTE i2c_id = 2;				// i2c_2
	BYTE device_id = 0xDC;			// 0x6e
	WORD register_value = 0x0;
	BYTE addr_width = _16BIT;
	BYTE show_console = OFF;
#endif	/* defined(FOR_TEST) */
	unsigned char device_addr = I2C_MDIN3xx_ADDR | ((device_id << 1) & 0x02);

	int retval = 0;
	int fd = -1;
	int i;
	char file_name[FILE_NAME_SIZE];
	unsigned char buf[TX_BUF_SIZE];
	unsigned int i2c_num = i2c_id;
	BYTE dev_addr;
	static struct i2c_rdwr_ioctl_data rdwr;
	static struct i2c_msg msg[MSG_SIZE];
	WORD reg_addr = (rAddr & 0x07ff);
	WORD data_width = _16BIT;
	int index = 0;
	int buf_idx = 0;
	WORD data = 0;
	WORD cur_addr = 0;

	dev_addr = (device_addr >> 1);

	sprintf(file_name, "/dev/i2c-%u", i2c_num);
	fd = open(file_name, O_RDWR);
	if (fd<0) {
		printf("[%s:%d]Open %s error!\n", __FILE__, __LINE__, file_name);
		retval = I2C_HOST_NACK;
		goto end0;
	}

	retval = ioctl(fd, I2C_SLAVE_FORCE, dev_addr);
	if(retval < 0) {
		printf("[%s:%d]set i2c device address error!\n", __FILE__, __LINE__);
		retval = I2C_HOST_NACK;
		goto end1;
	}

	for (index = reg_addr, cur_addr = reg_addr; index < reg_addr + (bytes / addr_width);index += 1, cur_addr++, buf_idx += addr_width) {
		if (addr_width == _16BIT) {
			buf[0] = HIBYTE(cur_addr);
			buf[1] = LOBYTE(cur_addr);
		} else {
			buf[0] = LOBYTE(cur_addr);
		}

		data = MAKEWORD(pBuff[buf_idx + 1], pBuff[buf_idx]);

		if (show_console == ON) {
#if defined(__DEBUG__)
			printf("[%s:%d]pBuff(rAddr:0x%x)(H0x%2x)(L0x%2x))(data:0x%4x)\n", __FILE__, __LINE__, cur_addr, *(pBuff+1), *pBuff, data);
#else
			printf("[%d] [0x%x] value : 0x%x\n", __LINE__, cur_addr, MAKEWORD(*(pBuff+1), *pBuff));
#endif	/* defined(__DEBUG__) */
		}

		if (data_width == _16BIT) {
			buf[data_width] = HIBYTE(data);
			buf[data_width + 1] = LOBYTE(data);
		} else {
			buf[1] = LOBYTE(data);
		}

		retval = write(fd, buf, (addr_width + data_width));
		if(retval < 0) {
			printf("i2c write error!\n");
			retval = -1;
			goto end1;
		}
	}

	retval = I2C_OK;

end1:
	close(fd);
end0:
	return retval;
}

//--------------------------------------------------------------------------------------------------------------------------

static BYTE MDINI2C_Read(BYTE nID, WORD rAddr, PBYTE pBuff, WORD bytes)
{
#if !defined(FOR_TEST)
	BYTE i2c_id = 2;				// i2c_2
	BYTE device_id = 0xDC;			// 0x6e
	WORD register_value = 0x0;
	BYTE addr_width = _16BIT;
	BYTE show_console = OFF;
#endif	/* defined(FOR_TEST) */

	unsigned char device_addr = I2C_MDIN3xx_ADDR | ((device_id << 1) & 0x02);

	int retval = 0;
	int fd = -1, tmp, i;
	char file_name[FILE_NAME_SIZE];
	unsigned char buf[RX_BUF_SIZE];
	unsigned int i2c_num = i2c_id;
	unsigned int dev_addr;
	WORD reg_addr = (rAddr & 0x07ff);
	WORD data_width = _16BIT;
	WORD reg_addr_end = reg_addr + (bytes / addr_width);
	static struct i2c_rdwr_ioctl_data rdwr;
	static struct i2c_msg msg[MSG_SIZE];
	WORD cur_addr = 0;

	memset(buf, 0x00, sizeof(buf));

	dev_addr = (device_addr >> 1);

	sprintf(file_name, "/dev/i2c-%u", i2c_num);
	fd = open(file_name, O_RDWR);
	if (fd < 0) {
		printf("[%s:%d]Open %s error!\n", __FILE__, __LINE__, file_name);
		retval = I2C_HOST_NACK;
		goto end0;
	}

	retval = ioctl(fd, I2C_SLAVE_FORCE, dev_addr);
	if (retval < 0) {
		printf("[%s:%d]CMD_SET_I2C_SLAVE error!\n", __FILE__, __LINE__);
		retval = I2C_HOST_NACK;
		goto end1;
	}

	msg[0].addr = dev_addr;
	msg[0].flags = 0;
	msg[0].len = addr_width;
	msg[0].buf = &buf[0];

	msg[1].addr = dev_addr;
	msg[1].flags = 0;
	msg[1].flags |= I2C_M_RD;
	msg[1].len = data_width;
	msg[1].buf = &buf[2];

	rdwr.msgs = &msg[0];
	rdwr.nmsgs = (__u32)2;

	for(cur_addr = reg_addr; cur_addr < reg_addr_end; cur_addr++) {
		if (addr_width == _16BIT) {
			buf[0] = HIBYTE(cur_addr);
			buf[1] = LOBYTE(cur_addr);
		}
		else {
			buf[0] = LOBYTE(cur_addr);
		}

		retval = ioctl(fd, I2C_RDWR, &rdwr);
		if (retval < 0) {
			printf("[%s:%d]CMD_I2C_READ error!\n", __FILE__, __LINE__);
			retval = I2C_HOST_NACK;
			goto end1;
		}

		if (data_width == _16BIT) {
			*(pBuff+1) = *(buf+addr_width + 1);
			*pBuff = *(buf+addr_width);
		}
		else {
			*pBuff = LOBYTE(buf[0]);
		}
		if (show_console == ON) {
#if defined(__DEBUG__)
			printf("[%s:%d]pBuff(rAddr:0x%x)(H0x%2x)(L0x%2x)\n", __FILE__, __LINE__, cur_addr, *(pBuff+1), *pBuff);
#else
			printf("[%d] [0x%x] value : 0x%x\n", __LINE__, cur_addr, MAKEWORD(*(pBuff+1), *pBuff));
#endif	/* defined(__DEBUG__) */
		}
	}

	retval = I2C_OK;

end1:
	close(fd);
end0:
	return retval;
}


#endif	/* defined(FOCUS_MODEL) */

/*  FILE_END_HERE */
