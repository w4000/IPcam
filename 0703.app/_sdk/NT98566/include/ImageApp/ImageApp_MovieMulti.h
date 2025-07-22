#ifndef IMAGEAPP_MOVIEMULTI_H
#define IMAGEAPP_MOVIEMULTI_H

#include "hdal.h"
#include "kflow_videoenc/media_def.h"
#include <kwrap/type.h>
#include "ImageApp/ImageApp_Common.h"
#include "hd_bsmux_lib.h"
#include "hd_fileout_lib.h"

/// ========== Callback function area ==========
typedef void (*MOVIE_RAWPROC_CB)(void);

/// ========== Enum area ==========
typedef enum {
	_CFG_REC_ID_1 = 0,
	_CFG_REC_ID_2,
	_CFG_REC_ID_3,
	_CFG_REC_ID_4,
	_CFG_REC_ID_MAX,
	_CFG_STRM_ID_1,
	_CFG_STRM_ID_2,
	_CFG_STRM_ID_3,
	_CFG_STRM_ID_4,
	_CFG_STRM_ID_MAX,
	_CFG_UVAC_ID_1,
	_CFG_UVAC_ID_2,
	_CFG_UVAC_ID_MAX,
	_CFG_CLONE_ID_1,
	_CFG_CLONE_ID_2,
	_CFG_CLONE_ID_3,
	_CFG_CLONE_ID_4,
	_CFG_CLONE_ID_MAX,
	_CFG_DISP_ID_1,
	_CFG_DISP_ID_2,
	_CFG_DISP_ID_MAX,
	_CFG_ETHCAM_ID_1,
	_CFG_ETHCAM_ID_2,
	_CFG_ETHCAM_ID_MAX,
	_CFG_CTRL_ID = 0x0000FF00,
	ENUM_DUMMY4WORD(MOVIE_CFG_REC_ID)
} MOVIE_CFG_REC_ID;

enum {
	MOVIE_CONFIG_RECORD_INFO  = 0xF000F000,
	MOVIE_CONFIG_STREAM_INFO,
	MOVIE_CONFIG_AUDIO_INFO,
	MOVIE_CONFIG_DISP_INFO,
	MOVIE_CONFIG_UVAC_INFO,
	MOVIE_CONFIG_ALG_INFO,
	MOVIE_CONFIG_ETHCAM_INFO,
	MOVIE_CONFIG_MAX_LINK_INFO,
	MOVIE_CONFIG_MEM_POOL_INFO,
	MOVIE_CONFIG_SENSOR_INFO,
	MOVIE_CONFIG_SENSOR_MAPPING,
	MOVIE_CONFIG_AD_MAP,
	MOVIE_CONFIG_DRAM,
};

typedef enum {
	MOVIE_IPL_SIZE_AUTO = 0,
	MOVIE_IPL_SIZE_MAIN,
	MOVIE_IPL_SIZE_CLONE,
	MOVIE_IPL_SIZE_WIFI,
	MOVIE_IPL_SIZE_ALG,
	MOVIE_IPL_SIZE_DISP,
	MOVIE_IPL_SIZE_USER,
	ENUM_DUMMY4WORD(MOVIE_IPL_SIZE)
} MOVIE_IPL_SIZE;

typedef struct {
	USIZE size;
	UINT32 fps;
} MOVIEMULTI_IPL_SIZE_INFO;

typedef enum {
	_CFG_VID_IN_1 = 0,
	_CFG_VID_IN_2,
	_CFG_VID_IN_3,
	_CFG_VID_IN_4,
	ENUM_DUMMY4WORD(MOVIE_CFG_VID_IN)
} MOVIE_CFG_VID_IN;

typedef enum {
	_CFG_ALG_PATH3 = 0,
	_CFG_ALG_PATH4,
	_CFG_ALG_PATH_MAX,
	ENUM_DUMMY4WORD(MOVIE_CFG_ALG_PATH)
} MOVIE_CFG_ALG_PATH;

typedef ISIZE        MOVIE_CFG_IMG_SIZE;

typedef UINT32       MOVIE_CFG_FRAME_RATE;

typedef UINT32       MOVIE_CFG_TARGET_RATE;

typedef ISIZE        MOVIE_CFG_IMG_RATIO;

typedef ISIZE        MOVIE_CFG_RAWENC_SIZE;

typedef UINT32       MOVIE_CFG_VCAP_FUNC;

typedef UINT32       MOVIE_CFG_VPROC_FUNC;

typedef BOOL         MOVIE_CFG_DISP_ENABLE;

typedef UINT32       MOVIE_CFG_GOP_NUM;

typedef BOOL         MOVIE_CFG_RTSP_STRM_OUT;

typedef enum {
	_CFG_CODEC_MJPG = MEDIAVIDENC_MJPG,
	_CFG_CODEC_H264 = MEDIAVIDENC_H264,
	_CFG_CODEC_H265 = MEDIAVIDENC_H265,
	ENUM_DUMMY4WORD(MOVIE_CFG_CODEC)
} MOVIE_CFG_CODEC;

typedef enum {
	_CFG_AUD_CODEC_NONE = 0,
	_CFG_AUD_CODEC_PCM,
	_CFG_AUD_CODEC_AAC,
	ENUM_DUMMY4WORD(MOVIE_CFG_AUD_CODEC)
} MOVIE_CFG_AUD_CODEC;

typedef enum {
	_CFG_REC_TYPE_AV        = MEDIAREC_AUD_VID_BOTH,
	_CFG_REC_TYPE_VID       = MEDIAREC_VID_ONLY,
	_CFG_REC_TYPE_TIMELAPSE = MEDIAREC_TIMELAPSE,
	_CFG_REC_TYPE_GOLFSHOT  = MEDIAREC_GOLFSHOT,
	ENUM_DUMMY4WORD(MOVIE_CFG_REC_MODE)
} MOVIE_CFG_REC_MODE;

typedef enum {
	_CFG_FILE_FORMAT_MP4     = MEDIA_FILEFORMAT_MP4,
	_CFG_FILE_FORMAT_TS      = MEDIA_FILEFORMAT_TS,
	_CFG_FILE_FORMAT_MOV     = MEDIA_FILEFORMAT_MOV,
	_CFG_FILE_FORMAT_JPG     = MEDIA_FILEFORMAT_JPG,
	_CFG_FILE_FORMAT_UNKNOWN = 0,
	ENUM_DUMMY4WORD(MOVIE_CFG_FILE_FORMAT)
} MOVIE_CFG_FILE_FORMAT;

typedef enum {
	_CFG_EMR_OFF         = 0x00000000,    /// EMR off
	_CFG_MAIN_EMR_BOTH   = 0x00000001,    /// Main remains + EMR
	_CFG_MAIN_EMR_PAUSE  = 0x00000002,    /// Main paused + EMR
	_CFG_MAIN_EMR_ONLY   = 0x00000004,    /// Main EMR only
	_CFG_CLONE_EMR_BOTH  = 0x00000010,    /// Clone remains + EMR
	_CFG_CLONE_EMR_PAUSE = 0x00000020,    /// Clone paused + EMR
	_CFG_CLONE_EMR_ONLY  = 0x00000040,    /// Clone EMR only
	_CFG_EMR_SET_CRASH   = 0x00000100,    /// set crash
	ENUM_DUMMY4WORD(MOVIE_CFG_FILE_OPTION)
} MOVIE_CFG_FILE_OPTION;

typedef UINT32       MOVIE_CFG_FILE_ENDTYPE;

typedef enum {
	//H264 profile
	_CFG_H264_PROFILE_BASELINE = HD_H264E_BASELINE_PROFILE,
	_CFG_H264_PROFILE_MAIN     = HD_H264E_MAIN_PROFILE,
	_CFG_H264_PROFILE_HIGH     = HD_H264E_HIGH_PROFILE,
	//H265 profile
	_CFG_H265_PROFILE_MAIN     = HD_H265E_MAIN_PROFILE,
	ENUM_DUMMY4WORD(MOVIE_CFG_PROFILE)
} MOVIE_CFG_PROFILE;

typedef enum _MOVIE_CFG_LEVEL {
	//H264 level
	_CFG_H264_LEVEL_1          = HD_H264E_LEVEL_1,
	_CFG_H264_LEVEL_1_1        = HD_H264E_LEVEL_1_1,
	_CFG_H264_LEVEL_1_2        = HD_H264E_LEVEL_1_2,
	_CFG_H264_LEVEL_1_3        = HD_H264E_LEVEL_1_3,
	_CFG_H264_LEVEL_2          = HD_H264E_LEVEL_2,
	_CFG_H264_LEVEL_2_1        = HD_H264E_LEVEL_2_1,
	_CFG_H264_LEVEL_2_2        = HD_H264E_LEVEL_2_2,
	_CFG_H264_LEVEL_3          = HD_H264E_LEVEL_3,
	_CFG_H264_LEVEL_3_1        = HD_H264E_LEVEL_3_1,
	_CFG_H264_LEVEL_3_2        = HD_H264E_LEVEL_3_2,
	_CFG_H264_LEVEL_4          = HD_H264E_LEVEL_4,
	_CFG_H264_LEVEL_4_1        = HD_H264E_LEVEL_4_1,
	_CFG_H264_LEVEL_4_2        = HD_H264E_LEVEL_4_2,
	_CFG_H264_LEVEL_5          = HD_H264E_LEVEL_5,
	_CFG_H264_LEVEL_5_1        = HD_H264E_LEVEL_5_1,
	//H265 level
	_CFG_H265_LEVEL_1          = HD_H265E_LEVEL_1,
	_CFG_H265_LEVEL_2          = HD_H265E_LEVEL_2,
	_CFG_H265_LEVEL_2_1        = HD_H265E_LEVEL_2_1,
	_CFG_H265_LEVEL_3          = HD_H265E_LEVEL_3,
	_CFG_H265_LEVEL_3_1        = HD_H265E_LEVEL_3_1,
	_CFG_H265_LEVEL_4          = HD_H265E_LEVEL_4,
	_CFG_H265_LEVEL_4_1        = HD_H265E_LEVEL_4_1,
	_CFG_H265_LEVEL_5          = HD_H265E_LEVEL_5,
	_CFG_H265_LEVEL_5_1        = HD_H265E_LEVEL_5_1,
	_CFG_H265_LEVEL_5_2        = HD_H265E_LEVEL_5_2,
	_CFG_H265_LEVEL_6          = HD_H265E_LEVEL_6,
	_CFG_H265_LEVEL_6_1        = HD_H265E_LEVEL_6_1,
	_CFG_H265_LEVEL_6_2        = HD_H265E_LEVEL_6_2,
	ENUM_DUMMY4WORD(MOVIE_CFG_LEVEL)
} MOVIE_CFG_LEVEL;

typedef enum {
	_CFG_DAR_DEFAULT,                      // default video display aspect ratio (the same as encoded image)
	_CFG_DAR_16_9,                         // 16:9 video display aspect ratio
	ENUM_DUMMY4WORD(MOVIE_CFG_DAR)
} MOVIE_CFG_DAR;

typedef struct {
	INT32  aq_enable;
	UINT32 aq_str;
	INT32  sraq_init_aslog2;
	INT32  max_sraq;
	INT32  min_sraq;
	INT32  sraq_const_aslog2;
} MOVIE_CFG_AQ_INFO;

typedef struct {
	UINT32 uiEnable;
	UINT32 uiStaticTime;
	UINT32 uiFrameRate;
	UINT32 uiByteRate;
	UINT32 uiGOP;
	UINT32 uiInitIQp;
	UINT32 uiMinIQp;
	UINT32 uiMaxIQp;
	UINT32 uiInitPQp;
	UINT32 uiMinPQp;
	UINT32 uiMaxPQp;
	INT32  iIPWeight;
	UINT32 uiRowRcEnalbe;
	UINT32 uiRowRcQpRange;
	UINT32 uiRowRcQpStep;
} MOVIE_CFG_CBR_INFO;

typedef enum {
	_CFG_AUDIO_CH_LEFT,              ///< Left
	_CFG_AUDIO_CH_RIGHT,             ///< Right
	_CFG_AUDIO_CH_STEREO,            ///< Stereo
	ENUM_DUMMY4WORD(MOVIE_CFG_AUDIO_CH)
} MOVIE_CFG_AUDIO_CH;

typedef struct {
	HD_PATH_ID      vout_ctrl;
	HD_PATH_ID      vout_path;
	USIZE           ratio;
	HD_VIDEO_DIR    dir;
} MOVIEMULTI_CFG_DISP_INFO;

typedef UINT32       MOVIE_CFG_FILE_SECOND;

typedef struct {
	MOVIE_CFG_REC_ID        rec_id;
	MOVIE_CFG_FILE_SECOND   seamless_sec;  // Seamless second
	MOVIE_CFG_FILE_OPTION   emr_on;        // pre-record emergency on
	MOVIE_CFG_FILE_SECOND   emr_sec;       // pre-record second for emergency (main_path)
	MOVIE_CFG_FILE_SECOND   keep_sec;      // post-record second for emergency (main path)
	MOVIE_CFG_FILE_SECOND   overlap_sec;   // overlap second
	MOVIE_CFG_FILE_ENDTYPE  end_type;      // such as MOVREC_ENDTYPE_CUTOVERLAP
	MOVIE_CFG_FILE_SECOND   rollback_sec;  // pre-record second for crash
	MOVIE_CFG_FILE_SECOND   forward_sec;   // post-record second for crash
	MOVIE_CFG_FILE_SECOND   clone_emr_sec; // pre-record second for emergency (clone path)
	MOVIE_CFG_FILE_SECOND   clone_keep_sec;// post-record second for emergency (clone path)
} MOVIE_RECODE_FILE_OPTION;

/// ========== Data structure area ==========
typedef struct {
	MOVIE_CFG_REC_ID             rec_id;
	MOVIE_CFG_VID_IN             vid_in;
	MOVIE_CFG_IMG_SIZE           size;
	MOVIE_CFG_FRAME_RATE         frame_rate;
	MOVIE_CFG_TARGET_RATE        target_bitrate;
	MOVIE_CFG_CODEC              codec;
	MOVIE_CFG_AUD_CODEC          aud_codec;
	MOVIE_CFG_REC_MODE           rec_mode;
	MOVIE_CFG_FILE_FORMAT        file_format;
	MOVIE_CFG_IMG_RATIO          ratio;
	MOVIE_CFG_RAWENC_SIZE        raw_enc_size;
	MOVIE_CFG_VCAP_FUNC          vcap_func;
	MOVIE_CFG_DISP_ENABLE        disp_enable;
	BOOL                         ipl_set_enable;
	MOVIE_CFG_DAR                dar;
	MOVIE_CFG_AQ_INFO            aq_info;
	MOVIE_CFG_CBR_INFO           cbr_info;
	BOOL                         sensor_rotate;
	MOVIE_CFG_VPROC_FUNC         vproc_func;
	MOVIE_CFG_IMG_RATIO			 disp_ratio;		// video display aspect ratio
} MOVIE_RECODE_INFO;

typedef struct {
	MOVIE_CFG_REC_ID             strm_id;
	MOVIE_CFG_VID_IN             vid_in;
	MOVIE_CFG_IMG_SIZE           size;
	MOVIE_CFG_FRAME_RATE         frame_rate;
	MOVIE_CFG_TARGET_RATE        max_bit_rate;
	MOVIE_CFG_CODEC              codec;
	MOVIE_CFG_GOP_NUM            gop_num;
	MOVIE_CFG_AUD_CODEC          aud_codec;
	MOVIE_CFG_RTSP_STRM_OUT      rstp_strm_out;
	MOVIE_CFG_AQ_INFO            aq_info;
	MOVIE_CFG_CBR_INFO           cbr_info;
	BOOL                         ipl_set_enable;
	MOVIE_CFG_IMG_RATIO          ratio;
	BOOL                         userproc_pull;
} MOVIE_STRM_INFO;

typedef struct {
	UINT32 aud_rate;
	UINT32 aud_ch;
	UINT32 aud_ch_num;
} MOVIEMULTI_AUDIO_INFO;

typedef struct {
	UINT32                   ImgFmt;
	USIZE                    ImgSize;
	URECT                    Window;
	UINT32                   ImgFps;
} MOVIE_ALG_IPL_INFO;

typedef struct {
	MOVIE_ALG_IPL_INFO      path13;
	MOVIE_ALG_IPL_INFO      path14;
	MOVIE_CFG_REC_ID        rec_id;
} MOVIE_ALG_INFO;

typedef struct {
	UINT32 MaxImgLink;
	UINT32 MaxDispLink;
	UINT32 MaxWifiLink;
	UINT32 MaxAudCapLink;
	UINT32 MaxEthCamLink;
} MOVIEMULTI_MAX_LINK_INFO;

typedef struct {
	MOVIE_CFG_REC_ID       rec_id;
	HD_VIDEOCAP_DRV_CONFIG vcap_cfg;
	HD_VIDEO_PXLFMT        senout_pxlfmt;
	HD_VIDEO_PXLFMT        capout_pxlfmt;
	UINT32                 data_lane;
	SENSOR_PATH_INFO       ae_path;
	SENSOR_PATH_INFO       awb_path;
	SENSOR_PATH_INFO       af_path;
	SENSOR_PATH_INFO       iq_path;
	SENSOR_PATH_INFO       iq_shading_path;
	SENSOR_PATH_INFO       iq_dpc_path;
	SENSOR_PATH_INFO       iq_ldc_path;
	char                   file_path[32];
} MOVIE_SENSOR_INFO;

typedef enum {
	RO_DEL_TYPE_PERCENT = 0,
	RO_DEL_TYPE_NUM,
	ENUM_DUMMY4WORD(MOVIE_RO_DEL_TYPE)
} MOVIE_RO_DEL_TYPE;

typedef enum {
	CCIR_AUTO = 0,
	CCIR_INTERLACE,
	CCIR_PROGRESSIVE,
	ENUM_DUMMY4WORD(MOVIE_CCIR_MODE)
} MOVIE_CCIR_MODE;

typedef struct {
	UINT64              givenSpace;  //in: free space
	//NMEDIAREC_CALCSEC_SETTING info[8];
} MOVIEMULTI_CALCSEC_SETTING2;

typedef struct {
	MEM_RANGE           mem_range;
	CHAR               *p_root_path;
	CHAR                drive; // 'A' ~ 'Z'
} MOVIE_CFG_FDB_INFO;

typedef struct {
	MOVIE_CFG_REC_ID    rec_id;
	CHAR*               movie;
	CHAR*               emr;
	CHAR*               snapshot;
	BOOL                file_naming;
} MOVIEMULTI_RECODE_FOLDER_NAMING;

typedef struct {
	UINT32              width;
	UINT32              height;
	UINT32              codec;
	UINT32              header_size;
	UINT8              *header;
} MOVIEMULTI_ETHCAM_DEC_INFO;

typedef struct {
	UINT32              width;
	UINT32              height;
	UINT32              vfr;
	UINT32              tbr;
	UINT32              ar;
	UINT32              gop;
	UINT32              codec;
	UINT32              aud_codec;
	UINT32              rec_mode;
	UINT32              rec_format;
	UINT32              buf_pa;
	UINT32              buf_size;
} MOVIEMULTI_ETHCAM_REC_INFO;

typedef struct {
	URECT Win;
} MOVIEMULTI_VCAP_CROP_INFO;

typedef struct {
	USIZE IMESize;
	URECT IMEWin;
} MOVIEMULTI_IME_CROP_INFO;

typedef struct {
	UINT32 en;
	UINT32 cnt1;
	UINT32 cnt2;
} MOVIEMULTI_BS_DROP_CNT;

#define MOVIEMULTI_DISK_MAX_DIR_NUM   8 //default

typedef struct {
	CHAR dir_path[128];
	UINT64 dir_size;
	UINT32 dir_ratio_of_disk;
	UINT32 file_num;
	UINT64 file_avg_size;
	UINT64 falloc_size;
	BOOL is_falloc;
	BOOL is_use;
} MOVIEMULTI_DIR_INFO;

typedef struct {
	UINT64 disk_size;
	UINT64 free_size;
	MOVIEMULTI_DIR_INFO dir_info[MOVIEMULTI_DISK_MAX_DIR_NUM];
	UINT64 other_size;
	UINT32 dir_num;
	UINT32 dir_depth;
	BOOL is_format_free; //0:init 1:true 2:false
} MOVIEMULTI_DISK_INFO;

typedef struct {
	UINT32 video_cap[4];
	UINT32 video_proc[4];
	UINT32 video_encode;
	UINT32 video_decode;
	UINT32 video_out;
} MOVIEMULTI_DRAM_CFG;

typedef struct {
	CHAR   path[128];
	UINT32 duration;
} MOVIEMULTI_CLOSE_FILE_INFO;

// =================== User Event Callback ===================
typedef enum {
	MOVIE_USER_CB_EVENT_START                 = 0x0000E300,
	// EVENT
	MOVIE_USER_CB_EVENT_VIDEO_READY           = MOVIE_USER_CB_EVENT_START,
	MOVIE_USER_CB_EVENT_STAMP_CB,                                            // value: (MP_VDOENC_YUV_SRC*)
	MOVIE_USER_CB_EVENT_REC_ONE_SECOND,
	MOVIE_USER_CB_EVENT_REC_STOP,
	MOVIE_USER_CB_EVENT_CARD_FULL,
	MOVIE_USER_CB_EVENT_REC_FILE_COMPLETED,
	MOVIE_USER_CB_EVENT_EMR_FILE_COMPLETED,
	MOVIE_USER_CB_EVENT_CARSH_FILE_COMPLETED,
	MOVIE_USER_CB_EVENT_SNAPSHOT_OK,
	MOVIE_USER_CB_EVENT_TIMELAPSE_ONESHOT,
	MOVIE_USER_CB_EVENT_FILENAMING_MOV_CB,
	MOVIE_USER_CB_EVENT_FILENAMING_EMR_CB,
	MOVIE_USER_CB_EVENT_FILENAMING_SNAPSHOT_CB,
	MOVIE_USER_CB_EVENT_KICKTHUMB,
	MOVIE_USER_CB_EVENT_CUTFILE,
	MOVIE_USER_CB_EVENT_CLOSE_RESULT,
	MOVIE_USER_CB_EVENT_OVERTIME,
	MOVIE_USER_CB_EVENT_RAWENC_PREPARED,
	MOVIE_USER_CB_EVENT_CLOSE_FILE_COMPLETED,
	MOVIE_USRR_CB_EVENT_TRIGGER_ETHCAM_THUMB,
	MOVIE_USRR_CB_EVENT_FILE_DELETED,
	MOVIE_USER_CB_EVENT_JENC_PREPARED,

	// ERROR
	MOVIE_USER_CB_ERROR_CARD_SLOW             = 0x0000E400,
	MOVIE_USER_CB_ERROR_CARD_WR_ERR,
	MOVIE_USER_CB_ERROR_SEAMLESS_REC_FULL,
	MOVIE_USER_CB_ERROR_SNAPSHOT_ERR,

	ENUM_DUMMY4WORD(MOVIE_USER_CB_EVENT)
} MOVIE_USER_CB_EVENT;

typedef enum {
	MOVIE_IMGCAP_THUMB = 0,
	MOVIE_IMGCAP_RAWENC,
	MOVIE_IMGCAP_EXIF_THUMB,
	ENUM_DUMMY4WORD(MOVIE_IMGCAP_TYPE)
} MOVIE_IMGCAP_TYPE;

typedef enum {
	MOVIE_PSEUDO_REC_NONE = 0,
	MOVIE_PSEUDO_REC_MODE_1,
	MOVIE_PSEUDO_REC_MODE_MAX,
	ENUM_DUMMY4WORD(MOVIE_PSEUDO_REC_TYPE)
} MOVIE_PSEUDO_REC_TYPE;

typedef struct {
	HD_PATH_ID        path_id;
	MOVIE_IMGCAP_TYPE type;
	USIZE             img_size;
} MOVIE_IMGCAP_INFO;

typedef struct {
	MOVIE_CFG_REC_ID  rec_id;
	UINT32            vin_id;
	UINT32            vcap_id_bit;
} MOVIE_AD_MAP;

typedef void (MovieUserEventCb)(UINT32 id, MOVIE_USER_CB_EVENT event_id, UINT32 value);

typedef void (MovieAudCapCb)(UINT32 pa, UINT32 va, UINT32 size);

typedef void (MovieAudBsCb)(MOVIE_CFG_REC_ID id, HD_AUDIO_BS *paenc_data);

typedef void (MovieVdoBsCb)(MOVIE_CFG_REC_ID id, HD_VIDEOENC_BS *pvenc_data);

// =================== Parameters ===================
enum {
	MOVIEMULTI_PARAM_START = 0x0000ED000,
	// ipl group
	MOVIEMULTI_PARAM_IPL_GROUP_BEGIN = MOVIEMULTI_PARAM_START,
	MOVIEMULTI_PARAM_IPL_MIRROR = MOVIEMULTI_PARAM_IPL_GROUP_BEGIN,                 /// var + vprc
	MOVIEMULTI_PARAM_VCAP_OUTFUNC,
	MOVIEMULTI_PARAM_IPL_FORCED_IMG_SIZE,
	MOVIEMULTI_PARAM_IPL_USER_IMG_SIZE,
	MOVIEMULTI_PARAM_VCAP_CCIR_FMT,                                                 /// 601 or 656
	MOVIEMULTI_PARAM_VCAP_CCIR_MIRROR_FLIP,                                         /// only valid for ccir sensor
	MOVIEMULTI_PARAM_VCAP_CCIR_MODE,                                                /// auto, interlace or progressive
	MOVIEMULTI_PARAM_VCAP_CROP,
	MOVIEMULTI_PARAM_IPL_GROUP_END,
	// image group
	MOVIEMULTI_PARAM_IMG_GROUP_BEGIN = MOVIEMULTI_PARAM_START + 0x0100,
	MOVIEMULTI_PARAM_IMG_GROUP_END,
	// video codec group
	MOVIEMULTI_PARAM_CODEC_GROUP_BEGIN = MOVIEMULTI_PARAM_START + 0x0200,
	MOVIEMULTI_PARAM_CODEC = MOVIEMULTI_PARAM_CODEC_GROUP_BEGIN,                    /// var + venc + bsmux
	MOVIEMULTI_PARAM_TIMELAPSE_TIME,                                                /// var + venc
	MOVIEMULTI_PARAM_ENCBUF_MS,                                                     /// var + venc
	MOVIEMULTI_PARAM_VDO_ENC_REQUEST_I,
	MOVIEMULTI_PARAM_VDO_ENC_REG_CB,
	MOVIEMULTI_PARAM_VDO_QUALITY_BASE_MODE_EN,
	MOVIEMULTI_PARAM_TARGETRATE,
	MOVIEMULTI_PARAM_CODEC_GROUP_END,
	// imgcap group
	MOVIEMULTI_PARAM_IMGCAP_GROUP_BEGIN = MOVIEMULTI_PARAM_START + 0x0300,
	MOVIEMULTI_PARAM_IMGCAP_EXIF_EN = MOVIEMULTI_PARAM_IMGCAP_GROUP_BEGIN,          /// var + venc
	MOVIEMULTI_PARAM_IMGCAP_THUM_SIZE,
	MOVIEMULTI_PARAM_IMGCAP_USE_VENC_BUF,
	MOVIEMULTI_PARAM_IMGCAP_GROUP_END,
	// audio codec group
	MOVIEMULTI_PARAM_AUD_GROUP_BEGIN = MOVIEMULTI_PARAM_START + 0x0400,
	MOVIEMULTI_PARAM_AUD_MUTE_ENC = MOVIEMULTI_PARAM_AUD_GROUP_BEGIN,               /// var + acap (mute from acap if MOVIEMULTI_PARAM_AUD_MUTE_ENC_FUNC_EN is not set)
	NOVIEMULTI_PARAM_AUD_ACAP_BY_HDAL,                                              /// var + acap
	MOVIEMULTI_PARAM_AUD_MUTE_ENC_FUNC_EN,                                          /// keep acap but only mute aenc
	MOVIEMULTI_PARAM_AUD_CAP_REG_CB,
	MOVIEMULTI_PARAM_AUD_ACAP_VOL,
	MOVIEMULTI_PARAM_AUD_ACAP_ALC_EN,
	MOVIEMULTI_PARAM_AUD_ENC_REG_CB,
	MOVIEMULTI_PARAM_AUD_ACAP_ANR,                                                  /// Set before ImageApp_MovieMulti_Open
	MOVIEMULTI_PARAM_AUD_ACAP_DEFAULT_SETTING,                                      /// Set after ImageApp_MovieMulti_Open and before audio start. VENDOR_AUDIOCAP_DEFAULT_SETTING_10DB ~ 30DB
	MOVIEMULTI_PARAM_AUD_ACAP_NOISEGATE_THRESHOLD,                                  /// Set after ImageApp_MovieMulti_Open and before audio start. -76 ~ -31 (dB)
	MOVIEMULTI_PARAM_AUD_GROUP_END,
	// file group
	MOVIEMULTI_PARAM_FILE_GROUP_BEGIN = MOVIEMULTI_PARAM_START + 0x0500,
	MOVIEMULTI_PARAM_FILE_FORMAT = MOVIEMULTI_PARAM_FILE_GROUP_BEGIN,               /// var + aenc + venc + bsmux
	MOVIEMULTI_PARAM_REC_FORMAT,                                                    /// var + venc + bsmux
	MOVIEMULTI_PARAM_FILE_SEAMLESSSEC,                                              /// var + bsmux
	MOVIEMULTI_PARAM_FILE_ENDTYPE,                                                  /// var + bsmux
	MOVIEMULTI_PARAM_FILE_PLAYFRAMERATE,                                            /// var + bsmux
	MOVIEMULTI_PARAM_FILE_BUFRESSEC,                                                /// var + bsmux
	MOVIEMULTI_PARAM_FILE_ROLLBACKSEC,                                              /// var + bsmux
	MOVIEMULTI_PARAM_FILE_KEEPSEC,                                                  /// var + bsmux
	MOVIEMULTI_PARAM_FILE_EMRON,                                                    /// var + venc + bsmux
	MOVIEMULTI_PARAM_FILE_FLUSH_SEC,                                                /// bsmux
	MOVIEMULTI_PARAM_FILE_WRITE_BLKSIZE,                                            /// bsmux
	MOVIEMULTI_PARAM_FILE_GPS_DATA,                                                 /// bsmux
	MOVIEMULTI_PARAM_FILE_FRONT_MOOV,                                               /// bsmux
	MOVIEMULTI_PARAM_FILE_FRONT_MOOV_FLUSH_SEC,                                     /// bsmux
	MOVIEMULTI_PARAM_FILE_2V1A_MODE,                                                /// bsmux (should reopen mode)
	MOVIEMULTI_PARAM_FILE_BSMUX_FAST_PUT,                                           /// bsmux
	MOVIEMULTI_PARAM_FILE_DROP_FRAME_WITHOUT_SLOW_CB,                               /// bsmux
	MOVIEMULTI_PARAM_RO_DEL_TYPE,                                                   /// var
	MOVIEMULTI_PARAM_RO_DEL_PERCENT,                                                /// var
	MOVIEMULTI_PARAM_RO_DEL_NUM,                                                    /// var
	MOVIEMULTI_PARAM_FILE_GPS_EN,                                                   /// bsmux
	MOVIEMULTI_PARAM_FILE_GPS_RATE,                                                 /// bsmux
	MOVIEMULTI_PARAM_FILE_GROUP_END,
	// display group
	MOVIEMULTI_PARAM_DISP_GROUP_BEGIN = MOVIEMULTI_PARAM_START + 0x0600,
	MOVIEMULTI_PARAM_DISP_REG_CB = MOVIEMULTI_PARAM_DISP_GROUP_BEGIN,               /// var
	MOVIEMULTI_PARAM_DISP_IME_CROP,
	MOVIEMULTI_PARAM_DISP_MIRROR_FLIP,
	MOVIEMULTI_PARAM_DISP_IME_CROP_AUTO_SCALING_EN,
	MOVIEMULTI_PARAM_DISP_WINDOW,
	MOVIEMULTI_PARAM_DISP_FPS,
	MOVIEMULTI_PARAM_DISP_GROUP_END,
	// wifi group
	MOVIEMULTI_PARAM_WIFI_GROUP_BEGIN = MOVIEMULTI_PARAM_START + 0x0700,
	MOVIEMULTI_PARAM_WIFI_REG_CB = MOVIEMULTI_PARAM_WIFI_GROUP_BEGIN,               /// var
	MOVIEMULTI_PARAM_WIFI_GROUP_END,
	// misc group
	MOVIEMULTI_PARAM_MISC_GROUP_BEGIN = MOVIEMULTI_PARAM_START + 0x0800,
	MOVIEMULTI_PARAM_FILEDB_FILTER = MOVIEMULTI_PARAM_MISC_GROUP_BEGIN,             /// var
	MOVIEMULTI_PRARM_FILEDB_MAX_MUM,                                                /// var
	MOVIEMULTI_PARAM_YUV_COMPRESS,                                                  /// var
	MOVIEMULTI_PRARM_DELETE_FILTER,                                                 /// var
	MOVIEMULTI_PARAM_PSEUDO_REC_MODE,
	MOVIEMULTI_PARAM_MISC_GROUP_END,
	// ethcam group
	MOVIEMULTI_PARAM_ETHCAM_GROUP_BEGIN = MOVIEMULTI_PARAM_START + 0x0900,
	MOVIEMULTI_RARAM_ETHCAM_RX_FUNC_EN = MOVIEMULTI_PARAM_ETHCAM_GROUP_BEGIN,       /// var
	MOVIEMULTI_PRARM_ETHCAM_DEC_INFO,
	MOVIEMULTI_PRARM_ETHCAM_REC_INFO,
	MOVIEMULTI_PARAM_ETHCAM_GROUP_END,
};

/// ========== Function Prototype area ==========
/// ImageApp_MovieMulti
extern ER ImageApp_MovieMulti_Open(void);
extern ER ImageApp_MovieMulti_Close(void);
extern ER ImageApp_MovieMulti_Config(UINT32 config_id, UINT32 value);
extern ER ImageApp_MovieMulti_FileOption(MOVIE_RECODE_FILE_OPTION *prec_option);
extern UINT32 ImageApp_MovieMulti_Recid2ImgLink(MOVIE_CFG_REC_ID id);
extern UINT32 ImageApp_MovieMulti_Recid2BsPort(MOVIE_CFG_REC_ID id);
extern UINT32 ImageApp_MovieMulti_BsPort2Recid(UINT32 port);
extern UINT32 ImageApp_MovieMulti_BsPort2Imglink(UINT32 port);
extern UINT32 ImageApp_MovieMulti_BsPort2EthCamlink(UINT32 port);
extern UINT32 ImageApp_MovieMulti_VePort2Imglink(UINT32 port);
extern HD_PATH_ID ImageApp_MovieMulti_GetVcapCtrlPort(MOVIE_CFG_REC_ID id);
extern HD_PATH_ID ImageApp_MovieMulti_GetVdoEncPort(MOVIE_CFG_REC_ID id);
extern HD_PATH_ID ImageApp_MovieMulti_GetRawEncPort(MOVIE_CFG_REC_ID id);
extern HD_PATH_ID ImageApp_MovieMulti_GetAlgDataPort(MOVIE_CFG_REC_ID id, MOVIE_CFG_ALG_PATH path);
extern void ImageApp_MovieMulti_Root_Path(CHAR *prootpath, UINT32 rec_id);
extern UINT32 ImageApp_MovieMulti_Folder_Naming(MOVIEMULTI_RECODE_FOLDER_NAMING *pfolder_naming);
extern BOOL ImageApp_MovieMulti_isFileNaming(MOVIE_CFG_REC_ID rec_id);
/// ImageApp_MovieMulti_AudCapLink
extern ER ImageApp_MovieMulti_AudCapStart(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_AudCapStop(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_AudCapGetFrame(MOVIE_CFG_REC_ID id, UINT8 *p_buf, UINT32 *p_len, INT32 wait_ms);
/// ImageApp_MovieMulti_CB
extern ER ImageApp_MovieMulti_RegUserCB(MovieUserEventCb *fncb);
extern ER ImageApp_MovieMulti_InitCrash(MOVIE_CFG_REC_ID rec_id);
extern ER ImageApp_MovieMulti_SetCrash(MOVIE_CFG_REC_ID rec_id, UINT32 is_set);
/// ImageApp_MovieMulti_DispLink
extern ER ImageApp_MovieMulti_DispStart(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_DispStop(MOVIE_CFG_REC_ID id);
extern HD_RESULT ImageApp_MovieMulti_DispPullOut(MOVIE_CFG_REC_ID id, HD_VIDEO_FRAME* p_video_frame, INT32 wait_ms);
extern HD_RESULT ImageApp_MovieMulti_DispPushIn(MOVIE_CFG_REC_ID id, HD_VIDEO_FRAME* p_video_frame, INT32 wait_ms);
extern HD_RESULT ImageApp_MovieMulti_DispReleaseOut(MOVIE_CFG_REC_ID id, HD_VIDEO_FRAME* p_video_frame);
extern HD_RESULT ImageApp_MovieMulti_DispWindowEnable(MOVIE_CFG_REC_ID id, UINT32 En);
/// ImageApp_MovieMulti_EthCamLink
extern ER ImageApp_MovieMulti_EthCamLinkForDisp(MOVIE_CFG_REC_ID id, UINT32 action, BOOL allow_pull);
extern UINT32 ImageApp_MovieMulti_EthCamLinkForDispStatus(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_EthCamLinkPushVdoBsToDecoder(MOVIE_CFG_REC_ID id, HD_VIDEODEC_BS *p_video_bs);
extern ER ImageApp_MovieMulti_EthCamLinkPushVdoBsToBsMux(MOVIE_CFG_REC_ID id, HD_VIDEOENC_BS *p_video_bs);
extern ER ImageApp_MovieMulti_EthCamLinkPushJpgToFileOut(MOVIE_CFG_REC_ID id, HD_FILEOUT_BUF *p_jpg);
/// ImageApp_MovieMulti_FileManage
extern INT32 ImageApp_MovieMulti_FMAllocate(CHAR drive);
extern INT32 ImageApp_MovieMulti_FMScan(CHAR drive, MOVIEMULTI_DISK_INFO* p_disk_info);
extern INT32 ImageApp_MovieMulti_FMDump(CHAR drive);
extern INT32 ImageApp_MovieMulti_FMOpen(CHAR drive);
extern INT32 ImageApp_MovieMulti_FMClose(CHAR drive);
extern INT32 ImageApp_MovieMulti_FMRefresh(CHAR drive);
extern INT32 ImageApp_MovieMulti_FMConfig(MOVIE_CFG_FDB_INFO *p_cfg);
extern void ImageApp_MovieMulti_FMSetSortBySN(CHAR *pDelimStr, UINT32 nDelimCount, UINT32 nNumOfSn);
/// ImageApp_MovieMulti_Operation
extern ER ImageApp_MovieMulti_ImgLinkForDisp(MOVIE_CFG_REC_ID id, UINT32 action, BOOL allow_pull);
extern UINT32 ImageApp_MovieMulti_EthCamLinkForDispStatus(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_ImgLinkForStreaming(MOVIE_CFG_REC_ID id, UINT32 action, BOOL allow_pull);
extern UINT32 ImageApp_MovieMulti_ImgLinkForStreamingStatus(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_ImgLinkForAlg(MOVIE_CFG_REC_ID id, MOVIE_CFG_ALG_PATH path, UINT32 action, BOOL allow_pull);
extern UINT32 ImageApp_MovieMulti_ImgLinkForAlgStatus(MOVIE_CFG_REC_ID id, MOVIE_CFG_ALG_PATH path);
extern ER ImageApp_MovieMulti_RecStart(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_RecStop(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_TriggerSnapshot(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_TrigEMR(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_TrigOnce(MOVIE_CFG_REC_ID id, UINT32 sec);
/// ImageApp_MovieMulti_Param
extern ER ImageApp_MovieMulti_SetParam(MOVIE_CFG_REC_ID id, UINT32 param, UINT32 value);
extern ER ImageApp_MovieMulti_GetParam(MOVIE_CFG_REC_ID id, UINT32 param, UINT32* value);
/// ImageApp_MovieMulti_Utilities
extern UINT32 ImageApp_MovieMulti_IsStreamRunning(MOVIE_CFG_REC_ID id);
extern UINT32 ImageApp_MovieMulti_GetFreeRec(HD_BSMUX_CALC_SEC *pSetting);
extern UINT32 ImageApp_MovieMulti_GetRemainSize(CHAR drive, UINT64 *p_size);
extern UINT32 ImageApp_MovieMulti_SetReservedSize(CHAR drive, UINT64 size, UINT32 is_loop);
extern UINT32 ImageApp_MovieMulti_SetExtendCrashInfo(MOVIE_CFG_REC_ID id, UINT32 unit, UINT32 max_num, UINT32 enable);
extern UINT32 ImageApp_MovieMulti_IsExtendCrash(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_SetExtendCrash(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_GetDropCnt(MOVIE_CFG_REC_ID id, MOVIEMULTI_BS_DROP_CNT *pcnt);
extern ER ImageApp_MovieMulti_GetAudEncBufInfo(MOVIE_CFG_REC_ID id, HD_AUDIOENC_BUFINFO *pinfo);
extern ER ImageApp_MovieMulti_GetVdoEncBufInfo(MOVIE_CFG_REC_ID id, HD_VIDEOENC_BUFINFO *pinfo);
extern ER ImageApp_MovieMulti_GetVcapSysInfo(MOVIE_CFG_REC_ID id, HD_VIDEOCAP_SYSINFO *pinfo);
extern INT32 ImageApp_MovieMulti_SetFormatFree(CHAR drive, BOOL is_on);
extern INT32 ImageApp_MovieMulti_SetFolderInfo(CHAR *p_path, UINT32 ratio, UINT64 f_size);
extern INT32 ImageApp_MovieMulti_GetFolderInfo(CHAR *p_path, UINT32 *ratio, UINT64 *f_size);
extern INT32 ImageApp_MovieMulti_SetCyclicRec(CHAR *p_path, BOOL is_on);
extern INT32 ImageApp_MovieMulti_SetExtStrType(CHAR drive, UINT32 type);
extern INT32 ImageApp_MovieMulti_SetUseHiddenFirst(CHAR drive, BOOL value);
extern INT32 ImageApp_MovieMulti_SetSyncTime(CHAR drive, BOOL value);
extern INT32 ImageApp_MovieMulti_SetFileUnit(CHAR *p_path, UINT32 unit);
extern INT32 ImageApp_MovieMulti_SetSkipReadOnly(CHAR drive, BOOL value);
/// ImageApp_MovieMulti_WifiLink
extern ER ImageApp_MovieMulti_StreamingStart(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_StreamingStop(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_UvacStart(MOVIE_CFG_REC_ID id);
extern ER ImageApp_MovieMulti_UvacStop(MOVIE_CFG_REC_ID id);
extern HD_RESULT ImageApp_MovieMulti_WifiPullOut(MOVIE_CFG_REC_ID id, HD_VIDEO_FRAME* p_video_frame, INT32 wait_ms);
extern HD_RESULT ImageApp_MovieMulti_WifiPushIn(MOVIE_CFG_REC_ID id, HD_VIDEO_FRAME* p_video_frame, INT32 wait_ms);
extern HD_RESULT ImageApp_MovieMulti_WifiReleaseOut(MOVIE_CFG_REC_ID id, HD_VIDEO_FRAME* p_video_frame);

// =================== Special definition area start ===================
#define ETHCAM_TX_MAGIC_KEY        0x67100000

#endif//IMAGEAPP_MOVIEMULTI_H
