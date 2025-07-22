

#ifndef __ONVIF_PKT_H__
#define __ONVIF_PKT_H__

#include "onvif.h"

#ifdef __cplusplus
extern "C" {
#endif

int bderr_rly_xml(char * p_buf, int mlen, const char * code, const char * subcode, const char * subcode_ex, const char * reason, const char * action);

int bdDeviceCapabilities_xml(char * p_buf, int mlen);
int bdDeviceServicesCapabilities_xml(char * p_buf, int mlen);
int bdEventsCapabilities_xml(char * p_buf, int mlen);
int bdEventsServicesCapabilities_xml(char * p_buf, int mlen);

#ifdef MEDIA_SUPPORT
int bdMediaCapabilities_xml(char * p_buf, int mlen);
int bdMediaServicesCapabilities_xml(char * p_buf, int mlen);
#endif // MEDIA_SUPPORT

#ifdef IMAGE_SUPPORT
int bdImagingCapabilities_xml(char * p_buf, int mlen);
int bdImagingServicesCapabilities_xml(char * p_buf, int mlen);
#endif // IMAGE_SUPPORT

#ifdef VIDEO_ANALYTICS
int bdAnalyticsCapabilities_xml(char * p_buf, int mlen);
int bdAnalyticsServicesCapabilities_xml(char * p_buf, int mlen);
#endif // VIDEO_ANALYTICS

#ifdef PROFILE_G_SUPPORT
int bdRecordingCapabilities_xml(char * p_buf, int mlen);
int bdRecordingServicesCapabilities_xml(char * p_buf, int mlen);
int bdSearchCapabilities_xml(char * p_buf, int mlen);
int bdSearchServicesCapabilities_xml(char * p_buf, int mlen);
int bdReplayCapabilities_xml(char * p_buf, int mlen);
int bdReplayServicesCapabilities_xml(char * p_buf, int mlen);
#endif // PROFILE_G_SUPPORT


#ifdef RECEIVER_SUPPORT
int bdReceiverCapabilities_xml(char * p_buf, int mlen);
int bdReceiverServicesCapabilities_xml(char * p_buf, int mlen);
#endif // RECEIVER_SUPPORT

#ifdef MEDIA2_SUPPORT
int bdMedia2ServicesCapabilities_xml(char * p_buf, int mlen);
#endif // MEDIA2_SUPPORT  

#ifdef PROFILE_C_SUPPORT
int bdAccessControlServicesCapabilities_xml(char * p_buf, int mlen);
int bdDoorControlServicesCapabilities_xml(char * p_buf, int mlen);
#endif // PROFILE_C_SUPPORT

#ifdef THERMAL_SUPPORT
int bdThermalServicesCapabilities_xml(char * p_buf, int mlen);
#endif // THERMAL_SUPPORT

#ifdef CREDENTIAL_SUPPORT
int bdCredentialServicesCapabilities_xml(char * p_buf, int mlen);
#endif // CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES
int bdAccessRulesServicesCapabilities_xml(char * p_buf, int mlen);
#endif // ACCESS_RULES

#ifdef SCHEDULE_SUPPORT
int bdScheduleServicesCapabilities_xml(char * p_buf, int mlen);
#endif // SCHEDULE_SUPPORT

#ifdef PROVISIONING_SUPPORT
int bdSourceCapabilities_xml(char * p_buf, int mlen, onvif_SourceCapabilities * p_req);
int bdProvisioningServicesCapabilities_xml(char * p_buf, int mlen);
#endif // PROVISIONING_SUPPORT

int bdVersion_xml(char * p_buf, int mlen, int major, int minor);
int bdFloatRange_xml(char * p_buf, int mlen, onvif_FloatRange * p_req);
int bdIntList_xml(char * p_buf, int mlen, onvif_IntList * p_req);
int bdFloatList_xml(char * p_buf, int mlen, onvif_FloatList * p_req);
int bdVideoResolution_xml(char * p_buf, int mlen, onvif_VideoResolution * p_req);
int bdGetServiceCapabilities_rly_xml(char * p_buf, int mlen, const char * argv);
int bdDot11Configuration_xml(char * p_buf, int mlen, onvif_Dot11Configuration * p_req);
int bdDot11AvailableNetworks_xml(char * p_buf, int mlen, onvif_Dot11AvailableNetworks * p_req);
int bdNetworkInterface_xml(char * p_buf, int mlen, onvif_NetworkInterface * p_req);
int bdtds_GetDeviceInformation_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetSystemUris_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetCapabilities_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetNetworkInterfaces_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetNetworkInterfaces_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SystemReboot_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetSystemFactoryDefault_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetSystemLog_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetSystemDateAndTime_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetSystemDateAndTime_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetServices_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetScopes_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_AddScopes_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetScopes_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_RemoveScopes_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetHostname_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetHostname_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetHostnameFromDHCP_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetNetworkProtocols_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetNetworkProtocols_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetNetworkDefaultGateway_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetNetworkDefaultGateway_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetDiscoveryMode_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetDiscoveryMode_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetDNS_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetNTP_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetNTP_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetZeroConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetZeroConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetDot11Capabilities_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetDot11Status_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_ScanAvailableDot11Networks_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetUsers_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_CreateUsers_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_DeleteUsers_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetUser_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetRemoteUser_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetRemoteUser_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_StartSystemRestore_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetWsdlUrl_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_GetEndpointReference_rly_xml(char * p_buf, int mlen, const char * argv);

#ifdef DEVICEIO_SUPPORT
int bdtds_SetRelayOutputSettings_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtds_SetRelayOutputState_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // DEVICEIO_SUPPORT


#ifdef PROFILE_Q_SUPPORT
int bdBase_EventProperties_xml(char * p_buf, int mlen);
#endif // PROFILE_Q_SUPPORT

int bdImaging_EventProperties_xml(char * p_buf, int mlen);

#ifdef VIDEO_ANALYTICS
int bdAnalytics_EventProperties_xml(char * p_buf, int mlen);
#endif

#ifdef DEVICEIO_SUPPORT
int bdDeviceIO_EventProperties_xml(char * p_buf, int mlen);
#endif // DEVICEIO_SUPPORT

#ifdef PROFILE_G_SUPPORT
int bdProfileG_EventProperties_xml(char * p_buf, int mlen);
#endif // end of PROFILE_G_SUPPORT

#ifdef PROFILE_C_SUPPORT
int bdProfileC_EventProperties_xml(char * p_buf, int mlen);
#endif // PROFILE_C_SUPPORT

#ifdef CREDENTIAL_SUPPORT
int bdCredential_EventProperties_xml(char * p_buf, int mlen);
#endif // CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES
int bdAccessRules_EventProperties_xml(char * p_buf, int mlen);
#endif // ACCESS_RULES

#ifdef SCHEDULE_SUPPORT
int bdSchedule_EventProperties_xml(char * p_buf, int mlen);
#endif // SCHEDULE_SUPPORT

#ifdef RECEIVER_SUPPORT
int bdReceiver_EventProperties_xml(char * p_buf, int mlen);
#endif // RECEIVER_SUPPORT

int bdSimpleItem_xml(char * p_buf, int mlen, onvif_SimpleItem * p_req);
int bdElementItem_xml(char * p_buf, int mlen, onvif_ElementItem * p_req);
int bdNotificationMessage_xml(char * p_buf, int mlen, onvif_NotificationMessage * p_req);
int bdNotify_xml(char * p_buf, int mlen, const char * argv);
int bdtev_GetEventProperties_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtev_Subscribe_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtev_Unsubscribe_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtev_Renew_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtev_CreatePullPointSubscription_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtev_PullMessages_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtev_SetSynchronizationPoint_rly_xml(char * p_buf, int mlen, const char * argv);

#ifdef IMAGE_SUPPORT
int bdImageSettings_xml(char * p_buf, int mlen);
int bdimg_GetImagingSettings_rly_xml(char * p_buf, int mlen, const char * argv);
int bdimg_GetOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdimg_SetImagingSettings_rly_xml(char * p_buf, int mlen, const char * argv);
int bdimg_GetMoveOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdimg_Move_rly_xml(char * p_buf, int mlen, const char * argv);
int bdimg_GetStatus_rly_xml(char * p_buf, int mlen, const char * argv);
int bdimg_Stop_rly_xml(char * p_buf, int mlen, const char * argv);
int bdimg_GetPresets_rly_xml(char * p_buf, int mlen, const char * argv);
int bdimg_GetCurrentPreset_rly_xml(char * p_buf, int mlen, const char * argv);
int bdimg_SetCurrentPreset_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // IMAGE_SUPPORT

#if defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)
int bdMulticastConfiguration_xml(char * p_buf, int mlen, onvif_MulticastConfiguration * p_cfg);
int bdOSD_xml(char * p_buf, int mlen, OSDConfigurationList * p_osd);
int bdVideoSourceConfiguration_xml(char * p_buf, int mlen, onvif_VideoSourceConfiguration * p_req);
int bdVideoSourceConfigurationOptions_xml(char * p_buf, int mlen, onvif_VideoSourceConfigurationOptions * p_req);
int bdMetadataConfiguration_xml(char * p_buf, int mlen, onvif_MetadataConfiguration * p_req);
int bdMetadataConfigurationOptions_xml(char * p_buf, int mlen, onvif_MetadataConfigurationOptions * p_req);


#endif // defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)

#ifdef MEDIA_SUPPORT
int bdVideoEncoderConfiguration_xml(char * p_buf, int mlen, onvif_VideoEncoder2Configuration * p_v_enc_cfg);
int bdProfile_xml(char * p_buf, int mlen, ONVIF_PROFILE * p_profile);
int bdH264Options_xml(char * p_buf, int mlen, onvif_H264Options * p_options);
int bdBitrateRange_xml(char * p_buf, int mlen, onvif_IntRange * p_req);
int bdtrt_GetProfiles_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetProfile_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_CreateProfile_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_DeleteProfile_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_AddVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_RemoveVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_AddVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_RemoveVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetStreamUri_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetVideoSources_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * token);
int bdtrt_GetVideoEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetCompatibleVideoEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetVideoSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * token);
int bdtrt_SetVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetVideoSourceConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetCompatibleVideoSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * token);
int bdtrt_GetVideoEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_SetVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_SetSynchronizationPoint_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetGuaranteedNumberOfVideoEncoderInstances_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetOSDs_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetOSD_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_SetOSD_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_CreateOSD_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_DeleteOSD_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetOSDOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_StartMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_StopMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetMetadataConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetCompatibleMetadataConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetMetadataConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_SetMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_AddMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_RemoveMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetVideoSourceModes_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_SetVideoSourceMode_rly_xml(char * p_buf, int mlen, const char * argv);


int bdtrt_GetAudioEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetAudioSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetCompatibleAudioSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetAudioSourceConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetAudioSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * token);

#ifdef DEVICEIO_SUPPORT
int bdtrt_GetAudioOutputs_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_AddAudioOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_RemoveAudioOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetAudioOutputConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetCompatibleAudioOutputConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetAudioOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetAudioOutputConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_SetAudioOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // DEVICEIO_SUPPORT


#ifdef VIDEO_ANALYTICS
int bdtrt_GetVideoAnalyticsConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_AddVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_RemoveVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_SetVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetAnalyticsConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrt_GetCompatibleVideoAnalyticsConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // VIDEO_ANALYTICS

#endif // MEDIA_SUPPORT

#ifdef VIDEO_ANALYTICS
int bdConfig_xml(char * p_buf, int mlen, onvif_Config * p_req);
int bdVideoAnalyticsConfiguration_xml(char * p_buf, int mlen, onvif_VideoAnalyticsConfiguration * p_req);
int bdItemListDescription_xml(char * p_buf, int mlen, onvif_ItemListDescription * p_req);
int bdConfigDescription_Messages_xml(char * p_buf, int mlen, onvif_ConfigDescription_Messages * p_req);
int bdtan_GetSupportedRules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_CreateRules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_DeleteRules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_GetRules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_ModifyRules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_CreateAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_DeleteAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_GetAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_ModifyAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_GetRuleOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_GetSupportedAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtan_GetAnalyticsModuleOptions_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // VIDEO_ANALYTICS

#ifdef PROFILE_G_SUPPORT
int bdRecordingSourceInformation_xml(char * p_buf, int mlen, onvif_RecordingSourceInformation * p_req);
int bdTrackInformation_xml(char * p_buf, int mlen, onvif_TrackInformation * p_req);
int bdRecordingInformation_xml(char * p_buf, int mlen, onvif_RecordingInformation * p_req);
int bdTrackAttributes_xml(char * p_buf, int mlen, onvif_TrackAttributes * p_req);
int bdMediaAttributes_xml(char * p_buf, int mlen, onvif_MediaAttributes * p_req);
int bdFindEventResult_xml(char * p_buf, int mlen, onvif_FindEventResult * p_req);
int bdtse_GetRecordingInformation_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_GetRecordingSummary_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_GetMediaAttributes_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_FindRecordings_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_GetRecordingSearchResults_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_FindEvents_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_GetEventSearchResults_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_FindMetadata_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_FindMetadataResult_xml(char * p_buf, int mlen, onvif_FindMetadataResult * p_res);
int bdtse_GetMetadataSearchResults_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_FindPTZPosition_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_GetPTZPositionSearchResults_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_EndSearch_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtse_GetSearchState_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_CreateRecording_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_DeleteRecording_rly_xml(char * p_buf, int mlen, const char * argv);
int bdRecordingConfiguration_xml(char * p_buf, int mlen, onvif_RecordingConfiguration * p_req);
int bdTrackConfiguration_xml(char * p_buf, int mlen, onvif_TrackConfiguration * p_req);
int bdRecordingJobConfiguration_xml(char * p_buf, int mlen, onvif_RecordingJobConfiguration * p_req);
int bdRecordingJobStateInformation_xml(char * p_buf, int mlen, onvif_RecordingJobStateInformation * p_res);
int bdtrc_GetRecordings_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_SetRecordingConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_GetRecordingConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_CreateTrack_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_DeleteTrack_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_GetTrackConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_SetTrackConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_CreateRecordingJob_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_DeleteRecordingJob_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_GetRecordingJobs_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_SetRecordingJobConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_GetRecordingJobConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_SetRecordingJobMode_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_GetRecordingJobState_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrc_GetRecordingOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrp_GetReplayUri_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrp_GetReplayConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrp_SetReplayConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // PROFILE_G_SUPPORT

#ifdef PROFILE_C_SUPPORT
int bdAccessPointInfo_xml(char * p_buf, int mlen, onvif_AccessPointInfo * p_res);
int bdDoorInfo_xml(char * p_buf, int mlen, onvif_DoorInfo * p_res);
int bdAreaInfo_xml(char * p_buf, int mlen, onvif_AreaInfo * p_res);
int bdtac_GetAccessPointList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_GetAccessPoints_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_CreateAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_SetAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_ModifyAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_DeleteAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_GetAccessPointInfoList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_GetAccessPointInfo_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_GetAreaList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_GetAreas_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_CreateArea_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_SetArea_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_ModifyArea_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_DeleteArea_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_GetAreaInfoList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_GetAreaInfo_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_GetAccessPointState_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_EnableAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtac_DisableAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_GetDoorList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_GetDoors_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_CreateDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_SetDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_ModifyDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_DeleteDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_GetDoorInfoList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_GetDoorInfo_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_GetDoorState_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_AccessDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_LockDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_UnlockDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_DoubleLockDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_BlockDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_LockDownDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_LockDownReleaseDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_LockOpenDoor_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtdc_LockOpenReleaseDoor_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // PROFILE_C_SUPPORT

#ifdef DEVICEIO_SUPPORT
int bdPaneLayout_xml(char * p_buf, int mlen, onvif_PaneLayout * p_PaneLayout);
int bdLayout_xml(char * p_buf, int mlen, onvif_Layout * p_Layout);
int bdVideoOutput_xml(char * p_buf, int mlen, onvif_VideoOutput * p_VideoOutput);
int bdAudioOutputConfiguration_xml(char * p_buf, int mlen, onvif_AudioOutputConfiguration * p_req);
int bdAudioOutputConfigurationOptions_xml(char * p_buf, int mlen, onvif_AudioOutputConfigurationOptions * p_req);
int bdParityBitList_xml(char * p_buf, int mlen, onvif_ParityBitList * p_req);
int bdtmd_GetVideoOutputs_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_GetVideoSources_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_GetVideoOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_SetVideoOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_GetVideoOutputConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_GetAudioOutputs_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_GetDigitalInputs_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_GetDigitalInputConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_SetDigitalInputConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_GetSerialPorts_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_GetSerialPortConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_GetSerialPortConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_SetSerialPortConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtmd_SendReceiveSerialCommand_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // DEVICEIO_SUPPORT

#ifdef MEDIA2_SUPPORT
int bdColorspaceRange_xml(char * p_buf, int mlen, onvif_ColorspaceRange * p_req);
int bdColorOptions_xml(char * p_buf, int mlen, onvif_ColorOptions * p_req);
int bdOSDColorOptions_xml(char * p_buf, int mlen, onvif_OSDColorOptions * p_req);
int bdOSDTextOptions_xml(char * p_buf, int mlen, onvif_OSDTextOptions * p_req);
int bdOSDImgOptions_xml(char * p_buf, int mlen, onvif_OSDImgOptions * p_req);
int bdPolygon_xml(char * p_buf, int mlen, onvif_Polygon * p_req);
int bdColor_xml(char * p_buf, int mlen, onvif_Color * p_req);
int bdMask_xml(char * p_buf, int mlen, onvif_Mask * p_req);
int bdVideoEncoder2Configuration_xml(char * p_buf, int mlen, onvif_VideoEncoder2Configuration * p_req);
int bdVideoEncoder2ConfigurationOptions_xml(char * p_buf, int mlen, onvif_VideoEncoder2ConfigurationOptions * p_req);
int bdAudioEncoder2Configuration_xml(char * p_buf, int mlen, onvif_AudioEncoder2Configuration * p_req);
int bdAudioEncoder2ConfigurationOptions_xml(char * p_buf, int mlen, onvif_AudioEncoder2ConfigurationOptions * p_req);
int bdtr2_GetVideoEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetVideoEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_SetVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_CreateProfile_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetProfiles_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_DeleteProfile_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_AddConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_RemoveConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetVideoSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetMetadataConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_SetVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_SetMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_SetAudioSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetVideoSourceConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetMetadataConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetVideoEncoderInstances_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetStreamUri_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_SetSynchronizationPoint_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetVideoSourceModes_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_SetVideoSourceMode_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetSnapshotUri_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_SetOSD_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetOSDOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetOSDs_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_CreateOSD_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_DeleteOSD_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_CreateMask_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_DeleteMask_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetMasks_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_SetMask_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetMaskOptions_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_StartMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_StopMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetAudioSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtr2_GetAudioEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv);

#ifdef VIDEO_ANALYTICS
int bdtr2_GetAnalyticsConfigurations_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // VIDEO_ANALYTICS

#endif // MEDIA2_SUPPORT


#ifdef CREDENTIAL_SUPPORT
int bdCredentialInfo_xml(char * p_buf, int mlen, onvif_CredentialInfo * p_res);
int bdCredentialIdentifierType_xml(char * p_buf, int mlen, onvif_CredentialIdentifierType * p_res);
int bdCredentialIdentifier_xml(char * p_buf, int mlen, onvif_CredentialIdentifier * p_res);
int bdCredentialAccessProfile_xml(char * p_buf, int mlen, onvif_CredentialAccessProfile * p_res);
int bdCredential_xml(char * p_buf, int mlen, onvif_Credential * p_res);
int bdCredentialIdentifierFormatTypeInfo_xml(char * p_buf, int mlen, onvif_CredentialIdentifierFormatTypeInfo * p_res);
int bdCredentialState_xml(char * p_buf, int mlen, onvif_CredentialState * p_res);
int bdtcr_GetCredentialInfo_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_GetCredentialInfoList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_GetCredentials_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_GetCredentialList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_CreateCredential_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_ModifyCredential_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_DeleteCredential_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_GetCredentialState_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_EnableCredential_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_DisableCredential_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_SetCredential_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_ResetAntipassbackViolation_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_GetSupportedFormatTypes_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_GetCredentialIdentifiers_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_SetCredentialIdentifier_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_DeleteCredentialIdentifier_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_GetCredentialAccessProfiles_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_SetCredentialAccessProfiles_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_DeleteCredentialAccessProfiles_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_GetWhitelist_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_AddToWhitelist_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_RemoveFromWhitelist_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_DeleteWhitelist_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_GetBlacklist_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_AddToBlacklist_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_RemoveFromBlacklist_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtcr_DeleteBlacklist_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES
int bdAccessProfileInfo_xml(char * p_buf, int mlen, onvif_AccessProfileInfo * p_res);
int bdAccessPolicy_xml(char * p_buf, int mlen, onvif_AccessPolicy * p_res);
int bdAccessProfile_xml(char * p_buf, int mlen, onvif_AccessProfile * p_res);
int bdtar_GetAccessProfileInfo_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtar_GetAccessProfileInfoList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtar_GetAccessProfiles_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtar_GetAccessProfileList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtar_CreateAccessProfile_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtar_ModifyAccessProfile_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtar_DeleteAccessProfile_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtar_SetAccessProfile_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // ACCESS_RULES

#ifdef SCHEDULE_SUPPORT
int bdScheduleInfo_xml(char * p_buf, int mlen, onvif_ScheduleInfo * p_res);
int bdTimePeriod_xml(char * p_buf, int mlen, onvif_TimePeriod * p_res);
int bdSpecialDaysSchedule_xml(char * p_buf, int mlen, onvif_SpecialDaysSchedule * p_res);
int bdSchedule_xml(char * p_buf, int mlen, onvif_Schedule * p_res);
int bdSpecialDayGroupInfo_xml(char * p_buf, int mlen, onvif_SpecialDayGroupInfo * p_res);
int bdSpecialDayGroup_xml(char * p_buf, int mlen, onvif_SpecialDayGroup * p_res);
int bdtsc_GetScheduleInfo_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_GetScheduleInfoList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_GetSchedules_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_GetScheduleList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_CreateSchedule_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_ModifySchedule_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_DeleteSchedule_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_GetSpecialDayGroupInfo_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_GetSpecialDayGroupInfoList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_GetSpecialDayGroups_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_GetSpecialDayGroupList_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_CreateSpecialDayGroup_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_ModifySpecialDayGroup_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_DeleteSpecialDayGroup_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_GetScheduleState_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_SetSchedule_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtsc_SetSpecialDayGroup_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // CHEDULE_SUPPORT

#ifdef RECEIVER_SUPPORT
int bdStreamSetup_xml(char * p_buf, int mlen, onvif_StreamSetup * p_res);
int bdtrv_ReceiverConfiguration_xml(char * p_buf, int mlen, onvif_ReceiverConfiguration * p_res);
int bdtrv_Receiver_xml(char * p_buf, int mlen, onvif_Receiver * p_res);
int bdtrv_GetReceivers_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrv_GetReceiver_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrv_CreateReceiver_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrv_DeleteReceiver_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrv_ConfigureReceiver_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrv_SetReceiverMode_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtrv_GetReceiverState_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // RECEIVER_SUPPORT

#ifdef PROVISIONING_SUPPORT
int bdtpv_PanMove_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtpv_TiltMove_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtpv_ZoomMove_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtpv_RollMove_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtpv_FocusMove_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtpv_Stop_rly_xml(char * p_buf, int mlen, const char * argv);
int bdtpv_GetUsage_rly_xml(char * p_buf, int mlen, const char * argv);
#endif // PROVISIONING_SUPPORT


#ifdef __cplusplus
}
#endif

#endif 


