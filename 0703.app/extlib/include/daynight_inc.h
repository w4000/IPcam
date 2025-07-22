#ifndef _DAYNIGHT_INC_H_
#define _DAYNIGHT_INC_H_

#ifdef __cplusplus
extern "C" {
#endif

/********* type defination*********/
#ifndef U32
#define U32 unsigned int
#endif

#ifndef S32
#define S32 int
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef U08
#define U08 	unsigned char
#endif



/********* macro declaration *********/
#define DAY_FLAG		1
#define NIGHT_FLAG		0
#define OTHER_FLAG 		-1

#define DEBUG_ERR		0x1 //error print，default
#define DEBUG_OPEN		0x2 //key print，when open
#define DEBUG_OTHER		0x3 //other print，when open


/********* enum declaration *********/

typedef enum
{
	DNC_MAX_POINT 		= 0x1,//Adapting the characteristics of different sensors
	DNC_START_POINT		= 0x2,//Adapting the characteristics of different sensors
	DNC_AINT_OSC		= 0x3,//Anti-shock parameters 
	DNC_ENV_LUMA 		= 0x4,//Setting or acquiring the lux calculated from the library
	DNC_BCD_OSC 		= 0x5,//Anti-BCD-shock enable/disable
}DNC_KEY_E;


/********* struct declaration *********/

typedef struct
{
	U32 expTime;//current expsure time, [10-40000]us
	U32 curY;	//current raw data luminance(wiht isp dgain), [0-255*1024]
	U32 iso;	//current total DB (wiht isp dgain)(Amplification factor) [100-xxx]，value 100 is euqal to 1 times
	U08 totalDB;//current total DB (wiht isp dgain)[0-100]
	U08 appGainLevel;//User-configurable  gain level [1-100]. If there is no such item, Set to 100. 
	U08 appIcrStatus;//current icr status.
	U08 res;
} DNC_AE_STATIS_S;

typedef struct
{
	U32 day2night;		//threshold  当前白天转夜晚的阈值，该值是计算后的亮度值
	U32 night2day;		//threshold  当前夜晚转白天的阈值，该值是计算后的亮度值
	U08 day2night_db;	//白天切夜晚时候的白天状态的临界增益，即day2night时候的db值
	U08 max_db;			//该sensor的最大增益
	U08 icrSensitity; 	//当前用户配置的灵敏度，range 0~7，default 4
	U08	enInnerSensitity;//是否启用内部灵敏度
} DNC_PARAM_S;



/********* function declaration *********/
int  get_daynight_flag(int chan, DNC_AE_STATIS_S sta);		// get current day/night status
void set_daynight_param(int chan, DNC_PARAM_S param);		// set some threshold params
void set_daynight_dbg(int chan, int value);					// set print level
void set_daynight_key(int chan, DNC_KEY_E key, int val);	// set some params
void get_daynight_key(int chan, DNC_KEY_E key, int *val);	// get some params


#ifdef __cplusplus
}
#endif

#endif

