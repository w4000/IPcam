#ifndef __NV_ISP_IQ_H__
#define __NV_ISP_IQ_H__

int nv_isp_iq_init(const char *conf_file);
int nv_isp_iq_uninit();

int nv_isp_iq_daynight(int night);

int nv_isp_iq_brightness(int val);
int nv_isp_iq_contrast(int val);
int nv_isp_iq_saturation(int val);

int nv_isp_iq_denoise(int val);
int nv_isp_iq_sharpen(int val);


int nv_isp_iq_wdr(int mode, int strength);

#endif /*__NV_ISP_IQ_H__*/
