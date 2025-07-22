#ifndef __NV_ISP_AE_H__
#define __NV_ISP_AE_H__

int nv_isp_ae_init(const char *conf_file);
int nv_isp_ae_uninit();

int nv_isp_ae_shutter_speed(int min, int max);
int nv_isp_ae_weight();
int nv_isp_ae_gain();

#endif /*__NV_ISP_AE_H__*/
