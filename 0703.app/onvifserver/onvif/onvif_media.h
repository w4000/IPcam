

#ifndef ONVIF_MEDIA_H
#define ONVIF_MEDIA_H

/***************************************************************************************/
typedef struct
{ 
    char    ProfileToken[100];              // required, this command requests a specific profile
} trt_GetProfile_REQ;

typedef struct
{
	uint32	TokenFlag	: 1;						    // Indicates whether the field Token is valid
	uint32 	Reserved	: 31;
	
	char 	Name[100];					    // required, friendly name of the profile to be created
	char 	Token[100];					    // optional, Optional token, specifying the unique identifier of the new profile. A device supports at least a token length of 12 characters and characters "A-Z" | "a-z" | "0-9" | "-."
} trt_CreateProfile_REQ;

typedef struct
{
    char    ProfileToken[100];	            // required, This element contains a  reference to the profile that should be deleted
} trt_DeleteProfile_REQ;

typedef struct
{
	char 	ProfileToken[100];			    // required, Reference to the profile where the configuration should be added
	char 	ConfigurationToken[100];	    // required, Contains a reference to the VideoSourceConfiguration to add
} trt_AddVideoSourceConfiguration_REQ;

typedef struct
{
    char 	ProfileToken[100];			    // required, Contains a reference to the media profile from which the VideoSourceConfiguration shall be removed
} trt_RemoveVideoSourceConfiguration_REQ;

typedef struct
{
	char 	ProfileToken[100];			    // required, Reference to the profile where the configuration should be added
	char 	ConfigurationToken[100];	    // required, Contains a reference to the VideoSourceConfiguration to add
} trt_AddVideoEncoderConfiguration_REQ;

typedef struct
{
    char 	ProfileToken[100];			    // required, Contains a reference to the media profile from which the VideoEncoderConfiguration shall be removed
} trt_RemoveVideoEncoderConfiguration_REQ;

typedef struct
{
	char 	ProfileToken[100];			    // required, Reference to the profile where the configuration should be added
	char 	ConfigurationToken[100];	    // required, Contains a reference to the AudioSourceConfiguration to add
} trt_AddAudioSourceConfiguration_REQ;

typedef struct
{
	char 	ProfileToken[100];			    // required, Reference to the profile where the configuration should be added
	char 	ConfigurationToken[100];	    // required, Contains a reference to the AudioSourceConfiguration to add
} trt_AddAudioEncoderConfiguration_REQ;

typedef struct
{
	char 	ProfileToken[100];			    // required, Reference to the profile where the configuration should be added
	char 	ConfigurationToken[100];	    // required, Contains a reference to the PTZConfiguration to add
} trt_AddPTZConfiguration_REQ;

typedef struct
{
	char 	ProfileToken[100];			    // required, The ProfileToken element indicates the media profile to use and will define the configuration of the content of the stream

	onvif_StreamSetup	StreamSetup;				    // required, 
} trt_GetStreamUri_REQ;

typedef struct
{  
    onvif_MediaUri  MediaUri;                           // required, 
} trt_GetStreamUri_RES;

typedef struct
{
	onvif_VideoEncoderConfiguration Configuration;      // required, Contains the modified video encoder configuration. The configuration shall exist in the device
	
	BOOL  	ForcePersistence;						    // required, The ForcePersistence element is obsolete and should always be assumed to be true
} trt_SetVideoEncoderConfiguration_REQ;

typedef struct
{
	uint32	ProfileTokenFlag		: 1;			    // Indicates whether the field ProfileToken is valid
	uint32	ConfigurationTokenFlag	: 1;			    // Indicates whether the field ConfigurationToken is valid
	uint32 	Reserved				: 30;
	
	char 	ProfileToken[100];			    // Optional ProfileToken that specifies an existing media profile that the options shall be compatible with
	char 	ConfigurationToken[100];	    // Optional video source configurationToken that specifies an existing configuration that the options are intended for
} trt_GetVideoSourceConfigurationOptions_REQ;

typedef struct
{
	onvif_VideoSourceConfiguration	Configuration;      // required, Contains the modified video source configuration. The configuration shall exist in the device

	BOOL  	ForcePersistence;						    // required, The ForcePersistence element is obsolete and should always be assumed to be true
} trt_SetVideoSourceConfiguration_REQ;

typedef struct
{
	uint32	ProfileTokenFlag		: 1;			    // Indicates whether the field ProfileToken is valid
	uint32	ConfigurationTokenFlag	: 1;			    // Indicates whether the field ConfigurationToken is valid
	uint32 	Reserved				: 30;
	
	char 	ProfileToken[100];			    // Optional ProfileToken that specifies an existing media profile that the options shall be compatible with
	char 	ConfigurationToken[100];	    // Optional video encoder configuration token that specifies an existing configuration that the options are intended for
} trt_GetVideoEncoderConfigurationOptions_REQ;

typedef struct
{
    onvif_VideoEncoderConfigurationOptions Options;
} trt_GetVideoEncoderConfigurationOptions_RES;

typedef struct
{
	uint32	ProfileTokenFlag		: 1;			    // Indicates whether the field ProfileToken is valid
	uint32	ConfigurationTokenFlag	: 1;			    // Indicates whether the field ConfigurationToken is valid
	uint32 	Reserved				: 30;
	
	char 	ProfileToken[100];			    // Optional ProfileToken that specifies an existing media profile that the options shall be compatible with
	char 	ConfigurationToken[100];	    // Optional audio source configuration token that specifies an existing configuration that the options are intended for
} trt_GetAudioSourceConfigurationOptions_REQ;

typedef struct
{
	onvif_AudioSourceConfiguration	Configuration;      // required, Contains the modified audio source configuration. The configuration shall exist in the device
	
	BOOL  	ForcePersistence;						    // required, The ForcePersistence element is obsolete and should always be assumed to be true
} trt_SetAudioSourceConfiguration_REQ;

typedef struct
{
	uint32	ProfileTokenFlag		: 1;			    // Indicates whether the field ProfileToken is valid
	uint32	ConfigurationTokenFlag	: 1;			    // Indicates whether the field ConfigurationToken is valid
	uint32 	Reserved				: 30;
	
	char 	ProfileToken[100];			    // Optional ProfileToken that specifies an existing media profile that the options shall be compatible with
	char 	ConfigurationToken[100];	    // Optional audio encoder configuration token that specifies an existing configuration that the options are intended fo
} trt_GetAudioEncoderConfigurationOptions_REQ;

typedef struct
{
	onvif_AudioEncoderConfiguration	Configuration;	    // required, Contains the modified audio encoder configuration. The configuration shall exist in the device
	
	BOOL  	ForcePersistence;						    // required, The ForcePersistence element is obsolete and should always be assumed to be true
} trt_SetAudioEncoderConfiguration_REQ;

typedef struct
{
	uint32	ConfigurationTokenFlag	: 1;			    // Indicates whether the field ConfigurationToken is valid
	uint32 	Reserved				: 31;
	
	char	ConfigurationToken[100];	    // Optional, Token of the Video Source Configuration, which has OSDs associated with are requested. If token not exist, request all available OSDs
} trt_GetOSDs_REQ;

typedef struct
{
	char	OSDToken[100];				    // required, The GetOSD command fetches the OSD configuration if the OSD token is known
} trt_GetOSD_REQ;

typedef struct
{
	onvif_OSDConfiguration	OSD;					    // required, Contains the modified OSD configuration
} trt_SetOSD_REQ;

typedef struct
{
	uint32	ConfigurationTokenFlag	: 1;			    // Indicates whether the field ConfigurationToken is valid
	uint32 	Reserved				: 31;
	
	char	ConfigurationToken[100];	    // Optional, Video Source Configuration Token that specifies an existing video source configuration that the options shall be compatible with
} trt_GetOSDOptions_REQ;

typedef struct
{
	onvif_OSDConfiguration	OSD;					    // required, Contain the initial OSD configuration for create
} trt_CreateOSD_REQ;

typedef struct
{
	char	OSDToken[100];				    // required, This element contains a reference to the OSD configuration that should be deleted
} trt_DeleteOSD_REQ;

typedef struct 
{
	uint32	ConfigurationTokenFlag	: 1;			    // Indicates whether the field ConfigurationToken is valid
	uint32	ProfileTokenFlag		: 1;			    // Indicates whether the field ProfileToken is valid
	uint32 	Reserved				: 30;
	
	char 	ConfigurationToken[256];				    // optional, Contains a list of audio output configurations that are compatible with the specified media profile
	char 	ProfileToken[100];			    // optional, Contains the token of an existing media profile the configurations shall be compatible with
} trt_GetMetadataConfigurationOptions_REQ;

typedef struct
{
	onvif_MetadataConfiguration	Configuration;		    // required,  Contains the modified metadata configuration. The configuration shall exist in the device
	BOOL 	ForcePersistence;						    // required, The ForcePersistence element is obsolete and should always be assumed to be true
} trt_SetMetadataConfiguration_REQ;

typedef struct
{
	char	ProfileToken[100];			    // required,  Reference to the profile where the configuration should be added
	char 	ConfigurationToken[100];	    // required,  Contains a reference to the MetadataConfiguration to add	
} trt_AddMetadataConfiguration_REQ;

typedef struct 
{
    uint32  ConfigurationTokenFlag  : 1;                // Indicates whether the field ConfigurationToken is valid
    uint32  ProfileTokenFlag        : 1;                // Indicates whether the field ProfileToken is valid
    uint32  Reserved                : 30;
    
	char    ConfigurationToken[100];	    // optional, Optional audio output configuration token that specifies an existing configuration that the options are intended for
	char    ProfileToken[100];	            // optional, Optional ProfileToken that specifies an existing media profile that the options shall be compatible with
} trt_GetAudioOutputConfigurationOptions_REQ;

typedef struct
{
    char    ConfigurationToken[100];        // required, Token of the requested audio output configuration
} trt_GetAudioOutputConfiguration_REQ;

typedef struct 
{
	char    ProfileToken[100];	            // required, Contains the token of an existing media profile the configurations shall be compatible with
} trt_GetCompatibleAudioOutputConfigurations_REQ;

typedef struct 
{
	char    ProfileToken[100];	            // required, Reference to the profile where the configuration should be added
	char    ConfigurationToken[100];        // required, Contains a reference to the AudioOutputConfiguration to add
} trt_AddAudioOutputConfiguration_REQ;

typedef struct 
{
	char    ProfileToken[100];	            // required, Contains a reference to the media profile from which the AudioOutputConfiguration shall be removed
} trt_RemoveAudioOutputConfiguration_REQ;

typedef struct 
{
	char    ProfileToken[100];	            // required, This element contains a reference to the profile where the configuration should be added
	char    ConfigurationToken[100];        // required, This element contains a reference to the AudioDecoderConfiguration to add
} trt_AddAudioDecoderConfiguration_REQ;

typedef struct 
{
	char    ProfileToken[100];	            // required, This element contains a  reference to the media profile from which the AudioDecoderConfiguration shall be removed
} trt_RemoveAudioDecoderConfiguration_REQ;

typedef struct 
{
	onvif_AudioDecoderConfiguration Configuration;	    // required, Contains the modified audio decoder configuration. The configuration shall exist in the device
	BOOL    ForcePersistence;	                        // required, The ForcePersistence element is obsolete and should always be assumed to be true
} trt_SetAudioDecoderConfiguration_REQ;

typedef struct 
{
    uint32  ConfigurationTokenFlag  : 1;                // Indicates whether the field ConfigurationToken is valid
    uint32  ProfileTokenFlag        : 1;                // Indicates whether the field ProfileToken is valid
    uint32  Reserved                : 30;
    
	char    ConfigurationToken[100];	    // optional, Optional audio decoder configuration token that specifies an existing configuration that the options are intended for
	char    ProfileToken[100];	            // optional, Optional ProfileToken that specifies an existing media profile that the options shall be compatible with
} trt_GetAudioDecoderConfigurationOptions_REQ;

typedef struct 
{
	char    ProfileToken[100];	            // required, Contains the token of an existing media profile the configurations shall be compatible with
} trt_GetCompatibleAudioDecoderConfigurations_REQ;

typedef struct 
{
    char    VideoSourceToken[100];	        // required
} trt_GetVideoSourceModes_REQ;

typedef struct
{
    char    VideoSourceToken[100];	        // required
	char    VideoSourceModeToken[100];	    // required
} trt_SetVideoSourceMode_REQ;

typedef struct
{
    BOOL    Reboot;                                     // required, The response contains information about rebooting after returning response. 
                                                        //  When Reboot is set true, a device will reboot automatically after setting mode
} trt_SetVideoSourceMode_RES;

typedef struct
{
    char    ProfileToken[100];              // required, 
} trt_GetSnapshotUri_REQ;

typedef struct
{
    onvif_MediaUri  MediaUri;                           // required, 
} trt_GetSnapshotUri_RES;

typedef struct
{
    char    ProfileToken[100];              // required, 
} trt_SetSynchronizationPoint_REQ;

typedef struct 
{
	char 	ProfileToken[100];		        // required, Reference to the profile where the configuration should be added
	char 	ConfigurationToken[100];        // required, Contains a reference to the VideoAnalyticsConfiguration to add
} trt_AddVideoAnalyticsConfiguration_REQ;

typedef struct 
{
	char 	ConfigurationToken[100];        // required, The requested video analytics configuration
} trt_GetVideoAnalyticsConfiguration_REQ;

typedef struct 
{
	char 	ProfileToken[100];			    // required, Contains a reference to the media profile from which the VideoAnalyticsConfiguration shall be removed
} trt_RemoveVideoAnalyticsConfiguration_REQ;

typedef struct
{
	onvif_VideoAnalyticsConfiguration	Configuration;  // required, Contains the modified video analytics configuration. The configuration shall exist in the device

	BOOL 	ForcePersistence;						    // required, The ForcePersistence element is obsolete and should always be assumed to be true
} trt_SetVideoAnalyticsConfiguration_REQ;

typedef struct 
{
	char 	ProfileToken[100];			    // required, Contains the token of an existing media profile the configurations shall be compatible with
} trt_GetCompatibleVideoAnalyticsConfigurations_REQ;


#ifdef __cplusplus
extern "C" {
#endif

ONVIF_RET trt_CreateProfile(const char * lip, uint32 lport, trt_CreateProfile_REQ * p_req);
ONVIF_RET trt_DeleteProfile(trt_DeleteProfile_REQ * p_req);
ONVIF_RET trt_AddVideoSourceConfiguration(trt_AddVideoSourceConfiguration_REQ * p_req);
ONVIF_RET trt_AddVideoEncoderConfiguration(trt_AddVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET trt_GetVideoEncoderConfigurationOptions(trt_GetVideoEncoderConfigurationOptions_REQ * p_req, trt_GetVideoEncoderConfigurationOptions_RES * p_res);
ONVIF_RET trt_GetStreamUri(const char * lip, uint32 lport, trt_GetStreamUri_REQ * p_req, trt_GetStreamUri_RES * p_res);
ONVIF_RET trt_RemoveVideoEncoderConfiguration(trt_RemoveVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET trt_RemoveVideoSourceConfiguration(trt_RemoveVideoSourceConfiguration_REQ * p_req);
ONVIF_RET trt_SetVideoEncoderConfiguration(trt_SetVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET trt_SetVideoSourceConfiguration(trt_SetVideoSourceConfiguration_REQ * p_req);
ONVIF_RET trt_SetVideoSourceMode(trt_SetVideoSourceMode_REQ * p_req, trt_SetVideoSourceMode_RES * p_res);
ONVIF_RET trt_GetSnapshotUri(const char * lip, uint32 lport, trt_GetSnapshotUri_REQ * p_req, trt_GetSnapshotUri_RES * p_res);
ONVIF_RET trt_SetSynchronizationPoint(trt_SetSynchronizationPoint_REQ * p_req);
ONVIF_RET trt_GetSnapshot(char * buff, int * rlen, char * profile_token);
ONVIF_RET trt_SetOSD(trt_SetOSD_REQ * p_req);
ONVIF_RET trt_CreateOSD(trt_CreateOSD_REQ * p_req);
ONVIF_RET trt_DeleteOSD(trt_DeleteOSD_REQ * p_req);
ONVIF_RET trt_StartMulticastStreaming(const char * token);
ONVIF_RET trt_StopMulticastStreaming(const char * token);
ONVIF_RET trt_SetMetadataConfiguration(trt_SetMetadataConfiguration_REQ * p_req);
ONVIF_RET trt_AddMetadataConfiguration(trt_AddMetadataConfiguration_REQ * p_req);
ONVIF_RET trt_RemoveMetadataConfiguration(const char * profile_token);

#ifdef VIDEO_ANALYTICS
ONVIF_RET trt_AddVideoAnalyticsConfiguration(trt_AddVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET trt_RemoveVideoAnalyticsConfiguration(trt_RemoveVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET trt_SetVideoAnalyticsConfiguration(trt_SetVideoAnalyticsConfiguration_REQ * p_req);
#endif


#ifdef DEVICEIO_SUPPORT
ONVIF_RET trt_AddAudioOutputConfiguration(trt_AddAudioOutputConfiguration_REQ * p_req);
ONVIF_RET trt_RemoveAudioOutputConfiguration(trt_RemoveAudioOutputConfiguration_REQ * p_req);
#endif

#ifdef __cplusplus
}
#endif


#endif


