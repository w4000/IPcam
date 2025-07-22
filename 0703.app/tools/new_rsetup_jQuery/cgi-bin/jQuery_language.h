
#ifndef COMMON_LANGUAGE_H_
#define COMMON_LANGUAGE_H_

#include "_res/_lan/lan_english.htm"

//#define _LANG_ENG_ONLY_
//#define _LANG_ENG_KOR_ONLY_

#ifdef _LANG_ENG_ONLY_


#define RES_ID_ARRAY(name)		\
		{ \
			 name##_ENG	\
			,name##_ENG \
			,name##_ENG \
			,name##_ENG \
			,name##_ENG \
		}

#define RES_LANG_ARRAY(lang) 	\
		{ \
			RES_ENGLISH_##lang, \
			RES_ENGLISH_##lang, \
			RES_ENGLISH_##lang, \
			RES_ENGLISH_##lang, \
			RES_ENGLISH_##lang \
		}

#else

#ifdef _LANG_ENG_KOR_ONLY_

// #include "_res/res_korean.htm"
#include "_res/_lan/lan_korean.htm"

#define MAX_LANGUAGE	(3)

#define RES_ID_ARRAY(name)		\
		{ \
			 name##_ENG \
			,name##_KOR \
			,name##_JPN \
			,name##_JPN \
			,name##_JPN \
		}

#define RES_LANG_ARRAY(lang) 	\
		{ \
			RES_ENGLISH_##lang, \
			RES_KOREAN_##lang, \
			RES_JAPANESE_##lang, \
			RES_JAPANESE_##lang, \
			RES_JAPANESE_##lang \
		}

#else


#include "_res/_lan/lan_korean.htm"
#include "_res/_lan/lan_japanese.htm"
#include "_res/_lan/lan_english.htm"



enum {
	LANG_ENG = 0,
	LANG_KOR,
	LANG_JPN,
	MAX_LANGUAGE
};

#define RES_ID_ARRAY(name)		\
		{ \
			 name##_ENG 	\
			,name##_KOR \
			,name##_JPN \
			,name##_JPN \
			,name##_JPN \
		}

#define RES_LANG_ARRAY(lang) 	\
		{ \
			RES_ENGLISH_##lang, \
			RES_KOREAN_##lang, \
			RES_JAPANESE_##lang, \
			RES_JAPANESE_##lang, \
			RES_JAPANESE_##lang \
		}

#endif
#endif

#define RES_S_MONTH_ARRAY(lang) \
		{ \
			RES_JAN_##lang, \
			RES_FEB_##lang, \
			RES_MAR_##lang, \
			RES_APR_##lang, \
			RES_MAY_##lang, \
			RES_JUN_##lang, \
			RES_JUL_##lang, \
			RES_AUG_##lang, \
			RES_SEP_##lang, \
			RES_OCT_##lang, \
			RES_NOV_##lang, \
			RES_DEC_##lang \
		}

#define RES_MONTH_ARRAY(lang) \
		{ \
			RES_JAN_2_##lang, \
			RES_FEB_2_##lang, \
			RES_MAR_2_##lang, \
			RES_APR_2_##lang, \
			RES_MAY_2_##lang, \
			RES_JUN_2_##lang, \
			RES_JUL_2_##lang, \
			RES_AUG_2_##lang, \
			RES_SEP_2_##lang, \
			RES_OCT_2_##lang, \
			RES_NOV_2_##lang, \
			RES_DEC_2_##lang \
		}

#define RES_S_WEEK_ARRAY(lang) \
		{ \
			RES_SUN_##lang, \
			RES_MON_##lang, \
			RES_TUE_##lang, \
			RES_WED_##lang, \
			RES_THU_##lang, \
			RES_FRI_##lang, \
			RES_SAT_##lang, \
			RES_HOL_##lang \
		}

#define RES_WEEK_ARRAY(lang) \
		{ \
			RES_SUN_2_##lang, \
			RES_MON_2_##lang, \
			RES_TUE_2_##lang, \
			RES_WED_2_##lang, \
			RES_THU_2_##lang, \
			RES_FRI_2_##lang, \
			RES_SAT_2_##lang, \
			RES_HOL_2_##lang \
		}

#define RES_DATEFMT_ARRAY(lang) \
		{ \
			RES_DATE_FMT_MMDDYYYY_##lang, \
			RES_DATE_FMT_DDMMYYYY_##lang, \
			RES_DATE_FMT_YYYYMMDD_##lang, \
			RES_DATE_FMT_mmDDYYYY_##lang, \
			RES_DATE_FMT_DDmmYYYY_##lang, \
			RES_DATE_FMT_YYYYmmDD_##lang  \
		}

#define RES_TIMEFMT_ARRAY(lang) \
		{ \
			RES_TIME_FMT_AMPM_##lang, \
			RES_TIME_FMT_24H_##lang, \
			RES_TIME_FMT_AMPM_FRONT_##lang \
		}

#define RES_QUALITY_ARRAY(lang) \
		{ \
			RES_LOW_QUALITY_##lang, \
			RES_STANDARD_QUALITY_##lang, \
			RES_HIGH_QUALITY_##lang, \
			RES_HIGHEST_QUALITY_##lang \
		}

#endif /* COMMON_LANGUAGE_H_ */
