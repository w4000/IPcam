

#ifndef ONVIF_THERMAL_H
#define ONVIF_THERMAL_H

#include "onvif.h"
#include "onvif_cm.h"


typedef struct
{
    char    VideoSourceToken[100];              // requied, Reference token to the VideoSource for which the Thermal Settings are requested
} tth_GetConfiguration_REQ;

typedef struct
{
    char    VideoSourceToken[100];              // requied, Reference token to the VideoSource for which the Thermal Settings are configured

    onvif_ThermalConfiguration Configuration;               // requied, Thermal Settings to be configured
} tth_SetConfiguration_REQ;

typedef struct
{
    char    VideoSourceToken[100];              // requied, Reference token to the VideoSource for which the Thermal Configuration Options are requested
} tth_GetConfigurationOptions_REQ;

typedef struct 
{
	char    VideoSourceToken[100];	            // required, Reference token to the VideoSource for which the Radiometry Configuration is requested
} tth_GetRadiometryConfiguration_REQ;

typedef struct 
{
	char    VideoSourceToken[100];	            // required, Reference token to the VideoSource for which the Radiometry settings are configured

	onvif_RadiometryConfiguration   Configuration;	        // required, Radiometry settings to be configured
} tth_SetRadiometryConfiguration_REQ;

typedef struct 
{
	char    VideoSourceToken[100];	            // required, Reference token to the VideoSource for which the Thermal Radiometry Options are requested
} tth_GetRadiometryConfigurationOptions_REQ;

#ifdef __cplusplus
extern "C" {
#endif

ONVIF_RET onvif_tth_SetConfiguration(tth_SetConfiguration_REQ * p_req);
ONVIF_RET onvif_tth_SetRadiometryConfiguration(tth_SetRadiometryConfiguration_REQ * p_req);

#ifdef __cplusplus
}
#endif

#endif


