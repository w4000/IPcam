#ifndef _H265_DEF_H_
#define _H265_DEF_H_

#include "kwrap/type.h"


#define H265_MAX_WIDTH		(4096)
#define H265_MAX_HEIGHT		(4096)


#define H265_GDC_MIN_W_1_TILE		(1280)
#define H265_D2D_MIN_W_1_TILE		(2048)

#define H265_GDC_MIN_W_1_TILE_528	(1280)
#define H265_D2D_MIN_W_1_TILE_528	(4096)

#define H265ENC_HEADER_MAXSIZE  256
#define CMODEL_VERIFY  0
enum NalUnitType {
	NAL_UNIT_CODED_SLICE_TRAIL_N = 0,
	NAL_UNIT_CODED_SLICE_TRAIL_R,

	NAL_UNIT_CODED_SLICE_TSA_N,
	NAL_UNIT_CODED_SLICE_TSA_R,

	NAL_UNIT_CODED_SLICE_STSA_N,
	NAL_UNIT_CODED_SLICE_STSA_R,

	NAL_UNIT_CODED_SLICE_RADL_N,
	NAL_UNIT_CODED_SLICE_RADL_R,

	NAL_UNIT_CODED_SLICE_RASL_N,
	NAL_UNIT_CODED_SLICE_RASL_R,

	NAL_UNIT_RESERVED_VCL_N10,
	NAL_UNIT_RESERVED_VCL_R11,
	NAL_UNIT_RESERVED_VCL_N12,
	NAL_UNIT_RESERVED_VCL_R13,
	NAL_UNIT_RESERVED_VCL_N14,
	NAL_UNIT_RESERVED_VCL_R15,

	NAL_UNIT_CODED_SLICE_BLA_W_LP,
	NAL_UNIT_CODED_SLICE_BLA_W_RADL,
	NAL_UNIT_CODED_SLICE_BLA_N_LP,
	NAL_UNIT_CODED_SLICE_IDR_W_RADL,
	NAL_UNIT_CODED_SLICE_IDR_N_LP,
	NAL_UNIT_CODED_SLICE_CRA,
	NAL_UNIT_RESERVED_IRAP_VCL22,
	NAL_UNIT_RESERVED_IRAP_VCL23,

	NAL_UNIT_RESERVED_VCL24,
	NAL_UNIT_RESERVED_VCL25,
	NAL_UNIT_RESERVED_VCL26,
	NAL_UNIT_RESERVED_VCL27,
	NAL_UNIT_RESERVED_VCL28,
	NAL_UNIT_RESERVED_VCL29,
	NAL_UNIT_RESERVED_VCL30,
	NAL_UNIT_RESERVED_VCL31,

	NAL_UNIT_VPS,
	NAL_UNIT_SPS,
	NAL_UNIT_PPS,
	NAL_UNIT_ACCESS_UNIT_DELIMITER,
	NAL_UNIT_EOS,
	NAL_UNIT_EOB,
	NAL_UNIT_FILLER_DATA,
	NAL_UNIT_PREFIX_SEI,
	NAL_UNIT_SUFFIX_SEI,
	NAL_UNIT_RESERVED_NVCL41,
	NAL_UNIT_RESERVED_NVCL42,
	NAL_UNIT_RESERVED_NVCL43,
	NAL_UNIT_RESERVED_NVCL44,
	NAL_UNIT_RESERVED_NVCL45,
	NAL_UNIT_RESERVED_NVCL46,
	NAL_UNIT_RESERVED_NVCL47,
	NAL_UNIT_UNSPECIFIED_48,
	NAL_UNIT_UNSPECIFIED_49,
	NAL_UNIT_UNSPECIFIED_50,
	NAL_UNIT_UNSPECIFIED_51,
	NAL_UNIT_UNSPECIFIED_52,
	NAL_UNIT_UNSPECIFIED_53,
	NAL_UNIT_UNSPECIFIED_54,
	NAL_UNIT_UNSPECIFIED_55,
	NAL_UNIT_UNSPECIFIED_56,
	NAL_UNIT_UNSPECIFIED_57,
	NAL_UNIT_UNSPECIFIED_58,
	NAL_UNIT_UNSPECIFIED_59,
	NAL_UNIT_UNSPECIFIED_60,
	NAL_UNIT_UNSPECIFIED_61,
	NAL_UNIT_UNSPECIFIED_62,
	NAL_UNIT_UNSPECIFIED_63,
	NAL_UNIT_INVALID,
};

typedef enum {
	HEVC_PROFILE_NONE = 0,
	HEVC_PROFILE_MAIN = 1,
	HEVC_PROFILE_MAIN10 = 2,
	HEVC_PROFILE_MAINSTILLPICTURE = 3,
} HEVC_PROFILE_TYPE;

typedef enum {
	HEVC_TIER_MAIN = 0,
	HEVC_TIER_HIGH = 1,
} HEVC_TIER_TYPE;

typedef enum {
	HEVC_LEVEL_NONE     = 0,
	HEVC_LEVEL_LEVEL1   = 30,
	HEVC_LEVEL_LEVEL2   = 60,
	HEVC_LEVEL_LEVEL2_1 = 63,
	HEVC_LEVEL_LEVEL3   = 90,
	HEVC_LEVEL_LEVEL3_1 = 93,
	HEVC_LEVEL_LEVEL4   = 120,
	HEVC_LEVEL_LEVEL4_1 = 123,
	HEVC_LEVEL_LEVEL5   = 150,
	HEVC_LEVEL_LEVEL5_1 = 153,
	HEVC_LEVEL_LEVEL5_2 = 156,
	HEVC_LEVEL_LEVEL6   = 180,
	HEVC_LEVEL_LEVEL6_1 = 183,
	HEVC_LEVEL_LEVEL6_2 = 186
} HEVC_LEVEL_TYPE;

typedef enum {
	HEVC_QUALITY_MAIN = 0,	///< set tile config and search range as original
	HEVC_QUALITY_BASE		///< set tile config and search range as saving memory mode
} HEVC_QLVL_TYPE;


#endif
