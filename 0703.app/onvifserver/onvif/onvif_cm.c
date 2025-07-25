


#include "sys_inc.h"
#include "onvif_cm.h"


HT_API const char * onvif_CapabilityCategoryToString(onvif_CapabilityCategory category)
{
	switch (category)
	{
	case CapabilityCategory_All:
		return "All";
		
	case CapabilityCategory_Analytics:
		return "Analytics";

	case CapabilityCategory_Device:
		return "Device";

	case CapabilityCategory_Events:
		return "Events";

	case CapabilityCategory_Imaging:
		return "Imaging";

	case CapabilityCategory_Media:
		return "Media";

	case CapabilityCategory_PTZ:
		return "PTZ";
        
    default:
        break;
	}

	return "All";
}

HT_API onvif_CapabilityCategory onvif_StringToCapabilityCategory(const char * str)
{
	if (strcasecmp(str, "All") == 0)
	{
		return CapabilityCategory_All;
	}
	else if (strcasecmp(str, "Analytics") == 0)
	{
		return CapabilityCategory_Analytics;
	}
	else if (strcasecmp(str, "Device") == 0)
	{
		return CapabilityCategory_Device;
	}
	else if (strcasecmp(str, "Events") == 0)
	{
		return CapabilityCategory_Events;
	}
	else if (strcasecmp(str, "Imaging") == 0)
	{
		return CapabilityCategory_Imaging;
	}
	else if (strcasecmp(str, "Media") == 0)
	{
		return CapabilityCategory_Media;
	}
	else if (strcasecmp(str, "PTZ") == 0)
	{
		return CapabilityCategory_PTZ;
	}

	return CapabilityCategory_Invalid;
}

HT_API const char * onvif_FactoryDefaultTypeToString(onvif_FactoryDefaultType type)
{
	switch (type)
	{
	case FactoryDefaultType_Hard:
		return "Hard";
		
	case FactoryDefaultType_Soft:
		return "Soft";
	}

	return "Hard";
}

HT_API onvif_FactoryDefaultType onvif_StringToFactoryDefaultType(const char * str)
{
	if (strcasecmp(str, "Hard") == 0)
	{
		return FactoryDefaultType_Hard;
	}
	else if (strcasecmp(str, "Soft") == 0)
	{
		return FactoryDefaultType_Soft;
	}

	return FactoryDefaultType_Hard;
}

HT_API onvif_SystemLogType onvif_StringToSystemLogType(const char * str)
{
	if (strcasecmp(str, "System") == 0)
	{
		return SystemLogType_System;
	}
	else if (strcasecmp(str, "Access") == 0)
	{
		return SystemLogType_Access;
	}

	return SystemLogType_System;
}

HT_API const char * onvif_VideoEncodingToString(onvif_VideoEncoding encoding)
{
	switch (encoding)
	{
	case VideoEncoding_H264:
		return "H264";

	case VideoEncoding_H265:
	    return "H265";
        
    case VideoEncoding_Unknown:
        break;
	}

	return "H264";
}

HT_API onvif_VideoEncoding onvif_StringToVideoEncoding(const char * str)
{
	if (strcasecmp(str, "H264") == 0)
	{
		return VideoEncoding_H264;
	}
	else if (strcasecmp(str, "H265") == 0)
	{
	    return VideoEncoding_H265;
	}

	return VideoEncoding_Unknown;
}

HT_API const char * onvif_AudioEncodingToString(onvif_AudioEncoding encoding)
{
	switch (encoding)
	{
	case AudioEncoding_G711:
		return "G711";
		
	case AudioEncoding_G726:
		return "G726";

	case AudioEncoding_AAC:
		return "AAC";
        
    case AudioEncoding_Unknown:
        break;
	}

	return "G711";
}

HT_API onvif_AudioEncoding onvif_StringToAudioEncoding(const char * str)
{
	if (strcasecmp(str, "G711") == 0)
	{
		return AudioEncoding_G711;
	}
	else if (strcasecmp(str, "G726") == 0)
	{
		return AudioEncoding_G726;
	}
	else if (strcasecmp(str, "AAC") == 0)
	{
		return AudioEncoding_AAC;
	}

	return AudioEncoding_Unknown;
}

HT_API const char * onvif_H264ProfileToString(onvif_H264Profile profile)
{
	switch (profile)
	{
	case H264Profile_Baseline:
		return "Baseline";
		
	case H264Profile_Main:
		return "Main";

	case H264Profile_Extended:
		return "Extended";

	case H264Profile_High:
		return "High";
	}

	return "Baseline";
}

HT_API onvif_H264Profile onvif_StringToH264Profile(const char * str)
{
	if (strcasecmp(str, "Baseline") == 0)
	{
		return H264Profile_Baseline;
	}
	else if (strcasecmp(str, "Main") == 0)
	{
		return H264Profile_Main;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return H264Profile_Extended;
	}
	else if (strcasecmp(str, "High") == 0)
	{
		return H264Profile_High;
	}

	return H264Profile_Baseline;
}

HT_API const char * onvif_MediaProfile2Media2Profile(const char * profile)
{
    if (strcasecmp(profile, "SP") == 0)
    {
        return "Simple"; 
    }
    else if (strcasecmp(profile, "ASP") == 0)
    {
        return "AdvancedSimple"; 
    }

    return profile;
}

HT_API const char * onvif_Mpeg4ProfileToString(onvif_Mpeg4Profile profile)
{
	switch (profile)
	{
	case Mpeg4Profile_SP:
		return "SP";
		
	case Mpeg4Profile_ASP:
		return "ASP";
	}

	return "SP";
}

HT_API onvif_Mpeg4Profile onvif_StringToMpeg4Profile(const char * str)
{
	if (strcasecmp(str, "SP") == 0)
	{
		return Mpeg4Profile_SP;
	}
	else if (strcasecmp(str, "ASP") == 0)
	{
		return Mpeg4Profile_ASP;
	}

	return Mpeg4Profile_SP;
}

HT_API const char * onvif_UserLevelToString(onvif_UserLevel level)
{
	switch (level)
	{
	case UserLevel_Administrator:
		return "Administrator";
		
	case UserLevel_Operator:
		return "Operator";

	case UserLevel_User:
		return "User";

	case UserLevel_Anonymous:
		return "Anonymous";

	case UserLevel_Extended:
		return "Extended";	
	}

	return "User";
}

HT_API onvif_UserLevel onvif_StringToUserLevel(const char * str)
{
	if (strcasecmp(str, "Administrator") == 0)
	{
		return UserLevel_Administrator;
	}
	else if (strcasecmp(str, "Operator") == 0)
	{
		return UserLevel_Operator;
	}
	else if (strcasecmp(str, "User") == 0)
	{
		return UserLevel_User;
	}
	else if (strcasecmp(str, "Anonymous") == 0)
	{
		return UserLevel_Anonymous;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return UserLevel_Extended;
	}

	return UserLevel_User;
}

HT_API const char * onvif_MoveStatusToString(onvif_MoveStatus status)
{
	switch (status)
	{
	case MoveStatus_IDLE:
		return "IDLE";
		
	case MoveStatus_MOVING:
		return "MOVING";

	case MoveStatus_UNKNOWN:
		return "UNKNOWN";
	}

	return "IDLE";
}

HT_API onvif_MoveStatus onvif_StringToMoveStatus(const char * str)
{
	if (strcasecmp(str, "IDLE") == 0)
	{
		return MoveStatus_IDLE;
	}
	else if (strcasecmp(str, "MOVING") == 0)
	{
		return MoveStatus_MOVING;
	}
	else if (strcasecmp(str, "UNKNOWN") == 0)
	{
		return MoveStatus_UNKNOWN;
	}

	return MoveStatus_IDLE;
}

HT_API const char * onvif_OSDTypeToString(onvif_OSDType type)
{
	switch (type)
	{
	case OSDType_Text:
		return "Text";
		
	case OSDType_Image:
		return "Image";

	case OSDType_Extended:
		return "Extended";
	}

	return "Text";
}

HT_API onvif_OSDType onvif_StringToOSDType(const char * type)
{
	if (strcasecmp(type, "Text") == 0)
	{
		return OSDType_Text;
	}
	else if (strcasecmp(type, "Image") == 0)
	{
		return OSDType_Image;
	}
	else if (strcasecmp(type, "Extended") == 0)
	{
		return OSDType_Extended;
	}

	return OSDType_Text;
}

HT_API const char * onvif_OSDPosTypeToString(onvif_OSDPosType type)
{
	switch (type)
	{
	case OSDPosType_UpperLeft:
		return "UpperLeft";
		
	case OSDPosType_UpperRight:
		return "UpperRight";
		
	case OSDPosType_LowerLeft:
		return "LowerLeft";
		
	case OSDPosType_LowerRight:
		return "LowerRight";
		
	case OSDPosType_Custom:	
		return "Custom";
	}

	return "UpperLeft";
}

HT_API onvif_OSDPosType onvif_StringToOSDPosType(const char * type)
{	
	if (strcasecmp(type, "UpperLeft") == 0)
	{
		return OSDPosType_UpperLeft;
	}
	else if (strcasecmp(type, "UpperRight") == 0)
	{
		return OSDPosType_UpperRight;
	}
	else if (strcasecmp(type, "LowerLeft") == 0)
	{
		return OSDPosType_LowerLeft;
	}
	else if (strcasecmp(type, "LowerRight") == 0)
	{
		return OSDPosType_LowerRight;
	}
	else if (strcasecmp(type, "Custom") == 0)
	{
		return OSDPosType_Custom;
	}

	return OSDPosType_UpperLeft;
}

HT_API const char * onvif_OSDTextTypeToString(onvif_OSDTextType type)
{
	switch (type)
	{
	case OSDTextType_Plain:
		return "Plain";
		
	case OSDTextType_Date:
		return "Date";
		
	case OSDTextType_Time:
		return "Time";
		
	case OSDTextType_DateAndTime:
		return "DateAndTime";
	}

	return "Plain";
}

HT_API onvif_OSDTextType	onvif_StringToOSDTextType(const char * type)
{
	if (strcasecmp(type, "Plain") == 0)
	{
		return OSDTextType_Plain;
	}
	else if (strcasecmp(type, "Date") == 0)
	{
		return OSDTextType_Date;
	}
	else if (strcasecmp(type, "Time") == 0)
	{
		return OSDTextType_Time;
	}
	else if (strcasecmp(type, "DateAndTime") == 0)
	{
		return OSDTextType_DateAndTime;
	}

	return OSDTextType_Plain;
}

HT_API const char * onvif_BacklightCompensationModeToString(onvif_BacklightCompensationMode mode)
{
	switch (mode)
	{
	case BacklightCompensationMode_OFF:
		return "OFF";
		
	case BacklightCompensationMode_ON:
		return "ON";
	}

	return "OFF";
}

HT_API onvif_BacklightCompensationMode onvif_StringToBacklightCompensationMode(const char * str)
{
	if (strcasecmp(str, "OFF") == 0)
	{
		return BacklightCompensationMode_OFF;
	}
	else if (strcasecmp(str, "ON") == 0)
	{

		return BacklightCompensationMode_ON;
	}

	return BacklightCompensationMode_OFF;
}

HT_API const char * onvif_ExposureModeToString(onvif_ExposureMode mode)
{
	switch (mode)
	{
	case ExposureMode_AUTO:
		return "AUTO";

	case ExposureMode_MANUAL:
		return "MANUAL";	
	}

	return "AUTO";
}

HT_API onvif_ExposureMode onvif_StringToExposureMode(const char * str)
{
	if (strcasecmp(str, "AUTO") == 0)
	{

		return ExposureMode_AUTO;
	}
	else if (strcasecmp(str, "MANUAL") == 0)
	{
		return ExposureMode_MANUAL;
	}

	return ExposureMode_AUTO;
}

HT_API const char * onvif_ExposurePriorityToString(onvif_ExposurePriority mode)
{

	return "LowNoise";
}

HT_API onvif_ExposurePriority onvif_StringToExposurePriority(const char * str)
{
	if (strcasecmp(str, "LowNoise") == 0)
	{

		return ExposurePriority_LowNoise;
	}
	else if (strcasecmp(str, "FrameRate") == 0)
	{
		return ExposurePriority_FrameRate;
	}

	return ExposurePriority_LowNoise;
}

HT_API const char * onvif_AutoFocusModeToString(onvif_AutoFocusMode mode)
{

	return "AUTO";
}

HT_API onvif_AutoFocusMode onvif_StringToAutoFocusMode(const char * str)
{
	if (strcasecmp(str, "AUTO") == 0)
	{

		return AutoFocusMode_AUTO;
	}
	else if (strcasecmp(str, "MANUAL") == 0)
	{
		return AutoFocusMode_MANUAL;
	}

	return AutoFocusMode_AUTO;
}

HT_API const char * onvif_WideDynamicModeToString(onvif_WideDynamicMode mode)
{
	switch (mode)
	{
	case WideDynamicMode_OFF:
		return "OFF";

	case WideDynamicMode_ON:
		return "ON";	
	}

	return "OFF";
}

HT_API onvif_WideDynamicMode onvif_StringToWideDynamicMode(const char * str)
{
	if (strcasecmp(str, "OFF") == 0)
	{

		return WideDynamicMode_OFF;
	}
	else if (strcasecmp(str, "ON") == 0)
	{
		return WideDynamicMode_ON;
	}

	return WideDynamicMode_OFF;
}

HT_API const char * onvif_IrCutFilterModeToString(onvif_IrCutFilterMode mode)
{
	switch (mode)
	{
	case IrCutFilterMode_ON:
		return "ON";

	case IrCutFilterMode_OFF:
		return "OFF";	

	case IrCutFilterMode_AUTO:
		return "AUTO";
	}

	return "ON";
}

HT_API onvif_IrCutFilterMode onvif_StringToIrCutFilterMode(const char * str)
{
	if (strcasecmp(str, "ON") == 0)
	{
		return IrCutFilterMode_ON;
	}
	else if (strcasecmp(str, "OFF") == 0)
	{
		return IrCutFilterMode_OFF;
	}
	else if (strcasecmp(str, "AUTO") == 0)
	{
		return IrCutFilterMode_AUTO;
	}

	return IrCutFilterMode_ON;
}

HT_API const char * onvif_WhiteBalanceModeToString(onvif_WhiteBalanceMode mode)
{
	switch (mode)
	{
	case WhiteBalanceMode_AUTO:
		return "AUTO";

	case WhiteBalanceMode_MANUAL:
		return "MANUAL";	
	}

	return "AUTO";
}

HT_API onvif_WhiteBalanceMode onvif_StringToWhiteBalanceMode(const char * str)
{
	if (strcasecmp(str, "AUTO") == 0)
	{
		return WhiteBalanceMode_AUTO;
	}
	else if (strcasecmp(str, "MANUAL") == 0)
	{
		return WhiteBalanceMode_MANUAL;
	}

	return WhiteBalanceMode_AUTO;
}

HT_API const char * onvif_EFlipModeToString(onvif_EFlipMode mode)
{
    switch (mode)
	{
	case EFlipMode_OFF:
		return "OFF";

	case EFlipMode_ON:
		return "ON";

	case EFlipMode_Extended:
		return "Extended";	
	}

	return "OFF";
}

HT_API onvif_EFlipMode onvif_StringToEFlipMode(const char * str)
{
    if (strcasecmp(str, "OFF") == 0)
	{
		return EFlipMode_OFF;
	}
	else if (strcasecmp(str, "ON") == 0)
	{
		return EFlipMode_ON;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return EFlipMode_Extended;
	}

	return EFlipMode_OFF;
}

HT_API const char * onvif_ReverseModeToString(onvif_ReverseMode mode)
{
    switch (mode)
	{
	case ReverseMode_OFF:
		return "OFF";

	case ReverseMode_ON:
		return "ON";

    case ReverseMode_AUTO:
        return "AUTO";
        
	case ReverseMode_Extended:
		return "Extended";	
	}

	return "OFF";
}

HT_API onvif_ReverseMode onvif_StringToReverseMode(const char * str)
{
    if (strcasecmp(str, "OFF") == 0)
	{
		return ReverseMode_OFF;
	}
	else if (strcasecmp(str, "ON") == 0)
	{
		return ReverseMode_ON;
	}
	else if (strcasecmp(str, "AUTO") == 0)
	{
		return ReverseMode_AUTO;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return ReverseMode_Extended;
	}

	return ReverseMode_OFF;
}

HT_API const char * onvif_DiscoveryModeToString(onvif_DiscoveryMode mode)
{
	switch (mode)
	{
	case DiscoveryMode_Discoverable:
		return "Discoverable";

	case DiscoveryMode_NonDiscoverable:
		return "NonDiscoverable";
	}

	return "Discoverable";
}

HT_API onvif_DiscoveryMode	onvif_StringToDiscoveryMode(const char * str)
{
	if (strcasecmp(str, "Discoverable") == 0)
	{
		return DiscoveryMode_Discoverable;
	}
	else if (strcasecmp(str, "NonDiscoverable") == 0)
	{
		return DiscoveryMode_NonDiscoverable;
	}

	return DiscoveryMode_Discoverable;
}

HT_API const char * onvif_SetDateTimeTypeToString(onvif_SetDateTimeType type)
{
	switch (type)
	{
	case SetDateTimeType_Manual:
		return "Manual";

	case SetDateTimeType_NTP:
		return "NTP";
	}

	return "Manual";
}

HT_API onvif_SetDateTimeType onvif_StringToSetDateTimeType(const char * str)
{
	if (strcasecmp(str, "Manual") == 0)
	{
		return SetDateTimeType_Manual;
	}
	else if (strcasecmp(str, "NTP") == 0)
	{
		return SetDateTimeType_NTP;
	}

	return SetDateTimeType_Manual;
}

HT_API const char * onvif_StreamTypeToString(onvif_StreamType type)
{
	switch (type)
	{
	case StreamType_RTP_Unicast:
		return "RTP-Unicast";

	case StreamType_RTP_Multicast:
		return "RTP-Multicast";
        
    case StreamType_Invalid:
        break;
	}

	return "RTP_Unicast";
}

HT_API onvif_StreamType onvif_StringToStreamType(const char * str)
{
	if (strcasecmp(str, "RTP-Unicast") == 0)
	{
		return StreamType_RTP_Unicast;
	}
	else if (strcasecmp(str, "RTP-Multicast") == 0)
	{
		return StreamType_RTP_Multicast;
	}

	return StreamType_Invalid;
}

HT_API const char * onvif_TransportProtocolToString(onvif_TransportProtocol type)
{
	switch (type)
	{
	case TransportProtocol_UDP:
		return "UDP";

	case TransportProtocol_TCP:
		return "TCP";

	case TransportProtocol_RTSP:
		return "RTSP";

	case TransportProtocol_HTTP:
		return "HTTP";
        
    case TransportProtocol_Invalid:
        break;
	}

	return "UDP";
}

HT_API onvif_TransportProtocol onvif_StringToTransportProtocol(const char * str)
{
	if (strcasecmp(str, "UDP") == 0)
	{
		return TransportProtocol_UDP;
	}
	else if (strcasecmp(str, "TCP") == 0)
	{
		return TransportProtocol_TCP;
	}
	else if (strcasecmp(str, "RTSP") == 0)
	{
		return TransportProtocol_RTSP;
	}
	else if (strcasecmp(str, "HTTP") == 0)
	{
		return TransportProtocol_HTTP;
	}

	return TransportProtocol_Invalid;
}

HT_API const char * onvif_DynamicDNSTypeToString(onvif_DynamicDNSType type)
{
	switch (type)
	{
	case DynamicDNSType_NoUpdate:
		return "NoUpdate";

	case DynamicDNSType_ClientUpdates:
		return "ClientUpdates";

	case DynamicDNSType_ServerUpdates:
		return "ServerUpdates";
	}

	return "NoUpdate";
}

HT_API onvif_DynamicDNSType onvif_StringToDynamicDNSType(const char * str)
{
	if (strcasecmp(str, "NoUpdate") == 0)
	{
		return DynamicDNSType_NoUpdate;
	}
	else if (strcasecmp(str, "ClientUpdates") == 0)
	{
		return DynamicDNSType_ClientUpdates;
	}
	else if (strcasecmp(str, "ServerUpdates") == 0)
	{
		return DynamicDNSType_ServerUpdates;
	}

	return DynamicDNSType_NoUpdate;
}

HT_API const char * onvif_TrackTypeToString(onvif_TrackType type)
{
	switch (type)
	{
	case TrackType_Video:
		return "Video";

	case TrackType_Audio:
		return "Audio";

	case TrackType_Metadata:
		return "Metadata";

	case TrackType_Extended:
		return "Extended";
            
    case TrackType_Invalid:
        break;
	}

	return "Video";
}

HT_API onvif_TrackType	onvif_StringToTrackType(const char * str)
{
	if (strcasecmp(str, "Video") == 0)
	{
		return TrackType_Video;
	}
	else if (strcasecmp(str, "Audio") == 0)
	{
		return TrackType_Audio;
	}
	else if (strcasecmp(str, "Metadata") == 0)
	{
		return TrackType_Metadata;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return TrackType_Extended;
	}

	return TrackType_Invalid;
}

HT_API const char * onvif_PropertyOperationToString(onvif_PropertyOperation type)
{
	switch (type)
	{
	case PropertyOperation_Initialized:
		return "Initialized";

	case PropertyOperation_Deleted:
		return "Deleted";

	case PropertyOperation_Changed:
		return "Changed";
            
    case PropertyOperation_Invalid:
        break;
	}

	return "Initialized";
}

HT_API onvif_PropertyOperation	onvif_StringToPropertyOperation(const char * str)
{
	if (strcasecmp(str, "Initialized") == 0)
	{
		return PropertyOperation_Initialized;
	}
	else if (strcasecmp(str, "Deleted") == 0)
	{
		return PropertyOperation_Deleted;
	}
	else if (strcasecmp(str, "Changed") == 0)
	{
		return PropertyOperation_Changed;
	}

	return PropertyOperation_Invalid;
}

HT_API const char * onvif_RecordingStatusToString(onvif_RecordingStatus status)
{
	switch (status)
	{
	case RecordingStatus_Initiated:
		return "Initiated";

	case RecordingStatus_Recording:
		return "Recording";

	case RecordingStatus_Stopped:
		return "Stopped";	

	case RecordingStatus_Removing:
		return "Removing";

	case RecordingStatus_Removed:
		return "Removed";

	case RecordingStatus_Unknown:
		return "Unknown";	
	}

	return "Initialized";
}

HT_API onvif_RecordingStatus onvif_StringToRecordingStatus(const char * str)
{
	if (strcasecmp(str, "Initiated") == 0)
	{
		return RecordingStatus_Initiated;
	}
	else if (strcasecmp(str, "Recording") == 0)
	{
		return RecordingStatus_Recording;
	}
	else if (strcasecmp(str, "Stopped") == 0)
	{
		return RecordingStatus_Stopped;
	}
	else if (strcasecmp(str, "Removing") == 0)
	{
		return RecordingStatus_Removing;
	}
	else if (strcasecmp(str, "Removed") == 0)
	{
		return RecordingStatus_Removed;
	}
	else if (strcasecmp(str, "Unknown") == 0)
	{
		return RecordingStatus_Unknown;
	}

	return RecordingStatus_Unknown;
}

HT_API const char * onvif_SearchStateToString(onvif_SearchState state)
{
	switch (state)
	{
	case SearchState_Queued:
		return "Queued";

	case SearchState_Searching:
		return "Searching";

	case SearchState_Completed:
		return "Completed";	

	case SearchState_Unknown:
		return "Unknown";
	}

	return "Queued";
}

HT_API onvif_SearchState onvif_StringToSearchState(const char * str)
{
	if (strcasecmp(str, "Queued") == 0)
	{
		return SearchState_Queued;
	}
	else if (strcasecmp(str, "Searching") == 0)
	{
		return SearchState_Searching;
	}
	else if (strcasecmp(str, "Completed") == 0)
	{
		return SearchState_Completed;
	}
	else if (strcasecmp(str, "Unknown") == 0)
	{
		return SearchState_Unknown;
	}

	return SearchState_Unknown;
}

HT_API const char * onvif_RotateModeToString(onvif_RotateMode mode)
{    
	switch (mode)
	{
	case RotateMode_OFF:
		return "OFF";

	case RotateMode_ON:
		return "ON";

	case RotateMode_AUTO:
		return "AUTO";
	}

	return "OFF";
}

HT_API onvif_RotateMode onvif_StringToRotateMode(const char * str)
{
	if (strcasecmp(str, "OFF") == 0)
	{
		return RotateMode_OFF;
	}
	else if (strcasecmp(str, "ON") == 0)
	{
		return RotateMode_ON;
	}
	else if (strcasecmp(str, "AUTO") == 0)
	{
		return RotateMode_AUTO;
	}

	return RotateMode_OFF;
}

HT_API const char * onvif_ScopeDefinitionToString(onvif_ScopeDefinition def)
{
	switch (def)
	{
	case ScopeDefinition_Fixed:
		return "Fixed";

	case ScopeDefinition_Configurable:
		return "Configurable";
	}

	return "Configurable";
}

HT_API onvif_ScopeDefinition onvif_StringToScopeDefinition(const char * str)
{
	if (strcasecmp(str, "Fixed") == 0)
	{
		return ScopeDefinition_Fixed;
	}
	else if (strcasecmp(str, "Configurable") == 0)
	{
		return ScopeDefinition_Configurable;
	}

	return ScopeDefinition_Configurable;
}

HT_API const char * onvif_Dot11AuthAndMangementSuiteToString(onvif_Dot11AuthAndMangementSuite req)
{
    switch (req)
	{
	case Dot11AuthAndMangementSuite_None:
		return "None";

	case Dot11AuthAndMangementSuite_Dot1X:
		return "Dot1X";

	case Dot11AuthAndMangementSuite_PSK:
		return "PSK";

	case Dot11AuthAndMangementSuite_Extended:
		return "Extended";	
	}

	return "None";
}

HT_API onvif_Dot11AuthAndMangementSuite onvif_StringToDot11AuthAndMangementSuite(const char * str)
{
    if (strcasecmp(str, "None") == 0)
	{
		return Dot11AuthAndMangementSuite_None;
	}
	else if (strcasecmp(str, "Dot1X") == 0)
	{
		return Dot11AuthAndMangementSuite_Dot1X;
	}
	else if (strcasecmp(str, "PSK") == 0)
	{
		return Dot11AuthAndMangementSuite_PSK;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return Dot11AuthAndMangementSuite_Extended;
	}

	return Dot11AuthAndMangementSuite_None;
}

HT_API const char * onvif_Dot11CipherToString(onvif_Dot11Cipher req)
{
    switch (req)
	{
	case Dot11Cipher_CCMP:
		return "CCMP";

	case Dot11Cipher_TKIP:
		return "TKIP";

	case Dot11Cipher_Any:
		return "Any";

	case Dot11Cipher_Extended:
		return "Extended";	
	}

	return "CCMP";
}

HT_API onvif_Dot11Cipher onvif_StringToDot11Cipher(const char * str)
{
    if (strcasecmp(str, "CCMP") == 0)
	{
		return Dot11Cipher_CCMP;
	}
	else if (strcasecmp(str, "TKIP") == 0)
	{
		return Dot11Cipher_TKIP;
	}
	else if (strcasecmp(str, "Any") == 0)
	{
		return Dot11Cipher_Any;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return Dot11Cipher_Extended;
	}

	return Dot11Cipher_CCMP;
}

HT_API const char * onvif_Dot11SignalStrengthToString(onvif_Dot11SignalStrength req)
{
    switch (req)
	{
	case Dot11SignalStrength_None:
		return "None";

	case Dot11SignalStrength_VeryBad:
		return "Very Bad";

	case Dot11SignalStrength_Bad:
		return "Bad";

	case Dot11SignalStrength_Good:
		return "Good";	

	case Dot11SignalStrength_VeryGood:
		return "Very Good";

	case Dot11SignalStrength_Extended:
		return "Extended";	
	}

	return "None";
}

HT_API onvif_Dot11SignalStrength onvif_StringToDot11SignalStrength(const char * str)
{
    if (strcasecmp(str, "None") == 0)
	{
		return Dot11SignalStrength_None;
	}
	else if (strcasecmp(str, "Very Bad") == 0)
	{
		return Dot11SignalStrength_VeryBad;
	}
	else if (strcasecmp(str, "Bad") == 0)
	{
		return Dot11SignalStrength_Bad;
	}
	else if (strcasecmp(str, "Good") == 0)
	{
		return Dot11SignalStrength_Good;
	}
	else if (strcasecmp(str, "Very Good") == 0)
	{
		return Dot11SignalStrength_VeryGood;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return Dot11SignalStrength_Extended;
	}

	return Dot11SignalStrength_None;
}

HT_API const char * onvif_Dot11StationModeToString(onvif_Dot11StationMode req)
{
    switch (req)
	{
	case Dot11StationMode_Ad_hoc:
		return "Ad-hoc";

	case Dot11StationMode_Infrastructure:
		return "Infrastructure";

	case Dot11StationMode_Extended:
		return "Extended";	
	}

	return "Ad-hoc";
}

HT_API onvif_Dot11StationMode onvif_StringToDot11StationMode(const char * str)
{
    if (strcasecmp(str, "Ad-hoc") == 0)
	{
		return Dot11StationMode_Ad_hoc;
	}
	else if (strcasecmp(str, "Infrastructure") == 0)
	{
		return Dot11StationMode_Infrastructure;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return Dot11StationMode_Extended;
	}

	return Dot11StationMode_Ad_hoc;
}

HT_API const char * onvif_Dot11SecurityModeToString(onvif_Dot11SecurityMode req)
{    
    switch (req)
	{
	case Dot11SecurityMode_None:
		return "None";

	case Dot11SecurityMode_WEP:
		return "WEP";

	case Dot11SecurityMode_PSK:
		return "PSK";

	case Dot11SecurityMode_Dot1X:
		return "Dot1X";	

	case Dot11SecurityMode_Extended:
		return "Extended";
	}

	return "None";
}

HT_API onvif_Dot11SecurityMode onvif_StringToDot11SecurityMode(const char * str)
{
    if (strcasecmp(str, "None") == 0)
	{
		return Dot11SecurityMode_None;
	}
	else if (strcasecmp(str, "WEP") == 0)
	{
		return Dot11SecurityMode_WEP;
	}
	else if (strcasecmp(str, "PSK") == 0)
	{
		return Dot11SecurityMode_PSK;
	}
	else if (strcasecmp(str, "Dot1X") == 0)
	{
		return Dot11SecurityMode_Dot1X;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return Dot11SecurityMode_Extended;
	}

	return Dot11SecurityMode_None;
}

#ifdef PROFILE_C_SUPPORT

HT_API const char * onvif_DoorPhysicalStateToString(onvif_DoorPhysicalState state)
{
    switch (state)
	{
	case DoorPhysicalState_Unknown:
		return "Unknown";

	case DoorPhysicalState_Open:
		return "Open";

	case DoorPhysicalState_Closed:
		return "Closed";	

	case DoorPhysicalState_Fault:
		return "Fault";
	}

	return "Unknown";
}

HT_API onvif_DoorPhysicalState	onvif_StringToDoorPhysicalState(const char * str)
{
    if (strcasecmp(str, "Unknown") == 0)
	{
		return DoorPhysicalState_Unknown;
	}
	else if (strcasecmp(str, "Open") == 0)
	{
		return DoorPhysicalState_Open;
	}
	else if (strcasecmp(str, "Closed") == 0)
	{
		return DoorPhysicalState_Closed;
	}
	else if (strcasecmp(str, "Fault") == 0)
	{
		return DoorPhysicalState_Fault;
	}

	return DoorPhysicalState_Unknown;
}

HT_API const char * onvif_LockPhysicalStateToString(onvif_LockPhysicalState state)
{    
    switch (state)
	{
	case LockPhysicalState_Unknown:
		return "Unknown";

	case LockPhysicalState_Locked:
		return "Locked";

	case LockPhysicalState_Unlocked:
		return "Unlocked";	

	case LockPhysicalState_Fault:
		return "Fault";
	}

	return "Unknown";
}

HT_API onvif_LockPhysicalState	onvif_StringToLockPhysicalState(const char * str)
{
    if (strcasecmp(str, "Unknown") == 0)
	{
		return LockPhysicalState_Unknown;
	}
	else if (strcasecmp(str, "Locked") == 0)
	{
		return LockPhysicalState_Locked;
	}
	else if (strcasecmp(str, "Unlocked") == 0)
	{
		return LockPhysicalState_Unlocked;
	}
	else if (strcasecmp(str, "Fault") == 0)
	{
		return LockPhysicalState_Fault;
	}

	return LockPhysicalState_Unknown;
}

HT_API const char * onvif_DoorAlarmStateToString(onvif_DoorAlarmState state)
{
    switch (state)
	{
	case DoorAlarmState_Normal:
		return "Normal";

	case DoorAlarmState_DoorForcedOpen:
		return "DoorForcedOpen";

	case DoorAlarmState_DoorOpenTooLong:
		return "DoorOpenTooLong";	
	}

	return "Normal";
}

HT_API onvif_DoorAlarmState onvif_StringToDoorAlarmState(const char * str)
{
    if (strcasecmp(str, "Normal") == 0)
	{
		return DoorAlarmState_Normal;
	}
	else if (strcasecmp(str, "DoorForcedOpen") == 0)
	{
		return DoorAlarmState_DoorForcedOpen;
	}
	else if (strcasecmp(str, "DoorOpenTooLong") == 0)
	{
		return DoorAlarmState_DoorOpenTooLong;
	}

	return DoorAlarmState_Normal;
}

HT_API const char * onvif_DoorTamperStateToString(onvif_DoorTamperState state)
{
    switch (state)
	{
	case DoorTamperState_Unknown:
		return "Unknown";

	case DoorTamperState_NotInTamper:
		return "NotInTamper";

	case DoorTamperState_TamperDetected:
		return "TamperDetected";	
	}

	return "Unknown";
}

HT_API onvif_DoorTamperState onvif_StringToDoorTamperState(const char * str)
{
    if (strcasecmp(str, "Unknown") == 0)
	{
		return DoorTamperState_Unknown;
	}
	else if (strcasecmp(str, "NotInTamper") == 0)
	{
		return DoorTamperState_NotInTamper;
	}
	else if (strcasecmp(str, "TamperDetected") == 0)
	{
		return DoorTamperState_TamperDetected;
	}

	return DoorTamperState_Unknown;
}

HT_API const char * onvif_DoorFaultStateToString(onvif_DoorFaultState state)
{
    switch (state)
	{
	case DoorFaultState_Unknown:
		return "Unknown";

	case DoorFaultState_NotInFault:
		return "NotInFault";

	case DoorFaultState_FaultDetected:
		return "FaultDetected";	
	}

	return "Unknown";
}

HT_API onvif_DoorFaultState onvif_StringToDoorFaultState(const char * str)
{
    if (strcasecmp(str, "Unknown") == 0)
	{
		return DoorFaultState_Unknown;
	}
	else if (strcasecmp(str, "NotInFault") == 0)
	{
		return DoorFaultState_NotInFault;
	}
	else if (strcasecmp(str, "FaultDetected") == 0)
	{
		return DoorFaultState_FaultDetected;
	}

	return DoorFaultState_Unknown;
}

HT_API const char * onvif_DoorModeToString(onvif_DoorMode mode)
{
    switch (mode)
	{
	case DoorMode_Unknown:
		return "Unknown";

	case DoorMode_Locked:
		return "Locked";

	case DoorMode_Unlocked:
		return "Unlocked";	

	case DoorMode_Accessed:
		return "Accessed";

	case DoorMode_Blocked:
		return "Blocked";

	case DoorMode_LockedDown:
		return "LockedDown";	

	case DoorMode_LockedOpen:
		return "LockedOpen";	

	case DoorMode_DoubleLocked:
		return "DoubleLocked";		
	}

	return "Unknown";
}

HT_API onvif_DoorMode onvif_StringToDoorMode(const char * str)
{
    if (strcasecmp(str, "Unknown") == 0)
	{
		return DoorMode_Unknown;
	}
	else if (strcasecmp(str, "Locked") == 0)
	{
		return DoorMode_Locked;
	}
	else if (strcasecmp(str, "Unlocked") == 0)
	{
		return DoorMode_Unlocked;
	}
	else if (strcasecmp(str, "Accessed") == 0)
	{
		return DoorMode_Accessed;
	}
	else if (strcasecmp(str, "Blocked") == 0)
	{
		return DoorMode_Blocked;
	}
	else if (strcasecmp(str, "LockedDown") == 0)
	{
		return DoorMode_LockedDown;
	}
	else if (strcasecmp(str, "LockedOpen") == 0)
	{
		return DoorMode_LockedOpen;
	}
	else if (strcasecmp(str, "DoubleLocked") == 0)
	{
		return DoorMode_DoubleLocked;
	}

	return DoorMode_Unknown;
}

#endif // end of PROFILE_C_SUPPORT

#ifdef DEVICEIO_SUPPORT

HT_API const char * onvif_RelayModeToString(onvif_RelayMode mode)
{
    switch (mode)
	{
	case RelayMode_Monostable:
		return "Monostable";

	case RelayMode_Bistable:
		return "Bistable";		
	}

	return "Bistable";
}

HT_API onvif_RelayMode onvif_StringToRelayMode(const char * str)
{
    if (strcasecmp(str, "Monostable") == 0)
	{
		return RelayMode_Monostable;
	}
	else if (strcasecmp(str, "Bistable") == 0)
	{
		return RelayMode_Bistable;
	}

	return RelayMode_Bistable;
}

HT_API const char * onvif_RelayIdleStateToString(onvif_RelayIdleState state)
{
    switch (state)
	{
	case RelayIdleState_closed:
		return "closed";

	case RelayIdleState_open:
		return "open";		
	}

	return "closed";
}

HT_API onvif_RelayIdleState onvif_StringToRelayIdleState(const char * str)
{
    if (strcasecmp(str, "closed") == 0)
	{
		return RelayIdleState_closed;
	}
	else if (strcasecmp(str, "open") == 0)
	{
		return RelayIdleState_open;
	}

	return RelayIdleState_closed;
}

HT_API const char * onvif_RelayLogicalStateToString(onvif_RelayLogicalState state)
{
    switch (state)
	{
	case RelayLogicalState_active:
		return "active";

	case RelayLogicalState_inactive:
		return "inactive";		
	}

	return "inactive";
}

HT_API onvif_RelayLogicalState onvif_StringToRelayLogicalState(const char * str)
{
    if (strcasecmp(str, "active") == 0)
	{
		return RelayLogicalState_active;
	}
	else if (strcasecmp(str, "inactive") == 0)
	{
		return RelayLogicalState_inactive;
	}

	return RelayLogicalState_inactive;
}

HT_API const char * onvif_DigitalIdleStateToString(onvif_DigitalIdleState state)
{
    switch (state)
	{
	case DigitalIdleState_closed:
		return "closed";

	case DigitalIdleState_open:
		return "open";		
	}

	return "closed";
}

HT_API onvif_DigitalIdleState onvif_StringToDigitalIdleState(const char * str)
{
    if (strcasecmp(str, "closed") == 0)
	{
		return DigitalIdleState_closed;
	}
	else if (strcasecmp(str, "open") == 0)
	{
		return DigitalIdleState_open;
	}

	return DigitalIdleState_closed;
}

HT_API const char * onvif_ParityBitToString(onvif_ParityBit type)
{
    switch (type)
	{
	case ParityBit_None:
		return "None";

	case ParityBit_Even:
		return "Even";

	case ParityBit_Odd:
		return "Odd";

	case ParityBit_Mark:
		return "Mark";	

	case ParityBit_Space:
		return "Space";

	case ParityBit_Extended:
		return "Extended";	
	}

	return "None";
}

HT_API onvif_ParityBit onvif_StringToParityBit(const char * str)
{
    if (strcasecmp(str, "None") == 0)
	{
		return ParityBit_None;
	}
	else if (strcasecmp(str, "Even") == 0)
	{
		return ParityBit_Even;
	}
	else if (strcasecmp(str, "Odd") == 0)
	{
		return ParityBit_Odd;
	}
	else if (strcasecmp(str, "Mark") == 0)
	{
		return ParityBit_Mark;
	}
	else if (strcasecmp(str, "Space") == 0)
	{
		return ParityBit_Space;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return ParityBit_Extended;
	}

	return ParityBit_None;
}

HT_API const char * onvif_SerialPortTypeToString(onvif_SerialPortType type)
{
    switch (type)
	{
	case SerialPortType_RS232:
		return "RS232";

	case SerialPortType_RS422HalfDuplex:
		return "RS422HalfDuplex";

	case SerialPortType_RS422FullDuplex:
		return "RS422FullDuplex";

	case SerialPortType_RS485HalfDuplex:
		return "RS485HalfDuplex";	

	case SerialPortType_RS485FullDuplex:
		return "RS485FullDuplex";

	case SerialPortType_Generic:
		return "Generic";	
	}

	return "RS232";
}

HT_API onvif_SerialPortType onvif_StringToSerialPortType(const char * str)
{
    if (strcasecmp(str, "RS232") == 0)
	{
		return SerialPortType_RS232;
	}
	else if (strcasecmp(str, "RS422HalfDuplex") == 0)
	{
		return SerialPortType_RS422HalfDuplex;
	}
	else if (strcasecmp(str, "RS422FullDuplex") == 0)
	{
		return SerialPortType_RS422FullDuplex;
	}
	else if (strcasecmp(str, "RS485HalfDuplex") == 0)
	{
		return SerialPortType_RS485HalfDuplex;
	}
	else if (strcasecmp(str, "RS485HalfDuplex") == 0)
	{
		return SerialPortType_RS485FullDuplex;
	}
	else if (strcasecmp(str, "Generic") == 0)
	{
		return SerialPortType_Generic;
	}

	return SerialPortType_RS232;
}

#endif // end of DEVICEIO_SUPPORT

#ifdef THERMAL_SUPPORT

HT_API const char * onvif_PolarityToString(onvif_Polarity type)
{
    switch (type)
	{
	case Polarity_WhiteHot:
		return "WhiteHot";

	case Polarity_BlackHot:
		return "BlackHot";
	}

	return "WhiteHot";
}

HT_API onvif_Polarity onvif_StringToPolarity(const char * str)
{
    if (strcasecmp(str, "WhiteHot") == 0)
	{
		return Polarity_WhiteHot;
	}
	else if (strcasecmp(str, "BlackHot") == 0)
	{
		return Polarity_BlackHot;
	}

	return Polarity_WhiteHot;
}

#endif // end of THERMAL_SUPPORT

#ifdef RECEIVER_SUPPORT

HT_API const char *  onvif_ReceiverModeToString(onvif_ReceiverMode mode)
{
    switch (mode)
	{
	case ReceiverMode_AutoConnect:
		return "AutoConnect";

	case ReceiverMode_AlwaysConnect:
		return "AlwaysConnect";

	case ReceiverMode_NeverConnect:
		return "NeverConnect";

	case ReceiverMode_Unknown:
		return "Unknown";	
	}

	return "Unknown";
}

HT_API onvif_ReceiverMode onvif_StringToReceiverMode(const char * str)
{
    if (strcasecmp(str, "AutoConnect") == 0)
	{
		return ReceiverMode_AutoConnect;
	}
	else if (strcasecmp(str, "AlwaysConnect") == 0)
	{
		return ReceiverMode_AlwaysConnect;
	}
	else if (strcasecmp(str, "NeverConnect") == 0)
	{
		return ReceiverMode_NeverConnect;
	}
	else if (strcasecmp(str, "Unknown") == 0)
	{
		return ReceiverMode_Unknown;
	}

	return ReceiverMode_Unknown;
}

HT_API const char * onvif_ReceiverStateToString(onvif_ReceiverState state)
{
    switch (state)
	{
	case ReceiverState_NotConnected:
		return "NotConnected";

	case ReceiverState_Connecting:
		return "Connecting";

	case ReceiverState_Connected:
		return "Connected";

	case ReceiverState_Unknown:
		return "Unknown";	
	}

	return "Unknown";
}

HT_API onvif_ReceiverState onvif_StringToReceiverState(const char * str)
{
    if (strcasecmp(str, "NotConnected") == 0)
	{
		return ReceiverState_NotConnected;
	}
	else if (strcasecmp(str, "Connecting") == 0)
	{
		return ReceiverState_Connecting;
	}
	else if (strcasecmp(str, "Connected") == 0)
	{
		return ReceiverState_Connected;
	}
	else if (strcasecmp(str, "Unknown") == 0)
	{
		return ReceiverState_Unknown;
	}

	return ReceiverState_Unknown;
}

#endif // end of RECEIVER_SUPPORT

#ifdef PROVISIONING_SUPPORT

HT_API const char * onvif_PanDirectionToString(onvif_PanDirection dir)
{
    switch (dir)
	{
	case PanDirection_Left:
		return "Left";

	case PanDirection_Right:
		return "Right";	
	}

	return "Left";
}

HT_API onvif_PanDirection onvif_StringToPanDirection(const char * str)
{
    if (strcasecmp(str, "Left") == 0)
	{
		return PanDirection_Left;
	}
	else if (strcasecmp(str, "Right") == 0)
	{
		return PanDirection_Right;
	}

	return PanDirection_Left;
}

HT_API const char * onvif_TiltDirectionToString(onvif_TiltDirection dir)
{
    switch (dir)
	{
	case TiltDirection_Up:
		return "Up";

	case TiltDirection_Down:
		return "Down";	
	}

	return "Up";
}

HT_API onvif_TiltDirection onvif_StringToTiltDirection(const char * str)
{
    if (strcasecmp(str, "Up") == 0)
	{
		return TiltDirection_Up;
	}
	else if (strcasecmp(str, "Down") == 0)
	{
		return TiltDirection_Down;
	}

	return TiltDirection_Up;
}

HT_API const char * onvif_ZoomDirectionToString(onvif_ZoomDirection dir)
{
    switch (dir)
	{
	case ZoomDirection_Wide:
		return "Wide";

	case ZoomDirection_Telephoto:
		return "Telephoto";	
	}

	return "Wide";
}

HT_API onvif_ZoomDirection onvif_StringToZoomDirection(const char * str)
{
    if (strcasecmp(str, "Wide") == 0)
	{
		return ZoomDirection_Wide;
	}
	else if (strcasecmp(str, "Telephoto") == 0)
	{
		return ZoomDirection_Telephoto;
	}

	return ZoomDirection_Wide;
}

HT_API const char * onvif_RollDirectionToString(onvif_RollDirection dir)
{
    switch (dir)
	{
	case RollDirection_Clockwise:
		return "Clockwise";

	case RollDirection_Counterclockwise:
		return "Counterclockwise";

	case RollDirection_Auto:
	    return "Auto";
	}

	return "Clockwise";
}

HT_API onvif_RollDirection onvif_StringToRollDirection(const char * str)
{
    if (strcasecmp(str, "Clockwise") == 0)
	{
		return RollDirection_Clockwise;
	}
	else if (strcasecmp(str, "Counterclockwise") == 0)
	{
		return RollDirection_Counterclockwise;
	}
	else if (strcasecmp(str, "Auto") == 0)
	{
	    return RollDirection_Auto;
	}

	return RollDirection_Clockwise;
}

HT_API const char * onvif_FocusDirectionToString(onvif_FocusDirection dir)
{
    switch (dir)
	{
	case FocusDirection_Near:
		return "Near";

	case FocusDirection_Far:
		return "Far";

	case FocusDirection_Auto:
	    return "Auto";
	}

	return "Near";
}

HT_API onvif_FocusDirection onvif_StringToFocusDirection(const char * str)
{
    if (strcasecmp(str, "Near") == 0)
	{
		return FocusDirection_Near;
	}
	else if (strcasecmp(str, "Far") == 0)
	{
		return FocusDirection_Far;
	}
	else if (strcasecmp(str, "Auto") == 0)
	{
	    return FocusDirection_Auto;
	}

	return FocusDirection_Auto;
}

#endif // end of PROVISIONING_SUPPORT



