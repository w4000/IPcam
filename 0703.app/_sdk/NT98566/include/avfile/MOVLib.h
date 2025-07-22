/**
    Header file of mov fileformat.

    Exported header file of mov fileformat.

    @file       MOVLib.h
    @ingroup    mIAVMOV
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef _MOVLIB_H
#define _MOVLIB_H

/*-----------------------------------------------------------------------------*/
/* Include Header Files                                                        */
/*-----------------------------------------------------------------------------*/
#if defined (__UITRON) || defined (__ECOS)
#include "Type.h"
#else
#include "kwrap/type.h"
#endif

/**
    @addtogroup mIAVMOV
*/
//@{

/*-----------------------------------------------------------------------------*/
/* Macro Constant Definitions                                                  */
/*-----------------------------------------------------------------------------*/
//timescale
//#define MOV_TIMESCALE  600 // should be precise to fit all kinds of frame rate (such as 10, 15, 24, 30, 60, 120fps...)
//#define MOV_TIMESCALE  60000 // should be precise to fit all kinds of frame rate (such as 10, 15, 24, 30, 60, 120fps...)
#define MOV_TIMESCALE  92400 // should be precise to fit all kinds of frame rate (such as 10, 15, 24, 28, 30, 55, 60, 120fps...)

//#NT#2012/08/22#Hideo Lin -begin
//#NT#For thumbnail and user data
//#define ATOM_THUM   0x7468756d  // 'thum'
#define ATOM_USER   0x75736572  // 'user'
//#NT#2012/08/22#Hideo Lin -end
#define VERSION_LENGTH  8       //'ver ' in frea atom

/*-----------------------------------------------------------------------------*/
/* Type Definitions                                                            */
/*-----------------------------------------------------------------------------*/
/*
    MOV_offset as a 32-bit pointer
*/
typedef UINT32  MOV_Offset;


/**
    @ name mov entry structure.

    Structure definition for a video/audio frame, including file offset, frame size,
    frame duration, and frame type.

*/
typedef struct {
	UINT64      pos;        ///< file offset of this frame
	UINT32      size;       ///< frame size
	//UINT32      duration;   ///< frame duration, used for av-sync
	UINT16      key_frame;  ///< frame type, whether key frame or not
	UINT8       updated;
	UINT8       rev;
} MOV_Ientry;

typedef struct {
	UINT32      pos;        ///< file offset of this frame
	UINT32      size;       ///< frame size
	//UINT32      duration;   ///< frame duration, used for av-sync
	UINT16      key_frame;  ///< frame type, whether key frame or not
	UINT8       updated;
	UINT8       rev;
} MOV_old32_Ientry;

#define MP_MOV_SIZE_ONEENTRY 12// one entry is 12 bytes
#define MP_MOV_SIZE_CO64_ONEENTRY 16 // one entry is 16 bytes


//#NT#2012/09/11#Meg Lin -begin
typedef struct {
	UINT32 junkCC;      //oft: 0
	UINT32 junkSize;    //oft: 4
	UINT32 nidxCC;      //oft: 8
	UINT32 nidxSize;    //oft: c
	UINT32 audPos;      //oft: 10
	UINT32 audSize;     //oft: 14
	UINT32 lastjunkPos; //oft: 18
	UINT32 vidstartFN;  //oft: 1c
	UINT32 videoWid;    //oft: 20
	UINT32 videoHei;    //oft: 24
	UINT32 videoCodec;  //oft: 28
	UINT32 audSR;       //oft: 2c
	UINT32 audChs;      //oft: 30
	UINT32 hdrSize;     //oft: 34
	UINT32 versionCode; //oft: 38
	UINT32 vidPosLen;   //oft: 3c, thisVideoFrmDataLen
	UINT32 vidPosArray;//array of vidPos, vidSize, oft:40

} MOV_NIDXINFO;

typedef struct {
	UINT32 junkCC;      //oft: 0
	UINT32 junkSize;    //oft: 4
	UINT32 nidxCC;      //oft: 8
	UINT32 nidxSize;    //oft: c
	UINT32 audstartFN;  //oft: 10, this sec start framenumber
	UINT32 audcodec;    //oft: 14
	UINT32 lastjunkPos; //oft: 18
	UINT32 vidstartFN;  //oft: 1c, this sec start framenumber
	UINT32 videoWid;    //oft: 20
	UINT32 videoHei;    //oft: 24
	UINT32 videoCodec;  //oft: 28
	UINT32 videoFR;     //oft: 2c
	UINT32 audSR;       //oft: 30
	UINT32 audChs;      //oft: 34
	UINT32 hdrSize;     //oft: 38
	UINT32 versionCode; //oft: 3c
	UINT32 vidtotalFN;  //oft: 40, this sec total framenumber
	UINT32 vidPosArray;//array of (vidPos, vidSize)* vidtotalFN, oft:44
	UINT32 audtotalFN;  //oft: 44+vidFN*8
	UINT32 audPosArray;//array of (audPos, audSize)* audtotalFN, oft:48+vidFN*8

} MOV_SM_NIDXINFO;

typedef struct {
	UINT32 junkCC;      //oft: 0
	UINT32 junkSize;    //oft: 4
	UINT32 nidxCC;      //oft: 8
	UINT32 nidxSize;    //oft: c
	UINT32 audstartFN;  //oft: 10, this sec start framenumber
	UINT32 audcodec;    //oft: 14
	UINT32 lastjunkPos0; //oft: 18
	UINT32 lastjunkPos1; //oft: 1c
	UINT32 vidstartFN;  //oft: 20, this sec start framenumber
	UINT32 videoWid;    //oft: 24
	UINT32 videoHei;    //oft: 28
	UINT32 videoCodec;  //oft: 2c
	UINT32 videoFR;     //oft: 30
	UINT32 audSR;       //oft: 34
	UINT32 audChs;      //oft: 38
	UINT32 hdrSize;     //oft: 3c
	UINT32 versionCode; //oft: 40
	UINT32 vidtotalFN;  //oft: 44, this sec total framenumber
	UINT32 vidPosArray;//array of (vidPos, vidSize)* vidtotalFN, oft:48
	UINT32 audtotalFN;  //oft: 48+vidFN*8
	UINT32 audPosArray;//array of (audPos, audSize)* audtotalFN, oft:4c+vidFN*8

} MOV_SM_CO64_NIDXINFO;


typedef struct {
	UINT32 junkCC;      //oft: 0
	UINT32 junkSize;    //oft: 4
	UINT32 nidxCC;      //oft: 8
	UINT32 nidxSize;    //oft: c
	UINT32 timeID;      //oft: 10, this sec start framenumber
	UINT32 audstartFN;  //oft: 14, this sec start framenumber
	UINT32 audcodec;    //oft: 18
	UINT32 lastjunkPos; //oft: 1c
	UINT32 vidstartFN;  //oft: 20, this sec start framenumber
	UINT32 videoWid;    //oft: 24
	UINT32 videoHei;    //oft: 28
	UINT32 videoCodec;  //oft: 2c
	UINT32 videoFR;     //oft: 30
	UINT32 audSR;       //oft: 34
	UINT32 audChs;      //oft: 38
	UINT32 hdrSize;     //oft: 3c
	UINT32 versionCode; //oft: 40
	UINT32 vidtotalFN;  //oft: 44, this sec total framenumber
	UINT32 vidPosArray;//array of (vidPos, vidSize)* vidtotalFN, oft:48
	UINT32 audtotalFN;  //oft: 48+vidFN*8
	UINT32 audPosArray;//array of (audPos, audSize)* audtotalFN, oft:4c+vidFN*8

} MOV_2015_NIDXINFO;

typedef struct {
	UINT32 junkCC;      //oft: 0
	UINT32 junkSize;    //oft: 4
	UINT32 nidxCC;      //oft: 8
	UINT32 nidxSize;    //oft: c
	UINT32 timeID;      //oft: 10, this sec start framenumber
	UINT32 audstartFN;  //oft: 14, this sec start framenumber
	UINT32 audcodec;    //oft: 18
	UINT32 vidstartFN;  //oft: 1c, this sec start framenumber
	UINT32 lastjunkPos0; //oft: 20
	UINT32 lastjunkPos1; //oft: 24
	UINT32 videoWid;    //oft: 28
	UINT32 videoHei;    //oft: 2c
	UINT32 videoCodec;  //oft: 30
	UINT32 videoFR;     //oft: 34
	UINT32 audSR;       //oft: 38
	UINT32 audChs;      //oft: 3c
	UINT32 hdrSize;     //oft: 40
	UINT32 versionCode; //oft: 44
	UINT32 vidtotalFN;  //oft: 48, this sec total framenumber
	UINT32 vidPosArray;//array of (vidPos, vidSize)* vidtotalFN, oft:4c
	UINT32 audtotalFN;  //oft: 4c+vidFN*8
	UINT32 audPosArray;//array of (audPos, audSize)* audtotalFN, oft:50+vidFN*8

} MOV_2015_CO64_NIDXINFO;
#define MP_MOV_NIDX_FIXEDSIZE       0x3C //before vidPosArray, not including junk (8bytes)
#define MP_MOV_NIDXCO64_FIXEDSIZE   0x40 //before vidPosArray, not including junk (8bytes)

#define MP_MOV_2015NIDX_FIXEDSIZE   0x40 //before vidPosArray, not including junk (8bytes)
#define MP_MOV_2015CO64NIDX_FIXEDSIZE   0x44 //before vidPosArray, not including junk (8bytes)
//#NT#2012/09/11#Meg Lin -end

typedef struct {
	UINT32 ouputAddr;
	UINT32 makerLen;
	UINT32 modelLen;
	unsigned char *pMaker;
	unsigned char *pModel;
} MOV_USER_MAKERINFO;

/*-----------------------------------------------------------------------------*/
/* Extern Functions                                                            */
/*-----------------------------------------------------------------------------*/
typedef struct {
	UINT8               FrameResolution;
	UINT8               VideoQuality;
	UINT16              rev;
	UINT32              TimeResolution;
	UINT32              BufHeaderLen;
	UINT32              BufOffsetLen;
	UINT32              BufLen;
	UINT32              BufOffsetCnt;
	UINT32              DataPacketSize;//4K

	UINT8               decVideoCodec;
	UINT8               decAudioCodec;

	UINT8               encVideoCodec;
	UINT8               encAudioCodec;//MOVAUDENC_AAC
	UINT32              Preroll;
	UINT16              PacketPayloadType;
	UINT16              VideoFrameRate;
	UINT32              TotalAudioSampleCount;
	UINT16              AudioSampleRate;
	UINT16              AudioChannel;
	UINT32              AudioBitsPerSample;
	UINT32              AudioBitRate;
	UINT16              ImageWidth;             ///< [Out]     : mov Image Width
	UINT16              ImageHeight;            ///< [Out]     : mov Image Height
	UINT32              VideoPayloadSize;
	UINT32              AudioPayloadSize;
	UINT32              CreationDate;
	//UINT32              NowFilePos;
	UINT32              fileOffset;
	UINT32              audioFrameCount;
	UINT32              videoFrameCount;
	UINT64              totalMdatSize;
	UINT8               *ptempMoovBufAddr;
	UINT32              audioTotalSize;
	UINT64              IndexEntryTimeInterval; ///< [In/Out]  : Key frame Time Inerval
	UINT64              DataPacketsCount;
	UINT64              PlayDuration;
	UINT32              uiH264GopType;  // H.264 GOP type (Hideo@120322)
	UINT32              uiFileType;     // file type (Hideo@120626)

	//add for recovery -begin
	UINT32              bufaddr;
	UINT32              bufsize;
	UINT64              recv_filesize; //filesize before recovery
	UINT32              clustersize;
	UINT32              hdr_revSize;//MEDIAREC_AVI_HDR//2012/10/22 Meg
	//add for recovery -end
	//#NT#2013/04/17#Calvin Chang#Support Rotation information in Mov/Mp4 File format -begin
	UINT32              uiVideoRotate;
	//#NT#2013/04/17#Calvin Chang -end
	//UINT64              FileSize;
	UINT32              uinidxversion;
	UINT32              uiTempHdrAdr;
	//add for front moov -begin
	UINT32              outMoovSize;
	UINT32              vidDescAdr;
	UINT32              vidDescSize;
	UINT32              shiftfrea;
	//add for front moov -end
	//add for sub-stream -begin
	UINT8               SubVideoTrackEn;
	UINT8               SubencVideoCodec;
	UINT16              SubVideoFrameRate;
	UINT16              SubImageWidth;
	UINT16              SubImageHeight;
	UINT32              SubvideoFrameCount;
	UINT64              SubPlayDuration;
	//add for sub-stream -end
	//add for recovery
	UINT32              rcvy_blksize;
	//add for recovery
} MOVMJPG_FILEINFO;

typedef struct {
	UINT32 id;
	MOVMJPG_FILEINFO *pfileinfo;
} MAKEMOOV_INFO;

extern UINT32 MOVWrite_UserMakerModelData(MOV_USER_MAKERINFO *pMaker);
extern UINT32 MOVWrite_TransRoundVidFrameRate(UINT32 input);
extern void   MOVRcvy_OpenMsg(UINT32 value);//2015/11/09
extern void   MOV_EndFileWay(UINT32 value);
extern void   mov_setConMakerId(UINT32 value);
#endif//_MOVLIB_H

//@}