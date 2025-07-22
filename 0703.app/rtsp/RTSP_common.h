
#ifndef RTSP_COMMON_H_
#define RTSP_COMMON_H_

char *strcasestr(const char *haystack, const char *needle);
#include "rtsp/RTP_stream.h"





// RTSP	connection state.
enum _STATE {
    _WAIT_REQUEST,
    _SEND_REPLY,
	_SEND_DATA,         
};

enum _Transport {
    _TRANSPORT_SVR, 
};

enum _LowerTransport {
    _LOWER_TRANSPORT_TCP,           
};


typedef struct TransportField {
	int interleaved_min, interleaved_max;

	int port_min, port_max;

	int client_port_min, client_port_max;

	int server_port_min, server_port_max;

	int ttl;

	struct in_addr	destination; 		
	char source[INET_ADDRSTRLEN + 1];	

	enum _Transport transport;

	enum _LowerTransport _tpt;
}__attribute__((packed)) _TransportField;


typedef	struct st_CTX
{
	int			c_sd;			
	int			c_state;		
	int			c_term;	
	__u64		c_poll_err_tm;
	struct sockaddr_in
				c_cliaddr;		
	char		c_inbuf [4096];
	size_t		c_datsz;		
	char		c_outbuf [4096];	 	
	char		c_protocol [8];		
	char		c_sid [32];	
	char		c_cmd [16];			
	char		c_url[128];			
	char		c_prot[16];			
	char		c_auth [64];		
	int			c_retcode;			
	int			c_seq;				
	__u8		*c_body;			
	int			c_bodylen;			
	char		c_timeout;			

	__u64		c_lastcmdtime;
	struct TransportField
				c_trans [5];
	int			c_nrtrans;			
	int			ctid;			
	__s64		c_rstart, c_rend;	
	char		c_realchal[64];		
	char		c_server[64];		
	int			c_notice;			
	char		c_location [512];	
	int			c_getparam_supported;	


	struct st_CTX
				*c_c[4];	
	int			c_childidx;					
	struct st_CTX
				*c_p;			

	int			c_std;			
	int			c_rtcpsd;			
	int			c_svrsd;			
	int			c_rsvrsd;		
	struct sockaddr_in
				c_svndest;			
	struct sockaddr_in
				c_rsvrdest;			
	enum _LowerTransport
				c_rtpprot;			
	char		c_rtpurl[64];		
	int			c_maxpayload;		
	int			c_ssrc;				
	int			c_pt;				

	char		c_prop [256];		
	int			c_rtpseq;			
	int			c_clrat;			
	__u32		c_wst;		
	__u64		c_ntpt;		
	__u32		c_wclk;			
	__u64		c_tmptm_video;			
	__u64		c_last_stamp;

	struct clist
				c_nal_h;			
	int			c_nalnr;			
	int			c_nrsent;			
	int			c_octet;			


	struct st_NALU
				*c_tcpnal;

	struct st_pkthdl
				*c_pkthdl;			

	struct st_stream c_stream;			
	char		c_streamurl[128];	
	int			id;			
	int			c_mt;		
	__u64		c_bt;			
	__u64		c_gbltm;			
	__u64		c_oldtm;			
	int			c_cam;				
	int			cvc;			
	int			c_ab_codec;			
	int			cmc;			
	int			c_stcsz;			
	__u8		c_nalstc [8];		
	int 		c_next_I;
	int			c_err_cnt;
	__u64		c_err_time;
	struct HTTPAuthState  *n_astate;				

#ifdef SUPPORT_H265
    int         c_is_ua_vlc;
#endif
	int			c_tile_mod;
	int			c_tile_cnt;
} SVR_CTX;





#define	CONTEXT_IS_SVR(__ctx__)	((__ctx__)->c_sd >= 0)
#define	_CONTEXT_IS_SVR(__ctx__)		((__ctx__)->c_std >= 0)
#define _IS_LIVE(__ctx__)		ISLIVE (& (__ctx__)->c_stream, (__ctx__)->c_cam)


#define CENTEXT_TERM_PARENT(__ctx__) \
do {\
	if (__ctx__) { \
		int __c__;\
		for (__c__ = 0; __c__ < 4; __c__++)\
			if ((__ctx__)->c_c[__c__]) (__ctx__)->c_c[__c__]->c_term = 1;\
		(__ctx__)->c_term = 1;\
	}\
} while (0)

#define CONTEXT_TERM(__ctx__, __msg__)	\
do {\
	if (CONTEXT_IS_SVR((__ctx__)))\
		CENTEXT_TERM_PARENT((__ctx__));\
	else if (_CONTEXT_IS_SVR((__ctx__))) \
		CENTEXT_TERM_PARENT((__ctx__)->c_p);\
} while (0)

#define	foreach_context(__ctx__, __i__)	\
	for ((__i__) = 0; (__i__) < 16; (__i__) ++) { \
		if ((! CONTEXT_IS_SVR (& r_ctx [__i__]) && ! _CONTEXT_IS_SVR (& r_ctx [__i__])))\
			continue;\
		__ctx__ = & r_ctx [__i__];


#define	_foreach_context(__ctx__, __i__)	\
	for ((__i__) = 0; (__i__) < 16; (__i__) ++) { \
		if (! _CONTEXT_IS_SVR (& r_ctx [__i__]))\
			continue;\
		__ctx__ = & r_ctx [__i__];



#define	CONTEXT_EMPTY()	\
	({\
		int		__i__ = 0;\
		SVR_CTX	* slot = NULL;\
		for (; __i__ < 16; __i__ ++) {\
			if (! CONTEXT_IS_SVR (& r_ctx [__i__]) && ! _CONTEXT_IS_SVR (& r_ctx [__i__]) && !r_ctx [__i__].c_term) {\
				slot = & r_ctx [__i__];\
				break;\
			}\
		}\
		slot;\
	})



#define CLOSE_SOCKET(__var__) \
	do {\
		if ((__var__) >= 0) {\
			close (__var__);\
			__var__ = -1;\
		}\
	} while (0)




#define	_WR(ctx, arg...)	\
	do {\
		int	__n__ = strlen ((ctx)->c_outbuf);\
		snprintf ((ctx)->c_outbuf + __n__, 4096 - __n__, ##arg);\
	} while (0)




extern SVR_CTX	r_ctx[32] ;
extern int			rtsp_nr;
extern int			rtp_nr;
extern int			g_nal_nr;

extern void 		RTSPCTX_init (SVR_CTX *ctx);
extern void			RTSPCTX_release (SVR_CTX *ctx);
extern SVR_CTX *	find_rtp_session (char * sesid);



enum RTCPType {
    RTCP_SR     = 200,	 
};


#define _OFFSET 		2208988800ULL
#define _OFFSET_US	(_OFFSET * 1000000ULL)






enum HTTPAuthType {
    HTTP_AUTH_NONE = 0,     
    HTTP_AUTH_BASIC,        
};



typedef struct HTTPAuthState{

	enum HTTPAuthType auth_type;
	char realm[200];

} ST_HTTPAuthState;


typedef void (*parse_key_val_cb)(void *context, const char *key, int key_len, char **dest, int *dest_len);

extern void 	http_auth_handle_header(struct st_CTX	*ctx, const char *key, char *value);
extern char *	http_auth_create_response (struct HTTPAuthState *state, char *dest, int destsz, const char *uid, const char *passwd,
											const char *path, const char *method);



// Media type.
enum AVMediaType {
    AVMEDIA_TYPE_UNKNOWN = -1,
    AVMEDIA_TYPE_VIDEO,
};


#define AV_NOPTS_VALUE		INT64_C(0x8000000000000000)


enum CodecID {
    CODEC_ID_NONE,

    CODEC_ID_MPEG1VIDEO,
    CODEC_ID_MPEG2VIDEO,  
#if 0
    CODEC_ID_MPEG2VIDEO_XVMC,
#endif 
    CODEC_ID_H261,
    CODEC_ID_H263,
    CODEC_ID_RV10,      // 5
    CODEC_ID_RV20,
    CODEC_ID_MJPEG,
    CODEC_ID_MJPEGB,
    CODEC_ID_LJPEG,
    CODEC_ID_SP5X,      // 10
    CODEC_ID_JPEGLS,
    CODEC_ID_MPEG4,
    CODEC_ID_RAWVIDEO,
    CODEC_ID_MSMPEG4V1,
    CODEC_ID_MSMPEG4V2, // 15
    CODEC_ID_MSMPEG4V3,
    CODEC_ID_WMV1,
    CODEC_ID_WMV2,
    CODEC_ID_H263P,
    CODEC_ID_H263I,     // 20
    CODEC_ID_FLV1,
    CODEC_ID_SVQ1,
    CODEC_ID_SVQ3,
    CODEC_ID_DVVIDEO,
    CODEC_ID_HUFFYUV,   // 25
    CODEC_ID_CYUV,
    CODEC_ID_H264,
    CODEC_ID_INDEO3,
    CODEC_ID_VP3,
    CODEC_ID_THEORA,    // 30
    CODEC_ID_ASV1,
    CODEC_ID_ASV2,
    CODEC_ID_FFV1,
    CODEC_ID_4XM,       // 35
    CODEC_ID_VCR1,
    CODEC_ID_CLJR,
    CODEC_ID_MDEC,
    CODEC_ID_ROQ,
    CODEC_ID_INTERPLAY_VIDEO,   // 40
    CODEC_ID_XAN_WC3,
    CODEC_ID_XAN_WC4,
    CODEC_ID_RPZA,
    CODEC_ID_CINEPAK,
    CODEC_ID_WS_VQA,	// 45
    CODEC_ID_MSRLE,
    CODEC_ID_MSVIDEO1,
    CODEC_ID_IDCIN,
    CODEC_ID_8BPS,
    CODEC_ID_SMC,		// 50
    CODEC_ID_FLIC,
    CODEC_ID_TRUEMOTION1,
    CODEC_ID_VMDVIDEO,
    CODEC_ID_MSZH,
    CODEC_ID_ZLIB,		// 55
    CODEC_ID_QTRLE,
    CODEC_ID_TSCC,
    CODEC_ID_ULTI,
    CODEC_ID_QDRAW,
    CODEC_ID_VIXL,		// 60
    CODEC_ID_QPEG,
    CODEC_ID_PNG,
    CODEC_ID_PPM,
    CODEC_ID_PBM,
    CODEC_ID_PGM,		// 65
    CODEC_ID_PGMYUV,
    CODEC_ID_PAM,
    CODEC_ID_FFVHUFF,
    CODEC_ID_RV30,
    CODEC_ID_RV40,		// 70
    CODEC_ID_VC1,
    CODEC_ID_WMV3,
    CODEC_ID_LOCO,
    CODEC_ID_WNV1,
    CODEC_ID_AASC,		// 75
    CODEC_ID_INDEO2,
    CODEC_ID_FRAPS,
    CODEC_ID_TRUEMOTION2,
    CODEC_ID_BMP,
    CODEC_ID_CSCD,		// 80
    CODEC_ID_MMVIDEO,
    CODEC_ID_ZMBV,
    CODEC_ID_AVS,
    CODEC_ID_SMACKVIDEO,
    CODEC_ID_NUV,		// 85
    CODEC_ID_KMVC,
    CODEC_ID_FLASHSV,
    CODEC_ID_CAVS,
    CODEC_ID_JPEG2000,
    CODEC_ID_VMNC,		// 90
    CODEC_ID_VP5,
    CODEC_ID_VP6,
    CODEC_ID_VP6F,
    CODEC_ID_TARGA,
    CODEC_ID_DSICINVIDEO,	// 95
    CODEC_ID_TIERTEXSEQVIDEO,
    CODEC_ID_TIFF,
    CODEC_ID_GIF,
    CODEC_ID_DXA,
    CODEC_ID_DNXHD,		// 100
    CODEC_ID_THP,
    CODEC_ID_SGI,
    CODEC_ID_C93,
    CODEC_ID_BETHSOFTVID,
    CODEC_ID_PTX,
    CODEC_ID_TXD,
    CODEC_ID_VP6A,
    CODEC_ID_AMV,
    CODEC_ID_VB,
    CODEC_ID_PCX,
    CODEC_ID_SUNRAST,
    CODEC_ID_INDEO4,
    CODEC_ID_INDEO5,
    CODEC_ID_MIMIC,
    CODEC_ID_RL2,
    CODEC_ID_ESCAPE124,
    CODEC_ID_DIRAC,
    CODEC_ID_BFI,
    CODEC_ID_CMV,
    CODEC_ID_MOTIONPIXELS,
    CODEC_ID_TGV,
    CODEC_ID_TGQ,
    CODEC_ID_TQI,
    CODEC_ID_AURA,
    CODEC_ID_AURA2,
    CODEC_ID_V210X,
    CODEC_ID_TMV,
    CODEC_ID_V210,
    CODEC_ID_DPX,
    CODEC_ID_MAD,
    CODEC_ID_FRWU,
    CODEC_ID_FLASHSV2,
    CODEC_ID_CDGRAPHICS,
    CODEC_ID_R210,
    CODEC_ID_ANM,
    CODEC_ID_BINKVIDEO,
    CODEC_ID_IFF_ILBM,
#define CODEC_ID_IFF_BYTERUN1 CODEC_ID_IFF_ILBM
    CODEC_ID_KGV1,
    CODEC_ID_YOP,
    CODEC_ID_VP8,
    CODEC_ID_PICTOR,
    CODEC_ID_ANSI,
    CODEC_ID_A64_MULTI,
    CODEC_ID_A64_MULTI5,
    CODEC_ID_R10K,
    CODEC_ID_MXPEG,
    CODEC_ID_LAGARITH,
    CODEC_ID_PRORES,
    CODEC_ID_JV,
    CODEC_ID_DFA,
    CODEC_ID_WMV3IMAGE,
    CODEC_ID_VC1IMAGE,
    CODEC_ID_UTVIDEO,
    CODEC_ID_BMV_VIDEO,
    CODEC_ID_VBLE,
    CODEC_ID_DXTORY,
    CODEC_ID_V410,
    CODEC_ID_XWD,
    CODEC_ID_CDXL,
    CODEC_ID_XBM,
    CODEC_ID_ZEROCODEC,
    CODEC_ID_MSS1,
    CODEC_ID_MSA1,
    CODEC_ID_TSCC2,
    CODEC_ID_MTS2,
    CODEC_ID_CLLC,
    CODEC_ID_MSS2,
    CODEC_ID_VP9,
    CODEC_ID_AIC,
    CODEC_ID_ESCAPE130,
    CODEC_ID_G2M,
    CODEC_ID_WEBP,
    CODEC_ID_HNM4_VIDEO,
    CODEC_ID_HEVC,
#define CODEC_ID_H265 CODEC_ID_HEVC
    CODEC_ID_FIC,
    CODEC_ID_ALIAS_PIX,
    CODEC_ID_BRENDER_PIX,
    CODEC_ID_PAF_VIDEO,
    CODEC_ID_EXR,
    CODEC_ID_VP7,
    CODEC_ID_SANM,
    CODEC_ID_SGIRLE,
    CODEC_ID_MVC1,
    CODEC_ID_MVC2,
    CODEC_ID_HQX,
    CODEC_ID_TDSC,
    CODEC_ID_HQ_HQA,
    CODEC_ID_HAP,
    CODEC_ID_DDS,
    CODEC_ID_DXV,
    CODEC_ID_SCREENPRESSO,
    CODEC_ID_RSCC,

    CODEC_ID_Y41P = 0x8000,
    CODEC_ID_AVRP,
    CODEC_ID_012V,
    CODEC_ID_AVUI,
    CODEC_ID_AYUV,
    CODEC_ID_TARGA_Y216,
    CODEC_ID_V308,
    CODEC_ID_V408,
    CODEC_ID_YUV4,
    CODEC_ID_AVRN,
    CODEC_ID_CPIA,
    CODEC_ID_XFACE,
    CODEC_ID_SNOW,
    CODEC_ID_SMVJPEG,
    CODEC_ID_APNG,
    CODEC_ID_DAALA,
    CODEC_ID_CFHD,
    CODEC_ID_TRUEMOTION2RT,
    CODEC_ID_M101,
    CODEC_ID_MAGICYUV,
    CODEC_ID_SHEERVIDEO,
    CODEC_ID_YLC,
    CODEC_ID_PSD,
    CODEC_ID_PIXLET,
    CODEC_ID_SPEEDHQ,
    CODEC_ID_FMVC,
    CODEC_ID_SCPR,

    CODEC_ID_FIRST_AUDIO = 0x10000,    
    CODEC_ID_PCM_S16LE = 0x10000,
    CODEC_ID_PCM_S16BE,
    CODEC_ID_PCM_U16LE,
    CODEC_ID_PCM_U16BE,
    CODEC_ID_PCM_S8,
    CODEC_ID_PCM_U8,
    CODEC_ID_PCM_MULAW,
    CODEC_ID_PCM_ALAW,
    CODEC_ID_PCM_S32LE,
    CODEC_ID_PCM_S32BE,
    CODEC_ID_PCM_U32LE,
    CODEC_ID_PCM_U32BE,
    CODEC_ID_PCM_S24LE,
    CODEC_ID_PCM_S24BE,
    CODEC_ID_PCM_U24LE,
    CODEC_ID_PCM_U24BE,
    CODEC_ID_PCM_S24DAUD,
    CODEC_ID_PCM_ZORK,
    CODEC_ID_PCM_S16LE_PLANAR,
    CODEC_ID_PCM_DVD,
    CODEC_ID_PCM_F32BE,
    CODEC_ID_PCM_F32LE,
    CODEC_ID_PCM_F64BE,
    CODEC_ID_PCM_F64LE,
    CODEC_ID_PCM_BLURAY,
    CODEC_ID_PCM_LXF,
    CODEC_ID_S302M,
    CODEC_ID_PCM_S8_PLANAR,
    CODEC_ID_PCM_S24LE_PLANAR,
    CODEC_ID_PCM_S32LE_PLANAR,
    CODEC_ID_PCM_S16BE_PLANAR,

    CODEC_ID_PCM_S64LE = 0x10800,
    CODEC_ID_PCM_S64BE,
    CODEC_ID_PCM_F16LE,
    CODEC_ID_PCM_F24LE,

    CODEC_ID_ADPCM_IMA_QT = 0x11000,
    CODEC_ID_ADPCM_IMA_WAV,
    CODEC_ID_ADPCM_IMA_DK3,
    CODEC_ID_ADPCM_IMA_DK4,
    CODEC_ID_ADPCM_IMA_WS,
    CODEC_ID_ADPCM_IMA_SMJPEG,
    CODEC_ID_ADPCM_MS,
    CODEC_ID_ADPCM_4XM,
    CODEC_ID_ADPCM_XA,
    CODEC_ID_ADPCM_ADX,
    CODEC_ID_ADPCM_EA,
    CODEC_ID_ADPCM_G726,
    CODEC_ID_ADPCM_CT,
    CODEC_ID_ADPCM_SWF,
    CODEC_ID_ADPCM_YAMAHA,
    CODEC_ID_ADPCM_SBPRO_4,
    CODEC_ID_ADPCM_SBPRO_3,
    CODEC_ID_ADPCM_SBPRO_2,
    CODEC_ID_ADPCM_THP,
    CODEC_ID_ADPCM_IMA_AMV,
    CODEC_ID_ADPCM_EA_R1,
    CODEC_ID_ADPCM_EA_R3,
    CODEC_ID_ADPCM_EA_R2,
    CODEC_ID_ADPCM_IMA_EA_SEAD,
    CODEC_ID_ADPCM_IMA_EA_EACS,
    CODEC_ID_ADPCM_EA_XAS,
    CODEC_ID_ADPCM_EA_MAXIS_XA,
    CODEC_ID_ADPCM_IMA_ISS,
    CODEC_ID_ADPCM_G722,
    CODEC_ID_ADPCM_IMA_APC,
    CODEC_ID_ADPCM_VIMA,
#if 0
    CODEC_ID_VIMA = CODEC_ID_ADPCM_VIMA,
#endif

    CODEC_ID_ADPCM_AFC = 0x11800,
    CODEC_ID_ADPCM_IMA_OKI,
    CODEC_ID_ADPCM_DTK,
    CODEC_ID_ADPCM_IMA_RAD,
    CODEC_ID_ADPCM_G726LE,
    CODEC_ID_ADPCM_THP_LE,
    CODEC_ID_ADPCM_PSX,
    CODEC_ID_ADPCM_AICA,
    CODEC_ID_ADPCM_IMA_DAT4,
    CODEC_ID_ADPCM_MTAF,

    CODEC_ID_AMR_NB = 0x12000,
    CODEC_ID_AMR_WB,

    CODEC_ID_RA_144 = 0x13000,
    CODEC_ID_RA_288,

    CODEC_ID_ROQ_DPCM = 0x14000,
    CODEC_ID_INTERPLAY_DPCM,
    CODEC_ID_XAN_DPCM,
    CODEC_ID_SOL_DPCM,

    CODEC_ID_SDX2_DPCM = 0x14800,

    CODEC_ID_MP2 = 0x15000,
    CODEC_ID_MP3,  
    CODEC_ID_AAC,
    CODEC_ID_AC3,
    CODEC_ID_DTS,
    CODEC_ID_VORBIS,
    CODEC_ID_DVAUDIO,
    CODEC_ID_WMAV1,
    CODEC_ID_WMAV2,
    CODEC_ID_MACE3,
    CODEC_ID_MACE6,
    CODEC_ID_VMDAUDIO,
    CODEC_ID_FLAC,
    CODEC_ID_MP3ADU,
    CODEC_ID_MP3ON4,
    CODEC_ID_SHORTEN,
    CODEC_ID_ALAC,
    CODEC_ID_WESTWOOD_SND1,
    CODEC_ID_GSM, 
    CODEC_ID_QDM2,
    CODEC_ID_COOK,
    CODEC_ID_TRUESPEECH,
    CODEC_ID_TTA,
    CODEC_ID_SMACKAUDIO,
    CODEC_ID_QCELP,
    CODEC_ID_WAVPACK,
    CODEC_ID_DSICINAUDIO,
    CODEC_ID_IMC,
    CODEC_ID_MUSEPACK7,
    CODEC_ID_MLP,
    CODEC_ID_GSM_MS, 
    CODEC_ID_ATRAC3,
#if 0
    CODEC_ID_VOXWARE,
#endif
    CODEC_ID_APE,
    CODEC_ID_NELLYMOSER,
    CODEC_ID_MUSEPACK8,
    CODEC_ID_SPEEX,
    CODEC_ID_WMAVOICE,
    CODEC_ID_WMAPRO,
    CODEC_ID_WMALOSSLESS,
    CODEC_ID_ATRAC3P,
    CODEC_ID_EAC3,
    CODEC_ID_SIPR,
    CODEC_ID_MP1,
    CODEC_ID_TWINVQ,
    CODEC_ID_TRUEHD,
    CODEC_ID_MP4ALS,
    CODEC_ID_ATRAC1,
    CODEC_ID_BINKAUDIO_RDFT,
    CODEC_ID_BINKAUDIO_DCT,
    CODEC_ID_AAC_LATM,
    CODEC_ID_QDMC,
    CODEC_ID_CELT,
    CODEC_ID_G723_1,
    CODEC_ID_G729,
    CODEC_ID_8SVX_EXP,
    CODEC_ID_8SVX_FIB,
    CODEC_ID_BMV_AUDIO,
    CODEC_ID_RALF,
    CODEC_ID_IAC,
    CODEC_ID_ILBC,
    CODEC_ID_OPUS,
    CODEC_ID_COMFORT_NOISE,
    CODEC_ID_TAK,
    CODEC_ID_METASOUND,
    CODEC_ID_PAF_AUDIO,
    CODEC_ID_ON2AVC,
    CODEC_ID_DSS_SP,

    CODEC_ID_FFWAVESYNTH = 0x15800,
    CODEC_ID_SONIC,
    CODEC_ID_SONIC_LS,
    CODEC_ID_EVRC,
    CODEC_ID_SMV,
    CODEC_ID_DSD_LSBF,
    CODEC_ID_DSD_MSBF,
    CODEC_ID_DSD_LSBF_PLANAR,
    CODEC_ID_DSD_MSBF_PLANAR,
    CODEC_ID_4GV,
    CODEC_ID_INTERPLAY_ACM,
    CODEC_ID_XMA1,
    CODEC_ID_XMA2,
    CODEC_ID_DST,
    CODEC_ID_ATRAC3AL,
    CODEC_ID_ATRAC3PAL,

    CODEC_ID_FIRST_SUBTITLE = 0x17000,         
    CODEC_ID_DVD_SUBTITLE = 0x17000,
    CODEC_ID_DVB_SUBTITLE,
    CODEC_ID_TEXT,   
    CODEC_ID_XSUB,
    CODEC_ID_SSA,
    CODEC_ID_MOV_TEXT,
    CODEC_ID_HDMV_PGS_SUBTITLE,
    CODEC_ID_DVB_TELETEXT,
    CODEC_ID_SRT,

    CODEC_ID_MICRODVD   = 0x17800,
    CODEC_ID_EIA_608,
    CODEC_ID_JACOSUB,
    CODEC_ID_SAMI,
    CODEC_ID_REALTEXT,
    CODEC_ID_SUBVIEWER1,
    CODEC_ID_SUBVIEWER,
    CODEC_ID_SUBRIP,
    CODEC_ID_WEBVTT,
    CODEC_ID_MPL2,
    CODEC_ID_VPLAYER,
    CODEC_ID_PJS,
    CODEC_ID_ASS,
    CODEC_ID_HDMV_TEXT_SUBTITLE,

     
    CODEC_ID_FIRST_UNKNOWN = 0x18000,           
    CODEC_ID_TTF = 0x18000,

    CODEC_ID_SCTE_35, 
    CODEC_ID_BINTEXT    = 0x18800,
    CODEC_ID_XBIN,
    CODEC_ID_IDF,
    CODEC_ID_OTF,
    CODEC_ID_SMPTE_KLV,
    CODEC_ID_DVD_NAV,
    CODEC_ID_TIMED_ID3,
    CODEC_ID_BIN_DATA,


    CODEC_ID_PROBE = 0x19000, 

    CODEC_ID_MPEG2TS = 0x20000,  
    CODEC_ID_MPEG4SYSTEMS = 0x20001,  
    CODEC_ID_FFMETADATA = 0x21000,   
    CODEC_ID_WRAPPED_AVFRAME = 0x21001,  
};


typedef struct sdp_session_level
{
	int		sdp_version;		 
	int		id;					 
	int		version;			 
	int		start_time;			 

	int		end_time;			 

	int		ttl;				 
	char	*user;				 
	char	*src_addr;			 
	char	*src_type;			 
	char	*dst_addr;			 
	char	*dst_type;			 
	char	*name;				 
} __attribute__((packed)) ST_SDPSSN;



extern int 		GET_CLKRATE (int	codecid);
extern int		SDP_get_playload (int codec);
extern int		SDP_create (SVR_CTX *ctx, int n_files, char *buf, int size);




#define SPACE_CHARS " \t\r\n"

static inline int
get_stristart (char *str, char *p, char **pp)
{
    while (*p && toupper((unsigned)*p) == toupper((unsigned)*str)) {
        p++;
        str++;
    }
    if (!*p && pp)
        *pp = str;
    return !*p;
}


static inline void 
get_word_until_chars(char *buf, int buf_size,
                                 char *sep, char **pp)
{
    char *p;
    char *q;

    p = *pp;
    p += strspn(p, SPACE_CHARS);
    q = buf;
    while (!strchr(sep, *p) && *p != '\0') {
        if ((q - buf) < buf_size - 1)
            *q++ = *p;
        p++;
    }
    if (buf_size > 0)
        *q = '\0';
    *pp = p;
}

static inline void 
get_word_sep(char *buf, int buf_size, char *sep,
                         char **pp)
{
    if (**pp == '/') (*pp)++;
    get_word_until_chars(buf, buf_size, sep, pp);
}


extern void		__SVR_parse_line (SVR_CTX *ctx, char * buf);
extern void		__SVR_parser_range (SVR_CTX *ctx, char *p);

extern void		get_split(char *proto, int proto_size, 
                  char *a, int a_size,
                  char *hn, int hns,
                  int *port_ptr,
                  char *path, int path_size,
                  const char *url);

extern int 		RTSP_parse_protocol (SVR_CTX *ctx);



extern int RTSP_MAX_BUF;

extern int	__send_data (SVR_CTX *rtp_c);
extern int	_session (SVR_CTX *rtp_c, _TransportField *th, SVR_CTX *rtsp_c);
extern void	_enquenal (SVR_CTX *ctx);
extern void make_nal_header (SVR_CTX *rtp_c, ST_FRAME *frame);






extern int rtsp_fd;
extern void RTSP_open_listen(void);
extern void RTSP_send(SVR_CTX *ctx);
extern void RTSP_serv(void);









#endif /* RTSP_COMMON_H_ */



