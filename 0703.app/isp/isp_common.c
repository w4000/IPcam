

#include "common.h" 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/un.h>
#include "device/gpio.h"

#include "hdal.h"
#include "nv_define.h"
#include "nv_isp_iq.h"

#include "nv_isp_awb.h"
#include "isp_ctrl.h"
#include "nv_isp_ae.h"
#include "nv_isp.h"
#include "isp_ctrl.h"




static pthread_t  		isp_tid = 0;
static pthread_mutex_t	isp_mutex = PTHREAD_MUTEX_INITIALIZER;
static int isp_thread_running = 0;

static unsigned char	g_run_isp = 0;

static ISP_TDN_MODE		g_tdn_mode = TDN_UNKNOWN;
static ISP_TDN_MODE 	g_tdn_mode_old = TDN_UNKNOWN;
static ISP_TDN_STATUS 	g_tdn_status = TDN_STATUS_DAY;

static int g_use_autolight = 0;
static int g_wdr = 0;
static int g_brightness = 0;
static int g_contrast = 0;
static int g_saturation = 0;
static int g_sharpen = 0;
static int g_denoise = 0;
static int g_iq_update = 0;

static int64_t			g_motion_last_msec = 0;


static unsigned int		g_tdn_night_start = 1080;
static unsigned int		g_tdn_night_end = 360;



int nv_isp_iq_uninit()
{
	return 0;
}

int nv_isp_iq_daynight(int night)
{
	int ret;
	IQT_NIGHT_MODE night_mode = {0};

	night_mode.id = 0;
	night_mode.mode = (night)?IQ_UI_NIGHT_MODE_ON:IQ_UI_NIGHT_MODE_OFF;
	// printf("night mode? %s \n", (night)? "NIGHT MODE ON":"NIGHT MODE OFF");
	ret = vendor_isp_set_iq(IQT_ITEM_NIGHT_MODE, &night_mode);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int nv_isp_iq_brightness(int val)
{
	int ret;
	IQT_BRIGHTNESS_LV brightness_lv = {0};

	brightness_lv.lv = val*2;
	ret = vendor_isp_set_iq(IQT_ITEM_BRIGHTNESS_LV, &brightness_lv);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}


int nv_isp_iq_denoise(int val)
{
	int ret;
	IQT_NR_LV nr = {0};

	if(val < 100) {
		val = 100 - ((100-val)/2);
	}

	nr.lv = val;
	ret = vendor_isp_set_iq(IQT_ITEM_NR_LV, &nr);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}


int nv_isp_iq_sharpen(int val)
{
	int ret;
	IQT_SHARPNESS_LV sharpen = {0};

	if(val > 100) {
		val = 100 + ((val-100)/2);
	}

	sharpen.lv = val+65;
	ret = vendor_isp_set_iq(IQT_ITEM_SHARPNESS_LV, &sharpen);

	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}




int nv_isp_ae_init(const char *conf_file)
{
	int ret;
	AET_CFG_INFO cfg_info = {0};
	
	snprintf(cfg_info.path, CFG_NAME_LENGTH, conf_file);


	ret = vendor_isp_set_ae(AET_ITEM_RLD_CONFIG, &cfg_info);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int nv_isp_ae_uninit()
{
	return 0;
}


int nv_isp_ae_gain()
{
	int ret;
	
	AET_GAIN_BOUND gain = {0};
	

	gain.bound.l = 0;
	gain.bound.h = 20;
	ret = vendor_isp_set_ae(AET_ITEM_GAIN_BOUND, &gain);	
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int nv_isp_iq_contrast(int val)
{
	int ret;
	IQT_CONTRAST_LV contrast_lv = {0};

	contrast_lv.lv = val*2;
	ret = vendor_isp_set_iq(IQT_ITEM_CONTRAST_LV, &contrast_lv);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int nv_isp_iq_saturation(int val)
{
	int ret;
	IQT_SATURATION_LV saturation_lv = {0};

	saturation_lv.lv = val*2;
	ret = vendor_isp_set_iq(IQT_ITEM_SATURATION_LV, &saturation_lv);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}



int nv_isp_iq_init(const char *conf_file)
{
	int ret;
	IQT_CFG_INFO cfg_info = {0};

	snprintf(cfg_info.path, CFG_NAME_LENGTH, conf_file);

	ret = vendor_isp_set_iq(IQT_ITEM_RLD_CONFIG, &cfg_info);
	if(ret != HD_OK) {
		goto Fail;
	}


	return 0;
Fail:
	return -1;
}



int nv_isp_iq_wdr(int mode, int strength)
{
	int i;
	int ret;
	IQT_WDR_PARAM wdr = {0};
	static int first_time = 1;
	static IQ_WDR_AUTO_PARAM auto_param[IQ_GAIN_ID_MAX_NUM];

	ret = vendor_isp_get_iq(IQT_ITEM_WDR_PARAM, &wdr);
	if(ret != HD_OK) {
		goto Fail;
	}

	if(first_time) {
		for(i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
			memcpy(&auto_param[i], &wdr.wdr.auto_param[i], sizeof(IQ_WDR_AUTO_PARAM));
		}
		first_time = 0;
	}

	wdr.wdr.enable = (mode)?1:0;
	wdr.wdr.mode = (mode == 1)?IQ_OP_TYPE_AUTO:IQ_OP_TYPE_MANUAL;

	if(wdr.wdr.mode == IQ_OP_TYPE_AUTO) {
		for(i = 0; i < IQ_GAIN_ID_MAX_NUM; i++) {
			int level = (int)auto_param[i].level + (strength - 128);
			if(level < (int)auto_param[i].strength_min)
				level = (int)auto_param[i].strength_min;
			else if(level > (int)auto_param[i].strength_max)
				level = (int)auto_param[i].strength_max;
			wdr.wdr.auto_param[i].level = (UINT32)level;
		}
	}
	else {
		wdr.wdr.manual_param.strength = strength;
	}

	ret = vendor_isp_set_iq(IQT_ITEM_WDR_PARAM, &wdr);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}





int nv_isp_awb_init(const char *conf_file)
{
	int ret;
	AWBT_CFG_INFO cfg_info = {0};
	
	snprintf(cfg_info.path, CFG_NAME_LENGTH, conf_file);


	ret = vendor_isp_set_awb(AWBT_ITEM_RLD_CONFIG, &cfg_info);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int nv_isp_awb_uninit()
{
	return 0;
}


int nv_isp_awb_mode(char mode)
{
	int ret;
	AWBT_SCENE_MODE scene_mode = {0};

	switch(mode) {
		case ISP_AWB_MANUAL:
			scene_mode.mode = AWB_SCENE_MGAIN;
			break;
		case ISP_AWB_DAYLIGHT:
			scene_mode.mode = AWB_SCENE_DAYLIGHT;
			break;
		case ISP_AWB_CLOUDY:
			scene_mode.mode = AWB_SCENE_CLOUDY;
			break;
		case ISP_AWB_TUNGSTEN:
			scene_mode.mode = AWB_SCENE_TUNGSTEN;
			break;
		case ISP_AWB_SUNSET:
			scene_mode.mode = AWB_SCENE_SUNSET;
			break;
		case ISP_AWB_AUTO:
		default:
			scene_mode.mode = AWB_SCENE_AUTO;
			break;
	}

	ret = vendor_isp_set_awb(AWBT_ITEM_SCENE, &scene_mode);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int nv_isp_awb_ratio(unsigned char r_ratio, unsigned char b_ratio)
{
	int ret;
	AWBT_WB_RATIO wb_ratio = {0};

	if(r_ratio < 1) r_ratio = 1;
	if(b_ratio < 1) b_ratio = 1;

	wb_ratio.r = r_ratio;
	wb_ratio.b = b_ratio;

	ret = vendor_isp_set_awb(AWBT_ITEM_WB_RATIO, &wb_ratio);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int nv_isp_awb_manual_gain(char on, unsigned char r_gain, unsigned char g_gain, unsigned char b_gain)
{
	int ret;
	AWBT_MANUAL manual = {0};

	if(r_gain < 1) r_gain = 1;
	if(g_gain < 1) g_gain = 1;
	if(b_gain < 1) b_gain = 1;

	manual.manual.en = on;
	manual.manual.r_gain = r_gain;
	manual.manual.g_gain = g_gain;
	manual.manual.b_gain = b_gain;

	ret = vendor_isp_set_awb(AWBT_ITEM_MANUAL, &manual);
	if(ret != HD_OK) {
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}




int isp_tdn_mode(ISP_TDN_MODE mode)
{
	pthread_mutex_lock(&isp_mutex);

	if(g_tdn_mode != mode) {
		g_tdn_mode = mode;
	}

	pthread_mutex_unlock(&isp_mutex);

	return 0;
}


int isp_wdr(int wdr)
{
	pthread_mutex_lock(&isp_mutex);

	if(g_wdr != wdr) {
		g_wdr = wdr;
	}
	g_iq_update = 1;

	pthread_mutex_unlock(&isp_mutex);

	return 0;
}


int isp_brightness(int val)
{
	pthread_mutex_lock(&isp_mutex);

	if(g_brightness != val) {
		g_brightness = val;
	}
	g_iq_update = 1;

	pthread_mutex_unlock(&isp_mutex);

	return 0;
}

int isp_contrast(int val)
{
	pthread_mutex_lock(&isp_mutex);

	if(g_contrast != val) {
		g_contrast = val;
	}
	g_iq_update = 1;

	pthread_mutex_unlock(&isp_mutex);

	return 0;
}

int isp_saturation(int val)
{
	pthread_mutex_lock(&isp_mutex);

	if(g_saturation != val) {
		g_saturation = val;
	}
	g_iq_update = 1;

	pthread_mutex_unlock(&isp_mutex);

	return 0;
}


int isp_denoise(int val)
{
	pthread_mutex_lock(&isp_mutex);

	if(g_denoise != val) {
		g_denoise = val;
	}
	g_iq_update = 1;

	pthread_mutex_unlock(&isp_mutex);

	return 0;
}


int isp_sharpen(int  val)
{
	pthread_mutex_lock(&isp_mutex);

	if(g_sharpen != val) {
		g_sharpen = val;
	}
	g_iq_update = 1;

	pthread_mutex_unlock(&isp_mutex);

	return 0;
}

static int isp_iq_proc(ISP_TDN_MODE tdn_mode, ISP_TDN_STATUS tdn_status)
{
	AET_STATUS_INFO ae_status = {0};
	static ISP_TDN_STATUS tdn = TDN_STATUS_UNKNOWN;
	static unsigned int iso = 0;
	static int autolight = -1;
	static long iq_time = 0;
	static int color_daynight = 1; 
	int iso_gain;
	//w4000_tdn


	pthread_mutex_lock(&isp_mutex);

	if(vendor_isp_get_ae(AET_ITEM_STATUS, &ae_status) != HD_OK) {
		pthread_mutex_unlock(&isp_mutex);
		return -1;
	}
	iso_gain = ae_status.status_info.iso_gain[0];
	if(tdn_status != tdn || iso_gain != iso || g_use_autolight != autolight || g_iq_update) {
		if(g_use_autolight) {
			if(tdn_status == TDN_STATUS_DAY) {
			}
			else {
			}
		}
		else {

			if(tdn_status == TDN_STATUS_DAY) {

				if(!color_daynight) { 
					color_daynight = 1;
				}
				set_tdn_night_gpio(0);//w4000

				nv_isp_iq_brightness(g_brightness);
				nv_isp_iq_contrast(g_contrast);
				nv_isp_iq_saturation(g_saturation);
				nv_isp_iq_sharpen(g_sharpen-30);
				nv_isp_iq_denoise(0);

				nv_isp_iq_wdr(g_setup.nsetup.isp.blc.wdr.mode.val, g_setup.nsetup.isp.blc.wdr.strength.val);
				
				printf("TDN_STATUS_DAY %s, %d \r\n",__FUNCTION__,__LINE__);//w4000
			}
			else 
			{
				nv_isp_iq_brightness(g_brightness);
				nv_isp_iq_contrast(g_contrast);
				nv_isp_iq_saturation(g_saturation);
				nv_isp_iq_sharpen(g_sharpen);
				nv_isp_iq_denoise(g_denoise);
				
				 set_tdn_night_gpio(1);//w4000
				printf("TDN_STATUS_night %s, %d \r\n",__FUNCTION__,__LINE__);//w4000
			}
		}
	}

	iso = iso_gain;
	tdn = tdn_status;
	autolight = g_use_autolight;
	g_iq_update = 0;

	pthread_mutex_unlock(&isp_mutex);

	return 0;
}


void isp_exit(void)
{
	int status;
	isp_thread_running = 0;
	if (isp_tid > 0)
	{
		pthread_join(isp_tid, (void **)&status);
		isp_tid = 0;
	}
}

int is_run_isp()
{
	return 1;
}

extern int is_isp_auto()
{
	return 1;
}

extern int is_isp_tunning()
{
	return 0;
}

static int isp_update_ae_blc()
{
	nv_isp_ae_weight();

	return 0;
}
#define w4000_default
int isp_update_iq()//w4000_default
{
#if 0
	return 0;
#endif 


	if(!isp_thread_running)
	{
		return -1;
	}


	isp_brightness(g_setup.nsetup.isp.iq.color.brightness.val);
	isp_contrast(g_setup.nsetup.isp.iq.color.contrast.val);
	isp_saturation(g_setup.nsetup.isp.iq.color.saturation.val);

	isp_denoise(g_setup.nsetup.isp.iq.nr.denoise.val);
	isp_sharpen(g_setup.nsetup.isp.iq.sharpeness.val);




	return 0;
}

int isp_update_ae()
{
	if(!isp_thread_running)
		return -1;

	isp_shutter_iris(&g_setup.nsetup.isp.ae);


	return 0;
}

int isp_update_dn()
{
	if(!isp_thread_running)
		return -1;


	int dn_mode = TDN_DAY;
	switch(g_setup.nsetup.isp.dn.mode.val) {
		case 0: dn_mode = 3; break;
		case 1: dn_mode = 1; break;
		case 2: dn_mode = 2; break;
		default: dn_mode = 0;
	}

	isp_tdn_mode(dn_mode);
	g_tdn_mode_old = TDN_UNKNOWN;

	return 0;
}


int nv_isp_init(const char *conf_file)
{
	int ret;
	ISPT_FUNC func_info = {0};

	ret = vendor_isp_init();
	if(ret != HD_OK) {
		printf("Failed to call vendor_isp_init()!!! (ret=%d)\n", ret);
		goto Fail;
	}

	nv_isp_ae_init(conf_file);
	nv_isp_awb_init(conf_file);
	nv_isp_iq_init(conf_file);

	vendor_isp_get_common(ISPT_ITEM_FUNC, &func_info);

	return 0;
Fail:
	return -1;
}

int nv_isp_uninit()
{
	nv_isp_ae_uninit();
	nv_isp_awb_uninit();
	nv_isp_iq_uninit();
	vendor_isp_uninit();

	return 0;
}



int nv_isp_ae_shutter_speed(int min, int max)
{
	int ret;
	AET_EXPT_BOUND expt = {0};

	expt.bound.l = min;
	expt.bound.h = max;
	ret = vendor_isp_set_ae(AET_ITEM_EXPT_BOUND, &expt);	
	if(ret != HD_OK) {
		printf("Failed to call vendor_isp_set_ae(AET_ITEM_EXPT_BOUND)!!! (ret=%d)\n", ret);
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}

int nv_isp_ae_weight()
{
	int i;
	int ret;
	AET_METER_WINDOW meter_window = {0};
	unsigned int matrix[8*8] = {1, 1, 1, 1, 1, 1, 1, 1,1, 2, 2, 2, 2, 2, 2, 1,1, 2, 3, 3, 3, 3, 2, 1,1, 2, 3, 4, 4, 3, 2, 1,1, 2, 3, 4, 4, 3, 2, 1,1, 2, 3, 3, 3, 3, 2, 1,1, 2, 2, 2, 2, 2, 2, 1,1, 1, 1, 1, 1, 1, 1, 1};
	ret = vendor_isp_get_ae(AET_ITEM_METER_WIN, &meter_window);
	if(ret != HD_OK) {
		printf("Failed to call vendor_isp_get_ae(AET_ITEM_METER_WIN)!!! (ret=%d)\n", ret);
		goto Fail;
	}

	for(i = 0; i < 8*8; i++) {
		meter_window.meter_window.matrix[i] = matrix[i];
	}

	ret = vendor_isp_set_ae(AET_ITEM_METER_WIN, &meter_window);
	if(ret != HD_OK) {
		printf("Failed to call vendor_isp_set_ae(AET_ITEM_METER_WIN)!!! (ret=%d)\n", ret);
		goto Fail;
	}

	return 0;
Fail:
	return -1;
}


int isp_shutter_iris(NSETUP_ISP_AE_T *ae)
{
	nv_isp_ae_shutter_speed(ae->shutter.min.val, ae->shutter.max.val);

	return 0;
}

static void update_camera_config(int update_type)
{
	_CAMSETUP set;
	_CAMSETUP_EXT set_ext;

	read_enc_setup(&set, &set_ext);
	g_setup = set;
	g_setup_ext = set_ext;

	switch(update_type)
	{
		case UPDATE_FCGI_ISP_IQ:
			isp_brightness(g_setup.nsetup.isp.iq.color.brightness.val);
			isp_contrast(g_setup.nsetup.isp.iq.color.contrast.val);
			isp_saturation(g_setup.nsetup.isp.iq.color.saturation.val);
			isp_denoise(g_setup.nsetup.isp.iq.nr.denoise.val);
			isp_sharpen(g_setup.nsetup.isp.iq.sharpeness.val);
			isp_iq_proc(g_tdn_mode, g_tdn_status);
			onvifserver_set_setup(&g_setup, &g_setup_ext);
			onvifserver_update_isp();
			break;
		case UPDATE_FCGI_ISP_AE:
			isp_update_ae();
			onvifserver_set_setup(&g_setup, &g_setup_ext);
			onvifserver_update_isp();
			break;
		case UPDATE_FCGI_ISP_BLC:
			isp_update_blc();
			onvifserver_set_setup(&g_setup, &g_setup_ext);
			onvifserver_update_isp();
			break;
		case UPDATE_FCGI_ISP_AWB:
			isp_update_awb();
			onvifserver_set_setup(&g_setup, &g_setup_ext);
			onvifserver_update_isp();
			break;
		case UPDATE_FCGI_ISP_DN:
			isp_update_dn();
			break;
	}
}

static int isp_tdn_proc(int first);
void *isp_proc(void *data)
{
	int first = 1;
	int cnt = 0;
	char update_type;
	char *path = "/tmp/camera_config";
	int skfd, res;
	struct sockaddr_un svr;
	struct sockaddr_in cli;
	int clilen;
	fd_set rdfd;
	struct timeval tval;

	if (!access(path, F_OK))
		unlink(path);
	memset(&svr, 0, sizeof(svr));
	svr.sun_family = AF_UNIX;
	strcpy(svr.sun_path, path);
	skfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (skfd < 0)
		printf("========> Error : isp_proc : socket open error <=========\n");
	else 
	{
		if (bind(skfd, (struct sockaddr *)&svr, sizeof(svr)) < 0)
		{
			printf("========> Error : isp_proc : socket bind error <=========\n");
			skfd = -1;
			close(skfd);
		}
		else
		{
			clilen = sizeof(struct sockaddr_in);
			memset((void *)&cli, 0, clilen);
		}
	}

	while(isp_thread_running)
	{
		first = isp_tdn_proc(first);
		if (cnt > 100)	// 1 sec
		{
			isp_iq_proc(g_tdn_mode, g_tdn_status);
			cnt = 0;
		}
		if (skfd > 0)
		{
			FD_ZERO(&rdfd);
			FD_SET(skfd, &rdfd);
			tval.tv_sec = 0;
			tval.tv_usec = 10 * 1000;
			if (select(skfd + 1, &rdfd, NULL, NULL, &tval) <= 0)
			{
				cnt++;
				continue;
			}
			if (recvfrom(skfd, (void *)&update_type, 1, 0, (struct sockaddr *)&cli, (socklen_t *)&clilen) <= 0)
				continue;
			if (update_type == UPDATE_FCGI_ISP_IQ)
				cnt = 0;
			update_camera_config(update_type);
		}
		else
		{
			cnt++;
			usleep(10 * 1000);
		}
	}
	isp_tdn_mode(TDN_DAY);
	isp_tdn_proc(0);

	if (skfd)
		close(skfd);
	pthread_exit(NULL);
}

int isp_init()
{
	g_run_isp = 1;

	// create thread
	isp_thread_running = 1;
	if (pthread_create (&isp_tid, NULL, isp_proc, NULL) < 0) {
		return -1;
	}

	return 0;
}


int isp_update_blc()
{
	if(!isp_thread_running)
		return -1;

	isp_update_ae_blc();
	isp_wdr(g_setup.nsetup.isp.blc.wdr.mode.val);

	return 0;
}

int isp_update_awb()
{
	if(!isp_thread_running)
		return -1;


	nv_isp_awb_mode(g_setup.nsetup.isp.awb.mode.val);
	nv_isp_awb_ratio(g_setup.nsetup.isp.awb.auto_ratio.r_ratio.val, g_setup.nsetup.isp.awb.auto_ratio.b_ratio.val);
	nv_isp_awb_manual_gain((g_setup.nsetup.isp.awb.mode.val == ISP_AWB_MANUAL)?1:0,
			g_setup.nsetup.isp.awb.manual_gain.r_gain.val,
			g_setup.nsetup.isp.awb.manual_gain.g_gain.val,
			g_setup.nsetup.isp.awb.manual_gain.b_gain.val);

	return 0;
}



static void icr_day_mode(void)
{
	printf("%s, %d \r\n",__FUNCTION__,__LINE__);//w4000
	gpio_tdn_mode(1);
	//w4000 dn PQ
	isp_brightness(g_setup.nsetup.isp.iq.color.brightness.val);
	isp_contrast(g_setup.nsetup.isp.iq.color.contrast.val);
	
}

static void icr_night_mode(void)
{
	printf("%s, %d \r\n",__FUNCTION__,__LINE__);//w4000
	gpio_tdn_mode(0);
	isp_brightness(g_setup.nsetup.isp.iq.color.brightness.val+15);
	isp_contrast(g_setup.nsetup.isp.iq.color.contrast.val+5);
}

void set_tdn_night_gpio(int en)//w4000
{
	if(en)//night
		{
		gpio_tdn_mode(0);
		gpio_tdn_enable();

		}
	else//day
		{
		gpio_tdn_mode(1);//w4000_tdn
		gpio_tdn_enable();

		}
}
static int isp_tdn_proc(int first)
{
	static long tdn_status_time = 0;
	static int tdn_status_step = 0;
	static unsigned char cds_in_old = 255;
	static long tdn_auto_time = 0;


	static int prev_is_day = -1;

	int tdnSwitch = 0;
	int i;
	int is_day = gpio_is_tdn_day();
#if 1
	{
	static unsigned int setup_delay=0;
		if(g_tdn_mode==TDN_AUTO||g_tdn_mode==TDN_AUTO_EXP)
		{
			if (is_day) 
			{
				g_tdn_status = TDN_STATUS_NIGHT;
			}
			else 
			{
				g_tdn_status = TDN_STATUS_DAY;
			}

		}
		else
		{

		}
	}
#endif
	if (is_day != prev_is_day)
	{
		prev_is_day = is_day;
		tdnSwitch = 1;
	}


	if(first) {
		g_tdn_mode_old = TDN_UNKNOWN;
	}
	else 
	{
		if (g_tdn_mode_old != g_tdn_mode)
		{
			isp_saturation(g_setup.nsetup.isp.iq.color.saturation.val);
			tdnSwitch = 1;
		}
	}

	if(g_tdn_mode == TDN_UNKNOWN) {
		isp_saturation(g_setup.nsetup.isp.iq.color.saturation.val);
		return 1;
	}

	switch(g_tdn_mode)
	{
		case TDN_AUTO:
		case TDN_AUTO_EXP:
			if(g_tdn_mode_old != g_tdn_mode || tdnSwitch) {

				// printf("is_day? %d , tdnSwitch? %d \n", is_day, tdnSwitch);
				if (is_day) {
					//w4000 g_tdn_status = TDN_STATUS_DAY;
					g_tdn_status = TDN_STATUS_NIGHT;
					tdn_status_step = 0;
				}
				else {
					//w4000 g_tdn_status = TDN_STATUS_NIGHT;
					g_tdn_status = TDN_STATUS_DAY;
					tdn_status_step = 0;
				}
			}

			break;


		case TDN_DAY:
			if(g_tdn_mode_old != TDN_DAY)
			{
				g_tdn_status = TDN_STATUS_DAY;
				tdn_status_step = 0;
			}
			break;

		case TDN_NIGHT:
			if(g_tdn_mode_old != TDN_NIGHT)
			{
				g_tdn_status = TDN_STATUS_NIGHT;
				tdn_status_step = 0;
			}
			break;

		default:
			break;
	}

	g_tdn_mode_old = g_tdn_mode;

	if(tdn_status_step == 0 && g_tdn_status == TDN_STATUS_NIGHT) {
		g_motion_last_msec = get_time_msec();
	}

#if 1//w4000
	 if(g_tdn_mode==TDN_AUTO||g_tdn_mode==TDN_AUTO_EXP)
	{
		 static int day_mode=100;
		 static int night_mode=100;
		if(g_tdn_status == TDN_STATUS_NIGHT)
			{
			if(night_mode<100)
				{
				night_mode++;
				return 0;
				}
			if(day_mode)
				{
					printf("set TDN_AUTO night= %s, %d \r\n",__FUNCTION__,__LINE__);//w4000
					
					isp_saturation(0);
					set_tdn_night_gpio(0);

				}
			day_mode=0;

			}
		else
			if(g_tdn_status == TDN_STATUS_DAY)
			{
			if(day_mode<100)
				{
				day_mode++;
				return 0;
				}
			if(night_mode)
				{
				printf("set TDN_AUTO Day= %s, %d \r\n",__FUNCTION__,__LINE__);//w4000
				
				isp_saturation(g_setup.nsetup.isp.iq.color.saturation.val);
					set_tdn_night_gpio(1);
				}
			night_mode=0;

			}

	}
	 else
#endif
	if (g_tdn_mode_old != g_tdn_mode || tdnSwitch)
	{		
		if(g_use_autolight) {
			icr_day_mode();
			usleep(10*1000);
			nv_isp_iq_daynight(0);
			printf("set g_use_autolight = %s, %d \r\n",__FUNCTION__,__LINE__);//w4000
		}
		else 
		{
			switch(g_tdn_status)//w4000 manual
			{
				case TDN_STATUS_DAY:
				gpio_tdn_mode(0);
				gpio_tdn_enable();
				
				isp_saturation(g_setup.nsetup.isp.iq.color.saturation.val);
				
				isp_iq_proc(g_tdn_mode, g_tdn_status);//w4000 add
				printf("set g_use_autolight TDN_STATUS_DAY = %s, %d \r\n",__FUNCTION__,__LINE__);//w4000
				break;


				case TDN_STATUS_NIGHT:	
				gpio_tdn_enable();
				isp_saturation(0);
				isp_iq_proc(g_tdn_mode, g_tdn_status);//w4000 add
				gpio_tdn_mode(1);
				printf("set g_use_autolight TDN_STATUS_NIGHT = %s, %d \r\n",__FUNCTION__,__LINE__);//w4000
				break;
				
			}
		}
	}
	return 0;
}

