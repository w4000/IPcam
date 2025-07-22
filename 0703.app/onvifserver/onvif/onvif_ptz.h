

#ifndef ONVIF_PTZ_H
#define ONVIF_PTZ_H

#include "sys_inc.h"
#include "onvif.h"
#include <stdint.h>


typedef struct
{
	uint32	TimeoutFlag 	: 1;				// Indicates whether the field Timeout is valid
	uint32  Reserved	 	: 31;
	
	char	ProfileToken[100];		// required, A reference to the MediaProfile
	
	onvif_PTZSpeed	Velocity;					// required, A Velocity vector specifying the velocity of pan, tilt and zoom
	
	int		Timeout;							// optional, An optional Timeout parameter, unit is second
} ptz_ContinuousMove_REQ;

typedef struct 
{
	uint32	PanTiltFlag 	: 1;				// Indicates whether the field PanTilt is valid
	uint32	ZoomFlag 		: 1;				// Indicates whether the field Zoom is valid
	uint32  Reserved	 	: 30;
	
	char	ProfileToken[100];		// required, A reference to the MediaProfile that indicate what should be stopped

	BOOL	PanTilt;							// optional, Set true when we want to stop ongoing pan and tilt movements.If PanTilt arguments are not present, this command stops these movements
	BOOL	Zoom;								// optional, Set true when we want to stop ongoing zoom movement.If Zoom arguments are not present, this command stops ongoing zoom movement
} ptz_Stop_REQ;

typedef struct
{
	uint32	SpeedFlag 		: 1;				// Indicates whether the field Speed is valid
	uint32  Reserved	 	: 31;
	
    char	ProfileToken[100];		// required, A reference to the MediaProfile

	onvif_PTZVector	Position;					// required, A Position vector specifying the absolute target position
	onvif_PTZSpeed	Speed;						// optional, An optional Speed    
} ptz_AbsoluteMove_REQ;

typedef struct
{
	uint32	SpeedFlag 		: 1;				// Indicates whether the field Speed is valid
	uint32  Reserved	 	: 31;
	
    char	ProfileToken[100];		// required, A reference to the MediaProfile
    
    onvif_PTZVector	Translation;				// required, A positional Translation relative to the current position
	onvif_PTZSpeed	Speed;						// optional, An optional Speed parameter
} ptz_RelativeMove_REQ;

typedef struct
{
	uint32	PresetTokenFlag : 1;				// Indicates whether the field PresetToken is valid
	uint32	PresetNameFlag 	: 1;				// Indicates whether the field PresetName is valid
	uint32  Reserved	 	: 30;
	
    char	ProfileToken[100];		// required, A reference to the MediaProfile where the operation should take place
    char	PresetToken[100];		// optional, A requested preset token
    char    PresetName[100];			// optional, A requested preset name
} ptz_SetPreset_REQ;

typedef struct
{
	char	ProfileToken[100];		// required, A reference to the MediaProfile where the operation should take place
    char	PresetToken[100];		// required, A requested preset token
} ptz_RemovePreset_REQ;

typedef struct
{
	uint32	SpeedFlag 		: 1;				// Indicates whether the field Speed is valid
	uint32  Reserved	 	: 31;
	
	char	ProfileToken[100];		// required, A reference to the MediaProfile where the operation should take place
    char	PresetToken[100];		// required, A requested preset token

	onvif_PTZSpeed	Speed;						// optional, A requested speed.The speed parameter can only be specified when Speed Spaces are available for the PTZ Node
} ptz_GotoPreset_REQ;

typedef struct
{
	uint32	SpeedFlag 		: 1;				// Indicates whether the field Speed is valid
	uint32  Reserved	 	: 31;
	
    char	ProfileToken[100];		// required, A reference to the MediaProfile where the operation should take place

    onvif_PTZSpeed	Speed;						// optional, A requested speed.The speed parameter can only be specified when Speed Spaces are available for the PTZ Node
} ptz_GotoHomePosition_REQ;

typedef struct
{
	onvif_PTZConfiguration  PTZConfiguration;	// required, 

	BOOL	ForcePersistence;					// required, 	
} ptz_SetConfiguration_REQ;

typedef struct
{
    char    ProfileToken[100];	    // required, Contains the token of an existing media profile the configurations shall be compatible with
} ptz_GetCompatibleConfigurations_REQ;

typedef struct
{
    char    ProfileToken[100];      // required
} ptz_GetPresetTours_REQ;

typedef struct
{
    PresetTourList *  PresetTour;
} ptz_GetPresetTours_RES;

typedef struct
{
    char    ProfileToken[100];      // required
	char    PresetTourToken[100];   // required
} ptz_GetPresetTour_REQ;

typedef struct
{
    onvif_PresetTour    PresetTour;
} ptz_GetPresetTour_RES;

typedef struct
{
    uint32	PresetTourTokenFlag : 1;	        // Indicates whether the field PresetTourToken is valid
	uint32  Reserved	 	    : 31;
	
    char    ProfileToken[100];      // required
	char    PresetTourToken[100];   // optional
} ptz_GetPresetTourOptions_REQ;

typedef struct
{
    onvif_PTZPresetTourOptions  Options;        // required
} ptz_GetPresetTourOptions_RES;

typedef struct
{
    char    ProfileToken[100];      // required
} ptz_CreatePresetTour_REQ;

typedef struct
{
    char    PresetTourToken[100];   // required, 
} ptz_CreatePresetTour_RES;

typedef struct
{
    char    ProfileToken[100];      // required
    
	onvif_PresetTour    PresetTour;             // required
} ptz_ModifyPresetTour_REQ;

typedef struct
{
    char    ProfileToken[100];      // required
	char    PresetTourToken[100];   // required
	
	onvif_PTZPresetTourOperation Operation;     // required
} ptz_OperatePresetTour_REQ;

typedef struct
{
    char    ProfileToken[100];      // required
	char    PresetTourToken[100];   // required
} ptz_RemovePresetTour_REQ;

typedef struct
{
    char    ProfileToken[100];      // required, 
	char    AuxiliaryData[64];                  // required, 
} ptz_SendAuxiliaryCommand_REQ;

typedef struct
{
    char    AuxiliaryResponse[256];             // required, 
} ptz_SendAuxiliaryCommand_RES;

typedef struct
{
    uint32	SpeedFlag       : 1;	            // Indicates whether the field Speed is valid
    uint32	AreaHeightFlag  : 1;	            // Indicates whether the field AreaHeight is valid
    uint32	AreaWidthFlag   : 1;	            // Indicates whether the field AreaWidth is valid
	uint32  Reserved        : 29;
	
    char    ProfileToken[100];	    // required, A reference to the MediaProfile

    onvif_GeoLocation   Target;	                // required, The geolocation of the target position
    onvif_PTZSpeed      Speed;	                // optional, An optional Speed

    float   AreaHeight;	                        // optional, An optional indication of the height of the target/area
    float   AreaWidth;	                        // optional, An optional indication of the width of the target/area
} ptz_GeoMove_REQ;


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif


