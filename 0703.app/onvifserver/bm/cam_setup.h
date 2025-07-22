#ifndef __CAM_SETUP_H__
#define __CAM_SETUP_H__

#ifdef __cplusplus
extern "C" {
#endif
//w4000
#include "common.h"
#include "setup/setup.h"
#include "onvifserver.h"
#include "onvif.h"
#include "onvif_device.h"
#include "onvif_deviceio.h"
#include "onvif_media.h"
#include "onvif_media2.h"
#include "onvif_image.h"
#include "ipc/ipc_protocol.h"
#include "ipc/ipc_client.h"

void cam_setup_update(_CAMSETUP *setup, _CAMSETUP_EXT *setup_ext);
int check_cam_setup_update(void);

_CAMSETUP *cam_setup();
_CAMSETUP_EXT *cam_setup_ext();
NSETUP_T *cam_nsetup();

int scope_file_remove();
int scope_file_update();
int scope_file_read();

/* device */
int cam_setup_apply_discovery();
int cam_setup_apply_datetime(onvif_DateTime *datetime);
int cam_setup_apply_network_interface(tds_SetNetworkInterfaces_REQ *p_req);
int cam_setup_apply_network_protocol(tds_SetNetworkProtocols_REQ *p_req);
int cam_setup_apply_network_default_gateway(tds_SetNetworkDefaultGateway_REQ *p_req);
int cam_setup_apply_ntp(tds_SetNTP_REQ *p_req);
int cam_setup_apply_user();

/* media */
int cam_setup_apply_video_encoder(int ch, trt_SetVideoEncoderConfiguration_REQ *p_req);
int cam_setup_apply_video_encoder2(int ch, tr2_SetVideoEncoderConfiguration_REQ *p_req);
int cam_setup_apply_profile(ONVIF_PROFILE *profiles);

/* imaging */
int cam_setup_apply_imaging(img_SetImagingSettings_REQ * p_req);



#ifdef __cplusplus
}
#endif


#endif /* __CAM_SETUP_H__ */


