

#ifndef _SOAP_PARSER_H_
#define _SOAP_PARSER_H_

/***************************************************************************************/
#include "xml_node.h"
#include "onvif.h"
#include "onvif_ptz.h"
#include "onvif_device.h"
#include "onvif_media.h"
#include "onvif_event.h"
#include "onvif_image.h"
#ifdef VIDEO_ANALYTICS
#include "onvif_analytics.h"
#endif
#ifdef PROFILE_G_SUPPORT
#include "onvif_recording.h"
#endif
#ifdef PROFILE_C_SUPPORT
#include "onvif_doorcontrol.h"
#endif
#ifdef DEVICEIO_SUPPORT
#include "onvif_deviceio.h"
#endif
#ifdef MEDIA2_SUPPORT
#include "onvif_media2.h"
#endif
#ifdef THERMAL_SUPPORT
#include "onvif_thermal.h"
#endif
#ifdef CREDENTIAL_SUPPORT
#include "onvif_credential.h"
#endif
#ifdef ACCESS_RULES
#include "onvif_accessrules.h"
#endif
#ifdef SCHEDULE_SUPPORT
#include "onvif_schedule.h"
#endif
#ifdef RECEIVER_SUPPORT
#include "onvif_receiver.h"
#endif
#ifdef PROVISIONING_SUPPORT
#include "onvif_provisioning.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


ONVIF_RET _StreamSetup(XMLN * p_node, onvif_StreamSetup * p_req);
ONVIF_RET _tds_GetCapabilities(XMLN * p_node, tds_GetCapabilities_REQ * p_req);
ONVIF_RET _tds_GetServices(XMLN * p_node, tds_GetServices_REQ * p_req);
#if 0
ONVIF_RET _tds_SetSystemDateAndTime(XMLN * p_node, tds_SetSystemDateAndTime_REQ * p_req);
#else
ONVIF_RET _tds_SetSystemDateAndTime(XMLN * p_node, tds_SetSystemDateAndTime_REQ * p_req, manager_tz_dt *p_dt);
#endif
ONVIF_RET _tds_AddScopes(XMLN * p_node, tds_AddScopes_REQ * p_req);
ONVIF_RET _tds_SetScopes(XMLN * p_node, tds_SetScopes_REQ * p_req);
ONVIF_RET _tds_RemoveScopes(XMLN * p_node, tds_RemoveScopes_REQ * p_req);
ONVIF_RET _tds_SetHostname(XMLN * p_node, tds_SetHostname_REQ * p_req);
ONVIF_RET _tds_SetHostnameFromDHCP(XMLN * p_node, tds_SetHostnameFromDHCP_REQ * p_req);
ONVIF_RET _tds_SetDiscoveryMode(XMLN * p_node, tds_SetDiscoveryMode_REQ * p_req);
ONVIF_RET _tds_SetDNS(XMLN * p_node, tds_SetDNS_REQ * p_req);
ONVIF_RET _tds_SetDynamicDNS(XMLN * p_node, tds_SetDynamicDNS_REQ * p_req);
ONVIF_RET _tds_SetNTP(XMLN * p_node, tds_SetNTP_REQ * p_req);
ONVIF_RET _tds_SetZeroConfiguration(XMLN * p_node, tds_SetZeroConfiguration_REQ * p_req);
ONVIF_RET _tds_GetDot11Status(XMLN * p_node, tds_GetDot11Status_REQ * p_req);
ONVIF_RET _tds_ScanAvailableDot11Networks(XMLN * p_node, tds_ScanAvailableDot11Networks_REQ * p_req);
ONVIF_RET _tds_SetNetworkProtocols(XMLN * p_node, tds_SetNetworkProtocols_REQ * p_req);
ONVIF_RET _tds_SetNetworkDefaultGateway(XMLN * p_node, tds_SetNetworkDefaultGateway_REQ * p_req);
ONVIF_RET _tds_SetNetworkInterfaces(XMLN * p_node, tds_SetNetworkInterfaces_REQ * p_req);
ONVIF_RET _tds_SetSystemFactoryDefault(XMLN * p_node, tds_SetSystemFactoryDefault_REQ * p_req);
ONVIF_RET _tds_CreateUsers(XMLN * p_node, tds_CreateUsers_REQ * p_req);
ONVIF_RET _tds_DeleteUsers(XMLN * p_node, tds_DeleteUsers_REQ * p_req);
ONVIF_RET _tds_SetUser(XMLN * p_node, tds_SetUser_REQ * p_req);
ONVIF_RET _tds_SetRemoteUser(XMLN * p_node, tds_SetRemoteUser_REQ * p_req);

#ifdef DEVICEIO_SUPPORT
ONVIF_RET _tds_SetRelayOutputSettings(XMLN * p_node, tmd_SetRelayOutputSettings_REQ * p_req);
#endif // DEVICEIO_SUPPORT

ONVIF_RET _Filter(XMLN * p_node, ONVIF_FILTER * p_req);
ONVIF_RET _tev_Subscribe(XMLN * p_node, tev_Subscribe_REQ * p_req);
ONVIF_RET _tev_Renew(XMLN * p_node, tev_Renew_REQ * p_req);
ONVIF_RET _tev_CreatePullPointSubscription(XMLN * p_node, tev_CreatePullPointSubscription_REQ * p_req);
ONVIF_RET _tev_PullMessages(XMLN * p_node, tev_PullMessages_REQ * p_req);

#ifdef IMAGE_SUPPORT
ONVIF_RET _img_GetImagingSettings(XMLN * p_node, img_GetImagingSettings_REQ * p_req);
ONVIF_RET _img_SetImagingSettings(XMLN * p_node, img_SetImagingSettings_REQ * p_req);
ONVIF_RET _img_GetOptions(XMLN * p_node, img_GetOptions_REQ * p_req);
ONVIF_RET _img_GetMoveOptions(XMLN * p_node, img_GetMoveOptions_REQ * p_req);
ONVIF_RET _img_GetStatus(XMLN * p_node, img_GetStatus_REQ * p_req);
ONVIF_RET _img_Stop(XMLN * p_node, img_Stop_REQ * p_req);
ONVIF_RET _img_GetPresets(XMLN * p_node, img_GetPresets_REQ * p_req);
ONVIF_RET _img_GetCurrentPreset(XMLN * p_node, img_GetCurrentPreset_REQ * p_req);
ONVIF_RET _img_SetCurrentPreset(XMLN * p_node, img_SetCurrentPreset_REQ * p_req);
#endif // IMAGE_SUPPORT

#if defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)
ONVIF_RET _MulticastConfiguration(XMLN * p_node, onvif_MulticastConfiguration * p_req);
ONVIF_RET _OSDColor(XMLN * p_node, onvif_OSDColor * p_req);
ONVIF_RET _OSDConfiguration(XMLN * p_node, onvif_OSDConfiguration * p_req);
ONVIF_RET _VideoSourceConfiguration(XMLN * p_node, onvif_VideoSourceConfiguration * p_req);
ONVIF_RET _MetadataConfiguration(XMLN * p_node, onvif_MetadataConfiguration * p_req);
ONVIF_RET _trt_SetOSD(XMLN * p_node, trt_SetOSD_REQ * p_req);
ONVIF_RET _trt_CreateOSD(XMLN * p_node, trt_CreateOSD_REQ * p_req);
ONVIF_RET _trt_DeleteOSD(XMLN * p_node, trt_DeleteOSD_REQ * p_req);

#ifdef DEVICEIO_SUPPORT
ONVIF_RET _AudioOutputConfiguration(XMLN * p_node, onvif_AudioOutputConfiguration * p_req);
#endif // DEVICEIO_SUPPORT

#endif // defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)
ONVIF_RET _trt_GetAudioEncoderConfigurationOptions(XMLN * p_node, trt_GetAudioEncoderConfigurationOptions_REQ * p_req);
#ifdef MEDIA_SUPPORT
ONVIF_RET _trt_SetVideoEncoderConfiguration(XMLN * p_node, trt_SetVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET _trt_SetSynchronizationPoint(XMLN * p_node, trt_SetSynchronizationPoint_REQ * p_req);
ONVIF_RET _trt_GetProfile(XMLN * p_node, trt_GetProfile_REQ * p_req);
ONVIF_RET _trt_CreateProfile(XMLN * p_node, trt_CreateProfile_REQ * p_req);
ONVIF_RET _trt_DeleteProfile(XMLN * p_node, trt_DeleteProfile_REQ * p_req);
ONVIF_RET _trt_AddVideoSourceConfiguration(XMLN * p_node, trt_AddVideoSourceConfiguration_REQ * p_req);
ONVIF_RET _trt_RemoveVideoSourceConfiguration(XMLN * p_node, trt_RemoveVideoSourceConfiguration_REQ * p_req);
ONVIF_RET _trt_AddVideoEncoderConfiguration(XMLN * p_node, trt_AddVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET _trt_RemoveVideoEncoderConfiguration(XMLN * p_node, trt_RemoveVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET _trt_GetStreamUri(XMLN * p_node, trt_GetStreamUri_REQ * p_req);
ONVIF_RET _trt_GetSnapshotUri(XMLN * p_node, trt_GetSnapshotUri_REQ * p_req);
ONVIF_RET _trt_GetVideoSourceConfigurationOptions(XMLN * p_node, trt_GetVideoSourceConfigurationOptions_REQ * p_req);
ONVIF_RET _trt_SetVideoSourceConfiguration(XMLN * p_node, trt_SetVideoSourceConfiguration_REQ * p_req);
ONVIF_RET _trt_GetVideoEncoderConfigurationOptions(XMLN * p_node, trt_GetVideoEncoderConfigurationOptions_REQ * p_req);
ONVIF_RET _trt_GetOSDs(XMLN * p_node, trt_GetOSDs_REQ * p_req);
ONVIF_RET _trt_GetOSD(XMLN * p_node, trt_GetOSD_REQ * p_req);
ONVIF_RET _trt_SetMetadataConfiguration(XMLN * p_node, trt_SetMetadataConfiguration_REQ * p_req);
ONVIF_RET _trt_AddMetadataConfiguration(XMLN * p_node, trt_AddMetadataConfiguration_REQ * p_req);
ONVIF_RET _trt_GetMetadataConfigurationOptions(XMLN * p_node, trt_GetMetadataConfigurationOptions_REQ * p_req);
ONVIF_RET _trt_GetVideoSourceModes(XMLN * p_node, trt_GetVideoSourceModes_REQ * p_req);
ONVIF_RET _trt_SetVideoSourceMode(XMLN * p_node, trt_SetVideoSourceMode_REQ * p_req);





#ifdef DEVICEIO_SUPPORT
ONVIF_RET _trt_GetAudioOutputConfiguration(XMLN * p_node, trt_GetAudioOutputConfiguration_REQ * p_req);
ONVIF_RET _trt_SetAudioOutputConfiguration(XMLN * p_node, tmd_SetAudioOutputConfiguration_REQ * p_req);
ONVIF_RET _trt_GetCompatibleAudioOutputConfigurations(XMLN * p_node, trt_GetCompatibleAudioOutputConfigurations_REQ * p_req);
ONVIF_RET _trt_AddAudioOutputConfiguration(XMLN * p_node, trt_AddAudioOutputConfiguration_REQ * p_req);
ONVIF_RET _trt_RemoveAudioOutputConfiguration(XMLN * p_node, trt_RemoveAudioOutputConfiguration_REQ * p_req);
#endif // DEVICEIO_SUPPORT


#ifdef VIDEO_ANALYTICS
ONVIF_RET _AnalyticsEngineConfiguration(XMLN * p_node, onvif_AnalyticsEngineConfiguration * p_req);
ONVIF_RET _RuleEngineConfiguration(XMLN * p_node, onvif_RuleEngineConfiguration * p_req);
ONVIF_RET _VideoAnalyticsConfiguration(XMLN * p_node, onvif_VideoAnalyticsConfiguration * p_req);
ONVIF_RET _trt_AddVideoAnalyticsConfiguration(XMLN * p_node, trt_AddVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET _trt_GetVideoAnalyticsConfiguration(XMLN * p_node, trt_GetVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET _trt_RemoveVideoAnalyticsConfiguration(XMLN * p_node, trt_RemoveVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET _trt_SetVideoAnalyticsConfiguration(XMLN * p_node, trt_SetVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET _trt_GetCompatibleVideoAnalyticsConfigurations(XMLN * p_node, trt_GetCompatibleVideoAnalyticsConfigurations_REQ * p_req);
#endif // VIDEO_ANALYTICS

#endif // MEDIA_SUPPORT

#ifdef PROFILE_G_SUPPORT
ONVIF_RET _RecordingConfiguration(XMLN * p_node, onvif_RecordingConfiguration * p_req);
ONVIF_RET _TrackConfiguration(XMLN * p_node, onvif_TrackConfiguration * p_req);
ONVIF_RET _JobConfiguration(XMLN * p_node, onvif_RecordingJobConfiguration * p_req);
ONVIF_RET _SearchScope(XMLN * p_node, onvif_SearchScope * p_req);
ONVIF_RET _trc_CreateRecording(XMLN * p_node, trc_CreateRecording_REQ * p_req);
ONVIF_RET _trc_SetRecordingConfiguration(XMLN * p_node, trc_SetRecordingConfiguration_REQ * p_req);
ONVIF_RET _trc_CreateTrack(XMLN * p_node, trc_CreateTrack_REQ * p_req);
ONVIF_RET _trc_DeleteTrack(XMLN * p_node, trc_DeleteTrack_REQ * p_req);
ONVIF_RET _trc_GetTrackConfiguration(XMLN * p_node, trc_GetTrackConfiguration_REQ * p_req);
ONVIF_RET _trc_SetTrackConfiguration(XMLN * p_node, trc_SetTrackConfiguration_REQ * p_req);
ONVIF_RET _trc_CreateRecordingJob(XMLN * p_node, trc_CreateRecordingJob_REQ * p_req);
ONVIF_RET _trc_SetRecordingJobConfiguration(XMLN * p_node, trc_SetRecordingJobConfiguration_REQ * p_req);
ONVIF_RET _trc_SetRecordingJobMode(XMLN * p_node, trc_SetRecordingJobMode_REQ * p_req);
ONVIF_RET _tse_GetRecordingInformation(XMLN * p_node, tse_GetRecordingInformation_REQ * p_req);
ONVIF_RET _tse_GetMediaAttributes(XMLN * p_node, tse_GetMediaAttributes_REQ * p_req);
ONVIF_RET _tse_FindRecordings(XMLN * p_node, tse_FindRecordings_REQ * p_req);
ONVIF_RET _tse_GetRecordingSearchResults(XMLN * p_node, tse_GetRecordingSearchResults_REQ * p_req);
ONVIF_RET _tse_FindEvents(XMLN * p_node, tse_FindEvents_REQ * p_req);
ONVIF_RET _tse_GetEventSearchResults(XMLN * p_node, tse_GetEventSearchResults_REQ * p_req);
ONVIF_RET _tse_FindMetadata(XMLN * p_node, tse_FindMetadata_REQ * p_req);
ONVIF_RET _tse_GetMetadataSearchResults(XMLN * p_node, tse_GetMetadataSearchResults_REQ * p_req);
ONVIF_RET _tse_EndSearch(XMLN * p_node, tse_EndSearch_REQ * p_req);
ONVIF_RET _tse_GetSearchState(XMLN * p_node, tse_GetSearchState_REQ * p_req);

ONVIF_RET _trp_GetReplayUri(XMLN * p_node, trp_GetReplayUri_REQ * p_req);
ONVIF_RET _trp_SetReplayConfiguration(XMLN * p_node, trp_SetReplayConfiguration_REQ * p_req);
#endif	// PROFILE_G_SUPPORT

#ifdef VIDEO_ANALYTICS
ONVIF_RET _SimpleItem(XMLN * p_node, onvif_SimpleItem * p_req);
ONVIF_RET _ElementItem(XMLN * p_node, onvif_ElementItem * p_req);
ONVIF_RET _Config(XMLN * p_node, onvif_Config * p_req);
ONVIF_RET _tan_GetSupportedRules(XMLN * p_node, tan_GetSupportedRules_REQ * p_req);
ONVIF_RET _tan_CreateRules(XMLN * p_node, tan_CreateRules_REQ * p_req);
ONVIF_RET _tan_DeleteRules(XMLN * p_node, tan_DeleteRules_REQ * p_req);
ONVIF_RET _tan_GetRules(XMLN * p_node, tan_GetRules_REQ * p_req);
ONVIF_RET _tan_ModifyRules(XMLN * p_node, tan_ModifyRules_REQ * p_req);
ONVIF_RET _tan_CreateAnalyticsModules(XMLN * p_node, tan_CreateAnalyticsModules_REQ * p_req);
ONVIF_RET _tan_DeleteAnalyticsModules(XMLN * p_node, tan_DeleteAnalyticsModules_REQ * p_req);
ONVIF_RET _tan_GetAnalyticsModules(XMLN * p_node, tan_GetAnalyticsModules_REQ * p_req);
ONVIF_RET _tan_ModifyAnalyticsModules(XMLN * p_node, tan_ModifyAnalyticsModules_REQ * p_req);
ONVIF_RET _tan_GetRuleOptions(XMLN * p_node, tan_GetRuleOptions_REQ * p_req);
ONVIF_RET _tan_GetSupportedAnalyticsModules(XMLN * p_node, tan_GetSupportedAnalyticsModules_REQ * p_req);
ONVIF_RET _tan_GetAnalyticsModuleOptions(XMLN * p_node, tan_GetAnalyticsModuleOptions_REQ * p_req);
#endif	// VIDEO_ANALYTICS

#ifdef PROFILE_C_SUPPORT
ONVIF_RET _tac_GetAccessPointList(XMLN * p_node, tac_GetAccessPointList_REQ * p_req);
ONVIF_RET _tac_GetAccessPoints(XMLN * p_node, tac_GetAccessPoints_REQ * p_req);
ONVIF_RET _tac_CreateAccessPoint(XMLN * p_node, tac_CreateAccessPoint_REQ * p_req);
ONVIF_RET _tac_SetAccessPoint(XMLN * p_node, tac_SetAccessPoint_REQ * p_req);
ONVIF_RET _tac_ModifyAccessPoint(XMLN * p_node, tac_ModifyAccessPoint_REQ * p_req);
ONVIF_RET _tac_DeleteAccessPoint(XMLN * p_node, tac_DeleteAccessPoint_REQ * p_req);
ONVIF_RET _tac_GetAccessPointInfoList(XMLN * p_node, tac_GetAccessPointInfoList_REQ * p_req);
ONVIF_RET _tac_GetAccessPointInfo(XMLN * p_node, tac_GetAccessPointInfo_REQ * p_req);
ONVIF_RET _tac_GetAreaList(XMLN * p_node, tac_GetAreaList_REQ * p_req);
ONVIF_RET _tac_GetAreas(XMLN * p_node, tac_GetAreas_REQ * p_req);
ONVIF_RET _tac_CreateArea(XMLN * p_node, tac_CreateArea_REQ * p_req);
ONVIF_RET _tac_SetArea(XMLN * p_node, tac_SetArea_REQ * p_req);
ONVIF_RET _tac_ModifyArea(XMLN * p_node, tac_ModifyArea_REQ * p_req);
ONVIF_RET _tac_DeleteArea(XMLN * p_node, tac_DeleteArea_REQ * p_req);
ONVIF_RET _tac_GetAreaInfoList(XMLN * p_node, tac_GetAreaInfoList_REQ * p_req);
ONVIF_RET _tac_GetAreaInfo(XMLN * p_node, tac_GetAreaInfo_REQ * p_req);
ONVIF_RET _tac_GetAccessPointState(XMLN * p_node, tac_GetAccessPointState_REQ * p_req);
ONVIF_RET _tac_EnableAccessPoint(XMLN * p_node, tac_EnableAccessPoint_REQ * p_req);
ONVIF_RET _tac_DisableAccessPoint(XMLN * p_node, tac_DisableAccessPoint_REQ * p_req);
ONVIF_RET _tdc_GetDoorList(XMLN * p_node, tdc_GetDoorList_REQ * p_req);
ONVIF_RET _tdc_GetDoors(XMLN * p_node, tdc_GetDoors_REQ * p_req);
ONVIF_RET _tdc_CreateDoor(XMLN * p_node, tdc_CreateDoor_REQ * p_req);
ONVIF_RET _tdc_SetDoor(XMLN * p_node, tdc_SetDoor_REQ * p_req);
ONVIF_RET _tdc_ModifyDoor(XMLN * p_node, tdc_ModifyDoor_REQ * p_req);
ONVIF_RET _tdc_DeleteDoor(XMLN * p_node, tdc_DeleteDoor_REQ * p_req);
ONVIF_RET _tdc_GetDoorInfoList(XMLN * p_node, tdc_GetDoorInfoList_REQ * p_req);
ONVIF_RET _tdc_GetDoorInfo(XMLN * p_node, tdc_GetDoorInfo_REQ * p_req);
ONVIF_RET _tdc_GetDoorState(XMLN * p_node, tdc_GetDoorState_REQ * p_req);
ONVIF_RET _tdc_AccessDoor(XMLN * p_node, tdc_AccessDoor_REQ * p_req);
ONVIF_RET _tdc_LockDoor(XMLN * p_node, tdc_LockDoor_REQ * p_req);
ONVIF_RET _tdc_UnlockDoor(XMLN * p_node, tdc_UnlockDoor_REQ * p_req);
ONVIF_RET _tdc_DoubleLockDoor(XMLN * p_node, tdc_DoubleLockDoor_REQ * p_req);
ONVIF_RET _tdc_BlockDoor(XMLN * p_node, tdc_BlockDoor_REQ * p_req);
ONVIF_RET _tdc_LockDownDoor(XMLN * p_node, tdc_LockDownDoor_REQ * p_req);
ONVIF_RET _tdc_LockDownReleaseDoor(XMLN * p_node, tdc_LockDownReleaseDoor_REQ * p_req);
ONVIF_RET _tdc_LockOpenDoor(XMLN * p_node, tdc_LockOpenDoor_REQ * p_req);
ONVIF_RET _tdc_LockOpenReleaseDoor(XMLN * p_node, tdc_LockOpenReleaseDoor_REQ * p_req);
#endif  // PROFILE_C_SUPPORT

#ifdef DEVICEIO_SUPPORT
ONVIF_RET _RelayOutputSettings(XMLN * p_node, onvif_RelayOutputSettings * p_req);
ONVIF_RET _tmd_SetRelayOutputState(XMLN * p_node, tmd_SetRelayOutputState_REQ * p_req);
ONVIF_RET _tmd_GetVideoOutputConfiguration(XMLN * p_node, tmd_GetVideoOutputConfiguration_REQ * p_req);
ONVIF_RET _tmd_SetVideoOutputConfiguration(XMLN * p_node, tmd_SetVideoOutputConfiguration_REQ * p_req);
ONVIF_RET _tmd_GetVideoOutputConfigurationOptions(XMLN * p_node, tmd_GetVideoOutputConfigurationOptions_REQ * p_req);
ONVIF_RET _tmd_GetAudioOutputConfiguration(XMLN * p_node, tmd_GetAudioOutputConfiguration_REQ * p_req);
ONVIF_RET _tmd_GetAudioOutputConfigurationOptions(XMLN * p_node, tmd_GetAudioOutputConfigurationOptions_REQ * p_req);
ONVIF_RET _tmd_GetRelayOutputOptions(XMLN * p_node, tmd_GetRelayOutputOptions_REQ * p_req);
ONVIF_RET _tmd_SetRelayOutputSettings(XMLN * p_node, tmd_SetRelayOutputSettings_REQ * p_req);
ONVIF_RET _tmd_GetDigitalInputConfigurationOptions(XMLN * p_node, tmd_GetDigitalInputConfigurationOptions_REQ * p_req);
ONVIF_RET _tmd_SetDigitalInputConfigurations(XMLN * p_node, tmd_SetDigitalInputConfigurations_REQ * p_req);
ONVIF_RET _tmd_GetSerialPortConfiguration(XMLN * p_node, tmd_GetSerialPortConfiguration_REQ * p_req);
ONVIF_RET _tmd_GetSerialPortConfigurationOptions(XMLN * p_node, tmd_GetSerialPortConfigurationOptions_REQ * p_req);
ONVIF_RET _tmd_SetSerialPortConfiguration(XMLN * p_node, tmd_SetSerialPortConfiguration_REQ * p_req);
ONVIF_RET _tmd_SendReceiveSerialCommand(XMLN * p_node, tmd_SendReceiveSerialCommand_REQ * p_req);
#endif // DEVICEIO_SUPPORT

#ifdef MEDIA2_SUPPORT
ONVIF_RET _ConfigurationRef(XMLN * p_node, onvif_ConfigurationRef * p_req);
BOOL      parse_Polygon(XMLN * p_node, onvif_Polygon * p_req);
BOOL      parse_Color(XMLN * p_node, onvif_Color * p_req);
ONVIF_RET _Mask(XMLN * p_node, onvif_Mask * p_req);
ONVIF_RET _tr2_GetConfiguration(XMLN * p_node, tr2_GetConfiguration * p_req);
ONVIF_RET _tr2_SetVideoEncoderConfiguration(XMLN * p_node, tr2_SetVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET _tr2_CreateProfile(XMLN * p_node, tr2_CreateProfile_REQ * p_req);
ONVIF_RET _tr2_GetProfiles(XMLN * p_node, tr2_GetProfiles_REQ * p_req);
ONVIF_RET _tr2_DeleteProfile(XMLN * p_node, tr2_DeleteProfile_REQ * p_req);
ONVIF_RET _tr2_AddConfiguration(XMLN * p_node, tr2_AddConfiguration_REQ * p_req);
ONVIF_RET _tr2_RemoveConfiguration(XMLN * p_node, tr2_RemoveConfiguration_REQ * p_req);
ONVIF_RET _tr2_SetVideoSourceConfiguration(XMLN * p_node, tr2_SetVideoSourceConfiguration_REQ * p_req);
ONVIF_RET _tr2_SetAudioEncoderConfiguration(XMLN * p_node, tr2_SetAudioEncoderConfiguration_REQ * p_req);
ONVIF_RET _tr2_SetMetadataConfiguration(XMLN * p_node, tr2_SetMetadataConfiguration_REQ * p_req);
ONVIF_RET _tr2_GetVideoEncoderInstances(XMLN * p_node, tr2_GetVideoEncoderInstances_REQ * p_req);
ONVIF_RET _tr2_GetStreamUri(XMLN * p_node, tr2_GetStreamUri_REQ * p_req);
ONVIF_RET _tr2_SetSynchronizationPoint(XMLN * p_node, tr2_SetSynchronizationPoint_REQ * p_req);
ONVIF_RET _tr2_GetVideoSourceModes(XMLN * p_node, tr2_GetVideoSourceModes_REQ * p_req);
ONVIF_RET _tr2_SetVideoSourceMode(XMLN * p_node, tr2_SetVideoSourceMode_REQ * p_req);
ONVIF_RET _tr2_GetSnapshotUri(XMLN * p_node, tr2_GetSnapshotUri_REQ * p_req);
ONVIF_RET _tr2_GetOSDs(XMLN * p_node, tr2_GetOSDs_REQ * p_req);
ONVIF_RET _tr2_CreateMask(XMLN * p_node, tr2_CreateMask_REQ * p_req);
ONVIF_RET _tr2_DeleteMask(XMLN * p_node, tr2_DeleteMask_REQ * p_req);
ONVIF_RET _tr2_StartMulticastStreaming(XMLN * p_node, tr2_StartMulticastStreaming_REQ * p_req);
ONVIF_RET _tr2_StopMulticastStreaming(XMLN * p_node, tr2_StopMulticastStreaming_REQ * p_req);

#ifdef DEVICEIO_SUPPORT
ONVIF_RET _tr2_GetAudioOutputConfigurations(XMLN * p_node, tr2_GetAudioOutputConfigurations_REQ * p_req);
ONVIF_RET _tr2_SetAudioOutputConfiguration(XMLN * p_node, tr2_SetAudioOutputConfiguration_REQ * p_req);
#endif // DEVICEIO_SUPPORT

#endif // MEDIA2_SUPPORT

#ifdef THERMAL_SUPPORT
ONVIF_RET _ColorPalette(XMLN * p_node, onvif_ColorPalette * p_req);
ONVIF_RET _NUCTable(XMLN * p_node, onvif_NUCTable * p_req);
ONVIF_RET _Cooler(XMLN * p_node, onvif_Cooler * p_req);
ONVIF_RET _ThermalConfiguration(XMLN * p_node, onvif_ThermalConfiguration * p_req);
ONVIF_RET _RadiometryGlobalParameters(XMLN * p_node, onvif_RadiometryGlobalParameters * p_req);
ONVIF_RET _RadiometryConfiguration(XMLN * p_node, onvif_RadiometryConfiguration * p_req);
ONVIF_RET _tth_GetConfiguration(XMLN * p_node, tth_GetConfiguration_REQ * p_req);
ONVIF_RET _tth_SetConfiguration(XMLN * p_node, tth_SetConfiguration_REQ * p_req);
ONVIF_RET _tth_GetConfigurationOptions(XMLN * p_node, tth_GetConfigurationOptions_REQ * p_req);
ONVIF_RET _tth_GetRadiometryConfiguration(XMLN * p_node, tth_GetRadiometryConfiguration_REQ * p_req);
ONVIF_RET _tth_SetRadiometryConfiguration(XMLN * p_node, tth_SetRadiometryConfiguration_REQ * p_req);
ONVIF_RET _tth_GetRadiometryConfigurationOptions(XMLN * p_node, tth_GetRadiometryConfigurationOptions_REQ * p_req);
#endif // THERMAL_SUPPORT

#ifdef CREDENTIAL_SUPPORT
ONVIF_RET _CredentialIdentifierType(XMLN * p_node, onvif_CredentialIdentifierType * p_req);
ONVIF_RET _CredentialIdentifier(XMLN * p_node, onvif_CredentialIdentifier * p_req);
ONVIF_RET _CredentialAccessProfile(XMLN * p_node, onvif_CredentialAccessProfile * p_req);
ONVIF_RET _Attribute(XMLN * p_node, onvif_Attribute * p_req);
ONVIF_RET _Credential(XMLN * p_node, onvif_Credential * p_req);
ONVIF_RET _CredentialState(XMLN * p_node, onvif_CredentialState * p_req);
ONVIF_RET _tcr_GetCredentialInfo(XMLN * p_node, tcr_GetCredentialInfo_REQ * p_req);
ONVIF_RET _tcr_GetCredentialInfoList(XMLN * p_node, tcr_GetCredentialInfoList_REQ * p_req);
ONVIF_RET _tcr_GetCredentials(XMLN * p_node, tcr_GetCredentials_REQ * p_req);
ONVIF_RET _tcr_GetCredentialList(XMLN * p_node, tcr_GetCredentialList_REQ * p_req);
ONVIF_RET _tcr_CreateCredential(XMLN * p_node, tcr_CreateCredential_REQ * p_req);
ONVIF_RET _tcr_ModifyCredential(XMLN * p_node, tcr_ModifyCredential_REQ * p_req);
ONVIF_RET _tcr_DeleteCredential(XMLN * p_node, tcr_DeleteCredential_REQ * p_req);
ONVIF_RET _tcr_GetCredentialState(XMLN * p_node, tcr_GetCredentialState_REQ * p_req);
ONVIF_RET _tcr_EnableCredential(XMLN * p_node, tcr_EnableCredential_REQ * p_req);
ONVIF_RET _tcr_DisableCredential(XMLN * p_node, tcr_DisableCredential_REQ * p_req);
ONVIF_RET _tcr_SetCredential(XMLN * p_node, tcr_SetCredential_REQ * p_req);
ONVIF_RET _tcr_ResetAntipassbackViolation(XMLN * p_node, tcr_ResetAntipassbackViolation_REQ * p_req);
ONVIF_RET _tcr_GetSupportedFormatTypes(XMLN * p_node, tcr_GetSupportedFormatTypes_REQ * p_req);
ONVIF_RET _tcr_GetCredentialIdentifiers(XMLN * p_node, tcr_GetCredentialIdentifiers_REQ * p_req);
ONVIF_RET _tcr_SetCredentialIdentifier(XMLN * p_node, tcr_SetCredentialIdentifier_REQ * p_req);
ONVIF_RET _tcr_DeleteCredentialIdentifier(XMLN * p_node, tcr_DeleteCredentialIdentifier_REQ * p_req);
ONVIF_RET _tcr_GetCredentialAccessProfiles(XMLN * p_node, tcr_GetCredentialAccessProfiles_REQ * p_req);
ONVIF_RET _tcr_SetCredentialAccessProfiles(XMLN * p_node, tcr_SetCredentialAccessProfiles_REQ * p_req);
ONVIF_RET _tcr_DeleteCredentialAccessProfiles(XMLN * p_node, tcr_DeleteCredentialAccessProfiles_REQ * p_req);
ONVIF_RET _tcr_GetWhitelist(XMLN * p_node, tcr_GetWhitelist_REQ * p_req);
ONVIF_RET _tcr_AddToWhitelist(XMLN * p_node, tcr_AddToWhitelist_REQ * p_req);
ONVIF_RET _tcr_RemoveFromWhitelist(XMLN * p_node, tcr_RemoveFromWhitelist_REQ * p_req);
ONVIF_RET _tcr_GetBlacklist(XMLN * p_node, tcr_GetBlacklist_REQ * p_req);
ONVIF_RET _tcr_AddToBlacklist(XMLN * p_node, tcr_AddToBlacklist_REQ * p_req);
ONVIF_RET _tcr_RemoveFromBlacklist(XMLN * p_node, tcr_RemoveFromBlacklist_REQ * p_req);
#endif // CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES
ONVIF_RET _AccessPolicy(XMLN * p_node, onvif_AccessPolicy * p_req);
ONVIF_RET _AccessProfile(XMLN * p_node, onvif_AccessProfile * p_req);
ONVIF_RET _tar_GetAccessProfileInfo(XMLN * p_node, tar_GetAccessProfileInfo_REQ * p_req);
ONVIF_RET _tar_GetAccessProfileInfoList(XMLN * p_node, tar_GetAccessProfileInfoList_REQ * p_req);
ONVIF_RET _tar_GetAccessProfiles(XMLN * p_node, tar_GetAccessProfiles_REQ * p_req);
ONVIF_RET _tar_GetAccessProfileList(XMLN * p_node, tar_GetAccessProfileList_REQ * p_req);
ONVIF_RET _tar_CreateAccessProfile(XMLN * p_node, tar_CreateAccessProfile_REQ * p_req);
ONVIF_RET _tar_ModifyAccessProfile(XMLN * p_node, tar_ModifyAccessProfile_REQ * p_req);
ONVIF_RET _tar_DeleteAccessProfile(XMLN * p_node, tar_DeleteAccessProfile_REQ * p_req);
ONVIF_RET _tar_SetAccessProfile(XMLN * p_node, tar_SetAccessProfile_REQ * p_req);
#endif // ACCESS_RULES

#ifdef SCHEDULE_SUPPORT
ONVIF_RET _TimePeriod(XMLN * p_node, onvif_TimePeriod * p_req);
ONVIF_RET _SpecialDaysSchedule(XMLN * p_node, onvif_SpecialDaysSchedule * p_req);
ONVIF_RET _Schedule(XMLN * p_node, onvif_Schedule * p_req);
ONVIF_RET _SpecialDayGroup(XMLN * p_node, onvif_SpecialDayGroup * p_req);
ONVIF_RET _tsc_GetScheduleInfo(XMLN * p_node, tsc_GetScheduleInfo_REQ * p_req);
ONVIF_RET _tsc_GetScheduleInfoList(XMLN * p_node, tsc_GetScheduleInfoList_REQ * p_req);
ONVIF_RET _tsc_GetSchedules(XMLN * p_node, tsc_GetSchedules_REQ * p_req);
ONVIF_RET _tsc_GetScheduleList(XMLN * p_node, tsc_GetScheduleList_REQ * p_req);
ONVIF_RET _tsc_CreateSchedule(XMLN * p_node, tsc_CreateSchedule_REQ * p_req);
ONVIF_RET _tsc_ModifySchedule(XMLN * p_node, tsc_ModifySchedule_REQ * p_req);
ONVIF_RET _tsc_DeleteSchedule(XMLN * p_node, tsc_DeleteSchedule_REQ * p_req);
ONVIF_RET _tsc_GetSpecialDayGroupInfo(XMLN * p_node, tsc_GetSpecialDayGroupInfo_REQ * p_req);
ONVIF_RET _tsc_GetSpecialDayGroupInfoList(XMLN * p_node, tsc_GetSpecialDayGroupInfoList_REQ * p_req);
ONVIF_RET _tsc_GetSpecialDayGroups(XMLN * p_node, tsc_GetSpecialDayGroups_REQ * p_req);
ONVIF_RET _tsc_GetSpecialDayGroupList(XMLN * p_node, tsc_GetSpecialDayGroupList_REQ * p_req);
ONVIF_RET _tsc_CreateSpecialDayGroup(XMLN * p_node, tsc_CreateSpecialDayGroup_REQ * p_req);
ONVIF_RET _tsc_ModifySpecialDayGroup(XMLN * p_node, tsc_ModifySpecialDayGroup_REQ * p_req);
ONVIF_RET _tsc_DeleteSpecialDayGroup(XMLN * p_node, tsc_DeleteSpecialDayGroup_REQ * p_req);
ONVIF_RET _tsc_GetScheduleState(XMLN * p_node, tsc_GetScheduleState_REQ * p_req);
ONVIF_RET _tsc_SetSchedule(XMLN * p_node, tsc_SetSchedule_REQ * p_req);
ONVIF_RET _tsc_SetSpecialDayGroup(XMLN * p_node, tsc_SetSpecialDayGroup_REQ * p_req);
#endif // SCHEDULE_SUPPORT

#ifdef RECEIVER_SUPPORT
ONVIF_RET _ReceiverConfiguration(XMLN * p_node, onvif_ReceiverConfiguration * p_req);
ONVIF_RET _trv_GetReceiver(XMLN * p_node, trv_GetReceiver_REQ * p_req);
ONVIF_RET _trv_CreateReceiver(XMLN * p_node, trv_CreateReceiver_REQ * p_req);
ONVIF_RET _trv_DeleteReceiver(XMLN * p_node, trv_DeleteReceiver_REQ * p_req);
ONVIF_RET _trv_ConfigureReceiver(XMLN * p_node, trv_ConfigureReceiver_REQ * p_req);
ONVIF_RET _trv_SetReceiverMode(XMLN * p_node, trv_SetReceiverMode_REQ * p_req);
ONVIF_RET _trv_GetReceiverState(XMLN * p_node, trv_GetReceiverState_REQ * p_req);
#endif // RECEIVER_SUPPORT

#ifdef PROVISIONING_SUPPORT
ONVIF_RET _tpv_PanMove(XMLN * p_node, tpv_PanMove_REQ * p_req);
ONVIF_RET _tpv_TiltMove(XMLN * p_node, tpv_TiltMove_REQ * p_req);
ONVIF_RET _tpv_ZoomMove(XMLN * p_node, tpv_ZoomMove_REQ * p_req);
ONVIF_RET _tpv_RollMove(XMLN * p_node, tpv_RollMove_REQ * p_req);
ONVIF_RET _tpv_FocusMove(XMLN * p_node, tpv_FocusMove_REQ * p_req);
ONVIF_RET _tpv_Stop(XMLN * p_node, tpv_Stop_REQ * p_req);
ONVIF_RET _tpv_GetUsage(XMLN * p_node, tpv_GetUsage_REQ * p_req);
#endif // PROVISIONING_SUPPORT


#ifdef __cplusplus
}
#endif

#endif


