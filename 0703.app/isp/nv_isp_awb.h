#ifndef __NV_ISP_AWB_H__
#define __NV_ISP_AWB_H__

int nv_isp_awb_init(const char *conf_file);
int nv_isp_awb_uninit();

int nv_isp_awb_mode(char mode);
int nv_isp_awb_ratio(unsigned char r_ratio, unsigned char b_ratio);
int nv_isp_awb_manual_gain(char on, unsigned char r_gain, unsigned char g_gain, unsigned char b_gain);

#endif /*__NV_ISP_AWB_H__*/
