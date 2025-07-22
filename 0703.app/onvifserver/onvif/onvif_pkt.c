

/***************************************************************************************/
#include "sys_inc.h"
#include "onvif.h"
#include "xml_node.h"
#include "onvif_device.h"
#include "onvif_pkt.h"
#include "onvif_event.h"
#include "onvif_ptz.h"
#include "onvif_utils.h"
#include "onvif_err.h"
#include "onvif_media.h"
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


#if __WINDOWS_OS__
#pragma warning(disable:4996)
#endif

/***************************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;

extern char xml_hdr[];
extern char onvif_xmlns[];
extern char soap_head[];
extern char soap_body[];
extern char soap_tailer[];

/***************************************************************************************/

int bderr_rly_xml
(
char * p_buf, 
int mlen, 
const char * code, 
const char * subcode, 
const char * subcode_ex, 
const char * reason,
const char * action
)
{
	int offset = snprintf(p_buf, mlen, "%s", xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, "%s", onvif_xmlns);

	if (action)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, soap_head, action);
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "%s", soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<s:Fault>");
    offset += snprintf(p_buf+offset, mlen-offset, "<s:Code>");    
	offset += snprintf(p_buf+offset, mlen-offset, "<s:Value>%s</s:Value>", code);

	if (subcode)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<s:Subcode>");
	    offset += snprintf(p_buf+offset, mlen-offset, "<s:Value>%s</s:Value>", subcode);

	    if (subcode_ex)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
                "<s:Subcode><s:Value>%s</s:Value></s:Subcode>", 
                subcode_ex);
        }

        offset += snprintf(p_buf+offset, mlen-offset, "</s:Subcode>");
	}
    
    offset += snprintf(p_buf+offset, mlen-offset, "</s:Code>");

    if (reason)
    {
	    offset += snprintf(p_buf+offset, mlen-offset, 
	        "<s:Reason><s:Text xml:lang=\"en\">%s</s:Text></s:Reason>", 
	        reason);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</s:Fault>");
    
	offset += snprintf(p_buf+offset, mlen-offset, "%s", soap_tailer);
	
	return offset;
}

/***************************************************************************************/

int bdDeviceCapabilities_xml(char * p_buf, int mlen)
{
#ifdef DEVICEIO_SUPPORT
    int i;
#endif

	int offset = 0;
	
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Device>"
    	"<tt:XAddr>%s</tt:XAddr>",
    	g_onvif_cfg.Capabilities.device.XAddr);    	
		    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Network>"
		    "<tt:ZeroConfiguration>%s</tt:ZeroConfiguration>"
		    "<tt:IPVersion6>%s</tt:IPVersion6>"
		    "<tt:Extension>"
		    	"<tt:Dot11Configuration>%s</tt:Dot11Configuration>"       
  			"</tt:Extension>"
	    "</tt:Network>",	    
    	g_onvif_cfg.Capabilities.device.ZeroConfiguration ? "true" : "false", "false",
    	g_onvif_cfg.Capabilities.device.Dot11Configuration ? "true" : "false");

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:System>"
		    "<tt:DiscoveryResolve>%s</tt:DiscoveryResolve>"
		    "<tt:DiscoveryBye>%s</tt:DiscoveryBye>"
		    "<tt:RemoteDiscovery>false</tt:RemoteDiscovery>"
		    "<tt:SystemBackup>false</tt:SystemBackup>"
		    "<tt:SystemLogging>%s</tt:SystemLogging>"
		    "<tt:FirmwareUpgrade>false</tt:FirmwareUpgrade>"
		    "<tt:SupportedVersions>"
			    "<tt:Major>2</tt:Major>"
			    "<tt:Minor>4</tt:Minor>"
		    "</tt:SupportedVersions>"
		    "<tt:Extension>"
				"<tt:HttpFirmwareUpgrade>false</tt:HttpFirmwareUpgrade>"
				"<tt:HttpSystemBackup>%s</tt:HttpSystemBackup>"
				"<tt:HttpSystemLogging>false</tt:HttpSystemLogging>"
				"<tt:HttpSupportInformation>true</tt:HttpSupportInformation>"
			"</tt:Extension>"
	    "</tt:System>",
	    g_onvif_cfg.Capabilities.device.DiscoveryResolve ? "true" : "false",
	    g_onvif_cfg.Capabilities.device.DiscoveryBye ? "true" : "false",
	    g_onvif_cfg.Capabilities.device.SystemLogging ? "true" : "false",
	    g_onvif_cfg.Capabilities.device.HttpSystemBackup ? "true" : "false");    

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Security>"
			"<tt:TLS1.1>%s</tt:TLS1.1>"
			"<tt:TLS1.2>%s</tt:TLS1.2>"
			"<tt:OnboardKeyGeneration>%s</tt:OnboardKeyGeneration>"
			"<tt:AccessPolicyConfig>%s</tt:AccessPolicyConfig>"
			"<tt:X.509Token>%s</tt:X.509Token>"
			"<tt:SAMLToken>%s</tt:SAMLToken>"
			"<tt:KerberosToken>%s</tt:KerberosToken>"
			"<tt:RELToken>%s</tt:RELToken>"
			"<tt:Extension>"
				"<tt:TLS1.0>%s</tt:TLS1.0>"
				"<tt:Extension>"
					"<tt:Dot1X>%s</tt:Dot1X>"					
					"<tt:SupportedEAPMethod>%d</tt:SupportedEAPMethod>"
					"<tt:RemoteUserHandling>%s</tt:RemoteUserHandling>"
				"</tt:Extension>"
			"</tt:Extension>"	
		"</tt:Security>",
		g_onvif_cfg.Capabilities.device.TLS11 ? "true" : "false",
		g_onvif_cfg.Capabilities.device.TLS12 ? "true" : "false",
		g_onvif_cfg.Capabilities.device.OnboardKeyGeneration ? "true" : "false",
		g_onvif_cfg.Capabilities.device.AccessPolicyConfig ? "true" : "false",
		g_onvif_cfg.Capabilities.device.X509Token ? "true" : "false",
		g_onvif_cfg.Capabilities.device.SAMLToken ? "true" : "false",
		g_onvif_cfg.Capabilities.device.KerberosToken ? "true" : "false",
		g_onvif_cfg.Capabilities.device.RELToken ? "true" : "false",
		g_onvif_cfg.Capabilities.device.TLS10 ? "true" : "false",
		g_onvif_cfg.Capabilities.device.Dot1X ? "true" : "false",
		g_onvif_cfg.Capabilities.device.SupportedEAPMethods,
		g_onvif_cfg.Capabilities.device.RemoteUserHandling ? "true" : "false");
		
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Device>");
	
	return offset;    	
}

int bdDeviceServicesCapabilities_xml(char * p_buf, int mlen)
{
    int i;
	int offset = 0;



#if 0
#else 

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>"
			"<tds:Network IPFilter=\"false\" ZeroConfiguration=\"%s\" "
			"IPVersion6=\"%s\" DynDNS=\"%s\" Dot11Configuration=\"%s\" "
			"Dot1XConfigurations=\"%d\" HostnameFromDHCP=\"%s\" NTP=\"%d\" DHCPv6=\"%s\"/>",
		g_onvif_cfg.Capabilities.device.ZeroConfiguration ? "true" : "false",
		g_onvif_cfg.Capabilities.device.IPVersion6 ? "true" : "false",
		g_onvif_cfg.Capabilities.device.DynDNS ? "true" : "false",
		g_onvif_cfg.Capabilities.device.Dot11Configuration ? "true" : "false",
		g_onvif_cfg.Capabilities.device.Dot1XConfigurations,
		g_onvif_cfg.Capabilities.device.HostnameFromDHCP ? "true" : "false",
		g_onvif_cfg.Capabilities.device.NTP,
		g_onvif_cfg.Capabilities.device.DHCPv6 ? "true" : "false");


	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:Security TLS1.0=\"%s\" TLS1.1=\"%s\" TLS1.2=\"%s\" "
			"OnboardKeyGeneration=\"%s\" AccessPolicyConfig=\"%s\" DefaultAccessPolicy=\"%s\" "
			"Dot1X=\"%s\" RemoteUserHandling=\"%s\" X.509Token=\"%s\" SAMLToken=\"%s\" "
			"KerberosToken=\"%s\" UsernameToken=\"%s\" HttpDigest=\"%s\" RELToken=\"%s\" "
			"SupportedEAPMethods=\"%d\" MaxUsers=\"16\" "
			"MaxUserNameLength=\"64\" MaxPasswordLength=\"64\">"
		"</tds:Security>",
		g_onvif_cfg.Capabilities.device.TLS10 ? "true" : "false",
		g_onvif_cfg.Capabilities.device.TLS11 ? "true" : "false",
		g_onvif_cfg.Capabilities.device.TLS12 ? "true" : "false",
		g_onvif_cfg.Capabilities.device.OnboardKeyGeneration ? "true" : "false",
		g_onvif_cfg.Capabilities.device.AccessPolicyConfig ? "true" : "false",
		g_onvif_cfg.Capabilities.device.DefaultAccessPolicy ? "true" : "false",
		g_onvif_cfg.Capabilities.device.Dot1X ? "true" : "false",
		g_onvif_cfg.Capabilities.device.RemoteUserHandling ? "true" : "false",
		g_onvif_cfg.Capabilities.device.X509Token ? "true" : "false",
		g_onvif_cfg.Capabilities.device.SAMLToken ? "true" : "false",
		g_onvif_cfg.Capabilities.device.KerberosToken ? "true" : "false",
		g_onvif_cfg.Capabilities.device.UsernameToken ? "true" : "false",
		g_onvif_cfg.Capabilities.device.HttpDigest ? "true" : "false",
		g_onvif_cfg.Capabilities.device.RELToken ? "true" : "false",
		g_onvif_cfg.Capabilities.device.SupportedEAPMethods);

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:System DiscoveryResolve=\"%s\" DiscoveryBye=\"%s\" "
			"RemoteDiscovery=\"false\" SystemBackup=\"false\" SystemLogging=\"%s\" "
			"FirmwareUpgrade=\"false\" HttpFirmwareUpgrade=\"false\" HttpSystemBackup=\"false\" "
			"HttpSystemLogging=\"false\" HttpSupportInformation=\"true\" StorageConfiguration=\"false\" "
			"MaxStorageConfigurations=\"%d\" GeoLocationEntries=\"%d\" AutoGeo=\"%s\" StorageTypesSupported=\"\">"
		"</tds:System>",
		g_onvif_cfg.Capabilities.device.DiscoveryResolve? "true" : "false",
		g_onvif_cfg.Capabilities.device.DiscoveryBye? "true" : "false",
		g_onvif_cfg.Capabilities.device.SystemLogging? "true" : "false",
		g_onvif_cfg.Capabilities.device.MaxStorageConfigurations,
		g_onvif_cfg.Capabilities.device.GeoLocationEntries,
		g_onvif_cfg.Capabilities.device.AutoGeo);

    offset += snprintf(p_buf+offset, mlen-offset, "<tds:Misc AuxiliaryCommands=\"");
    for (i = 0; i < g_onvif_cfg.Capabilities.device.sizeAuxiliaryCommands; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "%s ", g_onvif_cfg.Capabilities.device.AuxiliaryCommands[i]);
    }
    offset += snprintf(p_buf+offset, mlen-offset, "\"></tds:Misc>");
    

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
#endif

	return offset;
}

int bdEventsCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;	
     
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Events>"
		    "<tt:XAddr>%s</tt:XAddr>"
		    "<tt:WSSubscriptionPolicySupport>%s</tt:WSSubscriptionPolicySupport>"
		    "<tt:WSPullPointSupport>%s</tt:WSPullPointSupport>"
		    "<tt:WSPausableSubscriptionManagerInterfaceSupport>%s</tt:WSPausableSubscriptionManagerInterfaceSupport>"
	    "</tt:Events>",		    
    	g_onvif_cfg.Capabilities.events.XAddr,
    	g_onvif_cfg.Capabilities.events.WSSubscriptionPolicySupport ? "true" : "false",
    	g_onvif_cfg.Capabilities.events.WSPullPointSupport ? "true" : "false",
    	g_onvif_cfg.Capabilities.events.WSPausableSubscriptionManagerInterfaceSupport ? "true" : "false");

	return offset;    	
}

int bdEventsServicesCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tev:Capabilities WSSubscriptionPolicySupport=\"%s\" WSPullPointSupport=\"%s\" "
			"WSPausableSubscriptionManagerInterfaceSupport=\"%s\" MaxNotificationProducers=\"%d\" "
			"MaxPullPoints=\"%d\" PersistentNotificationStorage=\"%s\">"
		"</tev:Capabilities>",
		g_onvif_cfg.Capabilities.events.WSSubscriptionPolicySupport ? "true" : "false",
		g_onvif_cfg.Capabilities.events.WSPullPointSupport ? "true" : "false",
		g_onvif_cfg.Capabilities.events.WSPausableSubscriptionManagerInterfaceSupport ? "true" : "false",
		g_onvif_cfg.Capabilities.events.MaxNotificationProducers,
		g_onvif_cfg.Capabilities.events.MaxPullPoints,
		g_onvif_cfg.Capabilities.events.PersistentNotificationStorage ? "true" : "false");

	return offset;
}


#ifdef MEDIA_SUPPORT

int bdMediaCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;	
     
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Media>"
		    "<tt:XAddr>%s</tt:XAddr>"
		    "<tt:StreamingCapabilities>"
			    "<tt:RTPMulticast>%s</tt:RTPMulticast>"
			    "<tt:RTP_TCP>%s</tt:RTP_TCP>"
			    "<tt:RTP_RTSP_TCP>%s</tt:RTP_RTSP_TCP>"
		    "</tt:StreamingCapabilities>"
		    "<tt:Extension>"
				"<tt:ProfileCapabilities>"
					"<tt:MaximumNumberOfProfiles>%d</tt:MaximumNumberOfProfiles>"
				"</tt:ProfileCapabilities>"
			"</tt:Extension>"
    	"</tt:Media>",		    
    	g_onvif_cfg.Capabilities.media.XAddr,
    	g_onvif_cfg.Capabilities.media.RTPMulticast ? "true" : "false",
    	g_onvif_cfg.Capabilities.media.RTP_TCP ? "true" : "false",
    	g_onvif_cfg.Capabilities.media.RTP_RTSP_TCP ? "true" : "false",
    	g_onvif_cfg.Capabilities.media.MaximumNumberOfProfiles);

	return offset;    	
}

int bdMediaServicesCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:Capabilities SnapshotUri=\"%s\" Rotation=\"false\" VideoSourceMode=\"%s\" "
		"OSD=\"%s\" TemporaryOSDText=\"%s\" EXICompression=\"%s\">",
		"false", 
		"false",
		g_onvif_cfg.Capabilities.media.VideoSourceMode ? "true" : "false", 
		"false",
		g_onvif_cfg.Capabilities.media.EXICompression ? "true" : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:ProfileCapabilities MaximumNumberOfProfiles=\"%d\" />"
		"<trt:StreamingCapabilities RTPMulticast=\"%s\" RTP_TCP=\"%s\" RTP_RTSP_TCP=\"%s\" "
			"NonAggregateControl=\"%s\" NoRTSPStreaming=\"%s\" />",
		g_onvif_cfg.Capabilities.media.MaximumNumberOfProfiles,
		g_onvif_cfg.Capabilities.media.RTPMulticast ? "true" : "false",
		g_onvif_cfg.Capabilities.media.RTP_TCP ? "true" : "false",
		g_onvif_cfg.Capabilities.media.RTP_RTSP_TCP ? "true" : "false",
		g_onvif_cfg.Capabilities.media.NonAggregateControl ? "true" : "false",
		g_onvif_cfg.Capabilities.media.NoRTSPStreaming ? "true" : "false");
			
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Capabilities>");

	return offset;
}

#endif // MEDIA_SUPPORT

#ifdef IMAGE_SUPPORT

int bdImagingCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;	
     
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Imaging>"
    		"<tt:XAddr>%s</tt:XAddr>"
    	"</tt:Imaging>",		    
    	g_onvif_cfg.Capabilities.image.XAddr);

	return offset;    	
}

int bdImagingServicesCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<timg:Capabilities ImageStabilization=\"%s\" Presets=\"%s\" AdaptablePreset=\"%s\" />",
		g_onvif_cfg.Capabilities.image.ImageStabilization ? "true" : "false", 
		g_onvif_cfg.Capabilities.image.Presets ? "true" : "false", 
		g_onvif_cfg.Capabilities.image.AdaptablePreset ? "true" : "false");

	return offset;
}

#endif // IMAGE_SUPPORT

#ifdef VIDEO_ANALYTICS

int bdAnalyticsCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;
     
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Analytics>"
    		"<tt:XAddr>%s</tt:XAddr>"
    		"<tt:RuleSupport>%s</tt:RuleSupport>"
		 	"<tt:AnalyticsModuleSupport>%s</tt:AnalyticsModuleSupport>"
    	"</tt:Analytics>",		    
    	g_onvif_cfg.Capabilities.analytics.XAddr,
    	g_onvif_cfg.Capabilities.analytics.RuleSupport ? "true" : "false",
    	g_onvif_cfg.Capabilities.analytics.AnalyticsModuleSupport ? "true" : "false");

	return offset;
}

int bdAnalyticsServicesCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tan:Capabilities RuleSupport=\"%s\" AnalyticsModuleSupport=\"%s\" "
		"CellBasedSceneDescriptionSupported=\"%s\" "
		"RuleOptionsSupported=\"%s\" AnalyticsModuleOptionsSupported=\"%s\" "
		"SupportedMetadata=\"%s\" ImageSendingType=\"%s\" />",
		g_onvif_cfg.Capabilities.analytics.RuleSupport ? "true" : "false",
		g_onvif_cfg.Capabilities.analytics.AnalyticsModuleSupport ? "true" : "false",
		g_onvif_cfg.Capabilities.analytics.CellBasedSceneDescriptionSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.analytics.RuleOptionsSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.analytics.AnalyticsModuleOptionsSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.analytics.SupportedMetadata ? "true" : "false",
		g_onvif_cfg.Capabilities.analytics.ImageSendingType);

	return offset;
}

#endif

#ifdef PROFILE_G_SUPPORT

int bdRecordingCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;	
     
    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Recording>"
			"<tt:XAddr>%s</tt:XAddr>"
			"<tt:ReceiverSource>%s</tt:ReceiverSource>"
			"<tt:MediaProfileSource>%s</tt:MediaProfileSource>"
			"<tt:DynamicRecordings>%s</tt:DynamicRecordings>"
			"<tt:DynamicTracks>%s</tt:DynamicTracks>"
			"<tt:MaxStringLength>%d</tt:MaxStringLength>"
		"</tt:Recording>",		    
    	g_onvif_cfg.Capabilities.recording.XAddr,
    	g_onvif_cfg.Capabilities.recording.ReceiverSource ? "true" : "false",
    	g_onvif_cfg.Capabilities.recording.MediaProfileSource ? "true" : "false",
    	g_onvif_cfg.Capabilities.recording.DynamicRecordings ? "true" : "false", 
    	g_onvif_cfg.Capabilities.recording.DynamicTracks ? "true" : "false",
    	g_onvif_cfg.Capabilities.recording.MaxStringLength);

	return offset;    	
}

int bdRecordingServicesCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;
	char Encoding[100];

	memset(Encoding, 0, sizeof(Encoding));
	
	if (g_onvif_cfg.Capabilities.recording.H264)
	{
		strcat(Encoding, "H264 ");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trc:Capabilities DynamicRecordings=\"%s\" DynamicTracks=\"%s\" Encoding=\"%s\" "
		"MaxRate=\"%0.1f\" MaxTotalRate=\"%0.1f\" MaxRecordings=\"%d\" MaxRecordingJobs=\"%d\" "
		"Options=\"%s\" MetadataRecording=\"%s\" SupportedExportFileFormats=\"%s\" />",
		g_onvif_cfg.Capabilities.recording.DynamicRecordings ? "true" : "false",
		g_onvif_cfg.Capabilities.recording.DynamicTracks ? "true" : "false",
		Encoding,
		g_onvif_cfg.Capabilities.recording.MaxRate,
		g_onvif_cfg.Capabilities.recording.MaxTotalRate,
		g_onvif_cfg.Capabilities.recording.MaxRecordings,
		g_onvif_cfg.Capabilities.recording.MaxRecordingJobs,
		g_onvif_cfg.Capabilities.recording.Options ? "true" : "false",
		g_onvif_cfg.Capabilities.recording.MetadataRecording ? "true" : "false",
		g_onvif_cfg.Capabilities.recording.SupportedExportFileFormats);

	return offset;
}

int bdSearchCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;	
     
    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Search>"
			"<tt:XAddr>%s</tt:XAddr>"
			"<tt:MetadataSearch>%s</tt:MetadataSearch>"
		"</tt:Search>",		    
    	g_onvif_cfg.Capabilities.search.XAddr,
    	g_onvif_cfg.Capabilities.search.MetadataSearch ? "true" : "false");

	return offset;    	
}

int bdSearchServicesCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:Capabilities MetadataSearch=\"%s\" GeneralStartEvents=\"%s\" />",
		g_onvif_cfg.Capabilities.search.MetadataSearch ? "true" : "false",
		g_onvif_cfg.Capabilities.search.GeneralStartEvents ? "true" : "false");

	return offset;
}

int bdReplayCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;	
     
    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Replay>"
			"<tt:XAddr>%s</tt:XAddr>"
		"</tt:Replay>",		    
    	g_onvif_cfg.Capabilities.replay.XAddr);

	return offset;
}

int bdReplayServicesCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trp:Capabilities ReversePlayback=\"%s\" SessionTimeoutRange=\"%0.1f %0.1f\" "
		"RTP_RTSP_TCP=\"%s\" RTSPWebSocketUri=\"%s\" />",
		g_onvif_cfg.Capabilities.replay.ReversePlayback ? "true" : "false",
		g_onvif_cfg.Capabilities.replay.SessionTimeoutRange.Min,
		g_onvif_cfg.Capabilities.replay.SessionTimeoutRange.Max,
		g_onvif_cfg.Capabilities.replay.RTP_RTSP_TCP ? "true" : "false",
		g_onvif_cfg.Capabilities.replay.RTSPWebSocketUri);

	return offset;
}

#endif // end of PROFILE_G_SUPPORT


#ifdef RECEIVER_SUPPORT

int bdReceiverCapabilities_xml(char * p_buf, int mlen)
{
    int offset = 0;	
     
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Receiver>");
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:XAddr>%s</tt:XAddr>", 
    	g_onvif_cfg.Capabilities.receiver.XAddr);
    	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:RTP_Multicast>%s</tt:RTP_Multicast>"
		"<tt:RTP_TCP>%s</tt:RTP_TCP>"
		"<tt:RTP_RTSP_TCP>%s</tt:RTP_RTSP_TCP>"
		"<tt:SupportedReceivers>%d</tt:SupportedReceivers>"
		"<tt:MaximumRTSPURILength>%d</tt:MaximumRTSPURILength>",
		g_onvif_cfg.Capabilities.receiver.RTP_USCOREMulticast ? "true" : "false",
		g_onvif_cfg.Capabilities.receiver.RTP_USCORETCP ? "true" : "false",
		g_onvif_cfg.Capabilities.receiver.RTP_USCORERTSP_USCORETCP ? "true" : "false",
		g_onvif_cfg.Capabilities.receiver.SupportedReceivers,
		g_onvif_cfg.Capabilities.receiver.MaximumRTSPURILength);

	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Receiver>");

	return offset;
}

int bdReceiverServicesCapabilities_xml(char * p_buf, int mlen)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
		"<trv:Capabilities RTP_Multicast=\"%s\" RTP_TCP=\"%s\" RTP_RTSP_TCP=\"%s\" SupportedReceivers=\"%d\" MaximumRTSPURILength=\"%d\" />",
		g_onvif_cfg.Capabilities.receiver.RTP_USCOREMulticast ? "true" : "false",
		g_onvif_cfg.Capabilities.receiver.RTP_USCORETCP ? "true" : "false",
		g_onvif_cfg.Capabilities.receiver.RTP_USCORERTSP_USCORETCP ? "true" : "false",
		g_onvif_cfg.Capabilities.receiver.SupportedReceivers,
		g_onvif_cfg.Capabilities.receiver.MaximumRTSPURILength);

    return offset;
}

#endif // end of RECEIVER_SUPPORT

#ifdef MEDIA2_SUPPORT

int bdMedia2ServicesCapabilities_xml(char * p_buf, int mlen)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tr2:Capabilities SnapshotUri=\"%s\" Rotation=\"false\" VideoSourceMode=\"%s\" OSD=\"%s\" "
		"TemporaryOSDText=\"%s\" Mask=\"%s\" SourceMask=\"%s\">","false",
		g_onvif_cfg.Capabilities.media2.VideoSourceMode ? "true" : "false","false",
		g_onvif_cfg.Capabilities.media2.TemporaryOSDText ? "true" : "false", "false", "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tr2:ProfileCapabilities MaximumNumberOfProfiles=\"%d\" ConfigurationsSupported=\"%s\" />"
		"<tr2:StreamingCapabilities RTSPStreaming=\"%s\" RTPMulticast=\"%s\" RTP_RTSP_TCP=\"%s\" "
		"NonAggregateControl=\"%s\" ", 
		g_onvif_cfg.Capabilities.media2.ProfileCapabilities.MaximumNumberOfProfiles,
		g_onvif_cfg.Capabilities.media2.ProfileCapabilities.ConfigurationsSupported,
		g_onvif_cfg.Capabilities.media2.StreamingCapabilities.RTSPStreaming ? "true" : "false",
		g_onvif_cfg.Capabilities.media2.StreamingCapabilities.RTPMulticast ? "true" : "false",
		g_onvif_cfg.Capabilities.media2.StreamingCapabilities.RTP_RTSP_TCP ? "true" : "false",
		g_onvif_cfg.Capabilities.media2.StreamingCapabilities.NonAggregateControl ? "true" : "false");

    if (g_onvif_cfg.Capabilities.media2.StreamingCapabilities.RTSPWebSocketUriFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "RTSPWebSocketUri=\"%s\" ", 
            g_onvif_cfg.Capabilities.media2.StreamingCapabilities.RTSPWebSocketUri);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "AutoStartMulticast=\"%s\" />", 
        g_onvif_cfg.Capabilities.media2.StreamingCapabilities.AutoStartMulticast ? "true" : "false");
			
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Capabilities>");

	return offset;
}

#endif

#ifdef PROFILE_C_SUPPORT

int bdAccessControlServicesCapabilities_xml(char * p_buf, int mlen)
{
    int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tac:Capabilities MaxLimit=\"%d\" MaxAccessPoints=\"%d\" MaxAreas=\"%d\" "
		"ClientSuppliedTokenSupported=\"%s\" AccessPointManagementSupported=\"%s\" "
		"AreaManagementSupported=\"%s\" />",
		g_onvif_cfg.Capabilities.accesscontrol.MaxLimit,
		g_onvif_cfg.Capabilities.accesscontrol.MaxAccessPoints,
		g_onvif_cfg.Capabilities.accesscontrol.MaxAreas,
		g_onvif_cfg.Capabilities.accesscontrol.ClientSuppliedTokenSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.accesscontrol.AccessPointManagementSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.accesscontrol.AreaManagementSupported ? "true" : "false");

	return offset;
}

int bdDoorControlServicesCapabilities_xml(char * p_buf, int mlen)
{
    int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tdc:Capabilities MaxLimit=\"%d\" MaxDoors=\"%d\" ClientSuppliedTokenSupported=\"%s\" DoorManagementSupported=\"%s\" />",
		g_onvif_cfg.Capabilities.doorcontrol.MaxLimit,
		g_onvif_cfg.Capabilities.doorcontrol.MaxDoors,
		g_onvif_cfg.Capabilities.doorcontrol.ClientSuppliedTokenSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.doorcontrol.DoorManagementSupported ? "true" : "false");

	return offset;
}

#endif // PROFILE_C_SUPPORT

#ifdef THERMAL_SUPPORT

int bdThermalServicesCapabilities_xml(char * p_buf, int mlen)
{
    int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tth:Capabilities Radiometry=\"%s\" />",
		g_onvif_cfg.Capabilities.thermal.Radiometry ? "true" : "false");

	return offset;
}

#endif // THERMAL_SUPPORT

#ifdef CREDENTIAL_SUPPORT

int bdCredentialServicesCapabilities_xml(char * p_buf, int mlen)
{
    int i;
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tcr:Capabilities MaxLimit=\"%d\" CredentialValiditySupported=\"%s\" "
		"CredentialAccessProfileValiditySupported=\"%s\" ValiditySupportsTimeValue=\"%s\" "
		"MaxCredentials=\"%d\" MaxAccessProfilesPerCredential=\"%d\" ResetAntipassbackSupported=\"%s\" "
		"ClientSuppliedTokenSupported=\"%s\" DefaultCredentialSuspensionDuration=\"%s\" "
		"MaxWhitelistedItems=\"%d\" MaxBlacklistedItems=\"%d\">",
		g_onvif_cfg.Capabilities.credential.MaxLimit,
		g_onvif_cfg.Capabilities.credential.CredentialValiditySupported ? "true" : "false",
		g_onvif_cfg.Capabilities.credential.CredentialAccessProfileValiditySupported ? "true" : "false",
		g_onvif_cfg.Capabilities.credential.ValiditySupportsTimeValue ? "true" : "false",
		g_onvif_cfg.Capabilities.credential.MaxCredentials,
		g_onvif_cfg.Capabilities.credential.MaxAccessProfilesPerCredential,
		g_onvif_cfg.Capabilities.credential.ResetAntipassbackSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.credential.ClientSuppliedTokenSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.credential.DefaultCredentialSuspensionDuration,
		g_onvif_cfg.Capabilities.credential.MaxWhitelistedItems,
		g_onvif_cfg.Capabilities.credential.MaxBlacklistedItems);

    for (i = 0; i < g_onvif_cfg.Capabilities.credential.sizeSupportedIdentifierType; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:SupportedIdentifierType>%s</tcr:SupportedIdentifierType>",
            g_onvif_cfg.Capabilities.credential.SupportedIdentifierType[i]);
    }

    if (g_onvif_cfg.Capabilities.credential.ExtensionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tcr:Extension>");

        for (i = 0; i < g_onvif_cfg.Capabilities.credential.Extension.sizeSupportedExemptionType; i++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
                "<tcr:SupportedExemptionType>%s</tcr:SupportedExemptionType>",
                g_onvif_cfg.Capabilities.credential.Extension.SupportedExemptionType[i]);
        }
    
        offset += snprintf(p_buf+offset, mlen-offset, "</tcr:Extension>");
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:Capabilities>");

    return offset;
}

#endif // end of CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES

int bdAccessRulesServicesCapabilities_xml(char * p_buf, int mlen)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tar:Capabilities MaxLimit=\"%d\" MaxAccessProfiles=\"%d\" "
		"MaxAccessPoliciesPerAccessProfile=\"%d\" MultipleSchedulesPerAccessPointSupported=\"%s\" "
		"ClientSuppliedTokenSupported=\"%s\" />",
		g_onvif_cfg.Capabilities.accessrules.MaxLimit,
		g_onvif_cfg.Capabilities.accessrules.MaxAccessProfiles,
		g_onvif_cfg.Capabilities.accessrules.MaxAccessPoliciesPerAccessProfile,
		g_onvif_cfg.Capabilities.accessrules.MultipleSchedulesPerAccessPointSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.accessrules.ClientSuppliedTokenSupported ? "true" : "false");

    return offset;
}

#endif // end of ACCESS_RULES

#ifdef SCHEDULE_SUPPORT

int bdScheduleServicesCapabilities_xml(char * p_buf, int mlen)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tsc:Capabilities MaxLimit=\"%d\" MaxSchedules=\"%d\" MaxTimePeriodsPerDay=\"%d\" "
		"MaxSpecialDayGroups=\"%d\" MaxDaysInSpecialDayGroup=\"%d\" MaxSpecialDaysSchedules=\"%d\" "
		"ExtendedRecurrenceSupported=\"%s\" SpecialDaysSupported=\"%s\" StateReportingSupported=\"%s\" "
		"ClientSuppliedTokenSupported=\"%s\" />",
		g_onvif_cfg.Capabilities.schedule.MaxLimit,
		g_onvif_cfg.Capabilities.schedule.MaxSchedules,
		g_onvif_cfg.Capabilities.schedule.MaxTimePeriodsPerDay,
		g_onvif_cfg.Capabilities.schedule.MaxSpecialDayGroups,
		g_onvif_cfg.Capabilities.schedule.MaxDaysInSpecialDayGroup,
		g_onvif_cfg.Capabilities.schedule.MaxSpecialDaysSchedules,
		g_onvif_cfg.Capabilities.schedule.ExtendedRecurrenceSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.schedule.SpecialDaysSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.schedule.StateReportingSupported ? "true" : "false",
		g_onvif_cfg.Capabilities.schedule.ClientSuppliedTokenSupported ? "true" : "false");

    return offset;
}

#endif // end of SCHEDULE_SUPPORT

#ifdef PROVISIONING_SUPPORT

int bdSourceCapabilities_xml(char * p_buf, int mlen, onvif_SourceCapabilities * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:Source VideoSourceToken=\"%s\"", p_req->VideoSourceToken);

    if (p_req->MaximumPanMovesFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, " MaximumPanMoves=\"%d\"", p_req->MaximumPanMoves);
    }

    if (p_req->MaximumTiltMovesFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, " MaximumTiltMoves=\"%d\"", p_req->MaximumTiltMoves);
    }

    if (p_req->MaximumZoomMovesFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, " MaximumZoomMoves=\"%d\"", p_req->MaximumZoomMoves);
    }

    if (p_req->MaximumRollMovesFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, " MaximumRollMoves=\"%d\"", p_req->MaximumRollMoves);
    }

    if (p_req->AutoLevelFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, " AutoLevel=\"%s\"", p_req->AutoLevel ? "true" : "false");
    }

    if (p_req->MaximumFocusMovesFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, " MaximumFocusMoves=\"%d\"", p_req->MaximumFocusMoves);
    }

    if (p_req->AutoFocusFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, " AutoFocus=\"%s\"", p_req->AutoFocus ? "true" : "false");
    }

    offset += snprintf(p_buf+offset, mlen-offset, "></tpv:Source>");
    
    return offset;
}

int bdProvisioningServicesCapabilities_xml(char * p_buf, int mlen)
{
    int i;
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:Capabilities>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:DefaultTimeout>PT%dS</tpv:DefaultTimeout>", g_onvif_cfg.Capabilities.provisioning.DefaultTimeout);

    for (i = 0; i < g_onvif_cfg.Capabilities.provisioning.sizeSource; i++)
    {
        offset += bdSourceCapabilities_xml(p_buf+offset, mlen-offset, &g_onvif_cfg.Capabilities.provisioning.Source[i]);
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tpv:Capabilities>");

    return offset;
}

#endif // PROVISIONING_SUPPORT

int bdVersion_xml(char * p_buf, int mlen, int major, int minor)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:Version>"
		    "<tt:Major>%d</tt:Major>"
		    "<tt:Minor>%d</tt:Minor>"
	    "</tds:Version>",
	    major, minor);

	return offset;
}

int bdFloatRange_xml(char * p_buf, int mlen, onvif_FloatRange * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:Min>%0.2f</tt:Min>"
        "<tt:Max>%0.2f</tt:Max>",
        p_req->Min, p_req->Max);

    return offset;
}

int bdIntList_xml(char * p_buf, int mlen, onvif_IntList * p_req)
{
    int i, offset = 0;

    for (i = 0; i < p_req->sizeItems; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:Items>%d</tt:Items>", p_req->Items[i]);
    }       

    return offset;
}

int bdFloatList_xml(char * p_buf, int mlen, onvif_FloatList * p_req)
{
    int i, offset = 0;

    for (i = 0; i < p_req->sizeItems; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:Items>%0.2f</tt:Items>", p_req->Items[i]);
    }       

    return offset;
}

int bdVideoResolution_xml(char * p_buf, int mlen, onvif_VideoResolution * p_req)
{
    int offset = 0;
    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:ResolutionsAvailable>"
			"<tt:Width>%d</tt:Width>"
			"<tt:Height>%d</tt:Height>"
		"</tt:ResolutionsAvailable>",
		p_req->Width, 
		p_req->Height);

	return offset;
}

int bdGetServiceCapabilities_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	onvif_CapabilityCategory category = *(onvif_CapabilityCategory *)argv;

	if (CapabilityCategory_Device == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetServiceCapabilitiesResponse>");
		offset += bdDeviceServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetServiceCapabilitiesResponse>");
	}
#ifdef MEDIA2_SUPPORT
    else if (CapabilityCategory_Media2 == category)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetServiceCapabilitiesResponse>");
	    offset += bdMedia2ServicesCapabilities_xml(p_buf+offset, mlen-offset);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetServiceCapabilitiesResponse>");
    }
#endif
#ifdef MEDIA_SUPPORT
	else if (CapabilityCategory_Media == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetServiceCapabilitiesResponse>");
		offset += bdMediaServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetServiceCapabilitiesResponse>");
	}
#endif	
	else if (CapabilityCategory_Events == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tev:GetServiceCapabilitiesResponse>");
		offset += bdEventsServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</tev:GetServiceCapabilitiesResponse>");
	}
#ifdef IMAGE_SUPPORT
	else if (CapabilityCategory_Imaging == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetServiceCapabilitiesResponse>");
		offset += bdImagingServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</timg:GetServiceCapabilitiesResponse>");
	}
#endif	
#ifdef VIDEO_ANALYTICS	
	else if (CapabilityCategory_Analytics == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetServiceCapabilitiesResponse>");
		offset += bdAnalyticsServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetServiceCapabilitiesResponse>");
	}	
#endif	
#ifdef PROFILE_G_SUPPORT
	else if (CapabilityCategory_Recording == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetServiceCapabilitiesResponse>");
		offset += bdRecordingServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</trc:GetServiceCapabilitiesResponse>");
	}
	else if (CapabilityCategory_Search == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetServiceCapabilitiesResponse>");
		offset += bdSearchServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</tse:GetServiceCapabilitiesResponse>");
	}
	else if (CapabilityCategory_Replay == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trp:GetServiceCapabilitiesResponse>");
		offset += bdReplayServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</trp:GetServiceCapabilitiesResponse>");
	}
#endif
#ifdef PROFILE_C_SUPPORT
    else if (CapabilityCategory_AccessControl == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tac:GetServiceCapabilitiesResponse>");
		offset += bdAccessControlServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</tac:GetServiceCapabilitiesResponse>");
	}
	else if (CapabilityCategory_DoorControl == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tdc:GetServiceCapabilitiesResponse>");
		offset += bdDoorControlServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</tdc:GetServiceCapabilitiesResponse>");
	}
#endif
#ifdef THERMAL_SUPPORT
    else if (CapabilityCategory_Thermal == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tth:GetServiceCapabilitiesResponse>");
		offset += bdThermalServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</tth:GetServiceCapabilitiesResponse>");
	}
#endif
#ifdef CREDENTIAL_SUPPORT
    else if (CapabilityCategory_Credential == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetServiceCapabilitiesResponse>");
		offset += bdCredentialServicesCapabilities_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetServiceCapabilitiesResponse>");
	}
#endif
#ifdef ACCESS_RULES
    else if (CapabilityCategory_AccessRules == category)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tar:GetServiceCapabilitiesResponse>");
        offset += bdAccessRulesServicesCapabilities_xml(p_buf+offset, mlen-offset);
        offset += snprintf(p_buf+offset, mlen-offset, "</tar:GetServiceCapabilitiesResponse>");
    }
#endif
#ifdef SCHEDULE_SUPPORT
    else if (CapabilityCategory_Schedule == category)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetServiceCapabilitiesResponse>");
        offset += bdScheduleServicesCapabilities_xml(p_buf+offset, mlen-offset);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:GetServiceCapabilitiesResponse>");
    }
#endif
#ifdef RECEIVER_SUPPORT
    else if (CapabilityCategory_Receiver == category)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<trv:GetServiceCapabilitiesResponse>");
        offset += bdReceiverServicesCapabilities_xml(p_buf+offset, mlen-offset);
        offset += snprintf(p_buf+offset, mlen-offset, "</trv:GetServiceCapabilitiesResponse>");
    }
#endif
#ifdef PROVISIONING_SUPPORT
    else if (CapabilityCategory_Provisioning == category)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tpv:GetServiceCapabilitiesResponse>");
        offset += bdProvisioningServicesCapabilities_xml(p_buf+offset, mlen-offset);
        offset += snprintf(p_buf+offset, mlen-offset, "</tpv:GetServiceCapabilitiesResponse>");
    }
#endif

	return offset;
}

/***************************************************************************************/

int bdDot11Configuration_xml(char * p_buf, int mlen, onvif_Dot11Configuration * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:SSID>%s</tt:SSID>"
        "<tt:Mode>%s</tt:Mode>"
        "<tt:Alias>%s</tt:Alias>"
        "<tt:Priority>%d</tt:Priority>", 
        p_req->SSID,
        onvif_Dot11StationModeToString(p_req->Mode),
        p_req->Alias,
        p_req->Priority);

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Security>");
    
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", 
        onvif_Dot11SecurityModeToString(p_req->Security.Mode));

    if (p_req->Security.AlgorithmFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Algorithm>%s</tt:Algorithm>", 
            onvif_Dot11CipherToString(p_req->Security.Algorithm));
    }

    if (p_req->Security.PSKFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:PSK>");
        
        if (p_req->Security.PSK.KeyFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Key>%s</tt:Key>", p_req->Security.PSK.Key);
        }

        if (p_req->Security.PSK.PassphraseFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Passphrase>%s</tt:Passphrase>", p_req->Security.PSK.Passphrase);
        }
        
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:PSK>");
    }

    if (p_req->Security.Dot1XFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Dot1X>%s</tt:Dot1X>", p_req->Security.Dot1X);
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Security>");
    return offset;
}

int bdDot11AvailableNetworks_xml(char * p_buf, int mlen, onvif_Dot11AvailableNetworks * p_req)
{
    int i;
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:SSID>%s</tt:SSID>", p_req->BSSID);

    if (p_req->BSSIDFlag)
    {
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:BSSID>%s</tt:BSSID>", p_req->BSSID);
	}

	for (i = 0; i < p_req->sizeAuthAndMangementSuite; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:AuthAndMangementSuite>%s</tt:AuthAndMangementSuite>", 
	        onvif_Dot11AuthAndMangementSuiteToString(p_req->AuthAndMangementSuite[i]));
	}

	for (i = 0; i < p_req->sizePairCipher; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:PairCipher>%s</tt:PairCipher>", 
	        onvif_Dot11CipherToString(p_req->PairCipher[i]));
	}

	for (i = 0; i < p_req->sizeGroupCipher; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:GroupCipher>%s</tt:GroupCipher>", 
	        onvif_Dot11CipherToString(p_req->GroupCipher[i]));
	}

	if (p_req->SignalStrengthFlag)
    {
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:SignalStrength>%s</tt:SignalStrength>", 
	        onvif_Dot11SignalStrengthToString(p_req->SignalStrength));
	}

    return offset;
}

int bdNetworkInterface_xml(char * p_buf, int mlen, onvif_NetworkInterface * p_req)
{
    int i;
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Enabled>%s</tt:Enabled>", p_req->Enabled ? "true" : "false");
		
	if (p_req->InfoFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Info>");
		if (p_req->Info.NameFlag)
		{
		    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Name>%s</tt:Name>", p_req->Info.Name);
		}    
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:HwAddress>%s</tt:HwAddress>", p_req->Info.HwAddress);
		if (p_req->Info.MTUFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MTU>%d</tt:MTU>", p_req->Info.MTU);
		}	
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Info>");
	}

	if (p_req->IPv4Flag)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4>");
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Enabled>%s</tt:Enabled>", p_req->IPv4.Enabled ? "true" : "false");
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Config>");

		if (p_req->IPv4.Config.DHCP == FALSE)
		{
		    offset += snprintf(p_buf+offset, mlen-offset, 
		        "<tt:Manual>"
			        "<tt:Address>%s</tt:Address>"
			        "<tt:PrefixLength>%d</tt:PrefixLength>"
		        "</tt:Manual>",
		        p_req->IPv4.Config.Address, 
		        p_req->IPv4.Config.PrefixLength);				
		}
		else
		{
		    offset += snprintf(p_buf+offset, mlen-offset, 
		        "<tt:FromDHCP>"
			        "<tt:Address>%s</tt:Address>"
			        "<tt:PrefixLength>%d</tt:PrefixLength>"
		        "</tt:FromDHCP>",
		        p_req->IPv4.Config.Address, 
		        p_req->IPv4.Config.PrefixLength);
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:DHCP>%s</tt:DHCP>", p_req->IPv4.Config.DHCP ? "true" : "false");

		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Config>");
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:IPv4>");
	}

    if (p_req->ExtensionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Extension>");
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:InterfaceType>%d</tt:InterfaceType>", p_req->Extension.InterfaceType);

        for (i = 0; i < p_req->Extension.sizeDot11; i++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Dot11>");
            offset += bdDot11Configuration_xml(p_buf+offset, mlen-offset, &p_req->Extension.Dot11[i]);
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:Dot11>");
        }

        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Extension>");
    }
    
    return offset;
}

int bdtds_GetDeviceInformation_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	
#define _unknown "unknown1"//w4000_info
#if 0//w4000_xml
	if(is_auth_w4000()==0)
		{
		printf("w4000 %s \r\n",__FUNCTION__);
			offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetDeviceInformationResponse>"
		    "<tds:Manufacturer>%s</tds:Manufacturer>"
		    "<tds:Model>21C26IP6</tds:Model>"
		    "<tds:FirmwareVersion>%s</tds:FirmwareVersion>"
		    "<tds:SerialNumber>%s</tds:SerialNumber>"
		    "<tds:HardwareId>%s</tds:HardwareId>"
	    "</tds:GetDeviceInformationResponse>", 
				_unknown,
				_unknown,
				_unknown,
				_unknown);
		return;
		}
#endif

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetDeviceInformationResponse>"
		    "<tds:Manufacturer>%s</tds:Manufacturer>"
		    "<tds:Model>21C26IP6</tds:Model>"
		    "<tds:FirmwareVersion>%s</tds:FirmwareVersion>"
		    "<tds:SerialNumber>%s</tds:SerialNumber>"
		    "<tds:HardwareId>%s</tds:HardwareId>"
	    "</tds:GetDeviceInformationResponse>", 
    	g_onvif_cfg.DeviceInformation.Manufacturer, 
    	g_onvif_cfg.DeviceInformation.FirmwareVersion, 
    	g_onvif_cfg.DeviceInformation.SerialNumber, 
    	g_onvif_cfg.DeviceInformation.HardwareId);


	// printf("p_buf? %s \n", p_buf);
	return offset;
}

int bdtds_GetSystemUris_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tds_GetSystemUris_RES * p_res = (tds_GetSystemUris_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetSystemUrisResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SystemLogUris>");
        
    if (p_res->SystemLogUriFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:SystemLog>"
                "<tt:Type>System</tt:Type>"
                "<tt:Uri>%s</tt:Uri>"
            "</tt:SystemLog>", p_res->SystemLogUri);
    }

    if (p_res->AccessLogUriFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:SystemLog>"
                "<tt:Type>Access</tt:Type>"
                "<tt:Uri>%s</tt:Uri>"
            "</tt:SystemLog>", p_res->AccessLogUri);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</tds:SystemLogUris>");

    if (p_res->SupportInfoUriFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tds:SupportInfoUri>%s</tds:SupportInfoUri>",
            p_res->SupportInfoUri);
    }

    if (p_res->SystemBackupUriFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tds:SystemBackupUri>%s</tds:SystemBackupUri>",
            p_res->SystemBackupUri);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetSystemUrisResponse>");
	
	return offset;
}

int bdtds_GetCapabilities_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tds_GetCapabilities_REQ * p_req = (tds_GetCapabilities_REQ *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetCapabilitiesResponse><tds:Capabilities>");

	if (CapabilityCategory_Device == p_req->Category)
	{
	    offset += bdDeviceCapabilities_xml(p_buf+offset, mlen-offset);
	}
    else if (CapabilityCategory_Events == p_req->Category)
    {
        offset += bdEventsCapabilities_xml(p_buf+offset, mlen-offset);
    }
#ifdef MEDIA_SUPPORT    
    else if (CapabilityCategory_Media == p_req->Category)
	{
	    offset += bdMediaCapabilities_xml(p_buf+offset, mlen-offset);
	}
#endif	
#ifdef IMAGE_SUPPORT	
    else if (CapabilityCategory_Imaging == p_req->Category)
    {
        offset += bdImagingCapabilities_xml(p_buf+offset, mlen-offset);
    }
#endif    
#ifdef VIDEO_ANALYTICS
	else if (CapabilityCategory_Analytics == p_req->Category)
    {
        offset += bdAnalyticsCapabilities_xml(p_buf+offset, mlen-offset);
    }
#endif    
	else if (CapabilityCategory_All == p_req->Category)
	{
#ifdef VIDEO_ANALYTICS
		offset += bdAnalyticsCapabilities_xml(p_buf+offset, mlen-offset);
#endif	
	    offset += bdDeviceCapabilities_xml(p_buf+offset, mlen-offset);
	    offset += bdEventsCapabilities_xml(p_buf+offset, mlen-offset);
#ifdef IMAGE_SUPPORT
	    offset += bdImagingCapabilities_xml(p_buf+offset, mlen-offset);
#endif
#ifdef MEDIA_SUPPORT	    
	    offset += bdMediaCapabilities_xml(p_buf+offset, mlen-offset);
#endif	    
	    // offset += snprintf(p_buf+offset, mlen-offset, "<tt:Extension>");
	    // offset += snprintf(p_buf+offset, mlen-offset, "</tt:Extension>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities></tds:GetCapabilitiesResponse>");
	
	return offset;
}

int bdtds_GetNetworkInterfaces_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	NetworkInterfaceList * p_net_inf = g_onvif_cfg.network.interfaces;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkInterfacesResponse>");
	
	while (p_net_inf)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:NetworkInterfaces token=\"%s\">", p_net_inf->NetworkInterface.token);			
		offset += bdNetworkInterface_xml(p_buf+offset, mlen-offset, &p_net_inf->NetworkInterface);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkInterfaces>");
		
		p_net_inf = p_net_inf->next;
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetNetworkInterfacesResponse>");		
	
	return offset;
}

int bdtds_SetNetworkInterfaces_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:SetNetworkInterfacesResponse>"
			"<tds:RebootNeeded>false</tds:RebootNeeded>"
		"</tds:SetNetworkInterfacesResponse>");		
	
	return offset;
}

int bdtds_SystemReboot_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:SystemRebootResponse>"
			"<tds:Message>Rebooting</tds:Message>"
		"</tds:SystemRebootResponse>");	    
	
	return offset;
}

int bdtds_SetSystemFactoryDefault_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetSystemFactoryDefaultResponse />");	    
	return offset;
}


int bdtds_GetSystemDateAndTime_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	time_t nowtime;
	struct tm *gtime;

	time(&nowtime);
	gtime = gmtime(&nowtime);
    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetSystemDateAndTimeResponse>"
			"<tds:SystemDateAndTime>"
			"<tt:DateTimeType>%s</tt:DateTimeType>"
			"<tt:DaylightSavings>%s</tt:DaylightSavings>",
			onvif_SetDateTimeTypeToString(g_onvif_cfg.SystemDateTime.DateTimeType), 
			g_onvif_cfg.SystemDateTime.DaylightSavings ? "true" : "false");

	if (g_onvif_cfg.SystemDateTime.TimeZoneFlag && 
		g_onvif_cfg.SystemDateTime.TimeZone.TZ[0] != '\0')
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:TimeZone><tt:TZ>%s</tt:TZ></tt:TimeZone>", 
			g_onvif_cfg.SystemDateTime.TimeZone.TZ);			
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, 			
			"<tt:UTCDateTime>"
				"<tt:Time>"
					"<tt:Hour>%d</tt:Hour>"
					"<tt:Minute>%d</tt:Minute>"
					"<tt:Second>%d</tt:Second>"
				"</tt:Time>"
				"<tt:Date>"
					"<tt:Year>%d</tt:Year>"
					"<tt:Month>%d</tt:Month>"
					"<tt:Day>%d</tt:Day>"
				"</tt:Date>"
			"</tt:UTCDateTime>"	
			"</tds:SystemDateAndTime>"
		"</tds:GetSystemDateAndTimeResponse>",
		gtime->tm_hour, gtime->tm_min, gtime->tm_sec, 
		gtime->tm_year+1900, gtime->tm_mon+1, gtime->tm_mday);		
	
	return offset;
}


int bdtds_SetSystemDateAndTime_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetSystemDateAndTimeResponse></tds:SetSystemDateAndTimeResponse>");
	return offset;
}

int bdtds_GetServices_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tds_GetServices_REQ * p_req = (tds_GetServices_REQ *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetServicesResponse>");

	// device manager
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tds:Service>"
	    "<tds:Namespace>http://www.onvif.org/ver10/device/wsdl</tds:Namespace>"
	    "<tds:XAddr>%s</tds:XAddr>", 
	    g_onvif_cfg.Capabilities.device.XAddr);	
	if (p_req->IncludeCapability)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");        
        offset += bdDeviceServicesCapabilities_xml(p_buf+offset, mlen-offset);				
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
	}    
	offset += bdVersion_xml(p_buf+offset, mlen-offset, 19, 12);
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");

    // event service
	if (g_onvif_cfg.Capabilities.events.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/events/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.events.XAddr);	
		if (p_req->IncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdEventsServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 19, 6);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
	
#ifdef MEDIA2_SUPPORT
    // media service 2
	if (g_onvif_cfg.Capabilities.media2.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver20/media/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.media2.XAddr);	
		if (p_req->IncludeCapability)
		{
	        offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdMedia2ServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 19, 6);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
#endif

#ifdef MEDIA_SUPPORT
	// media service 1
	if (g_onvif_cfg.Capabilities.media.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/media/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.media.XAddr);	
		if (p_req->IncludeCapability)
		{
	        offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdMediaServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 19, 6);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
#endif


#ifdef IMAGE_SUPPORT
	// image
	if (g_onvif_cfg.Capabilities.image.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver20/imaging/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.image.XAddr);	
		if (p_req->IncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdImagingServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 19, 6);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
#endif

#ifdef VIDEO_ANALYTICS
	// analytics
	if (g_onvif_cfg.Capabilities.analytics.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver20/analytics/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.analytics.XAddr);	
		if (p_req->IncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdAnalyticsServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 16, 12);
		//offset += bdVersion_xml(p_buf+offset, mlen-offset, 19, 12);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
#endif

#ifdef PROFILE_G_SUPPORT

	// recording
	if (g_onvif_cfg.Capabilities.recording.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/recording/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.recording.XAddr);	
		if (p_req->IncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdRecordingServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 18, 6);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}

	// search
	if (g_onvif_cfg.Capabilities.search.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/search/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.search.XAddr);	
		if (p_req->IncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdSearchServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 2, 4);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}

	// replay
	if (g_onvif_cfg.Capabilities.replay.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/replay/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.replay.XAddr);	
		if (p_req->IncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdReplayServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 18, 6);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
	
#endif // end of PROFILE_G_SUPPORT

#ifdef PROFILE_C_SUPPORT

    // access control
    if (g_onvif_cfg.Capabilities.accesscontrol.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/accesscontrol/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.accesscontrol.XAddr);	
		if (p_req->IncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdAccessControlServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 19, 12);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
    
    // door control
    if (g_onvif_cfg.Capabilities.doorcontrol.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/doorcontrol/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    g_onvif_cfg.Capabilities.doorcontrol.XAddr);	
		if (p_req->IncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += bdDoorControlServicesCapabilities_xml(p_buf+offset, mlen-offset);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += bdVersion_xml(p_buf+offset, mlen-offset, 19, 12);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
	
#endif // end of PROFILE_C_SUPPORT


#ifdef THERMAL_SUPPORT
    // thermal
    if (g_onvif_cfg.Capabilities.thermal.support)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tds:Service>"
            "<tds:Namespace>http://www.onvif.org/ver10/thermal/wsdl</tds:Namespace>"
            "<tds:XAddr>%s</tds:XAddr>", 
            g_onvif_cfg.Capabilities.thermal.XAddr);   
        if (p_req->IncludeCapability)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
            offset += bdThermalServicesCapabilities_xml(p_buf+offset, mlen-offset);                
            offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
        }    
        offset += bdVersion_xml(p_buf+offset, mlen-offset, 17, 6);
        offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
    }
#endif // end of THERMAL_SUPPORT

#ifdef CREDENTIAL_SUPPORT
    // credential
    if (g_onvif_cfg.Capabilities.credential.support)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tds:Service>"
            "<tds:Namespace>http://www.onvif.org/ver10/credential/wsdl</tds:Namespace>"
            "<tds:XAddr>%s</tds:XAddr>", 
            g_onvif_cfg.Capabilities.credential.XAddr);   
        if (p_req->IncludeCapability)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
            offset += bdCredentialServicesCapabilities_xml(p_buf+offset, mlen-offset);                
            offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
        }    
        offset += bdVersion_xml(p_buf+offset, mlen-offset, 19, 12);
        offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
    }
#endif // end of CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES
    // access rules
    if (g_onvif_cfg.Capabilities.accessrules.support)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tds:Service>"
            "<tds:Namespace>http://www.onvif.org/ver10/accessrules/wsdl</tds:Namespace>"
            "<tds:XAddr>%s</tds:XAddr>", 
            g_onvif_cfg.Capabilities.accessrules.XAddr);   
        if (p_req->IncludeCapability)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
            offset += bdAccessRulesServicesCapabilities_xml(p_buf+offset, mlen-offset);                
            offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
        }    
        offset += bdVersion_xml(p_buf+offset, mlen-offset, 18, 12);
        offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
    }
#endif // end of ACCESS_RULES

#ifdef SCHEDULE_SUPPORT
    // schedule
    if (g_onvif_cfg.Capabilities.schedule.support)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tds:Service>"
            "<tds:Namespace>http://www.onvif.org/ver10/schedule/wsdl</tds:Namespace>"
            "<tds:XAddr>%s</tds:XAddr>", 
            g_onvif_cfg.Capabilities.schedule.XAddr);   
        if (p_req->IncludeCapability)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
            offset += bdScheduleServicesCapabilities_xml(p_buf+offset, mlen-offset);                
            offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
        }    
        offset += bdVersion_xml(p_buf+offset, mlen-offset, 18, 12);
        offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
    }
#endif // end of SCHEDULE_SUPPORT

#ifdef RECEIVER_SUPPORT
    // receiver
    if (g_onvif_cfg.Capabilities.receiver.support)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tds:Service>"
            "<tds:Namespace>http://www.onvif.org/ver10/receiver/wsdl</tds:Namespace>"
            "<tds:XAddr>%s</tds:XAddr>", 
            g_onvif_cfg.Capabilities.receiver.XAddr);   
        if (p_req->IncludeCapability)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
            offset += bdReceiverServicesCapabilities_xml(p_buf+offset, mlen-offset);                
            offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
        }    
        offset += bdVersion_xml(p_buf+offset, mlen-offset, 2, 1);
        offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
    }
#endif // end of RECEIVER_SUPPORT

#ifdef PROVISIONING_SUPPORT
    // provisioning
    if (g_onvif_cfg.Capabilities.provisioning.support)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tds:Service>"
            "<tds:Namespace>http://www.onvif.org/ver10/provisioning/wsdl</tds:Namespace>"
            "<tds:XAddr>%s</tds:XAddr>", 
            g_onvif_cfg.Capabilities.provisioning.XAddr);   
        if (p_req->IncludeCapability)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
            offset += bdProvisioningServicesCapabilities_xml(p_buf+offset, mlen-offset);                
            offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
        }
        offset += bdVersion_xml(p_buf+offset, mlen-offset, 18, 12);
        offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
    }
#endif // end of RECEIVER_SUPPORT

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetServicesResponse>");
	
	return offset;
}

int bdtds_GetScopes_rly_xml(char * p_buf, int mlen, const char * argv)
{	
	int i;
	int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetScopesResponse>");

	for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.scopes); i++)
	{
		if (g_onvif_cfg.scopes[i].ScopeItem[0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tds:Scopes>"
					"<tt:ScopeDef>%s</tt:ScopeDef>"
					"<tt:ScopeItem>%s</tt:ScopeItem>"
				"</tds:Scopes>",
				onvif_ScopeDefinitionToString(g_onvif_cfg.scopes[i].ScopeDef), 
				g_onvif_cfg.scopes[i].ScopeItem);
		}
	}

#ifdef PROFILE_Q_SUPPORT
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:Scopes>");
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:ScopeDef>Fixed</tt:ScopeDef>");
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:ScopeItem>");
    offset += snprintf(p_buf+offset, mlen-offset, "onvif://www.onvif.org/Profile/Q/");
    
    if (g_onvif_cfg.device_state)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "Operational");
    }
    else
    {
        offset += snprintf(p_buf+offset, mlen-offset, "FactoryDefault");
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:ScopeItem>");
    offset += snprintf(p_buf+offset, mlen-offset, "</tds:Scopes>");
#endif

    offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetScopesResponse>");    	

	return offset;
}

int bdtds_AddScopes_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:AddScopesResponse />");    
	return offset;
}

int bdtds_SetScopes_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetScopesResponse />");    
	return offset;
}

int bdtds_RemoveScopes_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = 0;
	tds_RemoveScopes_REQ * p_req = (tds_RemoveScopes_REQ *)argv;
	
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:RemoveScopesResponse>");

    for (i = 0; i < ARRAY_SIZE(p_req->ScopeItem); i++)
	{
		if (p_req->ScopeItem[i][0] == '\0')
		{
			break;
		}

		offset += snprintf(p_buf+offset, mlen-offset, "<tds:ScopeItem>%s</tds:ScopeItem>", p_req->ScopeItem[i]);  
	}
	
    offset += snprintf(p_buf+offset, mlen-offset, "</tds:RemoveScopesResponse>");    

	return offset;
}

int bdtds_GetHostname_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tds:GetHostnameResponse>"
        	"<tds:HostnameInformation>"
	            "<tt:FromDHCP>%s</tt:FromDHCP>"
	            "<tt:Name>%s</tt:Name>"    
       		"</tds:HostnameInformation>"
       	"</tds:GetHostnameResponse>",
      	g_onvif_cfg.network.HostnameInformation.FromDHCP ? "true" : "false",
      	g_onvif_cfg.network.HostnameInformation.Name);

	return offset;
}

int bdtds_SetHostname_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetHostnameResponse />");
	return offset;
}

int bdtds_SetHostnameFromDHCP_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tds:SetHostnameFromDHCPResponse>"
    		"<tds:RebootNeeded>%s</tds:RebootNeeded>"
    	"</tds:SetHostnameFromDHCPResponse>",
    	g_onvif_cfg.network.HostnameInformation.RebootNeeded ? "true" : "false");
    
	return offset;
}

int bdtds_GetNetworkProtocols_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i = 0;
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkProtocolsResponse>");

	if (g_onvif_cfg.network.NetworkProtocol.HTTPFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tds:NetworkProtocols>"
			"<tt:Name>HTTP</tt:Name>"
			"<tt:Enabled>%s</tt:Enabled>", 
			g_onvif_cfg.network.NetworkProtocol.HTTPEnabled ? "true" : "false");

		for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.network.NetworkProtocol.HTTPPort); i++)
		{
			if (g_onvif_cfg.network.NetworkProtocol.HTTPPort[i] == 0)
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Port>%d</tt:Port>", g_onvif_cfg.network.NetworkProtocol.HTTPPort[i]);
		}

		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
	}

	if (g_onvif_cfg.network.NetworkProtocol.HTTPSFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tds:NetworkProtocols>"
			"<tt:Name>HTTPS</tt:Name>"
			"<tt:Enabled>%s</tt:Enabled>", 
			g_onvif_cfg.network.NetworkProtocol.HTTPSEnabled ? "true" : "false");

		for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.network.NetworkProtocol.HTTPSPort); i++)
		{
			if (g_onvif_cfg.network.NetworkProtocol.HTTPSPort[i] == 0)
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Port>%d</tt:Port>", g_onvif_cfg.network.NetworkProtocol.HTTPSPort[i]);
		}

		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
	}

	if (g_onvif_cfg.network.NetworkProtocol.RTSPFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tds:NetworkProtocols>"
			"<tt:Name>RTSP</tt:Name>"
			"<tt:Enabled>%s</tt:Enabled>", 
			g_onvif_cfg.network.NetworkProtocol.RTSPEnabled ? "true" : "false");

		for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.network.NetworkProtocol.RTSPPort); i++)
		{
			if (g_onvif_cfg.network.NetworkProtocol.RTSPPort[i] == 0)
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Port>%d</tt:Port>", g_onvif_cfg.network.NetworkProtocol.RTSPPort[i]);
		}

		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetNetworkProtocolsResponse>");	

	return offset;
}

int bdtds_SetNetworkProtocols_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNetworkProtocolsResponse />");
	return offset;
}

int bdtds_GetNetworkDefaultGateway_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;	
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkDefaultGatewayResponse><tds:NetworkGateway>");

	for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.network.NetworkGateway.IPv4Address); i++)
	{
		if (g_onvif_cfg.network.NetworkGateway.IPv4Address[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4Address>%s</tt:IPv4Address>", g_onvif_cfg.network.NetworkGateway.IPv4Address[i]);
		}

	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkGateway></tds:GetNetworkDefaultGatewayResponse>");		

	return offset;
}

int bdtds_SetNetworkDefaultGateway_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNetworkDefaultGatewayResponse />");
	return offset;
}

int bdtds_GetDiscoveryMode_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetDiscoveryModeResponse>"
   			"<tds:DiscoveryMode>%s</tds:DiscoveryMode>"
	  	"</tds:GetDiscoveryModeResponse>", 
	  	onvif_DiscoveryModeToString(g_onvif_cfg.network.DiscoveryMode));	

	return offset;
}

int bdtds_SetDiscoveryMode_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetDiscoveryModeResponse />");	
	return offset;
}

int bdtds_GetDNS_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetDNSResponse><tds:DNSInformation>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:FromDHCP>%s</tt:FromDHCP>", g_onvif_cfg.network.DNSInformation.FromDHCP ? "true" : "false");
	// offset += snprintf(p_buf+offset, mlen-offset, "<tt:FromDHCP>false</tt:FromDHCP>");

	
	for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.network.DNSInformation.DNSServer); i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:DNSManual>");
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:Type>IPv4</tt:Type><tt:IPv4Address>%s</tt:IPv4Address>",
			g_onvif_cfg.network.DNSInformation.DNSServer[i]);

		{
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:DNSManual>");
		}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:DNSInformation></tds:GetDNSResponse>");	

	return offset;
}



int bdtds_GetNTP_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNTPResponse><tds:NTPInformation>");

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tt:FromDHCP>true</tt:FromDHCP>");

	{
		{
		    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>DNS</tt:Type>");
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:DNSname>pool.ntp.org</tt:DNSname>");
		}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:NTPInformation></tds:GetNTPResponse>");	

	return offset;
}

int bdtds_SetNTP_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNTPResponse />");	
	return offset;
}

int bdtds_GetZeroConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetZeroConfigurationResponse><tds:ZeroConfiguration>");	
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tt:InterfaceToken>%s</tt:InterfaceToken>"
	    "<tt:Enabled>%s</tt:Enabled>",
	    g_onvif_cfg.network.ZeroConfiguration.InterfaceToken,
	    g_onvif_cfg.network.ZeroConfiguration.Enabled ? "true" : "false");
    for (i = 0; i < g_onvif_cfg.network.ZeroConfiguration.sizeAddresses; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:Addresses>%s</tt:Addresses>",
            g_onvif_cfg.network.ZeroConfiguration.Addresses[i]);	
    }
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:ZeroConfiguration></tds:GetZeroConfigurationResponse>");	

	return offset;
}

int bdtds_SetZeroConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetZeroConfigurationResponse />");	
	return offset;
}

int bdtds_GetDot11Capabilities_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tds:GetDot11CapabilitiesResponse>"
            "<tds:Capabilities>"
                "<tt:TKIP>%s</tt:TKIP>"
                "<tt:ScanAvailableNetworks>%s</tt:ScanAvailableNetworks>"
                "<tt:MultipleConfiguration>%s</tt:MultipleConfiguration>"
                "<tt:AdHocStationMode>%s</tt:AdHocStationMode>"
                "<tt:WEP>%s</tt:WEP>"
            "</tds:Capabilities>"    
	    "</tds:GetDot11CapabilitiesResponse>",
	    g_onvif_cfg.Capabilities.dot11.TKIP ? "true" : "false",
	    g_onvif_cfg.Capabilities.dot11.ScanAvailableNetworks ? "true" : "false",
	    g_onvif_cfg.Capabilities.dot11.MultipleConfiguration ? "true" : "false",
	    g_onvif_cfg.Capabilities.dot11.AdHocStationMode ? "true" : "false",
	    g_onvif_cfg.Capabilities.dot11.WEP ? "true" : "false");

	return offset;
}

int bdtds_GetDot11Status_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tds_GetDot11Status_RES * p_res = (tds_GetDot11Status_RES *)argv;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetDot11StatusResponse><tds:Status>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:SSID>%s</tt:SSID>", p_res->Status.SSID);

    if (p_res->Status.BSSIDFlag)
    {
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:BSSID>%s</tt:BSSID>", p_res->Status.BSSID);
	}

	if (p_res->Status.PairCipherFlag)
    {
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:PairCipher>%s</tt:PairCipher>", 
	        onvif_Dot11CipherToString(p_res->Status.PairCipher));
	}

	if (p_res->Status.GroupCipherFlag)
    {
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:GroupCipher>%s</tt:GroupCipher>", 
	        onvif_Dot11CipherToString(p_res->Status.GroupCipher));
	}

	if (p_res->Status.SignalStrengthFlag)
    {
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:SignalStrength>%s</tt:SignalStrength>", 
	        onvif_Dot11SignalStrengthToString(p_res->Status.SignalStrength));
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:ActiveConfigAlias>%s</tt:ActiveConfigAlias>", p_res->Status.ActiveConfigAlias);
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:Status></tds:GetDot11StatusResponse>");

	return offset;    
}

int bdtds_ScanAvailableDot11Networks_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tds_ScanAvailableDot11Networks_RES * p_res = (tds_ScanAvailableDot11Networks_RES *)argv;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:ScanAvailableDot11NetworksResponse>");

    for (i = 0; i < p_res->sizeNetworks; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tds:Networks>");
        offset += bdDot11AvailableNetworks_xml(p_buf+offset, mlen-offset, &p_res->Networks[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tds:Networks>");
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:ScanAvailableDot11NetworksResponse>");

	return offset; 
}

int bdtds_GetUsers_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetUsersResponse>");

	for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.users); i++)
	{
		if (g_onvif_cfg.users[i].Username[0] != '\0')
		{		
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tds:User>"
					"<tt:Username>%s</tt:Username>"
					"<tt:UserLevel>%s</tt:UserLevel>"
				"</tds:User>", 
				g_onvif_cfg.users[i].Username, 
				onvif_UserLevelToString(g_onvif_cfg.users[i].UserLevel));
		}
	}
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetUsersResponse>");    	

	return offset;
}

int bdtds_CreateUsers_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:CreateUsersResponse />");
	return offset;
}

int bdtds_DeleteUsers_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:DeleteUsersResponse />");
	return offset;
}

int bdtds_SetUser_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetUserResponse />");
	return offset;
}

int bdtds_GetRemoteUser_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tds_GetRemoteUser_RES * p_res = (tds_GetRemoteUser_RES *) argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetRemoteUserResponse>");

	if (p_res->RemoteUserFlag)
	{
    	offset += snprintf(p_buf+offset, mlen-offset, 
	        "<tds:RemoteUser>"
        	    "<tt:Username>%s</tt:Username>"
                "<tt:UseDerivedPassword>%s</tt:UseDerivedPassword>"
            "</tds:RemoteUser>",
            p_res->RemoteUser.Username,
    	    p_res->RemoteUser.UseDerivedPassword ? "true" : "false");
    }
    
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetRemoteUserResponse>");
	
	return offset;
}

int bdtds_SetRemoteUser_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetRemoteUserResponse />");
	return offset;
}


int bdtds_StartSystemRestore_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tds_StartSystemRestore_RES * p_res = (tds_StartSystemRestore_RES *) argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:StartSystemRestoreResponse>"
	   		"<tds:UploadUri>%s</tds:UploadUri>"
	   		"<tds:ExpectedDownTime>PT%dS</tds:ExpectedDownTime>"
  		"</tds:StartSystemRestoreResponse>", 
  		p_res->UploadUri, p_res->ExpectedDownTime);
	
	return offset;
}

int bdtds_GetWsdlUrl_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetWsdlUrlResponse>"
			"<tds:WsdlUrl>http://www.onvif.org/</tds:WsdlUrl>"
		"</tds:GetWsdlUrlResponse>");
		
	return offset;
}

int bdtds_GetEndpointReference_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetEndpointReferenceResponse>"
			"<tds:GUID>%s</tds:GUID>"
		"</tds:GetEndpointReferenceResponse>",
		g_onvif_cfg.EndpointReference);
	
	return offset;
}

#ifdef DEVICEIO_SUPPORT

int bdtds_SetRelayOutputSettings_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetRelayOutputSettingsResponse />");
	return offset;
}

int bdtds_SetRelayOutputState_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetRelayOutputStateResponse />");
	return offset;
}


#endif // DEVICEIO_SUPPORT

/***************************************************************************************/

#ifdef PROFILE_Q_SUPPORT

int bdBase_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 
        // tns1:Monitoring/ProcessorUsage
        "<tns1:Monitoring wstop:topic=\"true\">"
    		"<ProcessorUsage wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"true\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"Token\" Type=\"tt:ReferenceToken\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:SimpleItemDescription Name=\"Value\" Type=\"xs:float\"/>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</ProcessorUsage>"
    	"</tns1:Monitoring>"

    	// tns1:Monitoring/OperatingTime/LastReset
        "<tns1:Monitoring wstop:topic=\"true\">"
    		"<OperatingTime wstop:topic=\"true\">"
        		"<LastReset wstop:topic=\"true\">"
        		"<tt:MessageDescription IsProperty=\"true\">"
        		"<tt:Source>"        		    
        		"</tt:Source>"
        		"<tt:Data>"
        		    "<tt:SimpleItemDescription Name=\"Status\" Type=\"xs:dateTime\"/>"
        		"</tt:Data>"
        		"</tt:MessageDescription>"
        		"</LastReset>"
    		"</OperatingTime>"
    	"</tns1:Monitoring>"

    	// tns1:Monitoring/OperatingTime/LastReboot
        "<tns1:Monitoring wstop:topic=\"true\">"
    		"<OperatingTime wstop:topic=\"true\">"
        		"<LastReboot wstop:topic=\"true\">"
        		"<tt:MessageDescription IsProperty=\"true\">"
        		"<tt:Source>"        		    
        		"</tt:Source>"
        		"<tt:Data>"
        		    "<tt:SimpleItemDescription Name=\"Status\" Type=\"xs:dateTime\"/>"
        		"</tt:Data>"
        		"</tt:MessageDescription>"
        		"</LastReboot>"
    		"</OperatingTime>"
    	"</tns1:Monitoring>"

    	// tns1:Monitoring/OperatingTime/LastClockSynchronization
        "<tns1:Monitoring wstop:topic=\"true\">"
    		"<OperatingTime wstop:topic=\"true\">"
        		"<LastClockSynchronization wstop:topic=\"true\">"
        		"<tt:MessageDescription IsProperty=\"true\">"
        		"<tt:Source>"        		    
        		"</tt:Source>"
        		"<tt:Data>"
        		    "<tt:SimpleItemDescription Name=\"Status\" Type=\"xs:dateTime\"/>"
        		"</tt:Data>"
        		"</tt:MessageDescription>"
        		"</LastClockSynchronization>"
    		"</OperatingTime>"
    	"</tns1:Monitoring>"
    );

    return offset;
}

#endif // PROFILE_Q_SUPPORT

int bdImaging_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 

        "<tns1:VideoSource wstop:topic=\"true\">"
    		"<ImageTooBlurry wstop:topic=\"true\">"
        		"<ImagingService wstop:topic=\"true\">"
        		"<tt:MessageDescription IsProperty=\"true\">"
        		"<tt:Source>"
        		    "<tt:SimpleItemDescription Name=\"Source\" Type=\"tt:ReferenceToken\"/>"
        		"</tt:Source>"
        		"<tt:Data>"
        		    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:boolean\"/>"
        		"</tt:Data>"					
        		"</tt:MessageDescription>"
        		"</ImagingService>"
    		"</ImageTooBlurry>"
    	"</tns1:VideoSource>"

        "<tns1:VideoSource wstop:topic=\"true\">"
    		"<ImageTooDark wstop:topic=\"true\">"
        		"<ImagingService wstop:topic=\"true\">"
        		"<tt:MessageDescription IsProperty=\"true\">"
        		"<tt:Source>"
        		    "<tt:SimpleItemDescription Name=\"Source\" Type=\"tt:ReferenceToken\"/>"
        		"</tt:Source>"
        		"<tt:Data>"
        		    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:boolean\"/>"
        		"</tt:Data>"					
        		"</tt:MessageDescription>"
        		"</ImagingService>"
    		"</ImageTooDark>"
    	"</tns1:VideoSource>"

        "<tns1:VideoSource wstop:topic=\"true\">"
    		"<ImageTooBright wstop:topic=\"true\">"
        		"<ImagingService wstop:topic=\"true\">"
        		"<tt:MessageDescription IsProperty=\"true\">"
        		"<tt:Source>"
        		    "<tt:SimpleItemDescription Name=\"Source\" Type=\"tt:ReferenceToken\"/>"
        		"</tt:Source>"
        		"<tt:Data>"
        		    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:boolean\"/>"
        		"</tt:Data>"					
        		"</tt:MessageDescription>"
        		"</ImagingService>"
    		"</ImageTooBright>"
    	"</tns1:VideoSource>"

        "<tns1:VideoSource wstop:topic=\"true\">"
    		"<GlobalSceneChange wstop:topic=\"true\">"
        		"<ImagingService wstop:topic=\"true\">"
        		"<tt:MessageDescription IsProperty=\"true\">"
        		"<tt:Source>"
        		    "<tt:SimpleItemDescription Name=\"Source\" Type=\"tt:ReferenceToken\"/>"
        		"</tt:Source>"
        		"<tt:Data>"
        		    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:boolean\"/>"
        		"</tt:Data>"					
        		"</tt:MessageDescription>"
        		"</ImagingService>"
    		"</GlobalSceneChange>"
    	"</tns1:VideoSource>"

        "<tns1:VideoSource wstop:topic=\"true\">"
    		"<SignalLoss wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"true\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"Source\" Type=\"tt:ReferenceToken\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:boolean\"/>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</SignalLoss>"
    	"</tns1:VideoSource>"

		"<tns1:VideoSource wstop:topic=\"true\">"
			"<MotionAlarm wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
				"<tt:SimpleItemDescription Name=\"Source\" Type=\"tt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:boolean\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"	
			"</MotionAlarm>"
		"</tns1:VideoSource>"
    );

    return offset;
}

#ifdef MEDIA2_SUPPORT

int bdMedia2_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 
        // tns1:Media/ProfileChanged
		"<tns1:Media wstop:topic=\"true\">"
			"<ProfileChanged wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
				"<tt:SimpleItemDescription Name=\"Token\" Type=\"tt:ReferenceToken\"/>"
			"</tt:Source>"				
			"</tt:MessageDescription>"					
			"</ProfileChanged>"
		"</tns1:Media>"

		// tns1:Media/ConfigurationChanged
		"<tns1:Media wstop:topic=\"true\">"
			"<ConfigurationChanged wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
				"<tt:SimpleItemDescription Name=\"Token\" Type=\"tt:ReferenceToken\"/>"
				"<tt:SimpleItemDescription Name=\"Type\" Type=\"xs:string\"/>"
			"</tt:Source>"				
			"</tt:MessageDescription>"					
			"</ConfigurationChanged>"
		"</tns1:Media>"
    );

    return offset; 
}

#endif // end of MEDIA2_SUPPORT

#ifdef VIDEO_ANALYTICS

int bdAnalytics_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 
        // tns1:RuleEngine/MotionRegionDetector/Motion
		"<tns1:RuleEngine wstop:topic=\"true\">"
			"<MotionRegionDetector wstop:topic=\"true\">"
			"<Motion wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
				"<tt:SimpleItemDescription Name=\"VideoSource\" Type=\"tt:ReferenceToken\"/>"
				"<tt:SimpleItemDescription Name=\"RuleName\" Type=\"xs:string\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:boolean\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</Motion>"
			"</MotionRegionDetector>"
		"</tns1:RuleEngine>"
    );

    return offset;        
}

#endif

#ifdef DEVICEIO_SUPPORT

int bdDeviceIO_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 
        // tns1:Device/Trigger/DigitalInput
        "<tns1:Device wstop:topic=\"true\">"
    		"<Trigger wstop:topic=\"true\">"
    		"<DigitalInput wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"true\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"InputToken\" Type=\"tt:ReferenceToken\" />"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:SimpleItemDescription Name=\"LogicalState\" Type=\"xs:boolean\" />"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</DigitalInput>"
    		"</Trigger>"
    	"</tns1:Device>"

    	// tns1:Device/Trigger/Relay
        "<tns1:Device wstop:topic=\"true\">"
    		"<Trigger wstop:topic=\"true\">"
    		"<Relay wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"true\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RelayToken\" Type=\"tt:ReferenceToken\" />"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:SimpleItemDescription Name=\"LogicalState\" Type=\"tt:RelayLogicalState\" />"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</Relay>"
    		"</Trigger>"
    	"</tns1:Device>"
    );

    return offset;
}

#endif // end if DEVICEIO_SUPPORT

#ifdef PROFILE_G_SUPPORT

int bdProfileG_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 
        // tns1:RecordingConfig/CreateRecording
        "<tns1:RecordingConfig wstop:topic=\"true\">"
    		"<CreateRecording wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"false\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingToken\" Type=\"tt:RecordingReference\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</CreateRecording>"
    	"</tns1:RecordingConfig>"

    	// tns1:RecordingConfig/DeleteRecording
        "<tns1:RecordingConfig wstop:topic=\"true\">"
    		"<DeleteRecording wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"false\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingToken\" Type=\"tt:RecordingReference\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</DeleteRecording>"
    	"</tns1:RecordingConfig>"

    	// tns1:RecordingConfig/CreateTrack
        "<tns1:RecordingConfig wstop:topic=\"true\">"
    		"<CreateTrack wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"false\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingToken\" Type=\"tt:RecordingReference\"/>"
    		    "<tt:SimpleItemDescription Name=\"TrackToken\" Type=\"tt:TrackReference\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</CreateTrack>"
    	"</tns1:RecordingConfig>"

    	// tns1:RecordingConfig/DeleteTrack
        "<tns1:RecordingConfig wstop:topic=\"true\">"
    		"<DeleteTrack wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"false\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingToken\" Type=\"tt:RecordingReference\"/>"
    		    "<tt:SimpleItemDescription Name=\"TrackToken\" Type=\"tt:TrackReference\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</DeleteTrack>"
    	"</tns1:RecordingConfig>"

    	// tns1:RecordingConfig/JobState
        "<tns1:RecordingConfig wstop:topic=\"true\">"
    		"<JobState wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"true\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingJobToken\" Type=\"tt:RecordingJobReference\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:string\" />"
    		    "<tt:ElementItemDescription Name=\"Information\" Type=\"tt:RecordingJobStateInformation\"/>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</JobState>"
    	"</tns1:RecordingConfig>"

    	// tns1:RecordingConfig/RecordingConfiguration
        "<tns1:RecordingConfig wstop:topic=\"true\">"
    		"<RecordingConfiguration wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"false\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingToken\" Type=\"tt:RecordingReference\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:ElementItemDescription Name=\"Configuration\" Type=\"tt:RecordingConfiguration\"/>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</RecordingConfiguration>"
    	"</tns1:RecordingConfig>"

    	// tns1:RecordingConfig/TrackConfiguration
        "<tns1:RecordingConfig wstop:topic=\"true\">"
    		"<TrackConfiguration wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"false\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingToken\" Type=\"tt:RecordingReference\"/>"
    		    "<tt:SimpleItemDescription Name=\"TrackToken\" Type=\"tt:TrackReference\" />"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:ElementItemDescription Name=\"Configuration\" Type=\"tt:TrackConfiguration\"/>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</TrackConfiguration>"
    	"</tns1:RecordingConfig>"

    	// tns1:RecordingConfig/RecordingJobConfiguration
        "<tns1:RecordingConfig wstop:topic=\"true\">"
    		"<RecordingJobConfiguration wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"false\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingJobToken\" Type=\"tt:RecordingJobReference\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:ElementItemDescription Name=\"Configuration\" Type=\"tt:RecordingJobConfiguration\"/>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</RecordingJobConfiguration>"
    	"</tns1:RecordingConfig>"

    	// tns1:RecordingConfig/DeleteTrackData
    	"<tns1:RecordingConfig wstop:topic=\"true\">"
    		"<DeleteTrackData wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"false\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingToken\" Type=\"tt:RecordingReference\"/>"
    		    "<tt:SimpleItemDescription Name=\"TrackToken\" Type=\"tt:TrackReference\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:SimpleItemDescription Name=\"StartTime\" Type=\"xs:dateTime\"/>"
    		    "<tt:SimpleItemDescription Name=\"EndTime\" Type=\"xs:dateTime\"/>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</DeleteTrackData>"
    	"</tns1:RecordingConfig>"

    	// tns1:RecordingHistory/Recording/State
    	"<tns1:RecordingHistory wstop:topic=\"true\">"
    		"<Recording wstop:topic=\"true\">"
    		"<State wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"true\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingToken\" Type=\"tt:ReferenceToken\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:SimpleItemDescription Name=\"IsRecording\" Type=\"xs:boolean\"/>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</State>"
    		"</Recording>"
    	"</tns1:RecordingHistory>"

    	// tns1:RecordingHistory/Track/State
    	"<tns1:RecordingHistory wstop:topic=\"true\">"
    		"<Track wstop:topic=\"true\">"
    		"<State wstop:topic=\"true\">"
    		"<tt:MessageDescription IsProperty=\"true\">"
    		"<tt:Source>"
    		    "<tt:SimpleItemDescription Name=\"RecordingToken\" Type=\"tt:ReferenceToken\"/>"
    		    "<tt:SimpleItemDescription Name=\"Track\" Type=\"tt:ReferenceToken\"/>"
    		"</tt:Source>"
    		"<tt:Data>"
    		    "<tt:SimpleItemDescription Name=\"IsDataPresent\" Type=\"xs:boolean\"/>"
    		"</tt:Data>"					
    		"</tt:MessageDescription>"
    		"</State>"
    		"</Track>"
    	"</tns1:RecordingHistory>"
    );

    return offset;
}

#endif // end of PROFILE_G_SUPPORT

#ifdef PROFILE_C_SUPPORT

int bdProfileC_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 
    
        // tns1:AccessControl/AccessGranted/Anonymous
        "<tns1:AccessControl wstop:topic=\"true\">"
			"<AccessGranted wstop:topic=\"true\">"
			"<Anonymous wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"External\" Type=\"xs:boolean\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</Anonymous>"
			"</AccessGranted>"
		"</tns1:AccessControl>"

		// tns1:AccessControl/AccessGranted/Credential
        "<tns1:AccessControl wstop:topic=\"true\">"
			"<AccessGranted wstop:topic=\"true\">"
			"<Credential wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"External\" Type=\"xs:boolean\"/>"
			    "<tt:SimpleItemDescription Name=\"CredentialToken\" Type=\"pt:ReferenceToken\"/>"
			    "<tt:SimpleItemDescription Name=\"CredentialHolderName\" Type=\"xs:string\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</Credential>"
			"</AccessGranted>"
		"</tns1:AccessControl>"

		// tns1:AccessControl/AccessTaken/Anonymous
        "<tns1:AccessControl wstop:topic=\"true\">"
			"<AccessTaken wstop:topic=\"true\">"
			"<Anonymous wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Anonymous>"
			"</AccessTaken>"
		"</tns1:AccessControl>"

		// tns1:AccessControl/AccessTaken/Credential
        "<tns1:AccessControl wstop:topic=\"true\">"
			"<AccessTaken wstop:topic=\"true\">"
			"<Credential wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"CredentialToken\" Type=\"pt:ReferenceToken\"/>"
			    "<tt:SimpleItemDescription Name=\"CredentialHolderName\" Type=\"xs:string\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</Credential>"
			"</AccessTaken>"
		"</tns1:AccessControl>"

		// tns1:AccessControl/AccessNotTaken/Anonymous
        "<tns1:AccessControl wstop:topic=\"true\">"
			"<AccessNotTaken wstop:topic=\"true\">"
			"<Anonymous wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Anonymous>"
			"</AccessNotTaken>"
		"</tns1:AccessControl>"

		// tns1:AccessControl/AccessNotTaken/Credential
        "<tns1:AccessControl wstop:topic=\"true\">"
			"<AccessNotTaken wstop:topic=\"true\">"
			"<Credential wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"CredentialToken\" Type=\"pt:ReferenceToken\"/>"
			    "<tt:SimpleItemDescription Name=\"CredentialHolderName\" Type=\"xs:string\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</Credential>"
			"</AccessNotTaken>"
		"</tns1:AccessControl>"

		// tns1:tns1:AccessControl/Denied/Anonymous
        "<tns1:AccessControl wstop:topic=\"true\">"
			"<Denied wstop:topic=\"true\">"
			"<Anonymous wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"External\" Type=\"xs:boolean\"/>"
			    "<tt:SimpleItemDescription Name=\"Reason\" Type=\"xs:string\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</Anonymous>"
			"</Denied>"
		"</tns1:AccessControl>"

		// tns1:AccessControl/Denied/Credential
        "<tns1:AccessControl wstop:topic=\"true\">"
			"<Denied wstop:topic=\"true\">"
			"<Credential wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"External\" Type=\"xs:boolean\"/>"
			    "<tt:SimpleItemDescription Name=\"CredentialToken\" Type=\"pt:ReferenceToken\"/>"
			    "<tt:SimpleItemDescription Name=\"CredentialHolderName\" Type=\"xs:string\"/>"
			    "<tt:SimpleItemDescription Name=\"Reason\" Type=\"xs:string\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</Credential>"
			"</Denied>"
		"</tns1:AccessControl>"

        // tns1:AccessControl/Denied/CredentialNotFound
        "<tns1:AccessControl wstop:topic=\"true\">"
			"<Denied wstop:topic=\"true\">"
			"<CredentialNotFound wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"IdentifierType\" Type=\"xs:string\"/>"
			    "<tt:SimpleItemDescription Name=\"IdentifierValue\" Type=\"xs:hexBinary\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"
			"</CredentialNotFound>"
			"</Denied>"
		"</tns1:AccessControl>"
		
		// tns1:AccessControl/Denied/CredentialNotFound/Card
		"<tns1:AccessControl wstop:topic=\"true\">"
			"<Denied wstop:topic=\"true\">"
			"<CredentialNotFound wstop:topic=\"true\">"
			"<Card wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"Card\" Type=\"xs:string\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"
			"</Card>"
			"</CredentialNotFound>"
			"</Denied>"
		"</tns1:AccessControl>"

		// tns1:AccessControl/AccessGranted/Identifier
		"<tns1:AccessControl wstop:topic=\"true\">"
			"<AccessGranted wstop:topic=\"true\">"
			"<Identifier wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"IdentifierType\" Type=\"xs:string\"/>"
			    "<tt:SimpleItemDescription Name=\"FormatType\" Type=\"xs:string\"/>"
			    "<tt:SimpleItemDescription Name=\"IdentifierValue\" Type=\"xs:hexBinary\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"
			"</Identifier>"
			"</AccessGranted>"
		"</tns1:AccessControl>"

		// tns1:AccessControl/Denied/Identifier
		"<tns1:AccessControl wstop:topic=\"true\">"
			"<Denied wstop:topic=\"true\">"
			"<Identifier wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"IdentifierType\" Type=\"xs:string\"/>"
			    "<tt:SimpleItemDescription Name=\"FormatType\" Type=\"xs:string\"/>"
			    "<tt:SimpleItemDescription Name=\"IdentifierValue\" Type=\"xs:hexBinary\"/>"
			    "<tt:SimpleItemDescription Name=\"Reason\" Type=\"xs:string\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"
			"</Identifier>"
			"</Denied>"
		"</tns1:AccessControl>"

		// tns1:AccessControl/Duress
		"<tns1:AccessControl wstop:topic=\"true\">"
			"<Duress wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"CredentialToken\" Type=\"pt:ReferenceToken\"/>"
			    "<tt:SimpleItemDescription Name=\"CredentialHolderName\" Type=\"xs:string\"/>"
			    "<tt:SimpleItemDescription Name=\"Reason\" Type=\"xs:string\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"
			"</Duress>"
		"</tns1:AccessControl>"
		
        // tns1:AccessPoint/State/Enabled
        "<tns1:AccessPoint wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<Enabled wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:boolean\"/>"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</Enabled>"
			"</State>"
		"</tns1:AccessPoint>"

        // tns1:Configuration/AccessPoint/Changed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<AccessPoint wstop:topic=\"true\">"
			"<Changed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Changed>"
			"</AccessPoint>"
		"</tns1:Configuration>"

		// tns1:Configuration/AccessPoint/Removed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<AccessPoint wstop:topic=\"true\">"
			"<Removed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessPointToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Removed>"
			"</AccessPoint>"
		"</tns1:Configuration>"

		// Topic: tns1:Configuration/Area/Changed
		"<tns1:Configuration wstop:topic=\"true\">"
			"<Area wstop:topic=\"true\">"
			"<Changed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AreaToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Changed>"
			"</Area>"
		"</tns1:Configuration>"

		// tns1:Configuration/Area/Removed
		"<tns1:Configuration wstop:topic=\"true\">"
			"<Area wstop:topic=\"true\">"
			"<Removed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AreaToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Removed>"
			"</Area>"
		"</tns1:Configuration>"

		// tns1:Door/State/DoorMode
        "<tns1:Door wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<DoorMode wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"DoorToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"	
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"tdc:DoorMode\"/>"
			"</tt:Data>"
			"</tt:MessageDescription>"					
			"</DoorMode>"
			"</State>"
		"</tns1:Door>"

		// tns1:Door/State/DoorPhysicalState
		"<tns1:Door wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<DoorPhysicalState wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"DoorToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"	
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"tdc:DoorPhysicalState\"/>"
			"</tt:Data>"
			"</tt:MessageDescription>"					
			"</DoorPhysicalState>"
			"</State>"
		"</tns1:Door>"

		// tns1:Door/State/LockPhysicalState
		"<tns1:Door wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<LockPhysicalState wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"DoorToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"	
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"tdc:LockPhysicalState\"/>"
			"</tt:Data>"
			"</tt:MessageDescription>"					
			"</LockPhysicalState>"
			"</State>"
		"</tns1:Door>"

		// tns1:Door/State/DoubleLockPhysicalState
		"<tns1:Door wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<DoubleLockPhysicalState wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"DoorToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"	
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"tdc:LockPhysicalState\"/>"
			"</tt:Data>"
			"</tt:MessageDescription>"					
			"</DoubleLockPhysicalState>"
			"</State>"
		"</tns1:Door>"

        // tns1:Door/State/DoorAlarm
        "<tns1:Door wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<DoorAlarm wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"DoorToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"	
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"tdc:DoorAlarmState\"/>"
			"</tt:Data>"
			"</tt:MessageDescription>"					
			"</DoorAlarm>"
			"</State>"
		"</tns1:Door>"

        // tns1:Door/State/DoorTamper
        "<tns1:Door wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<DoorTamper wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"DoorToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"	
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"tdc:DoorTamperState\"/>"
			"</tt:Data>"
			"</tt:MessageDescription>"					
			"</DoorTamper>"
			"</State>"
		"</tns1:Door>"

		// tns1:Door/State/DoorFault
		"<tns1:Door wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<DoorFault wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"DoorToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"	
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"tdc:DoorFaultState\"/>"
			    "<tt:SimpleItemDescription Name=\"Reason\" Type=\"xs:string\"/>"
			"</tt:Data>"
			"</tt:MessageDescription>"					
			"</DoorFault>"
			"</State>"
		"</tns1:Door>"
		
		// tns1:Configuration/Door/Changed
		"<tns1:Configuration wstop:topic=\"true\">"
			"<Door wstop:topic=\"true\">"
			"<Changed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"DoorToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"</tt:MessageDescription>"					
			"</Changed>"
			"</Door>"
		"</tns1:Configuration>"
		
		// tns1:Configuration/Door/Removed
		"<tns1:Configuration wstop:topic=\"true\">"
			"<Door wstop:topic=\"true\">"
			"<Removed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"DoorToken\" Type=\"pt:ReferenceToken\"/>"
			"</tt:Source>"
			"</tt:MessageDescription>"					
			"</Removed>"
			"</Door>"
		"</tns1:Configuration>"
	);

    return offset;				
}

#endif // end of PROFILE_C_SUPPORT

#ifdef CREDENTIAL_SUPPORT

int bdCredential_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 
    
        // tns1:Credential/State/Enabled
        "<tns1:Credential wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<Enabled wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"CredentialToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"State\" Type=\"xs:boolean\" />"
			    "<tt:SimpleItemDescription Name=\"Reason\" Type=\"xs:string\" />"
			    "<tt:SimpleItemDescription Name=\"ClientUpdated\" Type=\"xs:boolean\" />"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</Enabled>"
			"</State>"
		"</tns1:Credential>"

		// tns1:Credential/State/ApbViolation
        "<tns1:Credential wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<ApbViolation wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"CredentialToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"ApbViolation\" Type=\"xs:boolean\" />"
			    "<tt:SimpleItemDescription Name=\"ClientUpdated\" Type=\"xs:boolean\" />"
			"</tt:Data>"					
			"</tt:MessageDescription>"					
			"</ApbViolation>"
			"</State>"
		"</tns1:Credential>"

		// tns1:Configuration/Credential/Changed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<Credential wstop:topic=\"true\">"
			"<Changed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"CredentialToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Changed>"
			"</Credential>"
		"</tns1:Configuration>"

		// tns1:Configuration/Credential/Removed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<Credential wstop:topic=\"true\">"
			"<Removed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"CredentialToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Removed>"
			"</Credential>"
		"</tns1:Configuration>"
	);

	return offset;
}

#endif // end of CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES

int bdAccessRules_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 

		// tns1:Configuration/AccessProfile/Changed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<AccessProfile wstop:topic=\"true\">"
			"<Changed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessProfileToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Changed>"
			"</AccessProfile>"
		"</tns1:Configuration>"

		// tns1:Configuration/AccessProfile/Removed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<AccessProfile wstop:topic=\"true\">"
			"<Removed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"AccessProfileToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Removed>"
			"</AccessProfile>"
		"</tns1:Configuration>"
	);

	return offset;
}

#endif // end of ACCESS_RULES

#ifdef SCHEDULE_SUPPORT

int bdSchedule_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 

		// tns1:Schedule/State/Active
        "<tns1:Schedule wstop:topic=\"true\">"
			"<State wstop:topic=\"true\">"
			"<Active wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"true\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"ScheduleToken\" Type=\"pt:ReferenceToken\" />"
			    "<tt:SimpleItemDescription Name=\"Name\" Type=\"xs:string\" />"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"Active\" Type=\"xs:boolean\" />"
			    "<tt:SimpleItemDescription Name=\"SpecialDay\" Type=\"xs:boolean\" />"
			"</tt:Data>"
			"</tt:MessageDescription>"					
			"</Active>"
			"</State>"
		"</tns1:Schedule>"

		// tns1:Configuration/Schedule/Changed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<Schedule wstop:topic=\"true\">"
			"<Changed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"ScheduleToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Changed>"
			"</Schedule>"
		"</tns1:Configuration>"

		// tns1:Configuration/Schedule/Removed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<Schedule wstop:topic=\"true\">"
			"<Removed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"ScheduleToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Removed>"
			"</Schedule>"
		"</tns1:Configuration>"

		// tns1:Configuration/SpecialDays/Changed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<SpecialDays wstop:topic=\"true\">"
			"<Changed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"SpecialDaysToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Changed>"
			"</SpecialDays>"
		"</tns1:Configuration>"

		// tns1:Configuration/SpecialDays/Removed
        "<tns1:Configuration wstop:topic=\"true\">"
			"<SpecialDays wstop:topic=\"true\">"
			"<Removed wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"SpecialDaysToken\" Type=\"pt:ReferenceToken\" />"
			"</tt:Source>"					
			"</tt:MessageDescription>"					
			"</Removed>"
			"</SpecialDays>"
		"</tns1:Configuration>"
	);

	return offset;
}

#endif // end of SCHEDULE_SUPPORT

#ifdef RECEIVER_SUPPORT

int bdReceiver_EventProperties_xml(char * p_buf, int mlen)
{
    int offset = snprintf(p_buf, mlen, 

		// tns1:Receiver/ChangeState
        "<tns1:Receiver wstop:topic=\"true\">"
			"<ChangeState wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"ReceiverToken\" Type=\"tt:ReferenceToken\" />"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"NewState\" Type=\"tt:ReceiverState\" />"
			    "<tt:SimpleItemDescription Name=\"MediaUri\" Type=\"tt:MediaUri\" />"
			"</tt:Data>"
			"</tt:MessageDescription>"
			"</ChangeState>"
		"</tns1:Receiver>"

		// tns1:Receiver/ConnectionFailed
        "<tns1:Receiver wstop:topic=\"true\">"
			"<ChangeState wstop:topic=\"true\">"
			"<tt:MessageDescription IsProperty=\"false\">"
			"<tt:Source>"
			    "<tt:SimpleItemDescription Name=\"ReceiverToken\" Type=\"tt:ReferenceToken\" />"
			"</tt:Source>"
			"<tt:Data>"
			    "<tt:SimpleItemDescription Name=\"MediaUri\" Type=\"tt:MediaUri\" />"
			"</tt:Data>"
			"</tt:MessageDescription>"					
			"</ChangeState>"
		"</tns1:Receiver>");

	return offset;
}

#endif // end of RECEIVER_SUPPORT

/***************************************************************************************/

int bdSimpleItem_xml(char * p_buf, int mlen, onvif_SimpleItem * p_req)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:SimpleItem Name=\"%s\" Value=\"%s\" />",
		p_req->Name, p_req->Value);

	return offset;
}

int bdElementItem_xml(char * p_buf, int mlen, onvif_ElementItem * p_req)
{
	int offset = 0;

	if (p_req->AnyFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItem Name=\"%s\">%s</tt:ElementItem>", p_req->Name, p_req->Any);
	}
	else
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItem Name=\"%s\" />", p_req->Name);
	}

	return offset;
}

int bdMessage_xml(char * p_buf, int mlen, onvif_Message * p_req)
{
	int offset = 0;
	char utctime[64];
	SimpleItemList * p_simpleitem;
	ElementItemList * p_elementitem;
	struct tm *gtime;
	
	gtime = gmtime(&p_req->UtcTime);	
	snprintf(utctime, sizeof(utctime), "%04d-%02d-%02dT%02d:%02d:%02dZ", 		 
		gtime->tm_year+1900, gtime->tm_mon+1, gtime->tm_mday, gtime->tm_hour, gtime->tm_min, gtime->tm_sec);
		
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Message UtcTime=\"%s\"", utctime);
	if (p_req->PropertyOperationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " PropertyOperation=\"%s\"", 
			onvif_PropertyOperationToString(p_req->PropertyOperation));
	}
	offset += snprintf(p_buf+offset, mlen-offset, ">");

	if (p_req->SourceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Source>");
		p_simpleitem = p_req->Source.SimpleItem;
		while (p_simpleitem)
		{
			offset += bdSimpleItem_xml(p_buf+offset, mlen-offset, &p_simpleitem->SimpleItem);
			p_simpleitem = p_simpleitem->next;
		}
		
		p_elementitem = p_req->Source.ElementItem;
		while (p_elementitem)
		{
			offset += bdElementItem_xml(p_buf+offset, mlen-offset, &p_elementitem->ElementItem);
			p_elementitem = p_elementitem->next;
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Source>");
	}

	if (p_req->KeyFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Key>");
		p_simpleitem = p_req->Key.SimpleItem;
		while (p_simpleitem)
		{
			offset += bdSimpleItem_xml(p_buf+offset, mlen-offset, &p_simpleitem->SimpleItem);
			p_simpleitem = p_simpleitem->next;
		}

		p_elementitem = p_req->Key.ElementItem;
		while (p_elementitem)
		{
			offset += bdElementItem_xml(p_buf+offset, mlen-offset, &p_elementitem->ElementItem);
			p_elementitem = p_elementitem->next;
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Key>");
	}

	if (p_req->DataFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Data>");
		p_simpleitem = p_req->Data.SimpleItem;
		while (p_simpleitem)
		{
			offset += bdSimpleItem_xml(p_buf+offset, mlen-offset, &p_simpleitem->SimpleItem);
			p_simpleitem = p_simpleitem->next;
		}

		p_elementitem = p_req->Data.ElementItem;
		while (p_elementitem)
		{
			offset += bdElementItem_xml(p_buf+offset, mlen-offset, &p_elementitem->ElementItem);
			p_elementitem = p_elementitem->next;
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Data>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Message>");		
	
	return offset;
}

int bdNotificationMessage_xml(char * p_buf, int mlen, onvif_NotificationMessage * p_req)
{
	int offset = 0;
		
	offset += snprintf(p_buf+offset, mlen-offset, 		
		"<wsnt:Topic Dialect=\"%s\">%s</wsnt:Topic>",
		p_req->Dialect,
		p_req->Topic);

	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:Message>");
	offset += bdMessage_xml(p_buf+offset, mlen-offset, &p_req->Message);
	offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:Message>");

	return offset;
}

int bdNotify_xml(char * p_buf, int mlen, const char * argv)
{
	NotificationMessageList * p_message = (NotificationMessageList *)argv;
	NotificationMessageList * p_tmp = p_message;
	
	int offset = snprintf(p_buf, mlen, "%s", xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, "%s", onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://docs.oasis-open.org/wsn/bw-2/NotificationConsumer/Notify");
	offset += snprintf(p_buf+offset, mlen-offset, "%s", soap_body);
		
	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:Notify>");
	while (p_tmp)
	{
    	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:NotificationMessage>");   
    	offset += bdNotificationMessage_xml(p_buf+offset, mlen-offset, &p_tmp->NotificationMessage);
		offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:NotificationMessage>");
		
		p_tmp = p_tmp->next;
	}			
	offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:Notify>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "%s", soap_tailer);

	return offset;
}

int bdtev_GetEventProperties_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tev:GetEventPropertiesResponse>"
			"<tev:TopicNamespaceLocation>"
				"http://www.onvif.org/onvif/ver10/topics/topicns.xml"
			"</tev:TopicNamespaceLocation>"
			"<wsnt:FixedTopicSet>true</wsnt:FixedTopicSet>"
			"<wstop:TopicSet xmlns=\"\">");

#ifdef PROFILE_Q_SUPPORT
    offset += bdBase_EventProperties_xml(p_buf+offset, mlen-offset);
#endif

    offset += bdImaging_EventProperties_xml(p_buf+offset, mlen-offset);

#ifdef MEDIA2_SUPPORT
    offset += bdMedia2_EventProperties_xml(p_buf+offset, mlen-offset);
#endif

#ifdef VIDEO_ANALYTICS
    offset += bdAnalytics_EventProperties_xml(p_buf+offset, mlen-offset);            
#endif

#ifdef DEVICEIO_SUPPORT
    offset += bdDeviceIO_EventProperties_xml(p_buf+offset, mlen-offset);
#endif

#ifdef PROFILE_G_SUPPORT
    offset += bdProfileG_EventProperties_xml(p_buf+offset, mlen-offset);                
#endif

#ifdef PROFILE_C_SUPPORT
    offset += bdProfileC_EventProperties_xml(p_buf+offset, mlen-offset);                
#endif

#ifdef CREDENTIAL_SUPPORT
    offset += bdCredential_EventProperties_xml(p_buf+offset, mlen-offset);  
#endif

#ifdef ACCESS_RULES
    offset += bdAccessRules_EventProperties_xml(p_buf+offset, mlen-offset);
#endif

#ifdef SCHEDULE_SUPPORT
    offset += bdSchedule_EventProperties_xml(p_buf+offset, mlen-offset);
#endif

#ifdef RECEIVER_SUPPORT
    offset += bdReceiver_EventProperties_xml(p_buf+offset, mlen-offset);
#endif

    offset += snprintf(p_buf+offset, mlen-offset, 				
			"</wstop:TopicSet>"	

			"<wsnt:TopicExpressionDialect>"
				"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet"										
			"</wsnt:TopicExpressionDialect>"
			"<wsnt:TopicExpressionDialect>"
				"http://docs.oasis-open.org/wsnt/t-1/TopicExpression/ConcreteSet"
			"</wsnt:TopicExpressionDialect>"
			"<wsnt:TopicExpressionDialect>"
				"http://docs.oasis-open.org/wsn/t-1/TopicExpression/Concrete"
			"</wsnt:TopicExpressionDialect>"	
			"<tev:MessageContentFilterDialect>"
				"http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter"
			"</tev:MessageContentFilterDialect>"
			"<tev:MessageContentSchemaLocation>"
				"http://www.onvif.org/onvif/ver10/schema/onvif.xsd"
			"</tev:MessageContentSchemaLocation>"
		"</tev:GetEventPropertiesResponse>");

	return offset;
}

int bdtev_Subscribe_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	EUA * p_eua = (EUA *) argv;
	char cur_time[100], term_time[100];
	
	if (NULL == p_eua)
	{
		return -1;
	}

	onvif_get_time_str(cur_time, sizeof(cur_time), 0);	
	
	if (g_onvif_cfg.evt_renew_time < p_eua->init_term_time)
	{
		onvif_get_time_str(term_time, sizeof(term_time), g_onvif_cfg.evt_renew_time);
	}
	else
	{
		onvif_get_time_str(term_time, sizeof(term_time), p_eua->init_term_time);
	}
	
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<wsnt:SubscribeResponse>"
	        "<wsnt:SubscriptionReference>"
	            "<wsa:Address>%s</wsa:Address>"
	        "</wsnt:SubscriptionReference>"
	        "<wsnt:CurrentTime>%s</wsnt:CurrentTime>"
	        "<wsnt:TerminationTime>%s</wsnt:TerminationTime>"
	    "</wsnt:SubscribeResponse>",
	    p_eua->producter_addr, cur_time, term_time);

	return offset;
}


int bdtev_Unsubscribe_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:UnsubscribeResponse></wsnt:UnsubscribeResponse>");
	return offset;
}

int bdtev_Renew_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	char cur_time[100], term_time[100];

	onvif_get_time_str(cur_time, sizeof(cur_time), 0);
	onvif_get_time_str(term_time, sizeof(term_time), g_onvif_cfg.evt_renew_time);
		
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<wsnt:RenewResponse>"			
	        "<wsnt:TerminationTime>%s</wsnt:TerminationTime>"
	        "<wsnt:CurrentTime>%s</wsnt:CurrentTime>"
		"</wsnt:RenewResponse>", term_time, cur_time);

	return offset;
}

int bdtev_CreatePullPointSubscription_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	char cur_time[100], term_time[100];
	EUA * p_eua = (EUA *) argv;

	onvif_get_time_str(cur_time, sizeof(cur_time), 0);

    if (g_onvif_cfg.evt_renew_time < p_eua->init_term_time)
	{
		onvif_get_time_str(term_time, sizeof(term_time), g_onvif_cfg.evt_renew_time);
	}
	else
	{
		onvif_get_time_str(term_time, sizeof(term_time), p_eua->init_term_time);
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tev:CreatePullPointSubscriptionResponse>"	
			"<tev:SubscriptionReference>"
	            "<wsa:Address>%s</wsa:Address>"
	        "</tev:SubscriptionReference>"	        
	        "<wsnt:CurrentTime>%s</wsnt:CurrentTime>"
	        "<wsnt:TerminationTime>%s</wsnt:TerminationTime>"
		"</tev:CreatePullPointSubscriptionResponse>", 
		p_eua->producter_addr,
		cur_time, term_time);

	return offset;
}

int bdtev_PullMessages_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0, msg_nums = 0;
	char cur_time[100], term_time[100];
	LINKED_NODE *p_node, *p_next;
	NotificationMessageList * p_tmp;
	NotificationMessageList * p_message;
	
	tev_PullMessages_REQ * p_req = (tev_PullMessages_REQ *)argv;
	EUA * p_eua = onvif_get_eua_by_index(p_req->eua_idx);

	assert(p_eua);

	onvif_get_time_str(cur_time, sizeof(cur_time), 0);
	onvif_get_time_str(term_time, sizeof(term_time), g_onvif_cfg.evt_renew_time);

	offset += snprintf(p_buf+offset, mlen-offset, "<tev:PullMessagesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, 
        "<tev:CurrentTime>%s</tev:CurrentTime>"
        "<tev:TerminationTime>%s</tev:TerminationTime>",
        cur_time, term_time);

	// get notify message from message list
	
	p_node = h_list_lookback_start(p_eua->msg_list);
	while (p_node && msg_nums < p_req->MessageLimit)
	{
		p_message = (NotificationMessageList *) p_node->p_data;	
		
		p_tmp = p_message;
		while (p_tmp)
		{	
		    if (onvif_event_filter(p_tmp, p_eua))
		    {
				offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:NotificationMessage>");
				offset += bdNotificationMessage_xml(p_buf+offset, mlen-offset, &p_tmp->NotificationMessage);
				offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:NotificationMessage>");

				msg_nums++;
			}

			p_tmp = p_tmp->next;
		}

		p_next = h_list_lookback_next(p_eua->msg_list, p_node);

		h_list_remove(p_eua->msg_list, p_node);
		onvif_free_NotificationMessage(p_message);

		p_node = p_next;
	}
	h_list_lookback_end(p_eua->msg_list);
        
	offset += snprintf(p_buf+offset, mlen-offset, "</tev:PullMessagesResponse>");

	return offset;
}

int bdtev_SetSynchronizationPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;		
	offset += snprintf(p_buf+offset, mlen-offset, "<tev:SetSynchronizationPointResponse />");
	return offset;
}

/***************************************************************************************/

#ifdef IMAGE_SUPPORT

int bdImageSettings_xml(char * p_buf, int mlen)
{
    int offset = 0;
    
    if (g_onvif_cfg.ImagingSettings.BacklightCompensationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:BacklightCompensation>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_BacklightCompensationModeToString(g_onvif_cfg.ImagingSettings.BacklightCompensation.Mode));
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:BacklightCompensation>");
	}

	if (g_onvif_cfg.ImagingSettings.BrightnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Brightness>%0.1f</tt:Brightness>", g_onvif_cfg.ImagingSettings.Brightness);
	}
	if (g_onvif_cfg.ImagingSettings.ColorSaturationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:ColorSaturation>%0.1f</tt:ColorSaturation>", g_onvif_cfg.ImagingSettings.ColorSaturation);
	}
	if (g_onvif_cfg.ImagingSettings.ContrastFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Contrast>%0.1f</tt:Contrast>", g_onvif_cfg.ImagingSettings.Contrast);
	}
	
	if (g_onvif_cfg.ImagingSettings.ExposureFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Exposure>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_ExposureModeToString(g_onvif_cfg.ImagingSettings.Exposure.Mode));
		if (g_onvif_cfg.ImagingSettings.Exposure.PriorityFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Priority>%s</tt:Priority>", onvif_ExposurePriorityToString(g_onvif_cfg.ImagingSettings.Exposure.Priority));
		}

		if (g_onvif_cfg.ImagingSettings.Exposure.MinExposureTimeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MinExposureTime>%0.1f</tt:MinExposureTime>", g_onvif_cfg.ImagingSettings.Exposure.MinExposureTime);
		}
		if (g_onvif_cfg.ImagingSettings.Exposure.MaxExposureTimeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaxExposureTime>%0.1f</tt:MaxExposureTime>", g_onvif_cfg.ImagingSettings.Exposure.MaxExposureTime);
		}
		if (g_onvif_cfg.ImagingSettings.Exposure.MinIrisFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MinIris>%0.1f</tt:MinIris>", g_onvif_cfg.ImagingSettings.Exposure.MinIris);
		}
		if (g_onvif_cfg.ImagingSettings.Exposure.MaxIrisFlag)
		{
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaxIris>%0.1f</tt:MaxIris>", g_onvif_cfg.ImagingSettings.Exposure.MaxIris);
	    }	
	    if (g_onvif_cfg.ImagingSettings.Exposure.GainFlag)
	    {
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Gain>%0.1f</tt:Gain>", g_onvif_cfg.ImagingSettings.Exposure.Gain);
	    }	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Exposure>");			
	}
    if (g_onvif_cfg.ImagingSettings.IrCutFilterFlag)
    {
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilter>%s</tt:IrCutFilter>", onvif_IrCutFilterModeToString(g_onvif_cfg.ImagingSettings.IrCutFilter));
	}

	if (g_onvif_cfg.ImagingSettings.SharpnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Sharpness>%0.1f</tt:Sharpness>", g_onvif_cfg.ImagingSettings.Sharpness);
	}


	if (g_onvif_cfg.ImagingSettings.WhiteBalanceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:WhiteBalance>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_WhiteBalanceModeToString(g_onvif_cfg.ImagingSettings.WhiteBalance.Mode));
		if (g_onvif_cfg.ImagingSettings.WhiteBalance.CrGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:CrGain>%0.1f</tt:CrGain>", g_onvif_cfg.ImagingSettings.WhiteBalance.CrGain);
		}
		if (g_onvif_cfg.ImagingSettings.WhiteBalance.CbGainFlag)
		{
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:CbGain>%0.1f</tt:CbGain>", g_onvif_cfg.ImagingSettings.WhiteBalance.CbGain);
	    }	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:WhiteBalance>");	
	}

	return offset;
}
int bdImageSettings2_xml(char * p_buf, int mlen)
{
    int offset = 0;
    
    if (g_onvif_cfg.ImagingSettings.BacklightCompensationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:BacklightCompensation>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_BacklightCompensationModeToString(g_onvif_cfg.ImagingSettings.BacklightCompensation.Mode));
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:BacklightCompensation>");
	}

	if (g_onvif_cfg.ImagingSettings.BrightnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Brightness>%0.1f</tt:Brightness>", g_onvif_cfg.ImagingSettings.Brightness);
	}
	if (g_onvif_cfg.ImagingSettings.ColorSaturationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:ColorSaturation>%0.1f</tt:ColorSaturation>", g_onvif_cfg.ImagingSettings.ColorSaturation);
	}
	if (g_onvif_cfg.ImagingSettings.ContrastFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Contrast>%0.1f</tt:Contrast>", g_onvif_cfg.ImagingSettings.Contrast);
	}

	if (g_onvif_cfg.ImagingSettings.ExposureFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Exposure>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_ExposureModeToString(g_onvif_cfg.ImagingSettings.Exposure.Mode));
		// if (g_onvif_cfg.ImagingSettings.Exposure.PriorityFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Priority>%s</tt:Priority>", onvif_ExposurePriorityToString(g_onvif_cfg.ImagingSettings.Exposure.Priority));
		}
		// if (g_onvif_cfg.ImagingSettings.Exposure.MinExposureTimeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MinExposureTime>%0.1f</tt:MinExposureTime>", g_onvif_cfg.ImagingSettings.Exposure.MinExposureTime);
		}
		// if (g_onvif_cfg.ImagingSettings.Exposure.MaxExposureTimeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaxExposureTime>%0.1f</tt:MaxExposureTime>", g_onvif_cfg.ImagingSettings.Exposure.MaxExposureTime);
		}
		// if (g_onvif_cfg.ImagingSettings.Exposure.MinIrisFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MinIris>%0.1f</tt:MinIris>", g_onvif_cfg.ImagingSettings.Exposure.MinIris);
		}
		// if (g_onvif_cfg.ImagingSettings.Exposure.MaxIrisFlag)
		{
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaxIris>%0.1f</tt:MaxIris>", g_onvif_cfg.ImagingSettings.Exposure.MaxIris);
	    }	
	    // if (g_onvif_cfg.ImagingSettings.Exposure.GainFlag)
	    {
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Gain>%0.1f</tt:Gain>", g_onvif_cfg.ImagingSettings.Exposure.Gain);
	    }	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Exposure>");			
	}

    if (g_onvif_cfg.ImagingSettings.IrCutFilterFlag)
    {
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilter>%s</tt:IrCutFilter>", onvif_IrCutFilterModeToString(g_onvif_cfg.ImagingSettings.IrCutFilter));
	}

	if (g_onvif_cfg.ImagingSettings.SharpnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Sharpness>%0.1f</tt:Sharpness>", g_onvif_cfg.ImagingSettings.Sharpness);
	}


	if (g_onvif_cfg.ImagingSettings.WhiteBalanceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:WhiteBalance>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_WhiteBalanceModeToString(g_onvif_cfg.ImagingSettings.WhiteBalance.Mode));
		if (g_onvif_cfg.ImagingSettings.WhiteBalance.CrGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:CrGain>%0.1f</tt:CrGain>", g_onvif_cfg.ImagingSettings.WhiteBalance.CrGain);
		}
		if (g_onvif_cfg.ImagingSettings.WhiteBalance.CbGainFlag)
		{
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:CbGain>%0.1f</tt:CbGain>", g_onvif_cfg.ImagingSettings.WhiteBalance.CbGain);
	    }	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:WhiteBalance>");	
	}

	return offset;
}

int bdImagePreset_xml(char * p_buf, int mlen, onvif_ImagingPreset * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<timg:Preset token=\"%s\" type=\"%s\">"
            "<timg:Name>%s</timg:Name>"
        "</timg:Preset>",
        p_req->token, 
        p_req->type, 
        p_req->Name);

    return offset;
}

int bdimg_GetImagingSettings_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	img_GetImagingSettings_REQ * p_req = (img_GetImagingSettings_REQ *)argv;
	VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
	if (NULL == p_v_src)
	{
		return ONVIF_ERR_NoSource;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetImagingSettingsResponse><timg:ImagingSettings>");
#if 1
	offset += bdImageSettings_xml(p_buf+offset, mlen-offset);	
#else 

#endif 
	offset += snprintf(p_buf+offset, mlen-offset, "</timg:ImagingSettings></timg:GetImagingSettingsResponse>");

	return offset;
}

int bdimg_GetOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	img_GetOptions_REQ * p_req = (img_GetOptions_REQ *)argv;
	VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
	if (NULL == p_v_src)
	{
		return ONVIF_ERR_NoSource;
	}
#if 0



#else 
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetOptionsResponse><timg:ImagingOptions>");

	if (g_onvif_cfg.ImagingOptions.BacklightCompensationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:BacklightCompensation>");
		if (g_onvif_cfg.ImagingOptions.BacklightCompensation.Mode_OFF)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>OFF</tt:Mode>");
		}
		if (g_onvif_cfg.ImagingOptions.BacklightCompensation.Mode_ON)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>ON</tt:Mode>");
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:BacklightCompensation>");
	}

	if (g_onvif_cfg.ImagingOptions.BrightnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:Brightness>"
	    	    "<tt:Min>%0.1f</tt:Min>"
	    	    "<tt:Max>%0.1f</tt:Max>"
		    "</tt:Brightness>",
			g_onvif_cfg.ImagingOptions.Brightness.Min, 
			g_onvif_cfg.ImagingOptions.Brightness.Max);
	}

	if (g_onvif_cfg.ImagingOptions.ColorSaturationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:ColorSaturation>"
		        "<tt:Min>%0.1f</tt:Min>"
		        "<tt:Max>%0.1f</tt:Max>"
		    "</tt:ColorSaturation>",
			g_onvif_cfg.ImagingOptions.ColorSaturation.Min, 
			g_onvif_cfg.ImagingOptions.ColorSaturation.Max);
	}

	if (g_onvif_cfg.ImagingOptions.ContrastFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:Contrast>"
			    "<tt:Min>%0.1f</tt:Min>"
			    "<tt:Max>%0.1f</tt:Max>"
		    "</tt:Contrast>",
			g_onvif_cfg.ImagingOptions.Contrast.Min, 
			g_onvif_cfg.ImagingOptions.Contrast.Max);
	}
	
	
	if (g_onvif_cfg.ImagingOptions.IrCutFilterMode_ON)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilterModes>ON</tt:IrCutFilterModes>");
	}
	if (g_onvif_cfg.ImagingOptions.IrCutFilterMode_OFF)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilterModes>OFF</tt:IrCutFilterModes>");
	}
	if (g_onvif_cfg.ImagingOptions.IrCutFilterMode_AUTO)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilterModes>AUTO</tt:IrCutFilterModes>");
	}

	if (g_onvif_cfg.ImagingOptions.SharpnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:Sharpness>"
		        "<tt:Min>%0.1f</tt:Min>"
		        "<tt:Max>%0.1f</tt:Max>"
		    "</tt:Sharpness>",
			g_onvif_cfg.ImagingOptions.Sharpness.Min, 
			g_onvif_cfg.ImagingOptions.Sharpness.Max);
	}	


	if (g_onvif_cfg.ImagingOptions.WhiteBalanceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:WhiteBalance>");
		if (g_onvif_cfg.ImagingOptions.WhiteBalance.Mode_AUTO)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>AUTO</tt:Mode>");
		}
		if (g_onvif_cfg.ImagingOptions.WhiteBalance.Mode_MANUAL)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>MANUAL</tt:Mode>");
		}
		if (g_onvif_cfg.ImagingOptions.WhiteBalance.YrGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:YrGain>"
			        "<tt:Min>%0.1f</tt:Min>"
			        "<tt:Max>%0.1f</tt:Max>"
			    "</tt:YrGain>",
				g_onvif_cfg.ImagingOptions.WhiteBalance.YrGain.Min, 
				g_onvif_cfg.ImagingOptions.WhiteBalance.YrGain.Max);
		}	
		if (g_onvif_cfg.ImagingOptions.WhiteBalance.YbGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:YbGain>"
			        "<tt:Min>%0.1f</tt:Min>"
			        "<tt:Max>%0.1f</tt:Max>"
			    "</tt:YbGain>",
				g_onvif_cfg.ImagingOptions.WhiteBalance.YbGain.Min, 
				g_onvif_cfg.ImagingOptions.WhiteBalance.YbGain.Max);
		}	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:WhiteBalance>");
	}
#endif

	offset += snprintf(p_buf+offset, mlen-offset, "</timg:ImagingOptions></timg:GetOptionsResponse>");

	return offset;
}

int bdimg_SetImagingSettings_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:SetImagingSettingsResponse />");
	return offset;
}

int bdimg_GetMoveOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{	
	int offset = 0;
	img_GetMoveOptions_RES * p_res = (img_GetMoveOptions_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetMoveOptionsResponse><timg:MoveOptions>");

	if (p_res->MoveOptions.AbsoluteFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Absolute>");
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Position>");
	    offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_res->MoveOptions.Absolute.Position);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Position>");
	    if (p_res->MoveOptions.Absolute.SpeedFlag)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Speed>");
    	    offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_res->MoveOptions.Absolute.Speed);
    	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Speed>");
	    }
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Absolute>");
	}
	
	if (p_res->MoveOptions.RelativeFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Relative>");
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Distance>");
	    offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_res->MoveOptions.Relative.Distance);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Distance>");
	    if (p_res->MoveOptions.Absolute.SpeedFlag)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Speed>");
    	    offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_res->MoveOptions.Relative.Speed);
    	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Speed>");
	    }
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Relative>");
	}
	
	if (p_res->MoveOptions.ContinuousFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Continuous><tt:Speed>");
	    offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_res->MoveOptions.Continuous.Speed);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Speed></tt:Continuous>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</timg:MoveOptions></timg:GetMoveOptionsResponse>");
	
	return offset;
}

int bdimg_Move_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:MoveResponse />");
	return offset;
}

int bdimg_GetStatus_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	img_GetStatus_RES * p_res = (img_GetStatus_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetStatusResponse>");
	
	if (p_res->Status.FocusStatusFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<timg:Status>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:FocusStatus20>");
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:Position>%0.1f</tt:Position>"
			"<tt:MoveStatus>%s</tt:MoveStatus>",
			p_res->Status.FocusStatus.Position,
			onvif_MoveStatusToString(p_res->Status.FocusStatus.MoveStatus));
		if (p_res->Status.FocusStatus.ErrorFlag && p_res->Status.FocusStatus.Error[0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Error>%s</tt:Error>", p_res->Status.FocusStatus.Error);
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:FocusStatus20>");
		offset += snprintf(p_buf+offset, mlen-offset, "</timg:Status>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</timg:GetStatusResponse>");	
	
	return offset;
}

int bdimg_Stop_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:StopResponse />");
	return offset;
}

int bdimg_GetPresets_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	img_GetPresets_REQ * p_req = (img_GetPresets_REQ *)argv;
	ImagingPresetList * p_preset;
    VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
    if (NULL == p_v_src)
    {
        return ONVIF_ERR_NoVideoSource;
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetPresetsResponse>");

    p_preset = p_v_src->Presets;
    while (p_preset)
    {
        bdImagePreset_xml(p_buf+offset, mlen-offset, &p_preset->Preset);

        p_preset = p_preset->next;
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</timg:GetPresetsResponse>");

	return offset;
}

int bdimg_GetCurrentPreset_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	img_GetCurrentPreset_REQ * p_req = (img_GetCurrentPreset_REQ *)argv;
	ImagingPresetList * p_preset;
    VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
    if (NULL == p_v_src)
    {
        return ONVIF_ERR_NoVideoSource;
    }

	p_preset = onvif_find_ImagingPreset(p_v_src->Presets, p_v_src->CurrentPrestToken);

    offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetCurrentPresetResponse>");

    if (p_preset)
    {
        bdImagePreset_xml(p_buf+offset, mlen-offset, &p_preset->Preset);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</timg:GetCurrentPresetResponse>");

	return offset;
}

int bdimg_SetCurrentPreset_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:SetCurrentPresetResponse />");
	return offset;
}

#endif // IMAGE_SUPPORT

/***************************************************************************************/

#if defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)

int bdMulticastConfiguration_xml(char * p_buf, int mlen, onvif_MulticastConfiguration * p_cfg)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Multicast>"
			"<tt:Address>"
				"<tt:Type>IPv4</tt:Type>"
				"<tt:IPv4Address>%s</tt:IPv4Address>"
			"</tt:Address>"
			"<tt:Port>%d</tt:Port>"
			"<tt:AutoStart>%s</tt:AutoStart>"
		"</tt:Multicast>", 
	    p_cfg->IPv4Address,
	    p_cfg->Port,
	    p_cfg->AutoStart ? "true" : "false");

	return offset;	    
}

int bdOSD_xml(char * p_buf, int mlen, OSDConfigurationList * p_osd)
{
	int offset = 0;
	
	return offset;
}

int bdVideoSourceConfiguration_xml(char * p_buf, int mlen, onvif_VideoSourceConfiguration * p_req)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Name>%s</tt:Name>"
	    "<tt:UseCount>%d</tt:UseCount>"
	    "<tt:SourceToken>%s</tt:SourceToken>"
	    "<tt:Bounds x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" />",
    	p_req->Name, 
        p_req->UseCount, 
        p_req->SourceToken,
        p_req->Bounds.x,
        p_req->Bounds.y,
        p_req->Bounds.width, 
        p_req->Bounds.height);

	return offset;
}

int bdVideoSourceConfigurationOptions_xml(char * p_buf, int mlen, onvif_VideoSourceConfigurationOptions * p_req)
{
    int offset = 0;
    VideoSourceList * p_v_src = g_onvif_cfg.v_src;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:BoundsRange>"
			"<tt:XRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:XRange>"
			"<tt:YRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:YRange>"
			"<tt:WidthRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:WidthRange>"
			"<tt:HeightRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:HeightRange>"
		"</tt:BoundsRange>", 
		p_req->BoundsRange.XRange.Min, 
		p_req->BoundsRange.XRange.Max,
		p_req->BoundsRange.YRange.Min, 
		p_req->BoundsRange.YRange.Max,
		p_req->BoundsRange.WidthRange.Min, 
		p_req->BoundsRange.WidthRange.Max,
		p_req->BoundsRange.HeightRange.Min, 
		p_req->BoundsRange.HeightRange.Max);
	
	while (p_v_src)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:VideoSourceTokensAvailable>%s</tt:VideoSourceTokensAvailable>", 
		    p_v_src->VideoSource.token);
		
		p_v_src = p_v_src->next;
	}

	return offset;
}

int bdMetadataConfiguration_xml(char * p_buf, int mlen, onvif_MetadataConfiguration * p_req)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tt:Name>%s</tt:Name>"
    	"<tt:UseCount>%d</tt:UseCount>",
    	p_req->Name,
    	p_req->UseCount);

	if (p_req->PTZStatusFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset,
			"<tt:PTZStatus>"
				"<tt:Status>%s</tt:Status>"
				"<tt:Position>%s</tt:Position>"
			"</tt:PTZStatus>",
	    	p_req->PTZStatus.Status ? "true" : "false",
	    	p_req->PTZStatus.Position ? "true" : "false");
	}

    if (p_req->EventsFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
			"<tt:Events>"
    			"<tt:Filter>"
    				"<wsnt:TopicExpression Dialect=\"%s\">%s</wsnt:TopicExpression>"
    			"</tt:Filter>"	
			"</tt:Events>",
			p_req->Events.Dialect,
	    	p_req->Events.TopicExpression);
    }
    
	if (p_req->AnalyticsFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset,
			"<tt:Analytics>%s</tt:Analytics>",
	    	p_req->Analytics ? "true" : "false");
	}

    // offset += bdMulticastConfiguration_xml(p_buf+offset, mlen-offset, &p_req->Multicast);

	offset += snprintf(p_buf+offset, mlen-offset,
			"<tt:SessionTimeout>PT%dS</tt:SessionTimeout>",
	    	p_req->SessionTimeout);

    return offset;
}

int bdMetadataConfigurationOptions_xml(char * p_buf, int mlen, onvif_MetadataConfigurationOptions * p_req)
{
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTZStatusFilterOptions>");

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:PanTiltStatusSupported>false</tt:PanTiltStatusSupported>"
		"<tt:ZoomStatusSupported>false</tt:ZoomStatusSupported>");

	if (p_req->PTZStatusFilterOptions.PanTiltPositionSupportedFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:PanTiltPositionSupported>false</tt:PanTiltPositionSupported>");
	}

	if (p_req->PTZStatusFilterOptions.ZoomPositionSupportedFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:ZoomPositionSupported>false</tt:ZoomPositionSupported>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:PTZStatusFilterOptions>");

	return offset;
}

int bdAudioSourceConfiguration_xml(char * p_buf, int mlen, onvif_AudioSourceConfiguration * p_req)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tt:Name>%s</tt:Name>"
	    "<tt:UseCount>%d</tt:UseCount>"
	    "<tt:SourceToken>%s</tt:SourceToken>", 
		p_req->Name, 
        p_req->UseCount, 
        p_req->SourceToken);

	return offset;            
}




int bdAudioEncoderConfiguration_xml(char * p_buf, int mlen, AudioEncoder2ConfigurationList * p_a_enc_cfg)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Name>%s</tt:Name>"
		"<tt:UseCount>%d</tt:UseCount>"
		"<tt:Encoding>%s</tt:Encoding>"
		"<tt:Bitrate>%d</tt:Bitrate>"
		"<tt:SampleRate>%d</tt:SampleRate>", 
		p_a_enc_cfg->Configuration.Name, 
    	p_a_enc_cfg->Configuration.UseCount, 
    	onvif_AudioEncodingToString(p_a_enc_cfg->Configuration.AudioEncoding), 
	    p_a_enc_cfg->Configuration.Bitrate, 
	    p_a_enc_cfg->Configuration.SampleRate); 

	// offset += bdMulticastConfiguration_xml(p_buf+offset, mlen-offset, &p_a_enc_cfg->Configuration.Multicast);

	offset += snprintf(p_buf+offset, mlen-offset, 		
	    "<tt:SessionTimeout>PT%dS</tt:SessionTimeout>", 
		p_a_enc_cfg->Configuration.SessionTimeout);
		
	return offset;    	    
}

int bdAudioDecoderConfiguration_xml(char * p_buf, int mlen, onvif_AudioDecoderConfiguration * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:Name>%s</tt:Name>"
        "<tt:UseCount>%d</tt:UseCount>",
        p_req->Name,
        p_req->UseCount);

    return offset;        
}

int bdAudioSourceConfigurationOptions_xml(char * p_buf, int mlen)
{
	int offset = 0;	
	AudioSourceList * p_a_src = g_onvif_cfg.a_src;
	
	while (p_a_src)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
    		"<tt:InputTokensAvailable>%s</tt:InputTokensAvailable>", 
    		p_a_src->AudioSource.token);
		
		p_a_src = p_a_src->next;
	}
	
	return offset;
}




#endif // defined(MEDIA_SUPPORT) || defined(MEDIA2_SUPPORT)

/***************************************************************************************/

#ifdef MEDIA_SUPPORT

int bdVideoEncoderConfiguration_xml(char * p_buf, int mlen, onvif_VideoEncoder2Configuration * p_v_enc_cfg)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Name>%s</tt:Name>"
		"<tt:UseCount>%d</tt:UseCount>"
	    "<tt:Encoding>%s</tt:Encoding>"
	    "<tt:Resolution>"
	    	"<tt:Width>%d</tt:Width>"
	    	"<tt:Height>%d</tt:Height>"
	    "</tt:Resolution>",
	    p_v_enc_cfg->Name,
	    p_v_enc_cfg->UseCount,
	    onvif_VideoEncodingToString(p_v_enc_cfg->VideoEncoding==VideoEncoding_H264? VideoEncoding_H264:VideoEncoding_H265), 
	    p_v_enc_cfg->Resolution.Width, 
	    p_v_enc_cfg->Resolution.Height); 

	// if (p_v_enc_cfg->RateControlFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset,     
		    "<tt:RateControl>"
		    	"<tt:FrameRateLimit>%d</tt:FrameRateLimit>"
		    	"<tt:EncodingInterval>%d</tt:EncodingInterval>"
		    	"<tt:BitrateLimit>%d</tt:BitrateLimit>"
		    "</tt:RateControl>",		    
		    (int)p_v_enc_cfg->RateControl.FrameRateLimit,
		    p_v_enc_cfg->RateControl.EncodingInterval, 
		    p_v_enc_cfg->RateControl.BitrateLimit);
	}

	if (p_v_enc_cfg->VideoEncoding == VideoEncoding_H264)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:H264>"
				"<tt:GovLength>%d</tt:GovLength>"
    			"<tt:H264Profile>%s</tt:H264Profile>"
    		"</tt:H264>", 
    		p_v_enc_cfg->GovLength,
	    	p_v_enc_cfg->Profile);
	}

	// offset += bdMulticastConfiguration_xml(p_buf+offset, mlen-offset, &p_v_enc_cfg->Multicast);
	
	offset += snprintf(p_buf+offset, mlen-offset, 		
	    "<tt:SessionTimeout>PT%dS</tt:SessionTimeout>",
	    p_v_enc_cfg->SessionTimeout);

	return offset;    
}

int bdProfile_xml(char * p_buf, int mlen, ONVIF_PROFILE * p_profile)
{
	int offset = 0;
	int extension = 0;
	
	if (p_profile->v_src_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
        	"<tt:VideoSourceConfiguration token=\"%s\">", 
            p_profile->v_src_cfg->Configuration.token);            
        offset += bdVideoSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->v_src_cfg->Configuration);    
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoSourceConfiguration>");	            
    }


    if (p_profile->v_enc_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
        	"<tt:VideoEncoderConfiguration token=\"%s\">", 
        	p_profile->v_enc_cfg->Configuration.token);
		offset += bdVideoEncoderConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->v_enc_cfg->Configuration);        	    
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoEncoderConfiguration>");	            
    }


#ifdef VIDEO_ANALYTICS
    if (p_profile->va_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoAnalyticsConfiguration token=\"%s\">", p_profile->va_cfg->Configuration.token);
		offset += bdVideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->va_cfg->Configuration);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoAnalyticsConfiguration>");
    }
#endif


    if (p_profile->metadata_cfg)
    {
    	offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:MetadataConfiguration token=\"%s\">", 
            p_profile->metadata_cfg->Configuration.token);
        offset += bdMetadataConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->metadata_cfg->Configuration);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:MetadataConfiguration>");    
    }

#ifdef DEVICEIO_SUPPORT
    if (p_profile->a_output_cfg)
    {
		extension = 1;
    }
#endif


    if (extension)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Extension>");
    }
    
#ifdef DEVICEIO_SUPPORT
    if (p_profile->a_output_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:AudioOutputConfiguration token=\"%s\">", 
            p_profile->a_output_cfg->Configuration.token);
        offset += bdAudioOutputConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->a_output_cfg->Configuration);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:AudioOutputConfiguration>");
    }
#endif


    if (extension)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Extension>");
    }
    
    return offset;
}


int bdH264Options_xml(char * p_buf, int mlen, onvif_H264Options * p_options)
{
    int i;
    int offset = 0;
    
    for (i = 0; i < ARRAY_SIZE(p_options->ResolutionsAvailable); i++)
    {
        if (p_options->ResolutionsAvailable[i].Width == 0 || p_options->ResolutionsAvailable[i].Height == 0)
        {
            continue;
        }
        
        offset += bdVideoResolution_xml(p_buf+offset, mlen-offset, &p_options->ResolutionsAvailable[i]);
    }
    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:GovLengthRange>"
			"<tt:Min>1</tt:Min>"
			"<tt:Max>60</tt:Max>"
		"</tt:GovLengthRange>"
		"<tt:FrameRateRange>"
			"<tt:Min>1</tt:Min>"
			"<tt:Max>30</tt:Max>"
		"</tt:FrameRateRange>"
		"<tt:EncodingIntervalRange>"
			"<tt:Min>1</tt:Min>"
			"<tt:Max>30</tt:Max>"
		"</tt:EncodingIntervalRange>");



    return offset;
}

int bdBitrateRange_xml(char * p_buf, int mlen, onvif_IntRange * p_req)
{
    int offset = 0;

    offset = snprintf(p_buf+offset, mlen-offset, 
        "<tt:BitrateRange>"
            "<tt:Min>%d</tt:Min>"
            "<tt:Max>%d</tt:Max>"
        "</tt:BitrateRange>",
        p_req->Min, p_req->Max);

    return offset;      
}

int bdtrt_GetProfiles_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    ONVIF_PROFILE * profile = g_onvif_cfg.profiles;

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetProfilesResponse>");
	
	while (profile)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	        "<trt:Profiles token=\"%s\" fixed=\"%s\"><tt:Name>%s</tt:Name>",
	        profile->token, profile->fixed ? "true" : "false", profile->name);

	    offset += bdProfile_xml(p_buf+offset, mlen-offset, profile);
	    
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Profiles>");
	
	    profile = profile->next;
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetProfilesResponse>");            

	return offset;
}

int bdtrt_GetProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{	
	int offset = 0;
	trt_GetProfile_REQ * p_req = (trt_GetProfile_REQ *) argv;
    ONVIF_PROFILE * profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
    if (NULL == profile)
    {
    	return ONVIF_ERR_NoProfile;
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetProfileResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<trt:Profile token=\"%s\" fixed=\"%s\"><tt:Name>%s</tt:Name>",
        profile->token, profile->fixed ? "true" : "false", profile->name);

   	offset += bdProfile_xml(p_buf+offset, mlen-offset, profile);
	    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Profile>");    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetProfileResponse>"); 

	return offset;
}

int bdtrt_CreateProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	ONVIF_PROFILE * profile = onvif_find_profile(g_onvif_cfg.profiles, argv);
    if (NULL == profile)
    {
    	return -1;
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:CreateProfileResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<trt:Profile token=\"%s\" fixed=\"%s\"><tt:Name>%s</tt:Name>",
        profile->token, profile->fixed ? "true" : "false", profile->name);

    offset += bdProfile_xml(p_buf+offset, mlen-offset, profile);
    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Profile>");

    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:CreateProfileResponse>");            

	return offset;
}

int bdtrt_DeleteProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:DeleteProfileResponse />");
	return offset;
}

int bdtrt_AddVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddVideoSourceConfigurationResponse />");
	return offset;
}

int bdtrt_RemoveVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveVideoSourceConfigurationResponse />");
	return offset;
}

int bdtrt_AddVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddVideoEncoderConfigurationResponse />");
	return offset;
}

int bdtrt_RemoveVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveVideoEncoderConfigurationResponse />");
	return offset;
}

int bdtrt_GetStreamUri_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	trt_GetStreamUri_RES * p_res = (trt_GetStreamUri_RES *)argv;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<trt:GetStreamUriResponse>"
		    "<trt:MediaUri>"
			    "<tt:Uri>%s</tt:Uri>"
			    "<tt:InvalidAfterConnect>%s</tt:InvalidAfterConnect>"
			    "<tt:InvalidAfterReboot>%s</tt:InvalidAfterReboot>"
			    "<tt:Timeout>PT%dS</tt:Timeout>"
		    "</trt:MediaUri>"
	    "</trt:GetStreamUriResponse>", 
	    p_res->MediaUri.Uri,
	    p_res->MediaUri.InvalidAfterConnect ? "true" : "false",
	    p_res->MediaUri.InvalidAfterReboot ? "true" : "false",
	    p_res->MediaUri.Timeout);


	return offset;
}


int bdtrt_GetVideoSources_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	VideoSourceList * p_v_src = g_onvif_cfg.v_src;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourcesResponse>");

	while (p_v_src)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	    	"<trt:VideoSources token=\"%s\">"
		    	"<tt:Framerate>%0.1f</tt:Framerate>"
			    "<tt:Resolution>"
				    "<tt:Width>%d</tt:Width>"
					"<tt:Height>%d</tt:Height>"
				"</tt:Resolution>", 
			p_v_src->VideoSource.token, 
			p_v_src->VideoSource.Framerate, 
			p_v_src->VideoSource.Resolution.Width, 
			p_v_src->VideoSource.Resolution.Height); 

#ifdef IMAGE_SUPPORT			
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Imaging>");
		offset += bdImageSettings2_xml(p_buf+offset, mlen-offset);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Imaging>");
#endif

		offset += snprintf(p_buf+offset, mlen-offset, "</trt:VideoSources>");
	    
	    p_v_src = p_v_src->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourcesResponse>");
	
	return offset;
}

int bdtrt_GetVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * token)
{
	int offset = 0;
	VideoEncoder2ConfigurationList * p_v_enc_cfg = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, token);
    if (NULL == p_v_enc_cfg)
    {
    	return ONVIF_ERR_NoConfig;
    }
 	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_v_enc_cfg->Configuration.token);
    offset += bdVideoEncoderConfiguration_xml(p_buf+offset, mlen-offset, &p_v_enc_cfg->Configuration);	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoEncoderConfigurationResponse>");

	return offset;
}

int bdtrt_GetVideoEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    VideoEncoder2ConfigurationList * p_v_enc_cfg = g_onvif_cfg.v_enc_cfg;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationsResponse>");

	while (p_v_enc_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_v_enc_cfg->Configuration.token);
    	offset += bdVideoEncoderConfiguration_xml(p_buf+offset, mlen-offset, &p_v_enc_cfg->Configuration);
    	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
    	
    	p_v_enc_cfg = p_v_enc_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoEncoderConfigurationsResponse>");

	return offset;
}

int bdtrt_GetCompatibleVideoEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	VideoEncoder2ConfigurationList * p_v_enc_cfg;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, argv);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}
	
	p_v_enc_cfg = g_onvif_cfg.v_enc_cfg;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleVideoEncoderConfigurationsResponse>");

	while (p_v_enc_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_v_enc_cfg->Configuration.token);
    	offset += bdVideoEncoderConfiguration_xml(p_buf+offset, mlen-offset, &p_v_enc_cfg->Configuration);
    	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
    	
    	p_v_enc_cfg = p_v_enc_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleVideoEncoderConfigurationsResponse>");

	return offset;
}

int bdtrt_GetVideoSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    VideoSourceConfigurationList * p_v_src_cfg = g_onvif_cfg.v_src_cfg;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationsResponse>");

	while (p_v_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	    	"<trt:Configurations token=\"%s\">", 
	    	p_v_src_cfg->Configuration.token);
	    offset += bdVideoSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_v_src_cfg->Configuration);	
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	    
	    p_v_src_cfg = p_v_src_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationsResponse>");
	
	return offset;
}

int bdtrt_GetVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * token)
{
	int offset = 0;
    VideoSourceConfigurationList * p_v_src_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, token);
    if (NULL == p_v_src_cfg)
    {
    	return ONVIF_ERR_NoConfig;
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_v_src_cfg->Configuration.token);
    offset += bdVideoSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_v_src_cfg->Configuration);	
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");	    
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationResponse>");
	
	return offset;
}

int bdtrt_SetVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoSourceConfigurationResponse />");	
	return offset;
}

int bdtrt_GetVideoSourceConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	VideoSourceConfigurationList * p_v_src_cfg = NULL;
	
	trt_GetVideoSourceConfigurationOptions_REQ * p_req = (trt_GetVideoSourceConfigurationOptions_REQ *)argv;
	if (p_req->ProfileTokenFlag && p_req->ProfileToken[0] != '\0')
	{
		ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
		if (NULL == p_profile)
		{
			return ONVIF_ERR_NoProfile;
		}

		p_v_src_cfg = p_profile->v_src_cfg;
	}

	if (p_req->ConfigurationTokenFlag && p_req->ConfigurationToken[0] != '\0')
	{
		p_v_src_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->ConfigurationToken);
		if (NULL == p_v_src_cfg)
		{
			return ONVIF_ERR_NoConfig;
		}
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationOptionsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Options>");
    
    offset += bdVideoSourceConfigurationOptions_xml(p_buf+offset, mlen-offset, &g_onvif_cfg.VideoSourceConfigurationOptions);
    
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationOptionsResponse>");
	
	return offset;
}

int bdtrt_GetCompatibleVideoSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * token)
{
	int offset = 0;
	VideoSourceConfigurationList * p_v_src_cfg;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, token);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	p_v_src_cfg = g_onvif_cfg.v_src_cfg;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleVideoSourceConfigurationsResponse>");

	while (p_v_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_v_src_cfg->Configuration.token);
	    offset += bdVideoSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_v_src_cfg->Configuration);	
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	    
	    p_v_src_cfg = p_v_src_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleVideoSourceConfigurationsResponse>");
	
	return offset;
}

int bdtrt_GetVideoEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{   
	int offset = 0;
	trt_GetVideoEncoderConfigurationOptions_RES * p_res = (trt_GetVideoEncoderConfigurationOptions_RES *) argv;



	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationOptionsResponse><trt:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:QualityRange>"
			"<tt:Min>0</tt:Min>"
			"<tt:Max>0</tt:Max>"
		"</tt:QualityRange>");


	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264>");		
		offset += bdH264Options_xml(p_buf+offset, mlen-offset, &p_res->Options.H264);			
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:H264>");	
	}

	
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options></trt:GetVideoEncoderConfigurationOptionsResponse>");	    
	

	return offset;
}

int bdtrt_SetVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoEncoderConfigurationResponse />");		    
	return offset;
}

int bdtrt_SetSynchronizationPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetSynchronizationPointResponse />");		    
	return offset;
}

int bdtrt_GetGuaranteedNumberOfVideoEncoderInstances_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;

	int jpeg = 1;
	int h264 = 4;
	int mpeg4= 2;

	if (0 == g_onvif_cfg.VideoEncoderConfigurationOptions.JPEGFlag)
	{
	    jpeg = 0;
	}

	if (0 == g_onvif_cfg.VideoEncoderConfigurationOptions.MPEG4Flag)
	{
	    mpeg4 = 0;
	}

	if (0 == g_onvif_cfg.VideoEncoderConfigurationOptions.H264Flag)
	{
	    h264 = 0;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<trt:GetGuaranteedNumberOfVideoEncoderInstancesResponse>"
		    "<trt:TotalNumber>%d</trt:TotalNumber>"
		    "<trt:JPEG>%d</trt:JPEG>"
		    "<trt:H264>%d</trt:H264>"
		    "<trt:MPEG4>%d</trt:MPEG4>"
		"</trt:GetGuaranteedNumberOfVideoEncoderInstancesResponse>", 
		2, jpeg, h264, mpeg4);	

	return offset;
}

int bdtrt_GetOSDs_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	
	return offset;
}

int bdtrt_GetOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	
	return offset;
}

int bdtrt_SetOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetOSDResponse />");
	return offset;
}

int bdtrt_CreateOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;

	return offset;
}

int bdtrt_DeleteOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:DeleteOSDResponse />");
	return offset;
}

int bdtrt_GetOSDOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = 0;

	return offset;
}

int bdtrt_StartMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:StartMulticastStreamingResponse />");
	return offset;
}

int bdtrt_StopMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:StopMulticastStreamingResponse />");
	return offset;
}

int bdtrt_GetMetadataConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	MetadataConfigurationList * p_cfg = g_onvif_cfg.metadata_cfg;

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetMetadataConfigurationsResponse>");
	while (p_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_cfg->Configuration.token);
		offset += bdMetadataConfiguration_xml(p_buf+offset, mlen-offset, &p_cfg->Configuration);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
		
		p_cfg = p_cfg->next;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetMetadataConfigurationsResponse>");
	
	return offset;
}

int bdtrt_GetMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
    MetadataConfigurationList * p_cfg = onvif_find_MetadataConfiguration(g_onvif_cfg.metadata_cfg, argv);
    if (NULL == p_cfg)
    {
    	return ONVIF_ERR_NoConfig;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetMetadataConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_cfg->Configuration.token);
	offset += bdMetadataConfiguration_xml(p_buf+offset, mlen-offset, &p_cfg->Configuration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetMetadataConfigurationResponse>");

	return offset;
}

int bdtrt_GetCompatibleMetadataConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	MetadataConfigurationList * p_cfg = g_onvif_cfg.metadata_cfg;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, argv);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleMetadataConfigurationsResponse>");
	while (p_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_cfg->Configuration.token);
		offset += bdMetadataConfiguration_xml(p_buf+offset, mlen-offset, &p_cfg->Configuration);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
		
		p_cfg = p_cfg->next;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleMetadataConfigurationsResponse>");
	
	return offset;
}

int bdtrt_GetMetadataConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	ONVIF_PROFILE * p_profile = NULL;
	trt_GetMetadataConfigurationOptions_REQ * p_req = (trt_GetMetadataConfigurationOptions_REQ *) argv;

	if (p_req->ProfileTokenFlag)
	{
		p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
		if (NULL == p_profile)
		{
			return ONVIF_ERR_NoProfile;
		}
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetMetadataConfigurationOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:Options>");

	offset += bdMetadataConfigurationOptions_xml(p_buf+offset, mlen-offset, &g_onvif_cfg.MetadataConfigurationOptions);
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetMetadataConfigurationOptionsResponse>");
	
	return offset;
}

int bdtrt_SetMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetMetadataConfigurationResponse />");
	return offset;
}

int bdtrt_AddMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddMetadataConfigurationResponse />");
	return offset;
}

int bdtrt_RemoveMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveMetadataConfigurationResponse />");
	return offset;
}

int bdtrt_GetVideoSourceModes_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    trt_GetVideoSourceModes_REQ * p_req = (trt_GetVideoSourceModes_REQ *)argv;

    VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
    if (NULL == p_v_src)
    {
        return ONVIF_ERR_NoVideoSource;
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceModesResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:VideoSourceModes token=\"%s\" Enabled=\"%s\">",
        p_v_src->VideoSourceMode.token, p_v_src->VideoSourceMode.Enabled ? "true" : "false");

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<trt:MaxFramerate>%0.1f</trt:MaxFramerate>"
        "<trt:MaxResolution>"
            "<tt:Width>%d</tt:Width>"
            "<tt:Height>%d</tt:Height>"
        "</trt:MaxResolution>"
        "<trt:Encodings>%s</trt:Encodings>"
        "<trt:Reboot>%s</trt:Reboot>",
        p_v_src->VideoSourceMode.MaxFramerate,
        p_v_src->VideoSourceMode.MaxResolution.Width,
        p_v_src->VideoSourceMode.MaxResolution.Height,
        p_v_src->VideoSourceMode.Encodings,
        p_v_src->VideoSourceMode.Reboot ? "true" : "false");

    if (p_v_src->VideoSourceMode.DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<trt:Description>%s</trt:Description>",
            p_v_src->VideoSourceMode.Description);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</trt:VideoSourceModes>");
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceModesResponse>");

	return offset;
}

int bdtrt_SetVideoSourceMode_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    trt_SetVideoSourceMode_RES * p_res = (trt_SetVideoSourceMode_RES *) argv;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<trt:SetVideoSourceModeResponse>"
            "<trt:Reboot>%s</trt:Reboot>"
        "</trt:SetVideoSourceModeResponse>",
        p_res->Reboot ? "true" : "false");

	return offset;
}
int bdtrt_GetAudioSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    AudioSourceConfigurationList * p_a_src_cfg = g_onvif_cfg.a_src_cfg;
    
	offset += snprintf(p_buf + offset, mlen-offset, "<trt:GetAudioSourceConfigurationsResponse>");

	while (p_a_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_a_src_cfg->Configuration.token);
	    offset += bdAudioSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_a_src_cfg->Configuration);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	    
	    p_a_src_cfg = p_a_src_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourceConfigurationsResponse>");
	
	return offset;
}
int bdtrt_GetCompatibleAudioSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	AudioSourceConfigurationList * p_a_src_cfg;
	ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, argv);
	if (NULL == p_profile)
	{
		return ONVIF_ERR_NoProfile;
	}

	p_a_src_cfg = g_onvif_cfg.a_src_cfg;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleAudioSourceConfigurationsResponse>");

	while (p_a_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_a_src_cfg->Configuration.token);
	    offset += bdAudioSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_a_src_cfg->Configuration);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	    
	    p_a_src_cfg = p_a_src_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleAudioSourceConfigurationsResponse>");
	
	return offset;
}


int bdtrt_GetAudioSourceConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	AudioSourceConfigurationList * p_a_src_cfg = NULL;
	
	trt_GetAudioSourceConfigurationOptions_REQ * p_req = (trt_GetAudioSourceConfigurationOptions_REQ *)argv;
	if (p_req->ProfileTokenFlag && p_req->ProfileToken[0] != '\0')
	{
		ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
		if (NULL == p_profile)
		{
			return ONVIF_ERR_NoProfile;
		}

		p_a_src_cfg = p_profile->a_src_cfg;
	}

	if (p_req->ConfigurationTokenFlag && p_req->ConfigurationToken[0] != '\0')
	{
		p_a_src_cfg = onvif_find_AudioSourceConfiguration(g_onvif_cfg.a_src_cfg, p_req->ConfigurationToken);
		if (NULL == p_a_src_cfg)
		{
			return ONVIF_ERR_NoConfig;
		}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurationOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:Options>");

	offset += bdAudioSourceConfigurationOptions_xml(p_buf+offset, mlen-offset);

    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourceConfigurationOptionsResponse>");
	
	return offset;
}

int bdtrt_GetAudioEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{    
	int offset = 0;
	trt_GetAudioEncoderConfigurationOptions_REQ * p_req = (trt_GetAudioEncoderConfigurationOptions_REQ *) argv;
    AudioEncoder2ConfigurationOptionsList * p_option;

	if (p_req->ProfileTokenFlag && p_req->ProfileToken[0] != '\0')
	{
		ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
		if (NULL == p_profile)
		{
			return ONVIF_ERR_NoProfile;
		}
	}

	if (p_req->ConfigurationTokenFlag && p_req->ConfigurationToken[0] != '\0')
	{
		AudioEncoder2ConfigurationList * p_a_enc_cfg = onvif_find_AudioEncoder2Configuration(g_onvif_cfg.a_enc_cfg, p_req->ConfigurationToken);
		if (NULL == p_a_enc_cfg)
		{
			return ONVIF_ERR_NoConfig;
		}
	}

	p_option = g_onvif_cfg.a_enc_cfg_opt;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioEncoderConfigurationOptionsResponse><trt:Options>");

	while (p_option)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Options>");		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Options>");

		p_option = p_option->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options></trt:GetAudioEncoderConfigurationOptionsResponse>");
	
	return offset;
}




int bdtrt_GetAudioSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * token)
{
	int offset = 0;
    AudioSourceConfigurationList * p_a_src_cfg = onvif_find_AudioSourceConfiguration(g_onvif_cfg.a_src_cfg, token);
    if (NULL == p_a_src_cfg)
    {
    	return ONVIF_ERR_NoConfig;
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_a_src_cfg->Configuration.token);
    offset += bdAudioSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_a_src_cfg->Configuration);
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");	    
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourceConfigurationResponse>");
	
	return offset;
}
#ifdef DEVICEIO_SUPPORT

int bdtrt_GetAudioOutputs_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;

	return offset;
}

int bdtrt_AddAudioOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddAudioOutputConfigurationResponse />");
	return offset;
}

int bdtrt_RemoveAudioOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveAudioOutputConfigurationResponse />");
	return offset;
}

int bdtrt_GetAudioOutputConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    AudioOutputConfigurationList * p_cfg = g_onvif_cfg.a_output_cfg;

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioOutputConfigurationsResponse>");

    while (p_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<trt:Configurations token=\"%s\">", 
            p_cfg->Configuration.token);
        
        offset += bdAudioOutputConfiguration_xml(p_buf+offset, mlen-offset, &p_cfg->Configuration);
        
        offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
            
        p_cfg = p_cfg->next;
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioOutputConfigurationsResponse>");

	return offset;
}

int bdtrt_GetCompatibleAudioOutputConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    trt_GetCompatibleAudioOutputConfigurations_REQ * p_req = (trt_GetCompatibleAudioOutputConfigurations_REQ *)argv;
    AudioOutputConfigurationList * p_cfg = g_onvif_cfg.a_output_cfg;
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
    if (NULL == p_profile)
    {
        return ONVIF_ERR_NoProfile;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleAudioOutputConfigurationsResponse>");

    while (p_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<trt:Configurations token=\"%s\">", 
            p_cfg->Configuration.token);
        
        offset += bdAudioOutputConfiguration_xml(p_buf+offset, mlen-offset, &p_cfg->Configuration);
        
        offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
            
        p_cfg = p_cfg->next;
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleAudioOutputConfigurationsResponse>");

	return offset;
}

int bdtrt_GetAudioOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    trt_GetAudioOutputConfiguration_REQ * p_req = (trt_GetAudioOutputConfiguration_REQ *)argv; 
    AudioOutputConfigurationList * p_cfg = onvif_find_AudioOutputConfiguration(g_onvif_cfg.a_output_cfg, p_req->ConfigurationToken);
    if (NULL == p_cfg)
    {
        return ONVIF_ERR_NoConfig;
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioOutputConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<trt:Configuration token=\"%s\">", 
	    p_cfg->Configuration.token);
    offset += bdAudioOutputConfiguration_xml(p_buf+offset, mlen-offset, &p_cfg->Configuration);
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioOutputConfigurationResponse>");

	return offset;
}

int bdtrt_GetAudioOutputConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    AudioOutputConfigurationList * p_cfg = NULL;
    trt_GetAudioOutputConfigurationOptions_REQ * p_req = (trt_GetAudioOutputConfigurationOptions_REQ *)argv; 
    if (p_req->ConfigurationTokenFlag)
    {
        p_cfg = onvif_find_AudioOutputConfiguration(g_onvif_cfg.a_output_cfg, p_req->ConfigurationToken);
        if (NULL == p_cfg)
        {
            return ONVIF_ERR_NoConfig;
        }
    }

    if (p_req->ProfileTokenFlag)
    {
        ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
        if (NULL == p_profile)
        {
            return ONVIF_ERR_NoProfile;
        }
    }

    if (NULL == p_cfg)
    {
        p_cfg = g_onvif_cfg.a_output_cfg;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioOutputConfigurationOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:Options>");

	if (p_cfg)
	{
        offset += bdAudioOutputConfigurationOptions_xml(p_buf+offset, mlen-offset, &p_cfg->Options);
    }
    
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioOutputConfigurationOptionsResponse>");

	return offset;
}

int bdtrt_SetAudioOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetAudioOutputConfigurationResponse />");
	return offset;
}

#endif // DEVICEIO_SUPPORT

#ifdef VIDEO_ANALYTICS

int bdtrt_GetVideoAnalyticsConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	VideoAnalyticsConfigurationList * p_va_cfg = g_onvif_cfg.va_cfg;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoAnalyticsConfigurationsResponse>");

	while (p_va_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_va_cfg->Configuration.token);
		offset += bdVideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_va_cfg->Configuration);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");

		p_va_cfg = p_va_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoAnalyticsConfigurationsResponse>");

	return offset;
}

int bdtrt_AddVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddVideoAnalyticsConfigurationResponse />");
	return offset;
}

int bdtrt_GetVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	trt_GetVideoAnalyticsConfiguration_REQ * p_req = (trt_GetVideoAnalyticsConfiguration_REQ *)argv;
	VideoAnalyticsConfigurationList * p_va_cfg = onvif_find_VideoAnalyticsConfiguration(g_onvif_cfg.va_cfg, p_req->ConfigurationToken);
	if (NULL == p_va_cfg)
	{
		return ONVIF_ERR_NoConfig;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoAnalyticsConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_va_cfg->Configuration.token);
	offset += bdVideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_va_cfg->Configuration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoAnalyticsConfigurationResponse>");

	return offset;
}

int bdtrt_RemoveVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveVideoAnalyticsConfigurationResponse />");
	return offset;
}

int bdtrt_SetVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoAnalyticsConfigurationResponse />");
	return offset;
}

int bdtrt_GetAnalyticsConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    VideoAnalyticsConfigurationList * p_cfg = g_onvif_cfg.va_cfg;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAnalyticsConfigurationsResponse>");
	while (p_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_cfg->Configuration.token);
		offset += bdVideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_cfg->Configuration);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
		
		p_cfg = p_cfg->next;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAnalyticsConfigurationsResponse>");

	return offset;
}

int bdtrt_GetCompatibleVideoAnalyticsConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    trt_GetCompatibleVideoAnalyticsConfigurations_REQ * p_req = (trt_GetCompatibleVideoAnalyticsConfigurations_REQ *) argv;
    ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->ProfileToken);
	VideoAnalyticsConfigurationList * p_va_cfg = g_onvif_cfg.va_cfg;

    if (NULL == p_profile)
    {
        return ONVIF_ERR_NoProfile;
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleVideoAnalyticsConfigurationsResponse>");

	while (p_va_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_va_cfg->Configuration.token);
		offset += bdVideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_va_cfg->Configuration);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");

		p_va_cfg = p_va_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleVideoAnalyticsConfigurationsResponse>");

	return offset;
}

#endif // VIDEO_ANALYTICS

#endif // MEDIA_SUPPORT


#ifdef VIDEO_ANALYTICS

int bdConfig_xml(char * p_buf, int mlen, onvif_Config * p_req)
{
	int offset = 0;
	SimpleItemList * p_simpleitem;
	ElementItemList * p_elementitem;

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");

	p_simpleitem = p_req->Parameters.SimpleItem;
	while (p_simpleitem)
	{
		offset += bdSimpleItem_xml(p_buf+offset, mlen-offset, &p_simpleitem->SimpleItem);
		
		p_simpleitem = p_simpleitem->next;
	}

	p_elementitem = p_req->Parameters.ElementItem;
	while (p_elementitem)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItem Name=\"%s\">", p_elementitem->ElementItem.Name);

        if (p_elementitem->ElementItem.AnyFlag && p_elementitem->ElementItem.Any)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "%s", p_elementitem->ElementItem.Any);
        }

		offset += snprintf(p_buf+offset, mlen-offset, "</tt:ElementItem>");
		
		p_elementitem = p_elementitem->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");
	
	return offset;
}

int bdVideoAnalyticsConfiguration_xml(char * p_buf, int mlen, onvif_VideoAnalyticsConfiguration * p_req)
{
	int offset = 0;
	ConfigList * p_config;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Name>%s</tt:Name>"
    	"<tt:UseCount>%d</tt:UseCount>",
    	p_req->Name, p_req->UseCount);

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:AnalyticsEngineConfiguration>");

	p_config = p_req->AnalyticsEngineConfiguration.AnalyticsModule;
	while (p_config)
	{
	    if (p_config->Config.attrFlag)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, 
	            "<tt:AnalyticsModule Name=\"%s\" Type=\"%s\" %s>", 
	            p_config->Config.Name, p_config->Config.Type, p_config->Config.attr);
	    }
	    else
	    {
		    offset += snprintf(p_buf+offset, mlen-offset, 
		        "<tt:AnalyticsModule Name=\"%s\" Type=\"%s\">", 
		        p_config->Config.Name, p_config->Config.Type);
        }
        
		offset += bdConfig_xml(p_buf+offset, mlen-offset, &p_config->Config);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:AnalyticsModule>");

		p_config = p_config->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:AnalyticsEngineConfiguration>");

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:RuleEngineConfiguration>");

	p_config = p_req->RuleEngineConfiguration.Rule;
	while (p_config)
	{
	    if (p_config->Config.attrFlag)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, 
		        "<tt:Rule Name=\"%s\" Type=\"%s\" %s>", 
		        p_config->Config.Name, p_config->Config.Type, p_config->Config.attr);
	    }
	    else
	    {
		    offset += snprintf(p_buf+offset, mlen-offset, 
		        "<tt:Rule Name=\"%s\" Type=\"%s\">", 
		        p_config->Config.Name, p_config->Config.Type);
		}
		
		offset += bdConfig_xml(p_buf+offset, mlen-offset, &p_config->Config);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Rule>");

		p_config = p_config->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:RuleEngineConfiguration>");
	
	return offset;
}

int bdItemListDescription_xml(char * p_buf, int mlen, onvif_ItemListDescription * p_req)
{
	int offset = 0;
	SimpleItemDescriptionList * p_simpleitem_desc;
	
	p_simpleitem_desc = p_req->SimpleItemDescription;
	while (p_simpleitem_desc)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Name=\"%s\" Type=\"%s\" />",
			p_simpleitem_desc->SimpleItemDescription.Name, p_simpleitem_desc->SimpleItemDescription.Type);
		p_simpleitem_desc = p_simpleitem_desc->next;
	}

	p_simpleitem_desc = p_req->ElementItemDescription;
	while (p_simpleitem_desc)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItemDescription Name=\"%s\" Type=\"%s\" />",
			p_simpleitem_desc->SimpleItemDescription.Name, p_simpleitem_desc->SimpleItemDescription.Type);
		p_simpleitem_desc = p_simpleitem_desc->next;
	}

	return offset;
}

int bdConfigDescription_Messages_xml(char * p_buf, int mlen, onvif_ConfigDescription_Messages * p_req)
{
	int offset = 0;

	if (p_req->SourceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Source>");
		offset += bdItemListDescription_xml(p_buf+offset, mlen-offset, &p_req->Source);		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Source>");
	}

	if (p_req->KeyFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Key>");
		offset += bdItemListDescription_xml(p_buf+offset, mlen-offset, &p_req->Key);		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Key>");
	}

	if (p_req->DataFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Data>");
		offset += bdItemListDescription_xml(p_buf+offset, mlen-offset, &p_req->Data);		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Data>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:ParentTopic>%s</tt:ParentTopic>", p_req->ParentTopic);
	
	return offset;
}

int bdConfigDescription_xml(char * p_buf, int mlen, onvif_ConfigDescription * p_req)
{
    int offset = 0;
    SimpleItemDescriptionList * p_simpleitem_desc;
	ConfigDescription_MessagesList * p_cfg_desc_msg;
	
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");

	p_simpleitem_desc = p_req->Parameters.SimpleItemDescription;
	while (p_simpleitem_desc)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Name=\"%s\" Type=\"%s\" />", 
			p_simpleitem_desc->SimpleItemDescription.Name, p_simpleitem_desc->SimpleItemDescription.Type);
		p_simpleitem_desc = p_simpleitem_desc->next;
	}

	p_simpleitem_desc = p_req->Parameters.ElementItemDescription;
	while (p_simpleitem_desc)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItemDescription Name=\"%s\" Type=\"%s\" />", 
			p_simpleitem_desc->SimpleItemDescription.Name, p_simpleitem_desc->SimpleItemDescription.Type);
		p_simpleitem_desc = p_simpleitem_desc->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");

	p_cfg_desc_msg = p_req->Messages;
	while (p_cfg_desc_msg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Messages IsProperty=\"%s\">", p_cfg_desc_msg->Messages.IsProperty ? "true" : "false");
		offset += bdConfigDescription_Messages_xml(p_buf+offset, mlen-offset, &p_cfg_desc_msg->Messages);			
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Messages>");
		
		p_cfg_desc_msg = p_cfg_desc_msg->next;
	}

	return offset;
}

int bdtan_GetSupportedRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = 0;
	tan_GetSupportedRules_RES * p_res = (tan_GetSupportedRules_RES *)argv;
	ConfigDescriptionList * p_cfg_desc;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetSupportedRulesResponse><tan:SupportedRules>");

	for (i = 0; i < p_res->SupportedRules.sizeRuleContentSchemaLocation; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:RuleContentSchemaLocation>%s</tt:RuleContentSchemaLocation>",
			p_res->SupportedRules.RuleContentSchemaLocation[i]);
	}

	p_cfg_desc = p_res->SupportedRules.RuleDescription;
	while (p_cfg_desc)
	{		
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:RuleDescription Name=\"%s\" fixed=\"%s\" maxInstances=\"%d\">", 
		    p_cfg_desc->ConfigDescription.Name,
		    p_cfg_desc->ConfigDescription.fixed ? "true" : "false",
		    p_cfg_desc->ConfigDescription.maxInstances);

		offset += bdConfigDescription_xml(p_buf+offset, mlen-offset, &p_cfg_desc->ConfigDescription);
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:RuleDescription>");
		
		p_cfg_desc = p_cfg_desc->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:SupportedRules></tan:GetSupportedRulesResponse>");

	return offset;
}

int bdtan_CreateRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:CreateRulesResponse />");
	return offset;
}

int bdtan_DeleteRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:DeleteRulesResponse />");
	return offset;
}

int bdtan_GetRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tan_GetRules_RES * p_res = (tan_GetRules_RES *)argv;
	ConfigList * p_config;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetRulesResponse>");

	p_config = p_res->Rule;
	while (p_config)
	{
	    if (p_config->Config.attrFlag)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, 
		        "<tan:Rule Name=\"%s\" Type=\"%s\" %s>", 
		        p_config->Config.Name, p_config->Config.Type, p_config->Config.attr);
	    }
	    else
	    {
		    offset += snprintf(p_buf+offset, mlen-offset, 
		        "<tan:Rule Name=\"%s\" Type=\"%s\">", 
		        p_config->Config.Name, p_config->Config.Type);
		}
		
		offset += bdConfig_xml(p_buf+offset, mlen-offset, &p_config->Config);
		offset += snprintf(p_buf+offset, mlen-offset, "</tan:Rule>");
		
		p_config = p_config->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetRulesResponse>");

	return offset;
}

int bdtan_ModifyRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:ModifyRulesResponse />");
	return offset;
}

int bdtan_CreateAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:CreateAnalyticsModulesResponse />");
	return offset;
}

int bdtan_DeleteAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:DeleteAnalyticsModulesResponse />");
	return offset;
}

int bdtan_GetAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tan_GetAnalyticsModules_RES * p_res = (tan_GetAnalyticsModules_RES *)argv;
	ConfigList * p_config;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetAnalyticsModulesResponse>");

	p_config = p_res->AnalyticsModule;
	while (p_config)
	{
	    if (p_config->Config.attrFlag)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, 
	            "<tan:AnalyticsModule Name=\"%s\" Type=\"%s\" %s>", 
	            p_config->Config.Name, p_config->Config.Type, p_config->Config.attr);
	    }
	    else
	    {
		    offset += snprintf(p_buf+offset, mlen-offset, 
		        "<tan:AnalyticsModule Name=\"%s\" Type=\"%s\">", 
		        p_config->Config.Name, p_config->Config.Type);
        }
        
		offset += bdConfig_xml(p_buf+offset, mlen-offset, &p_config->Config);
		offset += snprintf(p_buf+offset, mlen-offset, "</tan:AnalyticsModule>");
		
		p_config = p_config->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetAnalyticsModulesResponse>");

	return offset;
}

int bdtan_ModifyAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:ModifyAnalyticsModulesResponse />");
	return offset;
}

int bdtan_GetRuleOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    ConfigDescriptionList * p_desc;
    ConfigOptionsList * p_options;
    tan_GetRuleOptions_REQ * p_req = (tan_GetRuleOptions_REQ *)argv;
    VideoAnalyticsConfigurationList * p_va_cfg = onvif_find_VideoAnalyticsConfiguration(g_onvif_cfg.va_cfg, p_req->ConfigurationToken);
    if (NULL == p_va_cfg)
    {
        return ONVIF_ERR_NoConfig;
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetRuleOptionsResponse>");

    p_desc = p_va_cfg->SupportedRules.RuleDescription;
    while (p_desc)
    {
        if (p_req->RuleType[0] == '\0' || 
            soap_strcmp(p_req->RuleType, p_desc->ConfigDescription.Name) == 0)
        {
            p_options = p_desc->ConfigOptions;
            while (p_options)
            {
                offset += snprintf(p_buf+offset, mlen-offset, "<tan:RuleOptions");

                if (p_options->Options.RuleTypeFlag)
                {
                    offset += snprintf(p_buf+offset, mlen-offset, " RuleType=\"%s\"", 
                        p_options->Options.RuleType);
                }
                
                offset += snprintf(p_buf+offset, mlen-offset, " Name=\"%s\" Type=\"%s\"",
                    p_options->Options.Name, p_options->Options.Type);                

                if (p_options->Options.AnalyticsModuleFlag)
                {
                    offset += snprintf(p_buf+offset, mlen-offset, " AnalyticsModule=\"%s\"", 
                        p_options->Options.AnalyticsModule);
                }

                offset += snprintf(p_buf+offset, mlen-offset, ">");
                
                if (p_options->Options.any)
                {
                    offset += snprintf(p_buf+offset, mlen-offset, "%s", p_options->Options.any);
                }
                
                offset += snprintf(p_buf+offset, mlen-offset, "</tan:RuleOptions>");
                
                p_options = p_options->next;
            }
        }
        
        p_desc = p_desc->next;
    }    

    offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetRuleOptionsResponse>");

    return offset;
}

int bdtan_GetSupportedAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    ConfigDescriptionList * p_cfg_desc;
    tan_GetSupportedAnalyticsModules_REQ * p_req = (tan_GetSupportedAnalyticsModules_REQ *)argv;
    VideoAnalyticsConfigurationList * p_va_cfg = onvif_find_VideoAnalyticsConfiguration(g_onvif_cfg.va_cfg, p_req->ConfigurationToken);
    if (NULL == p_va_cfg)
    {
        return ONVIF_ERR_NoConfig;
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetSupportedAnalyticsModulesResponse><tan:SupportedAnalyticsModules>");

    for (i = 0; i < p_va_cfg->SupportedAnalyticsModules.sizeAnalyticsModuleContentSchemaLocation; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:AnalyticsModuleContentSchemaLocation>%s</tt:AnalyticsModuleContentSchemaLocation>",
			p_va_cfg->SupportedAnalyticsModules.AnalyticsModuleContentSchemaLocation[i]);
	}

	p_cfg_desc = p_va_cfg->SupportedAnalyticsModules.AnalyticsModuleDescription;
	while (p_cfg_desc)
	{		
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:AnalyticsModuleDescription Name=\"%s\" fixed=\"%s\" maxInstances=\"%d\">", 
		    p_cfg_desc->ConfigDescription.Name,
		    p_cfg_desc->ConfigDescription.fixed ? "true" : "false",
		    p_cfg_desc->ConfigDescription.maxInstances);

		offset += bdConfigDescription_xml(p_buf+offset, mlen-offset, &p_cfg_desc->ConfigDescription);
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:AnalyticsModuleDescription>");
		
		p_cfg_desc = p_cfg_desc->next;
	}

    offset += snprintf(p_buf+offset, mlen-offset, "</tan:SupportedAnalyticsModules></tan:GetSupportedAnalyticsModulesResponse>");

    return offset;
}

int bdtan_GetAnalyticsModuleOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    ConfigDescriptionList * p_desc;
    ConfigOptionsList * p_options;
    tan_GetAnalyticsModuleOptions_REQ * p_req = (tan_GetAnalyticsModuleOptions_REQ *)argv;
    VideoAnalyticsConfigurationList * p_va_cfg = onvif_find_VideoAnalyticsConfiguration(g_onvif_cfg.va_cfg, p_req->ConfigurationToken);
    if (NULL == p_va_cfg)
    {
        return ONVIF_ERR_NoConfig;
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetAnalyticsModuleOptionsResponse>");

    p_desc = p_va_cfg->SupportedAnalyticsModules.AnalyticsModuleDescription;
    while (p_desc)
    {
        p_options = p_desc->ConfigOptions;
        while (p_options)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tan:Options");

            if (p_options->Options.RuleTypeFlag)
            {
                offset += snprintf(p_buf+offset, mlen-offset, " RuleType=\"%s\"",
                    p_options->Options.RuleType);
            }
            
            offset += snprintf(p_buf+offset, mlen-offset, " Name=\"%s\" Type=\"%s\"",
                p_options->Options.Name, p_options->Options.Type);

            if (p_options->Options.AnalyticsModuleFlag)
            {
                offset += snprintf(p_buf+offset, mlen-offset, " AnalyticsModule=\"%s\"",
                    p_options->Options.AnalyticsModule);
            }
            
            offset += snprintf(p_buf+offset, mlen-offset, ">");

            if (p_options->Options.any)
            {
                offset += snprintf(p_buf+offset, mlen-offset, "%s", p_options->Options.any);
            }
            
            offset += snprintf(p_buf+offset, mlen-offset, "</tan:Options>");
            
            p_options = p_options->next;
        }
        
        p_desc = p_desc->next;
    }    

    offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetAnalyticsModuleOptionsResponse>");

    return offset;
}

#endif // end of VIDEO_ANALYTICS

/***************************************************************************************/

#ifdef PROFILE_G_SUPPORT

int bdRecordingSourceInformation_xml(char * p_buf, int mlen, onvif_RecordingSourceInformation * p_req)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Source>"
			"<tt:SourceId>%s</tt:SourceId>"
			"<tt:Name>%s</tt:Name>"
			"<tt:Location>%s</tt:Location>"
			"<tt:Description>%s</tt:Description>"
			"<tt:Address>%s</tt:Address>"
		"</tt:Source>",
		p_req->SourceId,
		p_req->Name,
		p_req->Location,
		p_req->Description,
		p_req->Address);
		
	return offset;		
}

int bdTrackInformation_xml(char * p_buf, int mlen, onvif_TrackInformation * p_req)
{
	int offset = 0;
	char DataFrom[64], DataTo[64];

	onvif_get_time_str_s(DataFrom, sizeof(DataFrom), p_req->DataFrom, 0);
	onvif_get_time_str_s(DataTo, sizeof(DataTo), p_req->DataTo, 0);

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:TrackToken>%s</tt:TrackToken>"
		"<tt:TrackType>%s</tt:TrackType>"
		"<tt:Description>%s</tt:Description>"
		"<tt:DataFrom>%s</tt:DataFrom>"
		"<tt:DataTo>%s</tt:DataTo>",
		p_req->TrackToken,
		onvif_TrackTypeToString(p_req->TrackType),
		p_req->Description,
		DataFrom,
		DataTo);

	return offset;			
}

int bdRecordingInformation_xml(char * p_buf, int mlen, onvif_RecordingInformation * p_req)
{
	int i;
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingToken>%s</tt:RecordingToken>", p_req->RecordingToken);
	offset += bdRecordingSourceInformation_xml(p_buf+offset, mlen-offset, &p_req->Source);

	if (p_req->EarliestRecordingFlag)
	{
		char EarliestRecording[64];
		onvif_get_time_str_s(EarliestRecording, sizeof(EarliestRecording), p_req->EarliestRecording, 0);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:EarliestRecording>%s</tt:EarliestRecording>", EarliestRecording);
	}

	if (p_req->LatestRecordingFlag)
	{
		char LatestRecording[64];
		onvif_get_time_str_s(LatestRecording, sizeof(LatestRecording), p_req->LatestRecording, 0);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:LatestRecording>%s</tt:LatestRecording>", LatestRecording);
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Content>%s</tt:Content>", p_req->Content);

	for (i = 0; i < p_req->sizeTrack; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Track>");
		offset += bdTrackInformation_xml(p_buf+offset, mlen-offset, &p_req->Track[i]);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Track>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingStatus>%s</tt:RecordingStatus>", 
		onvif_RecordingStatusToString(p_req->RecordingStatus));

	return offset;		
}

int bdTrackAttributes_xml(char * p_buf, int mlen, onvif_TrackAttributes * p_req)
{
	int offset = 0;

	offset += bdTrackInformation_xml(p_buf+offset, mlen-offset, &p_req->TrackInformation);

	if (p_req->VideoAttributesFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoAttributes>");
		if (p_req->VideoAttributes.BitrateFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Bitrate>%d</tt:Bitrate>", p_req->VideoAttributes.Bitrate);
		}
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:Width>%d</tt:Width>"
			"<tt:Height>%d</tt:Height>"
			"<tt:Encoding>%s</tt:Encoding>"
			"<tt:Framerate>%0.1f</tt:Framerate>",
			p_req->VideoAttributes.Width,
			p_req->VideoAttributes.Height,
			onvif_VideoEncodingToString(p_req->VideoAttributes.VideoEncoding==VideoEncoding_H264? VideoEncoding_H264:VideoEncoding_H265),
			p_req->VideoAttributes.Framerate)
			;			
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoAttributes>");	
					
			
			
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoAttributes>");		
	}


	return offset;
}

int bdMediaAttributes_xml(char * p_buf, int mlen, onvif_MediaAttributes * p_req)
{
	int i, offset = 0;
	char From[64], Until[64];

	onvif_get_time_str_s(From, sizeof(From), p_req->From, 0);
	onvif_get_time_str_s(Until, sizeof(Until), p_req->Until, 0);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingToken>%s</tt:RecordingToken>", p_req->RecordingToken);

	for (i = 0; i < p_req->sizeTrackAttributes; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:TrackAttributes>");
		offset += bdTrackAttributes_xml(p_buf+offset, mlen-offset, &p_req->TrackAttributes[i]);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:TrackAttributes>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:From>%s</tt:From>"
    	"<tt:Until>%s</tt:Until>", 
    	From,
    	Until);

	return offset;    	
}

int bdEndpointReferenceType_xml(char * p_buf, int mlen, onvif_EndpointReferenceType * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:Address>%s</tt:Address>", 
        p_req->Address);

    return offset;        
}

int bdTopicExpressionType_xml(char * p_buf, int mlen, onvif_EndpointReferenceType * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:Address>%s</tt:Address>", 
        p_req->Address);

    return offset;
}

int bdNotificationMessageHolderType_xml(char * p_buf, int mlen, onvif_NotificationMessageHolderType * p_req)
{
    int offset = 0;

    if (p_req->SubscriptionReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:SubscriptionReference>");
        offset += bdEndpointReferenceType_xml(p_buf+offset, mlen-offset, &p_req->SubscriptionReference);
        offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:SubscriptionReference>");
    }

    if (p_req->TopicFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<wsnt:Topic Dialect=\"%s\">%s</wsnt:Topic>",
            p_req->Topic.Dialect,
            p_req->Topic.Topic);
    }

    if (p_req->ProducerReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:ProducerReference>");
        offset += bdEndpointReferenceType_xml(p_buf+offset, mlen-offset, &p_req->ProducerReference);
        offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:ProducerReference>");
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:Message>");
    offset += bdMessage_xml(p_buf+offset, mlen-offset, &p_req->Message);
    offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:Message>");
    
    return offset;
}

int bdFindEventResult_xml(char * p_buf, int mlen, onvif_FindEventResult * p_req)
{
	int offset = 0;
	char TimeBuff[64];

	onvif_get_time_str_s(TimeBuff, sizeof(TimeBuff), p_req->Time, 0);

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:RecordingToken>%s</tt:RecordingToken>"
     	"<tt:TrackToken>%s</tt:TrackToken>"
     	"<tt:Time>%s</tt:Time>",
     	p_req->RecordingToken, 
     	p_req->TrackToken, 
     	TimeBuff);

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Event>");
	offset += bdNotificationMessageHolderType_xml(p_buf+offset, mlen-offset, &p_req->Event);
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Event>");
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:StartStateEvent>%s</tt:StartStateEvent>",
		p_req->StartStateEvent ? "true" : "false");
	
	return offset;
}


int bdtse_GetRecordingInformation_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tse_GetRecordingInformation_RES * p_res = (tse_GetRecordingInformation_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetRecordingInformationResponse><tse:RecordingInformation>");	
	offset += bdRecordingInformation_xml(p_buf+offset, mlen-offset, &p_res->RecordingInformation);
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:RecordingInformation></tse:GetRecordingInformationResponse>");

	return offset;
}

int bdtse_GetRecordingSummary_rly_xml(char * p_buf, int mlen, const char * argv)
{
	char DataFrom[64];
	char DataUntil[64];

	int offset = 0;
	tse_GetRecordingSummary_RES * p_res = (tse_GetRecordingSummary_RES *)argv;

	onvif_get_time_str_s(DataFrom, sizeof(DataFrom), p_res->Summary.DataFrom, 0);
	onvif_get_time_str_s(DataUntil, sizeof(DataUntil), p_res->Summary.DataUntil, 0);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:GetRecordingSummaryResponse>"
			"<tse:Summary>"
				"<tt:DataFrom>%s</tt:DataFrom>"
				"<tt:DataUntil>%s</tt:DataUntil>"
				"<tt:NumberRecordings>%d</tt:NumberRecordings>"
			"</tse:Summary>"
		"</tse:GetRecordingSummaryResponse>",
		DataFrom,
		DataUntil,
		p_res->Summary.NumberRecordings);

	return offset;
}

int bdtse_GetMediaAttributes_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = 0;
	tse_GetMediaAttributes_RES * p_res = (tse_GetMediaAttributes_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetMediaAttributesResponse>");

	for (i = 0; i < p_res->sizeMediaAttributes; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:MediaAttributes>");
		offset += bdMediaAttributes_xml(p_buf+offset, mlen-offset, &p_res->MediaAttributes[i]);
		offset += snprintf(p_buf+offset, mlen-offset, "</tse:MediaAttributes>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:GetMediaAttributesResponse>");
	
	return offset;
}

int bdtse_FindRecordings_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tse_FindRecordings_RES * p_res = (tse_FindRecordings_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:FindRecordingsResponse>"
			"<tse:SearchToken>%s</tse:SearchToken>"
		"</tse:FindRecordingsResponse>",
		p_res->SearchToken);

	return offset;
}

int bdtse_GetRecordingSearchResults_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	RecordingInformationList * p_RecInf;
	tse_GetRecordingSearchResults_RES * p_res = (tse_GetRecordingSearchResults_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetRecordingSearchResultsResponse><tse:ResultList>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:SearchState>%s</tt:SearchState>", onvif_SearchStateToString(p_res->ResultList.SearchState));

	p_RecInf = p_res->ResultList.RecordInformation;
	while (p_RecInf)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingInformation>");
		offset += bdRecordingInformation_xml(p_buf+offset, mlen-offset, &p_RecInf->RecordingInformation);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:RecordingInformation>");

		p_RecInf = p_RecInf->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:ResultList></tse:GetRecordingSearchResultsResponse>");
	
	return offset;
}

int bdtse_FindEvents_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tse_FindEvents_RES * p_res = (tse_FindEvents_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:FindEventsResponse>"
			"<tse:SearchToken>%s</tse:SearchToken>"
		"</tse:FindEventsResponse>",
		p_res->SearchToken);

	return offset;
}

int bdtse_GetEventSearchResults_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	FindEventResultList * p_EventResult;
	tse_GetEventSearchResults_RES * p_res = (tse_GetEventSearchResults_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetEventSearchResultsResponse><tse:ResultList>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:SearchState>%s</tt:SearchState>", onvif_SearchStateToString(p_res->ResultList.SearchState));

	p_EventResult = p_res->ResultList.Result;
	while (p_EventResult)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Result>");
		offset += bdFindEventResult_xml(p_buf+offset, mlen-offset, &p_EventResult->Result);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Result>");

		p_EventResult = p_EventResult->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:ResultList></tse:GetEventSearchResultsResponse>");
	
	return offset;
}

int bdtse_FindMetadata_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tse_FindMetadata_RES * p_res = (tse_FindMetadata_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:FindMetadataResponse>"
			"<tse:SearchToken>%s</tse:SearchToken>"
		"</tse:FindMetadataResponse>",
		p_res->SearchToken);

	return offset;
}

int bdtse_FindMetadataResult_xml(char * p_buf, int mlen, onvif_FindMetadataResult * p_res)
{
    int offset = 0;
	char TimeBuff[64];

	onvif_get_time_str_s(TimeBuff, sizeof(TimeBuff), p_res->Time, 0);

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:RecordingToken>%s</tt:RecordingToken>"
     	"<tt:TrackToken>%s</tt:TrackToken>"
     	"<tt:Time>%s</tt:Time>",
     	p_res->RecordingToken, 
     	p_res->TrackToken, 
     	TimeBuff);	
	
	return offset;
}

int bdtse_GetMetadataSearchResults_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    FindMetadataResultList * p_Result;
	tse_GetMetadataSearchResults_RES * p_res = (tse_GetMetadataSearchResults_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetMetadataSearchResultsResponse><tse:ResultList>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:SearchState>%s</tt:SearchState>", onvif_SearchStateToString(p_res->ResultList.SearchState));

	p_Result = p_res->ResultList.Result;
	while (p_Result)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Result>");
		offset += bdtse_FindMetadataResult_xml(p_buf+offset, mlen-offset, &p_Result->Result);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Result>");

		p_Result = p_Result->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:ResultList></tse:GetMetadataSearchResultsResponse>");
	
	return offset;
}

int bdtse_FindPTZPosition_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tse_FindPTZPosition_RES * p_res = (tse_FindPTZPosition_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:FindPTZPositionResponse>"
			"<tse:SearchToken>%s</tse:SearchToken>"
		"</tse:FindPTZPositionResponse>",
		p_res->SearchToken);

	return offset;
}


int bdtse_EndSearch_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	char EndPoint[64];
	tse_EndSearch_RES * p_res = (tse_EndSearch_RES *)argv;
	
	onvif_get_time_str_s(EndPoint, sizeof(EndPoint), p_res->Endpoint, 0);

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:EndSearchResponse>"
			"<tse:Endpoint>%s</tse:Endpoint>"
		"</tse:EndSearchResponse>",
		EndPoint);

	return offset;
}

int bdtse_GetSearchState_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tse_GetSearchState_RES * p_res = (tse_GetSearchState_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:GetSearchStateResponse>"
			"<tse:State>%s</tse:State>"
		"</tse:GetSearchStateResponse>",
		onvif_SearchStateToString(p_res->State));

	return offset;
}

int bdtrc_CreateRecording_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trc:CreateRecordingResponse>"
			"<trc:RecordingToken>%s</trc:RecordingToken>"
		"</trc:CreateRecordingResponse>",
		argv);

	return offset;
}

int bdtrc_DeleteRecording_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:DeleteRecordingResponse />");
	return offset;
}

int bdRecordingConfiguration_xml(char * p_buf, int mlen, onvif_RecordingConfiguration * p_req)
{
	int offset = 0;
	
	offset += bdRecordingSourceInformation_xml(p_buf+offset, mlen-offset, &p_req->Source);
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Content>%s</tt:Content>", p_req->Content);
	
	if (p_req->MaximumRetentionTimeFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaximumRetentionTime>PT%dS</tt:MaximumRetentionTime>", p_req->MaximumRetentionTime);
	}

	return offset;
}

int bdTrackConfiguration_xml(char * p_buf, int mlen, onvif_TrackConfiguration * p_req)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:TrackType>%s</tt:TrackType>"
		"<tt:Description>%s</tt:Description>",
		onvif_TrackTypeToString(p_req->TrackType),
		p_req->Description);

	return offset;		
}

int bdRecordingJobConfiguration_xml(char * p_buf, int mlen, onvif_RecordingJobConfiguration * p_req)
{
	int i, j, offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:RecordingToken>%s</tt:RecordingToken>"
		"<tt:Mode>%s</tt:Mode>"
		"<tt:Priority>%d</tt:Priority>", 
		p_req->RecordingToken,
		p_req->Mode, 
		p_req->Priority);

	for (i = 0; i < p_req->sizeSource; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Source>");
		
		if (p_req->Source[i].SourceTokenFlag)
		{
			if (p_req->Source[i].SourceToken.TypeFlag)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:SourceToken Type=\"%s\">", p_req->Source[i].SourceToken.Type);
			}
			else
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:SourceToken>");
			}

			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Token>%s</tt:Token>", p_req->Source[i].SourceToken.Token);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:SourceToken>");
		}

		for (j = 0; j < p_req->Source[i].sizeTracks; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:Tracks>"
					"<tt:SourceTag>%s</tt:SourceTag>"
	      			"<tt:Destination>%s</tt:Destination>"
      			"</tt:Tracks>",
      			p_req->Source[i].Tracks[j].SourceTag,
      			p_req->Source[i].Tracks[j].Destination);
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Source>");
	}
	
	return offset;
}

int bdRecordingJobStateInformation_xml(char * p_buf, int mlen, onvif_RecordingJobStateInformation * p_res)
{
    int i, j;
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:RecordingToken>%s</tt:RecordingToken>" 
		"<tt:State>%s</tt:State>", 
		p_res->RecordingToken,
		p_res->State);

    for (i = 0; i < p_res->sizeSources; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Sources>");
		
		if (p_res->Sources[i].SourceToken.TypeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SourceToken Type=\"%s\">", 
				p_res->Sources[i].SourceToken.Type);
		}
		else
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SourceToken>");
		}
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Token>%s</tt:Token>",
			p_res->Sources[i].SourceToken.Token);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:SourceToken>");

		offset += snprintf(p_buf+offset, mlen-offset, "<tt:State>%s</tt:State>",
			p_res->Sources[i].State);

		// tracks
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Tracks>");

		for (j = 0; j < p_res->Sources[i].sizeTrack; j++)
		{
	      	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Track>");
	      	offset += snprintf(p_buf+offset, mlen-offset, 
	      		"<tt:SourceTag>%s</tt:SourceTag>"
	      		"<tt:Destination>%s</tt:Destination>", 
	      		p_res->Sources[i].Track[j].SourceTag,
	      		p_res->Sources[i].Track[j].Destination);
	      	if (p_res->Sources[i].Track[j].ErrorFlag)
	      	{
	      		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Error>%s</tt:Error>", 
		      		p_res->Sources[i].Track[j].Error);
	      	}
      		offset += snprintf(p_buf+offset, mlen-offset, "<tt:State>%s</tt:State>", 
	      		p_res->Sources[i].Track[j].State);
	      	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Track>");
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Tracks>");
		// end of tracks
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Sources>");	
	}  

	return offset;
}

int bdtrc_GetRecordings_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	RecordingList * p_recording = g_onvif_cfg.recordings;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingsResponse>");
	while (p_recording)
	{
		TrackList * p_track = p_recording->Recording.Tracks;
		
		offset += snprintf(p_buf+offset, mlen-offset, "<trc:RecordingItem>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingToken>%s</tt:RecordingToken>", p_recording->Recording.RecordingToken);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Configuration>");
		offset += bdRecordingConfiguration_xml(p_buf+offset, mlen-offset, &p_recording->Recording.Configuration);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Configuration>");
		
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Tracks>");
		while (p_track)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Track><tt:TrackToken>%s</tt:TrackToken><tt:Configuration>", p_track->Track.TrackToken);
			offset += bdTrackConfiguration_xml(p_buf+offset, mlen-offset, &p_track->Track.Configuration);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:Configuration></tt:Track>");
			
			p_track = p_track->next;
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Tracks>");
		
		offset += snprintf(p_buf+offset, mlen-offset, "</trc:RecordingItem>");
		
		p_recording = p_recording->next;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:GetRecordingsResponse>");

	return offset;
}

int bdtrc_SetRecordingConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:SetRecordingConfigurationResponse />");
	return offset;
}

int bdtrc_GetRecordingConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	RecordingList * p_recording = onvif_find_Recording(g_onvif_cfg.recordings, argv);
	if (NULL == p_recording)
	{
		return ONVIF_ERR_NoRecording;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingConfigurationResponse><trc:RecordingConfiguration>");
	offset += bdRecordingConfiguration_xml(p_buf+offset, mlen-offset, &p_recording->Recording.Configuration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:RecordingConfiguration></trc:GetRecordingConfigurationResponse>");

	return offset;
}

int bdtrc_CreateTrack_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trc:CreateTrackResponse>"
			"<trc:TrackToken>%s</trc:TrackToken>"
		"</trc:CreateTrackResponse>",
		argv);

	return offset;
}

int bdtrc_DeleteTrack_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:DeleteTrackResponse />");
	return offset;
}

int bdtrc_GetTrackConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	TrackList * p_track;
	RecordingList * p_recording;
	trc_GetTrackConfiguration_REQ * p_req = (trc_GetTrackConfiguration_REQ *)argv;

	p_recording = onvif_find_Recording(g_onvif_cfg.recordings, p_req->RecordingToken);
	if (NULL == p_recording)
	{
		return ONVIF_ERR_NoRecording;
	}

	p_track = onvif_find_Track(p_recording->Recording.Tracks, p_req->TrackToken);
	if (NULL == p_track)
	{
		return ONVIF_ERR_NoTrack;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetTrackConfigurationResponse><trc:TrackConfiguration>");
	offset += bdTrackConfiguration_xml(p_buf+offset, mlen-offset, &p_track->Track.Configuration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:TrackConfiguration></trc:GetTrackConfigurationResponse>");

	return offset;
}

int bdtrc_SetTrackConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:SetTrackConfigurationResponse />");
	return offset;
}

int bdtrc_CreateRecordingJob_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	trc_CreateRecordingJob_REQ * p_req = (trc_CreateRecordingJob_REQ *) argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<trc:CreateRecordingJobResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:JobToken>%s</trc:JobToken>", p_req->JobToken);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:JobConfiguration>");
	offset += bdRecordingJobConfiguration_xml(p_buf+offset, mlen-offset, &p_req->JobConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:JobConfiguration>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:CreateRecordingJobResponse>");

	return offset;
}

int bdtrc_DeleteRecordingJob_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:DeleteRecordingJobResponse />");
	return offset;
}

int bdtrc_GetRecordingJobs_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	RecordingJobList * p_recordingjob = g_onvif_cfg.recording_jobs;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingJobsResponse>");
	while (p_recordingjob)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trc:JobItem>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:JobToken>%s</tt:JobToken>", p_recordingjob->RecordingJob.JobToken);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:JobConfiguration>");
		offset += bdRecordingJobConfiguration_xml(p_buf+offset, mlen-offset, &p_recordingjob->RecordingJob.JobConfiguration);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:JobConfiguration>");
		offset += snprintf(p_buf+offset, mlen-offset, "</trc:JobItem>");

		p_recordingjob = p_recordingjob->next;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:GetRecordingJobsResponse>");

	return offset;
}

int bdtrc_SetRecordingJobConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	trc_SetRecordingJobConfiguration_REQ * p_req = (trc_SetRecordingJobConfiguration_REQ *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<trc:SetRecordingJobConfigurationResponse><trc:JobConfiguration>");
	offset += bdRecordingJobConfiguration_xml(p_buf+offset, mlen-offset, &p_req->JobConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:JobConfiguration></trc:SetRecordingJobConfigurationResponse>");
    
	return offset;
}

int bdtrc_GetRecordingJobConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	RecordingJobList * p_recordingjob = onvif_find_RecordingJob(g_onvif_cfg.recording_jobs, argv);
	if (NULL == p_recordingjob)
	{
		return ONVIF_ERR_NoRecordingJob;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingJobConfigurationResponse><trc:JobConfiguration>");
	offset += bdRecordingJobConfiguration_xml(p_buf+offset, mlen-offset, &p_recordingjob->RecordingJob.JobConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:JobConfiguration></trc:GetRecordingJobConfigurationResponse>");
	
	return offset;
}

int bdtrc_SetRecordingJobMode_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:SetRecordingJobModeResponse />");
	return offset;
}

int bdtrc_GetRecordingJobState_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	onvif_RecordingJobStateInformation * p_res = (onvif_RecordingJobStateInformation *) argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingJobStateResponse><trc:State>");
    offset += bdRecordingJobStateInformation_xml(p_buf+offset, mlen-offset, p_res);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:State></trc:GetRecordingJobStateResponse>");

	return offset;
}

int bdtrc_GetRecordingOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	onvif_RecordingOptions * p_res = (onvif_RecordingOptions *) argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingOptionsResponse><trc:Options>");

	// job options
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:Job");
	if (p_res->Job.SpareFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " Spare=\"%d\" ", p_res->Job.Spare);
	}
	if (p_res->Job.CompatibleSourcesFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " CompatibleSources=\"%s\"", p_res->Job.CompatibleSources);
	}
	offset += snprintf(p_buf+offset, mlen-offset, "></trc:Job>");

	// track options
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:Track");
	if (p_res->Track.SpareTotalFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " SpareTotal=\"%d\"", p_res->Track.SpareTotal);
	}
	if (p_res->Track.SpareVideoFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " SpareVideo=\"%d\"", p_res->Track.SpareVideo);
	}
	if (p_res->Track.SpareAudioFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " SpareAudio=\"%d\"", p_res->Track.SpareAudio);
	}
	if (p_res->Track.SpareMetadataFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " SpareMetadata=\"%d\"", p_res->Track.SpareMetadata);
	}
	offset += snprintf(p_buf+offset, mlen-offset, "></trc:Track>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:Options></trc:GetRecordingOptionsResponse>");

	return offset;
}

int bdtrp_GetReplayUri_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	trp_GetReplayUri_RES * p_res = (trp_GetReplayUri_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trp:GetReplayUriResponse>"
			"<trp:Uri>%s</trp:Uri>"
		"</trp:GetReplayUriResponse>",
		p_res->Uri);

	return offset;
}

int bdtrp_GetReplayConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	trp_GetReplayConfiguration_RES * p_res = (trp_GetReplayConfiguration_RES *)argv;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trp:GetReplayConfigurationResponse>"
			"<trp:Configuration>"
				"<tt:SessionTimeout>PT%dS</tt:SessionTimeout>"
			"</trp:Configuration>"
		"</trp:GetReplayConfigurationResponse>",
		p_res->SessionTimeout);

	return offset;
}

int bdtrp_SetReplayConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<trp:SetReplayConfigurationResponse />");
	return offset;
}

#endif // end of PROFILE_G_SUPPORT

#ifdef PROFILE_C_SUPPORT

int bdAccessPointInfo_xml(char * p_buf, int mlen, onvif_AccessPointInfo * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tac:Name>%s</tac:Name>", p_res->Name);

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tac:Description>%s</tac:Description>", p_res->Description);
    }
    if (p_res->AreaFromFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tac:AreaFrom>%s</tac:AreaFrom>", p_res->AreaFrom);
    }
    if (p_res->AreaToFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tac:AreaTo>%s</tac:AreaTo>", p_res->AreaTo);
    }
    if (p_res->EntityTypeFlag)
    {
        if (p_res->EntityTypeAttrFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
                "<tac:EntityType %s>%s</tac:EntityType>", 
                p_res->EntityTypeAttr, 
                p_res->EntityType);
        }
        else
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
                "<tac:EntityType>%s</tac:EntityType>", 
                p_res->EntityType);
        }
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<tac:Entity>%s</tac:Entity>", p_res->Entity);

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tac:Capabilities DisableAccessPoint=\"%s\" Duress=\"%s\" AnonymousAccess=\"%s\" "
        "AccessTaken=\"%s\" ExternalAuthorization=\"%s\" IdentiferAccess=\"%s\" ",
        p_res->Capabilities.DisableAccessPoint ? "true" : "false",
        p_res->Capabilities.Duress ? "true" : "false",
        p_res->Capabilities.AnonymousAccess ? "true" : "false",
        p_res->Capabilities.AccessTaken ? "true" : "false",
        p_res->Capabilities.ExternalAuthorization ? "true" : "false",
        p_res->Capabilities.IdentiferAccess ? "true" : "false");

    if (p_res->Capabilities.SupportedRecognitionTypesFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "SupportedRecognitionTypes=\"%s\" ",
            p_res->Capabilities.SupportedRecognitionTypes);
    }

    if (p_res->Capabilities.SupportedFeedbackTypesFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "SupportedFeedbackTypes=\"%s\" ",
            p_res->Capabilities.SupportedFeedbackTypes);
    }

    offset += snprintf(p_buf+offset, mlen-offset, " />");
    
    return offset;
}

int bdAreaInfo_xml(char * p_buf, int mlen, onvif_AreaInfo * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tac:Name>%s</tac:Name>", p_res->Name);

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tac:Description>%s</tac:Description>", p_res->Description);
    }

    return offset;
}

int bdDoorInfo_xml(char * p_buf, int mlen, onvif_DoorInfo * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tdc:Name>%s</tdc:Name>", p_res->Name);

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tdc:Description>%s</tdc:Description>", p_res->Description);
    }

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tdc:Capabilities Access=\"%s\" AccessTimingOverride=\"%s\" Lock=\"%s\" Unlock=\"%s\" Block=\"%s\" DoubleLock=\"%s\" LockDown=\"%s\" "
        "LockOpen=\"%s\" DoorMonitor=\"%s\" LockMonitor=\"%s\" DoubleLockMonitor=\"%s\" Alarm=\"%s\" Tamper=\"%s\" Fault=\"%s\" />",
        p_res->Capabilities.Access ? "true" : "false",
        p_res->Capabilities.AccessTimingOverride ? "true" : "false",
        p_res->Capabilities.Lock ? "true" : "false",
        p_res->Capabilities.Unlock ? "true" : "false",
        p_res->Capabilities.Block ? "true" : "false",
        p_res->Capabilities.DoubleLock ? "true" : "false",
        p_res->Capabilities.LockDown ? "true" : "false",
        p_res->Capabilities.LockOpen ? "true" : "false",
        p_res->Capabilities.DoorMonitor ? "true" : "false",
        p_res->Capabilities.LockMonitor ? "true" : "false",
        p_res->Capabilities.DoubleLockMonitor ? "true" : "false",
        p_res->Capabilities.Alarm ? "true" : "false",
        p_res->Capabilities.Tamper ? "true" : "false",
        p_res->Capabilities.Fault ? "true" : "false");

    return offset;
}

int bdTimings_xml(char * p_buf, int mlen, onvif_Timings * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tdc:ReleaseTime>PT%uS</tdc:ReleaseTime>"
        "<tdc:OpenTime>PT%uS</tdc:OpenTime>",
        p_res->ReleaseTime, p_res->OpenTime);

    if (p_res->ExtendedReleaseTimeFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:ExtendedReleaseTime>PT%uS</tdc:ExtendedReleaseTime>",
            p_res->ExtendedReleaseTime);
    }

    if (p_res->DelayTimeBeforeRelockFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:DelayTimeBeforeRelock>PT%uS</tdc:DelayTimeBeforeRelock>",
            p_res->DelayTimeBeforeRelock);
    }

    if (p_res->ExtendedOpenTimeFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:ExtendedOpenTime>PT%uS</tdc:ExtendedOpenTime>",
            p_res->ExtendedOpenTime);
    }

    if (p_res->PreAlarmTimeFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:PreAlarmTime>PT%uS</tdc:PreAlarmTime>",
            p_res->PreAlarmTime);
    }
    
    return offset;
}

int bdDoor_xml(char * p_buf, int mlen, onvif_Door * p_req)
{
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, "<tdc:Door token=\"%s\">", p_req->DoorInfo.token);

    offset += bdDoorInfo_xml(p_buf+offset, mlen-offset, &p_req->DoorInfo);

    offset += snprintf(p_buf+offset, mlen-offset, "<tdc:DoorType>%s</tdc:DoorType>", p_req->DoorType);

    offset += snprintf(p_buf+offset, mlen-offset, "<tdc:Timings>");
    offset += bdTimings_xml(p_buf+offset, mlen-offset, &p_req->Timings);
    offset += snprintf(p_buf+offset, mlen-offset, "</tdc:Timings>");
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tdc:Door>");
    
    return offset;
}

int bdtac_GetAccessPointList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tac_GetAccessPointList_RES * p_res = (tac_GetAccessPointList_RES *)argv;
    AccessPointList * p_info = p_res->AccessPoint;

	offset += snprintf(p_buf+offset, mlen-offset, "<tac:GetAccessPointListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tac:NextStartReference>%s</tac:NextStartReference>",
            p_res->NextStartReference);
    }

    while (p_info)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tac:AccessPoint token=\"%s\">", p_info->AccessPointInfo.token);
        offset += bdAccessPointInfo_xml(p_buf+offset, mlen-offset, &p_info->AccessPointInfo);
        offset += snprintf(p_buf+offset, mlen-offset, 
                "<tac:AuthenticationProfileToken>%s</tac:AuthenticationProfileToken>",
                p_info->AuthenticationProfileToken);
        offset += snprintf(p_buf+offset, mlen-offset, "</tac:AccessPoint>");
        
        p_info = p_info->next;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tac:GetAccessPointListResponse>");

	return offset;
}

int bdtac_GetAccessPoints_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i = 0;
    int offset = 0;
    tac_GetAccessPoints_REQ * p_req = (tac_GetAccessPoints_REQ *)argv;
    AccessPointList * p_accesspoint;

	offset += snprintf(p_buf+offset, mlen-offset, "<tac:GetAccessPointsResponse>");

    for (i = 0; i < ARRAY_SIZE(p_req->token); i++)
    {
        if (p_req->token[i][0] == '\0')
        {
            break;
        }

        p_accesspoint = onvif_find_AccessPoint(g_onvif_cfg.access_points, p_req->token[i]);
        if (p_accesspoint)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tac:AccessPoint token=\"%s\">", p_accesspoint->AccessPointInfo.token);
            offset += bdAccessPointInfo_xml(p_buf+offset, mlen-offset, &p_accesspoint->AccessPointInfo);
            offset += snprintf(p_buf+offset, mlen-offset, 
                "<tac:AuthenticationProfileToken>%s</tac:AuthenticationProfileToken>",
                p_accesspoint->AuthenticationProfileToken);
            offset += snprintf(p_buf+offset, mlen-offset, "</tac:AccessPoint>");
        }
    }
	
    offset += snprintf(p_buf+offset, mlen-offset, "</tac:GetAccessPointsResponse>");

	return offset;
}

int bdtac_CreateAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tac_CreateAccessPoint_RES * p_res = (tac_CreateAccessPoint_RES *)argv;
    
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tac:CreateAccessPointResponse>"
	        "<tac:Token>%s</tac:Token>"
	    "</tac:CreateAccessPointResponse>",
	    p_res->Token);
	
	return offset;
}

int bdtac_SetAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tac:SetAccessPointResponse />");
	return offset;
}

int bdtac_ModifyAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tac:ModifyAccessPointResponse />");
	return offset;
}

int bdtac_DeleteAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tac:DeleteAccessPointResponse />");
	return offset;
}

int bdtac_GetAccessPointInfoList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tac_GetAccessPointInfoList_RES * p_res = (tac_GetAccessPointInfoList_RES *)argv;
    AccessPointList * p_info = p_res->AccessPointInfo;

	offset += snprintf(p_buf+offset, mlen-offset, "<tac:GetAccessPointInfoListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tac:NextStartReference>%s</tac:NextStartReference>",
            p_res->NextStartReference);
    }

    while (p_info)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tac:AccessPointInfo token=\"%s\">", p_info->AccessPointInfo.token);
        offset += bdAccessPointInfo_xml(p_buf+offset, mlen-offset, &p_info->AccessPointInfo);
        offset += snprintf(p_buf+offset, mlen-offset, "</tac:AccessPointInfo>");
        
        p_info = p_info->next;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tac:GetAccessPointInfoListResponse>");

	return offset;
}

int bdtac_GetAccessPointInfo_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i = 0;
    int offset = 0;
    tac_GetAccessPointInfo_REQ * p_req = (tac_GetAccessPointInfo_REQ *)argv;
    AccessPointList * p_accesspoint;

	offset += snprintf(p_buf+offset, mlen-offset, "<tac:GetAccessPointInfoResponse>");

    for (i = 0; i < ARRAY_SIZE(p_req->token); i++)
    {
        if (p_req->token[i][0] == '\0')
        {
            break;
        }

        p_accesspoint = onvif_find_AccessPoint(g_onvif_cfg.access_points, p_req->token[i]);
        if (p_accesspoint)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tac:AccessPointInfo token=\"%s\">", p_accesspoint->AccessPointInfo.token);
            offset += bdAccessPointInfo_xml(p_buf+offset, mlen-offset, &p_accesspoint->AccessPointInfo);
            offset += snprintf(p_buf+offset, mlen-offset, "</tac:AccessPointInfo>");
        }
    }
	
    offset += snprintf(p_buf+offset, mlen-offset, "</tac:GetAccessPointInfoResponse>");

	return offset;
}

int bdtac_GetAreaList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tac_GetAreaList_RES * p_res = (tac_GetAreaList_RES *)argv;
    AreaList * p_info = p_res->Area;

	offset += snprintf(p_buf+offset, mlen-offset, "<tac:GetAreaListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tac:NextStartReference>%s</tac:NextStartReference>",
            p_res->NextStartReference);
    }

    while (p_info)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tac:Area token=\"%s\">", p_info->AreaInfo.token);
        offset += bdAreaInfo_xml(p_buf+offset, mlen-offset, &p_info->AreaInfo);
        offset += snprintf(p_buf+offset, mlen-offset, "</tac:Area>");
        
        p_info = p_info->next;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tac:GetAreaListResponse>");

	return offset;
}

int bdtac_GetAreas_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i = 0;
    int offset = 0;
    tac_GetAreas_REQ * p_req = (tac_GetAreas_REQ *)argv;
    AreaList * p_area;

	offset += snprintf(p_buf+offset, mlen-offset, "<tac:GetAreasResponse>");

    for (i = 0; i < ARRAY_SIZE(p_req->token); i++)
    {
        if (p_req->token[i][0] == '\0')
        {
            break;
        }

        p_area = onvif_find_Area(g_onvif_cfg.areas, p_req->token[i]);
        if (p_area)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tac:Area token=\"%s\">", p_area->AreaInfo.token);
            offset += bdAreaInfo_xml(p_buf+offset, mlen-offset, &p_area->AreaInfo);
            offset += snprintf(p_buf+offset, mlen-offset, "</tac:Area>");
        }
    }
	
    offset += snprintf(p_buf+offset, mlen-offset, "</tac:GetAreasResponse>");

	return offset;
}

int bdtac_CreateArea_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tac_CreateArea_RES * p_res = (tac_CreateArea_RES *)argv;
    
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tac:CreateAreaResponse>"
	        "<tac:Token>%s</tac:Token>"
	    "</tac:CreateAreaResponse>",
	    p_res->Token);
	
	return offset;
}

int bdtac_SetArea_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tac:SetAreaResponse />");
	return offset;
}

int bdtac_ModifyArea_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tac:ModifyAreaResponse />");
	return offset;
}

int bdtac_DeleteArea_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tac:DeleteAreaResponse />");
	return offset;
}

int bdtac_GetAreaInfoList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tac_GetAreaInfoList_RES * p_res = (tac_GetAreaInfoList_RES *)argv;
    AreaList * p_info = p_res->AreaInfo;

	offset += snprintf(p_buf+offset, mlen-offset, "<tac:GetAreaInfoListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tac:NextStartReference>%s</tac:NextStartReference>",
            p_res->NextStartReference);
    }

    while (p_info)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tac:AreaInfo token=\"%s\">", p_info->AreaInfo.token);
        offset += bdAreaInfo_xml(p_buf+offset, mlen-offset, &p_info->AreaInfo);
        offset += snprintf(p_buf+offset, mlen-offset, "</tac:AreaInfo>");
        
        p_info = p_info->next;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tac:GetAreaInfoListResponse>");

	return offset;
}

int bdtac_GetAreaInfo_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i = 0;
    int offset = 0;
    tac_GetAreaInfo_REQ * p_req = (tac_GetAreaInfo_REQ *)argv;
    AreaList * p_info;

	offset += snprintf(p_buf+offset, mlen-offset, "<tac:GetAreaInfoResponse>");

    for (i = 0; i < ARRAY_SIZE(p_req->token); i++)
    {
        if (p_req->token[i][0] == '\0')
        {
            break;
        }

        p_info = onvif_find_Area(g_onvif_cfg.areas, p_req->token[i]);
        if (p_info)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tac:AreaInfo token=\"%s\">", p_info->AreaInfo.token);
            offset += bdAreaInfo_xml(p_buf+offset, mlen-offset, &p_info->AreaInfo);
            offset += snprintf(p_buf+offset, mlen-offset, "</tac:AreaInfo>");
        }
    }
	
    offset += snprintf(p_buf+offset, mlen-offset, "</tac:GetAreaInfoResponse>");

	return offset;
}

int bdtac_GetAccessPointState_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tac_GetAccessPointState_REQ * p_req = (tac_GetAccessPointState_REQ *)argv;
    AccessPointList * p_accesspoint = onvif_find_AccessPoint(g_onvif_cfg.access_points, p_req->Token);
    if (NULL == p_accesspoint)
    {
        return ONVIF_ERR_NotFound;
    }

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tac:GetAccessPointStateResponse>"
	        "<tac:AccessPointState>"
                "<tac:Enabled>%s</tac:Enabled>"
            "</tac:AccessPointState>"
        "</tac:GetAccessPointStateResponse>",
        p_accesspoint->Enabled ? "true" : "false");
    
    return offset;
}

int bdtac_EnableAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tac:EnableAccessPointResponse />");
	return offset;
}

int bdtac_DisableAccessPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tac:DisableAccessPointResponse />");
	return offset;
}

int bdtdc_GetDoorList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tdc_GetDoorList_RES * p_res = (tdc_GetDoorList_RES *)argv;
    DoorList * p_door = p_res->Door;

	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:GetDoorListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:NextStartReference>%s</tdc:NextStartReference>",
            p_res->NextStartReference);
    }

    while (p_door)
    {
        offset += bdDoor_xml(p_buf+offset, mlen-offset, &p_door->Door);
        
        p_door = p_door->next;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tdc:GetDoorListResponse>");

	return offset;
}

int bdtdc_GetDoors_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i = 0;
    int offset = 0;
    tdc_GetDoors_REQ * p_req = (tdc_GetDoors_REQ *)argv;
    DoorList * p_door;

	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:GetDoorsResponse>");

    for (i = 0; i < ARRAY_SIZE(p_req->token); i++)
    {
        if (p_req->token[i][0] == '\0')
        {
            break;
        }

        p_door = onvif_find_Door(g_onvif_cfg.doors, p_req->token[i]);
        if (p_door)
        {
            offset += bdDoor_xml(p_buf+offset, mlen-offset, &p_door->Door);
        }
    }
	
    offset += snprintf(p_buf+offset, mlen-offset, "</tdc:GetDoorsResponse>");

	return offset;
}

int bdtdc_CreateDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tdc_CreateDoor_RES * p_res = (tdc_CreateDoor_RES *)argv;
    
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tdc:CreateDoorResponse>"
	        "<tdc:Token>%s</tdc:Token>"
	    "</tdc:CreateDoorResponse>",
	    p_res->Token);
	
	return offset;
}

int bdtdc_SetDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:SetDoorResponse />");
	return offset;
}

int bdtdc_ModifyDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:ModifyDoorResponse />");
	return offset;
}

int bdtdc_DeleteDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:DeleteDoorResponse />");
	return offset;
}

int bdtdc_GetDoorInfoList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tdc_GetDoorInfoList_RES * p_res = (tdc_GetDoorInfoList_RES *)argv;
    DoorInfoList * p_doorinfo = p_res->DoorInfo;

	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:GetDoorInfoListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:NextStartReference>%s</tdc:NextStartReference>",
            p_res->NextStartReference);
    }

    while (p_doorinfo)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tdc:DoorInfo token=\"%s\">", p_doorinfo->DoorInfo.token);
        
        offset += bdDoorInfo_xml(p_buf+offset, mlen-offset, &p_doorinfo->DoorInfo);

        offset += snprintf(p_buf+offset, mlen-offset, "</tdc:DoorInfo>");
        
        p_doorinfo = p_doorinfo->next;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tdc:GetDoorInfoListResponse>");

	return offset;
}

int bdtdc_GetDoorInfo_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i = 0;
    int offset = 0;
    tdc_GetDoorInfo_REQ * p_req = (tdc_GetDoorInfo_REQ *)argv;
    DoorList * p_door;

	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:GetDoorInfoResponse>");

    for (i = 0; i < ARRAY_SIZE(p_req->token); i++)
    {
        if (p_req->token[i][0] == '\0')
        {
            break;
        }

        p_door = onvif_find_Door(g_onvif_cfg.doors, p_req->token[i]);
        if (p_door)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tdc:DoorInfo token=\"%s\">", p_door->Door.DoorInfo.token);
            
            offset += bdDoorInfo_xml(p_buf+offset, mlen-offset, &p_door->Door.DoorInfo);

            offset += snprintf(p_buf+offset, mlen-offset, "</tdc:DoorInfo>");
        }
    }
	
    offset += snprintf(p_buf+offset, mlen-offset, "</tdc:GetDoorInfoResponse>");

	return offset;
}

int bdtdc_GetDoorState_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tdc_GetDoorState_REQ * p_req = (tdc_GetDoorState_REQ *)argv;
    DoorList * p_info = onvif_find_Door(g_onvif_cfg.doors, p_req->Token);
    if (NULL == p_info)
    {
        return ONVIF_ERR_NotFound;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:GetDoorStateResponse><tdc:DoorState>");

    if (p_info->DoorState.DoorPhysicalStateFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:DoorPhysicalState>%s</tdc:DoorPhysicalState>",
            onvif_DoorPhysicalStateToString(p_info->DoorState.DoorPhysicalState));
    }

    if (p_info->DoorState.LockPhysicalStateFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:LockPhysicalState>%s</tdc:LockPhysicalState>",
            onvif_LockPhysicalStateToString(p_info->DoorState.LockPhysicalState));
    }

    if (p_info->DoorState.DoubleLockPhysicalStateFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:DoubleLockPhysicalState>%s</tdc:DoubleLockPhysicalState>",
            onvif_LockPhysicalStateToString(p_info->DoorState.DoubleLockPhysicalState));
    }

    if (p_info->DoorState.AlarmFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:Alarm>%s</tdc:Alarm>",
            onvif_DoorAlarmStateToString(p_info->DoorState.Alarm));
    }

    if (p_info->DoorState.TamperFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tdc:Tamper>");
        if (p_info->DoorState.Tamper.ReasonFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
                "<tdc:Reason>%s</tdc:Reason>", 
                p_info->DoorState.Tamper.Reason);
        }
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:State>%s</tdc:State>",
            onvif_DoorTamperStateToString(p_info->DoorState.Tamper.State));
        offset += snprintf(p_buf+offset, mlen-offset, "</tdc:Tamper>");
    }

    if (p_info->DoorState.FaultFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tdc:Fault>");
        if (p_info->DoorState.Fault.ReasonFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
                "<tdc:Reason>%s</tdc:Reason>", 
                p_info->DoorState.Fault.Reason);
        }
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tdc:State>%s</tdc:State>",
            onvif_DoorFaultStateToString(p_info->DoorState.Fault.State));
        offset += snprintf(p_buf+offset, mlen-offset, "</tdc:Fault>");
    }

	offset += snprintf(p_buf+offset, mlen-offset, 
        "<tdc:DoorMode>%s</tdc:DoorMode>",
        onvif_DoorModeToString(p_info->DoorState.DoorMode));
            
    offset += snprintf(p_buf+offset, mlen-offset, "</tdc:DoorState></tdc:GetDoorStateResponse>");

    return offset;
}

int bdtdc_AccessDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:AccessDoorResponse />");
	return offset;
}

int bdtdc_LockDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:LockDoorResponse />");
	return offset;
}

int bdtdc_UnlockDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:UnlockDoorResponse />");
	return offset;
}

int bdtdc_DoubleLockDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:DoubleLockDoorResponse />");
	return offset;
}

int bdtdc_BlockDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:BlockDoorResponse />");
	return offset;
}

int bdtdc_LockDownDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:LockDownDoorResponse />");
	return offset;
}

int bdtdc_LockDownReleaseDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:LockDownReleaseDoorResponse />");
	return offset;
}

int bdtdc_LockOpenDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:LockOpenDoorResponse />");
	return offset;
}

int bdtdc_LockOpenReleaseDoor_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tdc:LockOpenReleaseDoorResponse />");
	return offset;
}

#endif // end of PROFILE_C_SUPPORT

#ifdef DEVICEIO_SUPPORT

int bdPaneLayout_xml(char * p_buf, int mlen, onvif_PaneLayout * p_PaneLayout)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:PaneLayout>"
            "<tt:Pane>%s</tt:Pane>"
            "<tt:Area bottom=\"%0.1f\" top=\"%0.1f\" right=\"%0.1f\" left=\"%0.1f\">"
            "</tt:Area>"
        "</tt:PaneLayout>",
        p_PaneLayout->Pane,
        p_PaneLayout->Area.bottom,
        p_PaneLayout->Area.top,
        p_PaneLayout->Area.right,
        p_PaneLayout->Area.left);

    return offset;        
}

int bdLayout_xml(char * p_buf, int mlen, onvif_Layout * p_Layout)
{
    int offset = 0;
    PaneLayoutList * p_PaneLayout = p_Layout->PaneLayout;

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Layout>");

    while (p_PaneLayout)
    {
        offset += bdPaneLayout_xml(p_buf+offset, mlen-offset, &p_PaneLayout->PaneLayout);
        
        p_PaneLayout = p_PaneLayout->next;
    }    

    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Layout>");

    return offset;
}

int bdVideoOutput_xml(char * p_buf, int mlen, onvif_VideoOutput * p_VideoOutput)
{
    int offset = 0;

    offset += bdLayout_xml(p_buf+offset, mlen-offset, &p_VideoOutput->Layout);
    if (p_VideoOutput->ResolutionFlag)
    {
        offset += bdVideoResolution_xml(p_buf+offset, mlen-offset, &p_VideoOutput->Resolution);
    }
    if (p_VideoOutput->RefreshRateFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:RefreshRate>%0.2f</tt:RefreshRate>", p_VideoOutput->RefreshRate);
    }
    if (p_VideoOutput->AspectRatioFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:AspectRatio>%0.2f</tt:AspectRatio>", p_VideoOutput->AspectRatio);
    }
    
	return offset;
}

int bdAudioOutputConfiguration_xml(char * p_buf, int mlen, onvif_AudioOutputConfiguration * p_req)
{
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:Name>%s</tt:Name>"
        "<tt:UseCount>%d</tt:UseCount>"
        "<tt:OutputToken>%s</tt:OutputToken>",
        p_req->Name,
        p_req->UseCount,
        p_req->OutputToken);

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:OutputLevel>%d</tt:OutputLevel>",
        p_req->OutputLevel);

    return offset;
}

int bdAudioOutputConfigurationOptions_xml(char * p_buf, int mlen, onvif_AudioOutputConfigurationOptions * p_req)
{
    int i, offset = 0;
    
    for (i = 0; i < p_req->sizeOutputTokensAvailable; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:OutputTokensAvailable>%s</tt:OutputTokensAvailable>",
            p_req->OutputTokensAvailable[i]);
    }

    for (i = 0; i < p_req->sizeSendPrimacyOptions; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:SendPrimacyOptions>%s</tt:SendPrimacyOptions>",
            p_req->SendPrimacyOptions[i]);
    }

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:OutputLevelRange>"
            "<tt:Min>%d</tt:Min>"
            "<tt:Max>%d</tt:Max>"
        "</tt:OutputLevelRange>",
        p_req->OutputLevelRange.Min,
        p_req->OutputLevelRange.Max); 

    return offset;        
}

int bdParityBitList_xml(char * p_buf, int mlen, onvif_ParityBitList * p_req)
{
    int i, offset = 0;

    for (i = 0; i < p_req->sizeItems; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:Items>%s</tt:Items>", onvif_ParityBitToString(p_req->Items[i]));
    }       

    return offset;
}

int bdtmd_GetVideoOutputs_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	VideoOutputList * p_output = g_onvif_cfg.v_output;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<tmd:GetVideoOutputsResponse>");
	
	while (p_output)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	        "<tmd:VideoOutputs token=\"%s\">",
	        p_output->VideoOutput.token);

	    offset += bdVideoOutput_xml(p_buf+offset, mlen-offset, &p_output->VideoOutput);
	    
	    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:VideoOutputs>");

	    p_output = p_output->next;
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:GetVideoOutputsResponse>");            

	return offset;
}

int bdtmd_GetVideoSources_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    VideoSourceList * p_v_src = g_onvif_cfg.v_src;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<tmd:GetVideoSourcesResponse>");

	while (p_v_src)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	    	"<tmd:Token>%s</tmd:Token>", p_v_src->VideoSource.token); 
	    
	    p_v_src = p_v_src->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tmd:GetVideoSourcesResponse>");
	
	return offset;
}

int bdtmd_GetVideoOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tmd_GetVideoOutputConfiguration_REQ * p_req = (tmd_GetVideoOutputConfiguration_REQ *)argv;
    VideoOutputConfigurationList * p_cfg = onvif_find_VideoOutputConfiguration_by_OutputToken(g_onvif_cfg.v_output_cfg, p_req->VideoOutputToken);
    if (NULL == p_cfg)
    {
    	return ONVIF_ERR_NoVideoOutput;
    }

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tmd:GetVideoOutputConfigurationResponse>"
	        "<tmd:VideoOutputConfiguration token=\"%s\">"
                "<tt:Name>%s</tt:Name>"
                "<tt:UseCount>%d</tt:UseCount>"
                "<tt:OutputToken>%s</tt:OutputToken>"
            "</tmd:VideoOutputConfiguration>"
	    "<tmd:GetVideoOutputConfigurationResponse>",
	    p_cfg->Configuration.token,
	    p_cfg->Configuration.Name,
	    p_cfg->Configuration.UseCount,
	    p_cfg->Configuration.OutputToken);
	    
    return offset;
}

int bdtmd_SetVideoOutputConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tmd:SetVideoOutputConfigurationResponse />");
	return offset;
}

int bdtmd_GetVideoOutputConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tmd_GetVideoOutputConfigurationOptions_REQ * p_req = (tmd_GetVideoOutputConfigurationOptions_REQ *)argv;
    VideoOutputList * p_output = onvif_find_VideoOutput(g_onvif_cfg.v_output, p_req->VideoOutputToken);
    if (NULL == p_output)
    {
        return ONVIF_ERR_NoVideoOutput;
    }
    
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tmd:GetVideoOutputConfigurationOptionsResponse>"
	        "<tt:VideoOutputConfigurationOptions />"
        "</tmd:GetVideoOutputConfigurationOptionsResponse>");
	return offset;
}

int bdtmd_GetAudioOutputs_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;

	return offset;
}


///

int bdtmd_GetDigitalInputs_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    DigitalInputList * p_input = g_onvif_cfg.digit_input;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<tmd:GetDigitalInputsResponse>");
	
	while (p_input)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	        "<tmd:DigitalInputs token=\"%s\"",
	        p_input->DigitalInput.token);
	        
        if (p_input->DigitalInput.IdleStateFlag)
        {
	        offset += snprintf(p_buf+offset, mlen-offset, 
	            " IdleState=\"%s\"",
	            onvif_DigitalIdleStateToString(p_input->DigitalInput.IdleState));
        }
        
	    offset += snprintf(p_buf+offset, mlen-offset,">");    
	    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:DigitalInputs>");

	    p_input = p_input->next;
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:GetDigitalInputsResponse>");            

	return offset;
}

int bdtmd_GetDigitalInputConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    onvif_DigitalInputConfigurationOptions * p_option = NULL;
    tmd_GetDigitalInputConfigurationOptions_REQ * p_req = (tmd_GetDigitalInputConfigurationOptions_REQ *)argv;
    
    if (p_req->TokenFlag)
    {
        DigitalInputList * p_input = onvif_find_DigitalInput(g_onvif_cfg.digit_input, p_req->Token);
        if (p_input)
        {
            p_option = &p_input->Options;
        }
    }

    if (NULL == p_option)
    {
        p_option = &g_onvif_cfg.DigitalInputConfigurationOptions;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tmd:GetDigitalInputConfigurationOptionsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<tmd:DigitalInputOptions>");

    if (p_option->DigitalIdleState_closedFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tmd:IdleState>closed</tmd:IdleState>");
    }
    if (p_option->DigitalIdleState_openFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tmd:IdleState>open</tmd:IdleState>");
    }    
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:DigitalInputOptions>");    
	offset += snprintf(p_buf+offset, mlen-offset, "</tmd:GetDigitalInputConfigurationOptionsResponse>");            

	return offset;
}

int bdtmd_SetDigitalInputConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tmd:SetDigitalInputConfigurationsResponse />");
	return offset;
}

int bdtmd_GetSerialPorts_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    SerialPortList * p_port = g_onvif_cfg.serial_port;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<tmd:GetSerialPortsResponse>");
	
	while (p_port)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	        "<tmd:SerialPort token=\"%s\"></tmd:SerialPort>",
	        p_port->SerialPort.token);

	    p_port = p_port->next;
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:GetSerialPortsResponse>");            

	return offset;
}

int bdtmd_GetSerialPortConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tmd_GetSerialPortConfiguration_REQ * p_req = (tmd_GetSerialPortConfiguration_REQ *)argv;

    SerialPortList * p_port = onvif_find_SerialPort(g_onvif_cfg.serial_port, p_req->SerialPortToken);
    if (NULL == p_port)
    {
        return ONVIF_ERR_InvalidSerialPort;
    }

    offset += snprintf(p_buf+offset, mlen-offset,
        "<tmd:GetSerialPortConfigurationResponse>"
            "<tmd:SerialPortConfiguration token=\"%s\" type=\"%s\">"
                "<tmd:BaudRate>%d</tmd:BaudRate>"
                "<tmd:ParityBit>%s</tmd:ParityBit>"
                "<tmd:CharacterLength>%d</tmd:CharacterLength>"
                "<tmd:StopBit>%0.1f</tmd:StopBit>"
                "</tmd:SerialPortConfiguration>"
            "</tmd:SerialPortConfiguration>"
        "</tmd:GetSerialPortConfigurationResponse>",
        p_port->Configuration.token,
        onvif_SerialPortTypeToString(p_port->Configuration.type),
        p_port->Configuration.BaudRate,
        onvif_ParityBitToString(p_port->Configuration.ParityBit),
        p_port->Configuration.CharacterLength,
        p_port->Configuration.StopBit);

	return offset;    
}

int bdtmd_GetSerialPortConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tmd_GetSerialPortConfigurationOptions_REQ * p_req = (tmd_GetSerialPortConfigurationOptions_REQ *)argv;

    SerialPortList * p_port = onvif_find_SerialPort(g_onvif_cfg.serial_port, p_req->SerialPortToken);
    if (NULL == p_port)
    {
        return ONVIF_ERR_InvalidSerialPort;
    }

    offset += snprintf(p_buf+offset, mlen-offset,
        "<tmd:GetSerialPortConfigurationOptionsResponse>"
            "<tmd:SerialPortOptions token=\"%s\">",
        p_port->Options.token);

    offset += snprintf(p_buf+offset, mlen-offset, "<tmd:BaudRateList>");
    offset += bdIntList_xml(p_buf+offset, mlen-offset, &p_port->Options.BaudRateList);
    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:BaudRateList>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tmd:ParityBitList>");
    offset += bdParityBitList_xml(p_buf+offset, mlen-offset, &p_port->Options.ParityBitList);
    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:ParityBitList>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tmd:CharacterLengthList>");
    offset += bdIntList_xml(p_buf+offset, mlen-offset, &p_port->Options.CharacterLengthList);
    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:CharacterLengthList>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tmd:StopBitList>");
    offset += bdFloatList_xml(p_buf+offset, mlen-offset, &p_port->Options.StopBitList);
    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:StopBitList>");
    
    offset += snprintf(p_buf+offset, mlen-offset,            
            "</tmd:SerialPortOptions>"
        "</tmd:GetSerialPortConfigurationOptionsResponse>");        

	return offset; 
}

int bdtmd_SetSerialPortConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tmd:SetSerialPortConfigurationResponse />");
	return offset;
}

int bdtmd_SendReceiveSerialCommand_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tmd_SendReceiveSerialCommand_RES * p_res = (tmd_SendReceiveSerialCommand_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tmd:SendReceiveSerialCommandResponse>");

	if (p_res->SerialDataFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tmd:SerialData>");
	    if (p_res->SerialData._union_SerialData == 0)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, 
	            "<tmd:Binary>%s</tmd:Binary>",
	            p_res->SerialData.union_SerialData.Binary);
	    }
	    else
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, 
	            "<tmd:String>%s</tmd:String>",
	            p_res->SerialData.union_SerialData.String);
	    }
	    offset += snprintf(p_buf+offset, mlen-offset, "</tmd:SerialData>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</tmd:SendReceiveSerialCommandResponse>");

	return offset;
}


#endif // end of DEVICEIO_SUPPORT

#ifdef MEDIA2_SUPPORT

BOOL find_ConfigurationType(tr2_GetProfiles_REQ * p_req, const char * type)
{
    int i;

    for (i = 0; i < p_req->sizeType; i++)
    {
        if (strcasecmp(p_req->Type[i], type) == 0 || 
            strcasecmp(p_req->Type[i], "all") == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

int bdColorspaceRange_xml(char * p_buf, int mlen, onvif_ColorspaceRange * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:X>"
            "<tt:Min>%0.2f</tt:Min>"
            "<tt:Max>%0.2f</tt:Max>"
        "</tt:X>"
        "<tt:Y>"
            "<tt:Min>%0.2f</tt:Min>"
            "<tt:Max>%0.2f</tt:Max>"
        "</tt:Y>"
        "<tt:Z>"
            "<tt:Min>%0.2f</tt:Min>"
            "<tt:Max>%0.2f</tt:Max>"
        "</tt:Z>"
        "<tt:Colorspace>%s</tt:Colorspace>",
        p_req->X.Min, p_req->X.Max,
        p_req->Y.Min, p_req->Y.Max,
        p_req->Z.Min, p_req->Z.Max,
        p_req->Colorspace);

    return offset;

}

int bdColorOptions_xml(char * p_buf, int mlen, onvif_ColorOptions * p_req)
{
    int i;
    int offset = 0;

    for (i = 0; i < p_req->sizeColorList; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:ColorList X=\"%0.2f\" Y=\"%0.2f\" Z=\"%0.2f\"", 
            p_req->ColorList[i].X, p_req->ColorList[i].Y, p_req->ColorList[i].Z);

        if (p_req->ColorList[i].ColorspaceFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
                " Colorspace=\"%s\"", p_req->ColorList[i].Colorspace);
        }

        offset += snprintf(p_buf+offset, mlen-offset, " />");
    }

    for (i = 0; i < p_req->sizeColorspaceRange; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:ColorspaceRange>");
        offset += bdColorspaceRange_xml(p_buf+offset, mlen-offset, &p_req->ColorspaceRange[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:ColorspaceRange>");
    }

    return offset;
}

int bdOSDColorOptions_xml(char * p_buf, int mlen, onvif_OSDColorOptions * p_req)
{
    int offset = 0;

    if (p_req->ColorFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Color>");
        offset += bdColorOptions_xml(p_buf+offset, mlen-offset, &p_req->Color);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Color>");
    }

    if (p_req->TransparentFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:Transparent>"
				"<tt:Min>%d</tt:Min>"				
				"<tt:Max>%d</tt:Max>"				
			"</tt:Transparent>", 
			p_req->Transparent.Min,
			p_req->Transparent.Max);
    }

    return offset;    
}

int bdOSDTextOptions_xml(char * p_buf, int mlen, onvif_OSDTextOptions * p_req)
{
    int i;
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:TextOption>");
		
	if (p_req->OSDTextType_Plain)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTextTypeToString(OSDTextType_Plain));
	}
	if (p_req->OSDTextType_Date)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTextTypeToString(OSDTextType_Date));
	}
	if (p_req->OSDTextType_Time)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTextTypeToString(OSDTextType_Time));
	}
	if (p_req->OSDTextType_DateAndTime)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTextTypeToString(OSDTextType_DateAndTime));
	}

	if (p_req->FontSizeRangeFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:FontSizeRange>"
				"<tt:Min>%d</tt:Min>"				
				"<tt:Max>%d</tt:Max>"				
			"</tt:FontSizeRange>", 
			p_req->FontSizeRange.Min,
			p_req->FontSizeRange.Max);
	}

	for (i = 0; i < p_req->DateFormatSize; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:DateFormat>%s</tt:DateFormat>",
			p_req->DateFormat[i]);
	}
	
	for (i = 0; i < p_req->TimeFormatSize; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:TimeFormat>%s</tt:TimeFormat>",
			p_req->TimeFormat[i]);
	}

	if (p_req->FontColorFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:FontColor>");
	    offset += bdOSDColorOptions_xml(p_buf+offset, mlen-offset, &p_req->FontColor);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:FontColor>");
	}

	if (p_req->BackgroundColorFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:BackgroundColor>");
	    offset += bdOSDColorOptions_xml(p_buf+offset, mlen-offset, &p_req->BackgroundColor);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:BackgroundColor>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</tt:TextOption>");

	return offset;
}

int bdOSDImgOptions_xml(char * p_buf, int mlen, onvif_OSDImgOptions * p_req)
{
    int i;
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:ImageOption>");

	for (i = 0; i < p_req->ImagePathSize; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:ImagePath>%s</tt:ImagePath>",
			p_req->ImagePath[i]);
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:ImageOption>");

	return offset;
}

int bdPolygon_xml(char * p_buf, int mlen, onvif_Polygon * p_req)
{
    int i;
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Polygon>");

    for (i = 0; i < p_req->sizePoint; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:Point x=\"%0.2f\" y=\"%0.2f\" />",
            p_req->Point[i].x, p_req->Point[i].y);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Polygon>");
    
    return offset;
}

int bdColor_xml(char * p_buf, int mlen, onvif_Color * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:Color X=\"%0.2f\" Y=\"%0.2f\" Z=\"%0.2f\"", 
        p_req->X, p_req->Y, p_req->Z);

    if (p_req->ColorspaceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            " Colorspace=\"%s\"", p_req->Colorspace);
    }

    offset += snprintf(p_buf+offset, mlen-offset, " />");

    return offset;
}

int bdMask_xml(char * p_buf, int mlen, onvif_Mask * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tr2:ConfigurationToken>%s</tr2:ConfigurationToken>",
        p_req->ConfigurationToken);

    offset += bdPolygon_xml(p_buf+offset, mlen-offset, &p_req->Polygon);

    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Type>%s</tr2:Type>", p_req->Type);

    if (p_req->ColorFlag)
    {
        offset += bdColor_xml(p_buf+offset, mlen-offset, &p_req->Color);
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tr2:Enabled>%s</tr2:Enabled>", 
        p_req->Enabled ? "true" : "false");
    
	return offset;    
}

int bdVideoEncoder2Configuration_xml(char * p_buf, int mlen, onvif_VideoEncoder2Configuration * p_req)
{
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Name>%s</tt:Name>"
		"<tt:UseCount>%d</tt:UseCount>"
	    "<tt:Encoding>%s</tt:Encoding>"
	    "<tt:Resolution>"
	    	"<tt:Width>%d</tt:Width>"
	    	"<tt:Height>%d</tt:Height>"
	    "</tt:Resolution>",
	    p_req->Name, 
	    p_req->UseCount, 
	    p_req->Encoding, 
	    p_req->Resolution.Width, 
	    p_req->Resolution.Height);

	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:RateControl");
	    if (p_req->RateControl.ConstantBitRateFlag)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, " ConstantBitRate=\"%s\"",
	            p_req->RateControl.ConstantBitRate ? "true" : "false");
	    }
	    offset += snprintf(p_buf+offset, mlen-offset, ">");
		    
		offset += snprintf(p_buf+offset, mlen-offset,  
		    	"<tt:FrameRateLimit>%0.1f</tt:FrameRateLimit>"
		    	"<tt:BitrateLimit>%d</tt:BitrateLimit>"
		    "</tt:RateControl>",		    
		    p_req->RateControl.FrameRateLimit,
		    p_req->RateControl.BitrateLimit);
	}



	return offset;  
}

int bdVideoEncoder2ConfigurationOptions_xml(char * p_buf, int mlen, onvif_VideoEncoder2ConfigurationOptions * p_req)
{
    int i;
    int offset = 0;
    
	    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:Encoding>%s</tt:Encoding>"
		"<tt:QualityRange>"
			"<tt:Min>0</tt:Min>"
			"<tt:Max>0</tt:Max>"
		"</tt:QualityRange>", p_req->Encoding);

    // for (i = 0; i < ARRAY_SIZE(p_req->ResolutionsAvailable); i++)
	for (i = 0; i < 4; i++)
    {
        if (p_req->ResolutionsAvailable[i].Width == 0 || p_req->ResolutionsAvailable[i].Height == 0)
        {
            continue;
        }
        
        offset += bdVideoResolution_xml(p_buf+offset, mlen-offset, &p_req->ResolutionsAvailable[i]);
    }
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:BitrateRange>"
			"<tt:Min>%d</tt:Min>"
			"<tt:Max>%d</tt:Max>"
		"</tt:BitrateRange>",
		p_req->BitrateRange.Min, 
		p_req->BitrateRange.Max);
	

    return offset;
}


int bdVideoEncoder2ConfigurationOptions_xml2(char * p_buf, int mlen, onvif_VideoEncoder2ConfigurationOptions * p_req)
{
    int i;
    int offset = 0;
    
	    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:Encoding>H265</tt:Encoding>"
		"<tt:QualityRange>"
			"<tt:Min>0</tt:Min>"
			"<tt:Max>0</tt:Max>"
		"</tt:QualityRange>");

    for (i = 0; i < ARRAY_SIZE(p_req->ResolutionsAvailable); i++)
    {
        if (p_req->ResolutionsAvailable[i].Width == 0 || p_req->ResolutionsAvailable[i].Height == 0)
        {
            continue;
        }
        
        offset += bdVideoResolution_xml(p_buf+offset, mlen-offset, &p_req->ResolutionsAvailable[i]);
    }
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:BitrateRange>"
			"<tt:Min>%d</tt:Min>"
			"<tt:Max>%d</tt:Max>"
		"</tt:BitrateRange>",
		p_req->BitrateRange.Min, 
		p_req->BitrateRange.Max);
	

    return offset;
}


int bdAudioEncoder2Configuration_xml(char * p_buf, int mlen, onvif_AudioEncoder2Configuration * p_req)
{
    int offset = 0;
	return offset;   
}

int bdAudioEncoder2ConfigurationOptions_xml(char * p_buf, int mlen, onvif_AudioEncoder2ConfigurationOptions * p_req)
{
    int offset = 0;

    return offset;
}

int bdtr2_GetVideoEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    BOOL loopflag = 0;
    int offset = 0;
    tr2_GetVideoEncoderConfigurations_REQ * p_req = (tr2_GetVideoEncoderConfigurations_REQ *)argv;
    VideoEncoder2ConfigurationList * p_v_enc_cfg = NULL;

    if (p_req->GetConfiguration.ConfigurationTokenFlag)
    {
        p_v_enc_cfg = onvif_find_VideoEncoder2Configuration(g_onvif_cfg.v_enc_cfg, p_req->GetConfiguration.ConfigurationToken);
        if (NULL == p_v_enc_cfg)
        {
            return ONVIF_ERR_NoConfig;
        }
    }

    if (p_req->GetConfiguration.ProfileTokenFlag)
    {
        ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->GetConfiguration.ProfileToken);
        if (NULL == p_profile)
        {
            return ONVIF_ERR_NoProfile;
        }
    }

    if (NULL == p_v_enc_cfg)
    {
        loopflag = TRUE;
        p_v_enc_cfg = g_onvif_cfg.v_enc_cfg;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetVideoEncoderConfigurationsResponse>");

    while (p_v_enc_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Configurations token=\"%s\"", p_v_enc_cfg->Configuration.token);
        if (p_v_enc_cfg->Configuration.GovLengthFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, " GovLength=\"%d\"", p_v_enc_cfg->Configuration.GovLength);
        }
	    if (p_v_enc_cfg->Configuration.ProfileFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, " Profile=\"%s\"", p_v_enc_cfg->Configuration.Profile);
        }
	    offset += snprintf(p_buf+offset, mlen-offset, ">");
    	offset += bdVideoEncoder2Configuration_xml(p_buf+offset, mlen-offset, &p_v_enc_cfg->Configuration);
    	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Configurations>");

    	if (loopflag)
    	{
    	    p_v_enc_cfg = p_v_enc_cfg->next;
    	}
    	else
    	{
    	    break;
    	}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetVideoEncoderConfigurationsResponse>");
    
    return offset;
}

int bdtr2_GetVideoEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
    tr2_GetVideoEncoderConfigurationOptions_RES * p_res = (tr2_GetVideoEncoderConfigurationOptions_RES *)argv;
	tr2_GetVideoEncoderConfigurationOptions_RES * p_res2 = (tr2_GetVideoEncoderConfigurationOptions_RES *)argv;
    VideoEncoder2ConfigurationOptionsList * p_option;

    p_option = p_res->Options;


if (p_option->ch == 0)
{
	offset += snprintf(p_buf + offset, mlen - offset,
		"<tr2:GetVideoEncoderConfigurationOptionsResponse>"
		"<tr2:Options GovLengthRange=\"1 60\" FrameRatesSupported=\"30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1\" ProfilesSupported=\"Baseline Main High\">"
		"<tt:Encoding>H264</tt:Encoding>"
		"<tt:QualityRange>"
		"<tt:Min>0</tt:Min>"
		"<tt:Max>0</tt:Max>"
		"</tt:QualityRange>"
		"<tt:ResolutionsAvailable>"
		"<tt:Width>640</tt:Width>"
		"<tt:Height>360</tt:Height>"
		"</tt:ResolutionsAvailable>"
		"<tt:ResolutionsAvailable>"
		"<tt:Width>1280</tt:Width>"
		"<tt:Height>720</tt:Height>"
		"</tt:ResolutionsAvailable>"
		"<tt:ResolutionsAvailable>"
		"<tt:Width>1920</tt:Width>"
		"<tt:Height>1080</tt:Height>"
		"</tt:ResolutionsAvailable>"
		"<tt:BitrateRange>"
		"<tt:Min>900</tt:Min>"
		"<tt:Max>16000</tt:Max>"
		"</tt:BitrateRange>"
		"</tr2:Options>"
		"<tr2:Options GovLengthRange=\"1 60\" FrameRatesSupported=\"30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1\" ProfilesSupported=\"Baseline Main High\">"
		"<tt:Encoding>H265</tt:Encoding>"
		"<tt:QualityRange>"
		"<tt:Min>0</tt:Min>"
		"<tt:Max>0</tt:Max>"
		"</tt:QualityRange>"
		"<tt:ResolutionsAvailable>"
		"<tt:Width>640</tt:Width>"
		"<tt:Height>360</tt:Height>"
		"</tt:ResolutionsAvailable>"
		"<tt:ResolutionsAvailable>"
		"<tt:Width>1280</tt:Width>"
		"<tt:Height>720</tt:Height>"
		"</tt:ResolutionsAvailable>"
		"<tt:ResolutionsAvailable>"
		"<tt:Width>1920</tt:Width>"
		"<tt:Height>1080</tt:Height>"
		"</tt:ResolutionsAvailable>"
		"<tt:BitrateRange>"
		"<tt:Min>900</tt:Min>"
		"<tt:Max>16000</tt:Max>"
		"</tt:BitrateRange>"
		"</tr2:Options>"
		"</tr2:GetVideoEncoderConfigurationOptionsResponse>");

	}
	else 
	{
		offset += snprintf(p_buf + offset, mlen - offset,
		"<tr2:GetVideoEncoderConfigurationOptionsResponse>"
			"<tr2:Options GovLengthRange=\"1 60\" FrameRatesSupported=\"30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 \" ProfilesSupported=\"Baseline Main High\">"
				"<tt:Encoding>H264</tt:Encoding>"
				"<tt:QualityRange>"
				"<tt:Min>0</tt:Min>"
				"<tt:Max>0</tt:Max>"
				"</tt:QualityRange>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>1280</tt:Width>"
				"<tt:Height>720</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>960</tt:Width>"
				"<tt:Height>540</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>800</tt:Width>"
				"<tt:Height>600</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>854</tt:Width>"
				"<tt:Height>480</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>640</tt:Width>"
				"<tt:Height>480</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>640</tt:Width>"
				"<tt:Height>360</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:BitrateRange>"
				"<tt:Min>200</tt:Min>"
				"<tt:Max>2000</tt:Max>"
				"</tt:BitrateRange>"
			"</tr2:Options>"
			"<tr2:Options GovLengthRange=\"1 60\" FrameRatesSupported=\"30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 \" ProfilesSupported=\"Baseline Main High\">"
				"<tt:Encoding>H265</tt:Encoding>"
				"<tt:QualityRange>"
				"<tt:Min>0</tt:Min>"
				"<tt:Max>0</tt:Max>"
				"</tt:QualityRange>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>1280</tt:Width>"
				"<tt:Height>720</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>960</tt:Width>"
				"<tt:Height>540</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>800</tt:Width>"
				"<tt:Height>600</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>854</tt:Width>"
				"<tt:Height>480</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>640</tt:Width>"
				"<tt:Height>480</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:ResolutionsAvailable>"
				"<tt:Width>640</tt:Width>"
				"<tt:Height>360</tt:Height>"
				"</tt:ResolutionsAvailable>"
				"<tt:BitrateRange>"
				"<tt:Min>200</tt:Min>"
				"<tt:Max>2000</tt:Max>"
				"</tt:BitrateRange>"
			"</tr2:Options>"
		"</tr2:GetVideoEncoderConfigurationOptionsResponse>");



#if 1
	// offset += snprintf(p_buf + offset, mlen - offset,
    // "<tr2:GetVideoEncoderConfigurationOptionsResponse>"
    // "<tr2:Options GovLengthRange=\"1 255\" FrameRatesSupported=\"30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1\" "
    // "ProfilesSupported=\"Baseline Main High\" ConstantBitRateSupported=\"true\">"
    // "<tt:Encoding>H264</tt:Encoding>"
    // "<tt:QualityRange>"
    // "<tt:Min>1</tt:Min>"
    // "<tt:Max>100</tt:Max>"
    // "</tt:QualityRange>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>1280</tt:Width>"
    // "<tt:Height>720</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>960</tt:Width>"
    // "<tt:Height>540</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>800</tt:Width>"
    // "<tt:Height>600</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>854</tt:Width>"
    // "<tt:Height>480</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>640</tt:Width>"
    // "<tt:Height>480</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>640</tt:Width>"
    // "<tt:Height>360</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:BitrateRange>"
    // "<tt:Min>100</tt:Min>"
    // "<tt:Max>16000</tt:Max>"
    // "</tt:BitrateRange>"
    // "</tr2:Options>"
    // "<tr2:Options GovLengthRange=\"1 255\" FrameRatesSupported=\"30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1\" "
    // "ProfilesSupported=\"Main\" ConstantBitRateSupported=\"true\">"
    // "<tt:Encoding>H265</tt:Encoding>"
    // "<tt:QualityRange>"
    // "<tt:Min>1</tt:Min>"
    // "<tt:Max>100</tt:Max>"
    // "</tt:QualityRange>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>1280</tt:Width>"
    // "<tt:Height>720</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>960</tt:Width>"
    // "<tt:Height>540</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>800</tt:Width>"
    // "<tt:Height>600</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>854</tt:Width>"
    // "<tt:Height>480</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>640</tt:Width>"
    // "<tt:Height>480</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:ResolutionsAvailable>"
    // "<tt:Width>640</tt:Width>"
    // "<tt:Height>360</tt:Height>"
    // "</tt:ResolutionsAvailable>"
    // "<tt:BitrateRange>"
    // "<tt:Min>100</tt:Min>"
    // "<tt:Max>16000</tt:Max>"
    // "</tt:BitrateRange>"
    // "</tr2:Options>"
    // "</tr2:GetVideoEncoderConfigurationOptionsResponse>");

#endif


	}


    return offset;
}

int bdtr2_SetVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:SetVideoEncoderConfigurationResponse />");		    
	return offset;
}

int bdtr2_CreateProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tr2_CreateProfile_RES * p_res = (tr2_CreateProfile_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tr2:CreateProfileResponse>"
	        "<tr2:Token>%s</tr2:Token>"
	    "</tr2:CreateProfileResponse>",
	    p_res->Token);		    
	
	return offset;
}

int bdtr2_Profile_xml(char * p_buf, int mlen, tr2_GetProfiles_REQ * p_req, ONVIF_PROFILE * p_profile)
{
    int offset = 0;
	
	if (p_profile->v_src_cfg && find_ConfigurationType(p_req, "VideoSource"))
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
        	"<tr2:VideoSource token=\"%s\">", 
            p_profile->v_src_cfg->Configuration.token);            
        offset += bdVideoSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->v_src_cfg->Configuration);    
        offset += snprintf(p_buf+offset, mlen-offset, "</tr2:VideoSource>");	            
    }

    if (p_profile->v_enc_cfg && find_ConfigurationType(p_req, "VideoEncoder"))
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
        	"<tr2:VideoEncoder token=\"%s\"", p_profile->v_enc_cfg->Configuration.token);
        if (p_profile->v_enc_cfg->Configuration.GovLengthFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
        	    " GovLength=\"%d\"", p_profile->v_enc_cfg->Configuration.GovLength);
        }
        if (p_profile->v_enc_cfg->Configuration.ProfileFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
        	    " Profile=\"%s\"", onvif_MediaProfile2Media2Profile(p_profile->v_enc_cfg->Configuration.Profile));
        }
        offset += snprintf(p_buf+offset, mlen-offset, ">");
		offset += bdVideoEncoder2Configuration_xml(p_buf+offset, mlen-offset, &p_profile->v_enc_cfg->Configuration);        	    
        offset += snprintf(p_buf+offset, mlen-offset, "</tr2:VideoEncoder>");	            
    }


#ifdef VIDEO_ANALYTICS
    if (p_profile->va_cfg && find_ConfigurationType(p_req, "Analytics"))
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Analytics token=\"%s\">", p_profile->va_cfg->Configuration.token);
		offset += bdVideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->va_cfg->Configuration);
		offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Analytics>");
    }
#endif

    if (p_profile->metadata_cfg && find_ConfigurationType(p_req, "Metadata"))
    {
    	offset += snprintf(p_buf+offset, mlen-offset, 
            "<tr2:Metadata token=\"%s\">", 
            p_profile->metadata_cfg->Configuration.token);
        offset += bdMetadataConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->metadata_cfg->Configuration);
        offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Metadata>");    
    }

#ifdef DEVICEIO_SUPPORT
    if (p_profile->a_output_cfg && find_ConfigurationType(p_req, "AudioOutput"))
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tr2:AudioOutput token=\"%s\">", 
            p_profile->a_output_cfg->Configuration.token);
        offset += bdAudioOutputConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->a_output_cfg->Configuration);
        offset += snprintf(p_buf+offset, mlen-offset, "</tr2:AudioOutput>");
    }
#endif

    
    return offset;
}

int bdtr2_GetProfiles_rly_xml(char * p_buf, int mlen, const char * argv)
{
    BOOL loopflag = 0;
    int offset = 0;
    tr2_GetProfiles_REQ * p_req = (tr2_GetProfiles_REQ *)argv;
    ONVIF_PROFILE * p_profile = NULL;

    if (p_req->TokenFlag)
    {
        p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->Token);
        if (NULL == p_profile)
        {
            return ONVIF_ERR_NoProfile;
        }
    }

    if (NULL == p_profile)
    {
        loopflag = TRUE;
        p_profile = g_onvif_cfg.profiles;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetProfilesResponse>");

    while (p_profile)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	        "<tr2:Profiles token=\"%s\" fixed=\"%s\"><tr2:Name>%s</tr2:Name><tr2:Configurations>",
	        p_profile->token, p_profile->fixed ? "true" : "false", p_profile->name);

	    offset += bdtr2_Profile_xml(p_buf+offset, mlen-offset, p_req, p_profile);
	    
	    offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Configurations></tr2:Profiles>");

    	if (loopflag)
    	{
    	    p_profile = p_profile->next;
    	}
    	else
    	{
    	    break;
    	}
	}    

	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetProfilesResponse>");
    
    return offset;
}

int bdtr2_DeleteProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:DeleteProfileResponse />");		    
	return offset;
}

int bdtr2_AddConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:AddConfigurationResponse />");		    
	return offset;
}

int bdtr2_RemoveConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:RemoveConfigurationResponse />");		    
	return offset;
}

int bdtr2_GetVideoSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    BOOL loopflag = 0;
    int offset = 0;
    tr2_GetVideoSourceConfigurations_REQ * p_req = (tr2_GetVideoSourceConfigurations_REQ *)argv;
    VideoSourceConfigurationList * p_v_src_cfg = NULL;

    if (p_req->GetConfiguration.ConfigurationTokenFlag)
    {
        p_v_src_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->GetConfiguration.ConfigurationToken);
        if (NULL == p_v_src_cfg)
        {
            return ONVIF_ERR_NoConfig;
        }
    }

    if (p_req->GetConfiguration.ProfileTokenFlag)
    {
        ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->GetConfiguration.ProfileToken);
        if (NULL == p_profile)
        {
            return ONVIF_ERR_NoProfile;
        }
    }

    if (NULL == p_v_src_cfg)
    {
        loopflag = TRUE;
        p_v_src_cfg = g_onvif_cfg.v_src_cfg;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetVideoSourceConfigurationsResponse>");

    while (p_v_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Configurations token=\"%s\">", p_v_src_cfg->Configuration.token);
    	offset += bdVideoSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_v_src_cfg->Configuration);
    	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Configurations>");

    	if (loopflag)
    	{
    	    p_v_src_cfg = p_v_src_cfg->next;
    	}
    	else
    	{
    	    break;
    	}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetVideoSourceConfigurationsResponse>");
    
    return offset;
}

int bdtr2_GetMetadataConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    BOOL loopflag = 0;
    int offset = 0;
    tr2_GetMetadataConfigurations_REQ * p_req = (tr2_GetMetadataConfigurations_REQ *)argv;
    MetadataConfigurationList * p_metadata_cfg = NULL;

    if (p_req->GetConfiguration.ConfigurationTokenFlag)
    {
        p_metadata_cfg = onvif_find_MetadataConfiguration(g_onvif_cfg.metadata_cfg, p_req->GetConfiguration.ConfigurationToken);
        if (NULL == p_metadata_cfg)
        {
            return ONVIF_ERR_NoConfig;
        }
    }

    if (p_req->GetConfiguration.ProfileTokenFlag)
    {
        ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->GetConfiguration.ProfileToken);
        if (NULL == p_profile)
        {
            return ONVIF_ERR_NoProfile;
        }
    }

    if (NULL == p_metadata_cfg)
    {
        loopflag = TRUE;
        p_metadata_cfg = g_onvif_cfg.metadata_cfg;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetMetadataConfigurationsResponse>");

    while (p_metadata_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Configurations token=\"%s\">", p_metadata_cfg->Configuration.token);
    	offset += bdMetadataConfiguration_xml(p_buf+offset, mlen-offset, &p_metadata_cfg->Configuration);
    	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Configurations>");

    	if (loopflag)
    	{
    	    p_metadata_cfg = p_metadata_cfg->next;
    	}
    	else
    	{
    	    break;
    	}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetMetadataConfigurationsResponse>");
    
    return offset;
}

int bdtr2_SetVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:SetVideoSourceConfigurationResponse />");		    
	return offset;
}

int bdtr2_SetMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:SetMetadataConfigurationResponse />");		    
	return offset;
}

int bdtr2_SetAudioSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:SetAudioSourceConfigurationResponse />");		    
	return offset;
}

int bdtr2_GetVideoSourceConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	VideoSourceConfigurationList * p_v_src_cfg = NULL;
	
	tr2_GetVideoSourceConfigurationOptions_REQ * p_req = (tr2_GetVideoSourceConfigurationOptions_REQ *)argv;
	if (p_req->GetConfiguration.ProfileTokenFlag && p_req->GetConfiguration.ProfileToken[0] != '\0')
	{
		ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->GetConfiguration.ProfileToken);
		if (NULL == p_profile)
		{
			return ONVIF_ERR_NoProfile;
		}

		p_v_src_cfg = p_profile->v_src_cfg;
	}

	if (p_req->GetConfiguration.ConfigurationTokenFlag && p_req->GetConfiguration.ConfigurationToken[0] != '\0')
	{
		p_v_src_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->GetConfiguration.ConfigurationToken);
		if (NULL == p_v_src_cfg)
		{
			return ONVIF_ERR_NoConfig;
		}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetVideoSourceConfigurationOptionsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Options>");
    
    offset += bdVideoSourceConfigurationOptions_xml(p_buf+offset, mlen-offset, &g_onvif_cfg.VideoSourceConfigurationOptions);
    
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetVideoSourceConfigurationOptionsResponse>");
	
	return offset;
}

int bdtr2_GetMetadataConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	ONVIF_PROFILE * p_profile = NULL;
	tr2_GetMetadataConfigurationOptions_REQ * p_req = (tr2_GetMetadataConfigurationOptions_REQ *) argv;

	if (p_req->GetConfiguration.ProfileTokenFlag)
	{
		p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->GetConfiguration.ProfileToken);
		if (NULL == p_profile)
		{
			return ONVIF_ERR_NoProfile;
		}
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetMetadataConfigurationOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Options>");

	offset += bdMetadataConfigurationOptions_xml(p_buf+offset, mlen-offset, &g_onvif_cfg.MetadataConfigurationOptions);
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetMetadataConfigurationOptionsResponse>");
	
	return offset;
}

int bdtr2_GetVideoEncoderInstances_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tr2_GetVideoEncoderInstances_RES * p_res = (tr2_GetVideoEncoderInstances_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetVideoEncoderInstancesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Info>");

    for (i = 0; i < p_res->Info.sizeCodec; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tr2:Codec>"
                "<tr2:Encoding>%s</tr2:Encoding>"
                "<tr2:Number>%d</tr2:Number>"
            "</tr2:Codec>",
            p_res->Info.Codec[i].Encoding,
            p_res->Info.Codec[i].Number);
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Total>%d</tr2:Total>", p_res->Info.Total);
    
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Info>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetVideoEncoderInstancesResponse>");
	
	return offset;
}

int bdtr2_GetStreamUri_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tr2_GetStreamUri_RES * p_res = (tr2_GetStreamUri_RES *)argv;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tr2:GetStreamUriResponse>"
		    "<tr2:Uri>%s</tr2:Uri>"
	    "</tr2:GetStreamUriResponse>", p_res->Uri);

	return offset;
}

int bdtr2_SetSynchronizationPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:SetSynchronizationPointResponse />");		    
	return offset;
}

int bdtr2_GetVideoSourceModes_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tr2_GetVideoSourceModes_REQ * p_req = (tr2_GetVideoSourceModes_REQ *)argv;

    VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
    if (NULL == p_v_src)
    {
        return ONVIF_ERR_NoVideoSource;
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetVideoSourceModesResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:VideoSourceModes token=\"%s\" Enabled=\"%s\">",
        p_v_src->VideoSourceMode.token, p_v_src->VideoSourceMode.Enabled ? "true" : "false");

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tr2:MaxFramerate>%0.1f</tr2:MaxFramerate>"
        "<tr2:MaxResolution>"
            "<tt:Width>%d</tt:Width>"
            "<tt:Height>%d</tt:Height>"
        "</tr2:MaxResolution>"
        "<tr2:Encodings>%s</tr2:Encodings>"
        "<tr2:Reboot>%s</tr2:Reboot>",
        p_v_src->VideoSourceMode.MaxFramerate,
        p_v_src->VideoSourceMode.MaxResolution.Width,
        p_v_src->VideoSourceMode.MaxResolution.Height,
        p_v_src->VideoSourceMode.Encodings,
        p_v_src->VideoSourceMode.Reboot ? "true" : "false");

    if (p_v_src->VideoSourceMode.DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tr2:Description>%s</tr2:Description>",
            p_v_src->VideoSourceMode.Description);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</tr2:VideoSourceModes>");
    offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetVideoSourceModesResponse>");

	return offset;
}

int bdtr2_SetVideoSourceMode_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tr2_SetVideoSourceMode_RES * p_res = (tr2_SetVideoSourceMode_RES *) argv;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tr2:SetVideoSourceModeResponse>"
            "<tr2:Reboot>%s</tr2:Reboot>"
        "</tr2:SetVideoSourceModeResponse>",
        p_res->Reboot ? "true" : "false");

	return offset;
}

int bdtr2_GetSnapshotUri_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	tr2_GetSnapshotUri_RES * p_res = (tr2_GetSnapshotUri_RES *) argv;
    
	offset += snprintf(p_buf+offset, mlen-offset, 
        "<tr2:GetSnapshotUriResponse>"
            "<tr2:Uri>%s</tr2:Uri>"
        "</tr2:GetSnapshotUriResponse>",
        p_res->Uri);
	
	return offset;
}

int bdtr2_SetOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:SetOSDResponse />");
	return offset;
}

int bdtr2_GetOSDOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	onvif_OSDConfigurationOptions * p_req = &g_onvif_cfg.OSDConfigurationOptions;

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetOSDOptionsResponse><tr2:OSDOptions>");

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaximumNumberOfOSDs Total=\"%d\"",
		p_req->MaximumNumberOfOSDs.Total);
	if (p_req->MaximumNumberOfOSDs.ImageFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " Image=\"%d\"", 
			p_req->MaximumNumberOfOSDs.Image);
	}
	if (p_req->MaximumNumberOfOSDs.PlainTextFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " PlainText=\"%d\"", 
			p_req->MaximumNumberOfOSDs.PlainText);
	}
	if (p_req->MaximumNumberOfOSDs.DateFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " Date=\"%d\"", 
			p_req->MaximumNumberOfOSDs.Date);
	}
	if (p_req->MaximumNumberOfOSDs.TimeFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " Time=\"%d\"", 
			p_req->MaximumNumberOfOSDs.Time);
	}
	if (p_req->MaximumNumberOfOSDs.DateAndTimeFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " DateAndTime=\"%d\"", 
			p_req->MaximumNumberOfOSDs.DateAndTime);
	}
	offset += snprintf(p_buf+offset, mlen-offset, "></tt:MaximumNumberOfOSDs>");

	if (p_req->OSDType_Text)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTypeToString(OSDType_Text));
	}
	if (p_req->OSDType_Image)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTypeToString(OSDType_Image));
	}
	if (p_req->OSDType_Extended)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTypeToString(OSDType_Extended));
	}

	if (p_req->OSDPosType_LowerLeft)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_LowerLeft));
	}
	if (p_req->OSDPosType_LowerRight)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_LowerRight));
	}
	if (p_req->OSDPosType_UpperLeft)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_UpperLeft));
	}
	if (p_req->OSDPosType_UpperRight)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_UpperRight));
	}
	if (p_req->OSDPosType_Custom)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_Custom));
	}

	if (p_req->TextOptionFlag)
	{
	    offset += bdOSDTextOptions_xml(p_buf+offset, mlen-offset, &p_req->TextOption);		
	}

	if (p_req->ImageOptionFlag)
	{
	    offset += bdOSDImgOptions_xml(p_buf+offset, mlen-offset, &p_req->ImageOption);
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:OSDOptions></tr2:GetOSDOptionsResponse>");

	return offset;
}

int bdtr2_GetOSDs_rly_xml(char * p_buf, int mlen, const char * argv)
{
    BOOL loopflag = 0;
    int offset = 0;
    tr2_GetOSDs_REQ * p_req = (tr2_GetOSDs_REQ *)argv;
    OSDConfigurationList * p_osd = NULL;

    if (p_req->OSDTokenFlag)
    {
        p_osd = onvif_find_OSDConfiguration(g_onvif_cfg.OSDs, p_req->OSDToken);
    	if (NULL == p_osd)
    	{
    		return ONVIF_ERR_NoConfig;
	    }
    }

    if (p_req->ConfigurationTokenFlag)
    {
        VideoSourceConfigurationList * p_v_src_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->ConfigurationToken);
        if (NULL == p_v_src_cfg)
        {
        }
    }

    if (NULL == p_osd)
    {
        loopflag = TRUE;
        p_osd = g_onvif_cfg.OSDs;
    }	

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetOSDsResponse>");

    while (p_osd)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:OSDs token=\"%s\">", p_osd->OSD.token);
		offset += bdOSD_xml(p_buf+offset, mlen-offset, p_osd);
		offset += snprintf(p_buf+offset, mlen-offset, "</tr2:OSDs>");

    	if (loopflag)
    	{
    	    p_osd = p_osd->next;
    	}
    	else
    	{
    	    break;
    	}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetOSDsResponse>");
    
    return offset;
}

int bdtr2_CreateOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tr2:CreateOSDResponse><tr2:OSDToken>%s</tr2:OSDToken></tr2:CreateOSDResponse>", argv);

	return offset;
}

int bdtr2_DeleteOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:DeleteOSDResponse />");

	return offset;
}

int bdtr2_CreateMask_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tr2:CreateMaskResponse><tr2:Token>%s</tr2:Token></tr2:CreateMaskResponse>", argv);

	return offset;
}

int bdtr2_DeleteMask_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:DeleteMaskResponse />");
	return offset;
}

int bdtr2_GetMasks_rly_xml(char * p_buf, int mlen, const char * argv)
{
    BOOL loopflag = 0;
    int offset = 0;
    tr2_GetMasks_REQ * p_req = (tr2_GetMasks_REQ *)argv;
    MaskList * p_mask = NULL;

    if (p_req->TokenFlag)
    {
        p_mask = onvif_find_Mask(g_onvif_cfg.mask, p_req->Token);
    	if (NULL == p_mask)
    	{
    		return ONVIF_ERR_NoConfig;
	    }
    }

    if (p_req->ConfigurationTokenFlag)
    {
        VideoSourceConfigurationList * p_v_src_cfg = onvif_find_VideoSourceConfiguration(g_onvif_cfg.v_src_cfg, p_req->ConfigurationToken);
        if (NULL == p_v_src_cfg)
        {
        }
    }

    if (NULL == p_mask)
    {
        loopflag = TRUE;
        p_mask = g_onvif_cfg.mask;
    }	

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetMasksResponse>");

    while (p_mask)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Masks token=\"%s\">", p_mask->Mask.token);
		offset += bdMask_xml(p_buf+offset, mlen-offset, &p_mask->Mask);
		offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Masks>");

    	if (loopflag)
    	{
    	    p_mask = p_mask->next;
    	}
    	else
    	{
    	    break;
    	}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetMasksResponse>");
    
    return offset;
}

int bdtr2_SetMask_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:SetMaskResponse />");
	return offset;
}

int bdtr2_GetMaskOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
	onvif_MaskOptions * p_req = &g_onvif_cfg.MaskOptions;

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetMaskOptionsResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tr2:Options RectangleOnly=\"%s\" SingleColorOnly=\"%s\">",
	    p_req->RectangleOnly ? "true" : "false",
	    p_req->SingleColorOnly ? "true" : "false");
	    
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tr2:MaxMasks>%d</tr2:MaxMasks>", p_req->MaxMasks);
        
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tr2:MaxPoints>%d</tr2:MaxPoints>", p_req->MaxPoints);

    for (i = 0; i < p_req->sizeTypes; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tr2:Types>%s</tr2:Types>", p_req->Types[i]);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Color>");
    offset += bdColorOptions_xml(p_buf+offset, mlen-offset, &p_req->Color);
    offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Color>");

    offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Options>");
    offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetMaskOptionsResponse>");
    
    return offset;        
}

int bdtr2_StartMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:StartMulticastStreamingResponse />");
	return offset;
}

int bdtr2_StopMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:StopMulticastStreamingResponse />");
	return offset;
}


int bdtr2_GetAudioSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    BOOL loopflag = 0;
    int offset = 0;
    tr2_GetAudioSourceConfigurations_REQ * p_req = (tr2_GetAudioSourceConfigurations_REQ *)argv;
    AudioSourceConfigurationList * p_a_src_cfg = NULL;

    if (p_req->GetConfiguration.ConfigurationTokenFlag)
    {
        p_a_src_cfg = onvif_find_AudioSourceConfiguration(g_onvif_cfg.a_src_cfg, p_req->GetConfiguration.ConfigurationToken);
        if (NULL == p_a_src_cfg)
        {
            return ONVIF_ERR_NoConfig;
        }
    }

    if (p_req->GetConfiguration.ProfileTokenFlag)
    {
        ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->GetConfiguration.ProfileToken);
        if (NULL == p_profile)
        {
            return ONVIF_ERR_NoProfile;
        }
    }

    if (NULL == p_a_src_cfg)
    {
        loopflag = TRUE;
        p_a_src_cfg = g_onvif_cfg.a_src_cfg;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetAudioSourceConfigurationsResponse>");

    while (p_a_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Configurations token=\"%s\">", p_a_src_cfg->Configuration.token);
    	offset += bdAudioSourceConfiguration_xml(p_buf+offset, mlen-offset, &p_a_src_cfg->Configuration);
    	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Configurations>");

    	if (loopflag)
    	{
    	    p_a_src_cfg = p_a_src_cfg->next;
    	}
    	else
    	{
    	    break;
    	}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetAudioSourceConfigurationsResponse>");
    
    return offset;
}


int bdtr2_GetAudioEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tr2_GetAudioEncoderConfigurationOptions_REQ * p_req = (tr2_GetAudioEncoderConfigurationOptions_REQ *)argv;
    AudioEncoder2ConfigurationList * p_a_enc_cfg = NULL;
    AudioEncoder2ConfigurationOptionsList * p_option;

    if (p_req->GetConfiguration.ConfigurationTokenFlag)
    {
        p_a_enc_cfg = onvif_find_AudioEncoder2Configuration(g_onvif_cfg.a_enc_cfg, p_req->GetConfiguration.ConfigurationToken);
        if (NULL == p_a_enc_cfg)
        {
            return ONVIF_ERR_NoConfig;
        }
    }

    if (p_req->GetConfiguration.ProfileTokenFlag)
    {
        ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->GetConfiguration.ProfileToken);
        if (NULL == p_profile)
        {
            return ONVIF_ERR_NoProfile;
        }
    }

    p_option = g_onvif_cfg.a_enc_cfg_opt;

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetAudioEncoderConfigurationOptionsResponse>");

    while (p_option)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Options>");        
    	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Options>");

	    p_option = p_option->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetAudioEncoderConfigurationOptionsResponse>");
    
    return offset;
}


#ifdef VIDEO_ANALYTICS

int bdtr2_GetAnalyticsConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    BOOL loopflag = 0;
    int offset = 0;
    tr2_GetAnalyticsConfigurations_REQ * p_req = (tr2_GetAnalyticsConfigurations_REQ *)argv;
    VideoAnalyticsConfigurationList * p_va_cfg = NULL;

    if (p_req->GetConfiguration.ConfigurationTokenFlag)
    {
        p_va_cfg = onvif_find_VideoAnalyticsConfiguration(g_onvif_cfg.va_cfg, p_req->GetConfiguration.ConfigurationToken);
        if (NULL == p_va_cfg)
        {
            return ONVIF_ERR_NoConfig;
        }
    }

    if (p_req->GetConfiguration.ProfileTokenFlag)
    {
        ONVIF_PROFILE * p_profile = onvif_find_profile(g_onvif_cfg.profiles, p_req->GetConfiguration.ProfileToken);
        if (NULL == p_profile)
        {
            return ONVIF_ERR_NoProfile;
        }
    }

    if (NULL == p_va_cfg)
    {
        loopflag = TRUE;
        p_va_cfg = g_onvif_cfg.va_cfg;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tr2:GetAnalyticsConfigurationsResponse>");

    while (p_va_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tr2:Configurations token=\"%s\">", p_va_cfg->Configuration.token);
    	offset += bdVideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_va_cfg->Configuration);
    	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:Configurations>");

    	if (loopflag)
    	{
    	    p_va_cfg = p_va_cfg->next;
    	}
    	else
    	{
    	    break;
    	}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tr2:GetAnalyticsConfigurationsResponse>");
    
    return offset;
}

#endif // end of VIDEO_ANALYTICS

#endif // end of MEDIA2_SUPPORT

#ifdef THERMAL_SUPPORT

int bdColorPalette_xml(char * p_buf, int mlen, onvif_ColorPalette * p_req)
{
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tth:ColorPalette token=\"%s\" Type=\"%s\">"
        "<tth:Name>%s</tth:Name>"
        "</tth:ColorPalette>",
        p_req->token, 
        p_req->Type,
        p_req->Name);

    return offset;        
}

int bdNUCTable_xml(char * p_buf, int mlen, onvif_NUCTable * p_req)
{
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, "<tth:NUCTable token=\"%s\"", p_req->token);
    if (p_req->LowTemperatureFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, " LowTemperature=\"%f\"", p_req->LowTemperature);
    }
    if (p_req->HighTemperatureFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, " HighTemperature=\"%f\"", p_req->HighTemperature);
    }
    offset += snprintf(p_buf+offset, mlen-offset, ">");
    offset += snprintf(p_buf+offset, mlen-offset, "<tth:Name>%s</tth:Name>", p_req->Name);
    offset += snprintf(p_buf+offset, mlen-offset, "</tth:NUCTable>"); 

    return offset;        
}

int bdThermalConfiguration_xml(char * p_buf, int mlen, onvif_ThermalConfiguration * p_req)
{
    int offset = 0;
    
    offset += bdColorPalette_xml(p_buf+offset, mlen-offset, &p_req->ColorPalette);

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tth:Polarity>%s</tth:Polarity>",
        onvif_PolarityToString(p_req->Polarity));

    if (p_req->NUCTableFlag)
    {
        offset += bdNUCTable_xml(p_buf+offset, mlen-offset, &p_req->NUCTable);     
    }

    if (p_req->CoolerFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tth:Cooler>");
        offset += snprintf(p_buf+offset, mlen-offset, "<tth:Enabled>%s</tth:Enabled>", p_req->Cooler.Enabled ? "true" : "false");
        if (p_req->Cooler.RunTimeFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tth:RunTime>%f</tth:RunTime>", p_req->Cooler.RunTime);
        }
        offset += snprintf(p_buf+offset, mlen-offset, "</tth:Cooler>");
    }

    return offset;
}

int bdRadiometryGlobalParameters_xml(char * p_buf, int mlen, onvif_RadiometryGlobalParameters * p_req)
{
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tth:ReflectedAmbientTemperature>%f</tth:ReflectedAmbientTemperature>"
        "<tth:Emissivity>%f</tth:Emissivity>"
        "<tth:DistanceToObject>%f</tth:DistanceToObject>",
        p_req->ReflectedAmbientTemperature, 
        p_req->Emissivity,
        p_req->DistanceToObject);

    if (p_req->RelativeHumidityFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tth:RelativeHumidity>%f</tth:RelativeHumidity>",
            p_req->RelativeHumidity);
    }

    if (p_req->AtmosphericTemperatureFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tth:AtmosphericTemperature>%f</tth:AtmosphericTemperature>",
            p_req->AtmosphericTemperature);
    }

    if (p_req->AtmosphericTransmittanceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tth:AtmosphericTransmittance>%f</tth:AtmosphericTransmittance>",
            p_req->AtmosphericTransmittance);
    }

    if (p_req->ExtOpticsTemperatureFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tth:ExtOpticsTemperature>%f</tth:ExtOpticsTemperature>",
            p_req->ExtOpticsTemperature);
    }

    if (p_req->ExtOpticsTransmittanceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tth:ExtOpticsTransmittance>%f</tth:ExtOpticsTransmittance>",
            p_req->ExtOpticsTransmittance);
    }
    
    return offset;
}

int bdRadiometryConfiguration_xml(char * p_buf, int mlen, onvif_RadiometryConfiguration * p_req)
{
    int offset = 0;

    if (p_req->RadiometryGlobalParametersFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tth:RadiometryGlobalParameters>");
        offset += bdRadiometryGlobalParameters_xml(p_buf+offset, mlen-offset, &p_req->RadiometryGlobalParameters);
        offset += snprintf(p_buf+offset, mlen-offset, "</tth:RadiometryGlobalParameters>");
    }
    
    return offset;
}

int bdRadiometryGlobalParameterOptions_xml(char * p_buf, int mlen, onvif_RadiometryGlobalParameterOptions * p_req)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tth:ReflectedAmbientTemperature>");
    offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_req->ReflectedAmbientTemperature);
    offset += snprintf(p_buf+offset, mlen-offset, "</tth:ReflectedAmbientTemperature>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tth:Emissivity>");
    offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_req->Emissivity);
    offset += snprintf(p_buf+offset, mlen-offset, "</tth:Emissivity>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tth:DistanceToObject>");
    offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_req->DistanceToObject);
    offset += snprintf(p_buf+offset, mlen-offset, "</tth:DistanceToObject>");

    if (p_req->RelativeHumidityFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tth:RelativeHumidity>");
        offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_req->RelativeHumidity);
        offset += snprintf(p_buf+offset, mlen-offset, "</tth:RelativeHumidity>");
    }

    if (p_req->AtmosphericTemperatureFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tth:AtmosphericTemperature>");
        offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_req->AtmosphericTemperature);
        offset += snprintf(p_buf+offset, mlen-offset, "</tth:AtmosphericTemperature>");
    }

    if (p_req->AtmosphericTransmittanceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tth:AtmosphericTransmittance>");
        offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_req->AtmosphericTransmittance);
        offset += snprintf(p_buf+offset, mlen-offset, "</tth:AtmosphericTransmittance>");
    }

    if (p_req->ExtOpticsTemperatureFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tth:ExtOpticsTemperature>");
        offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_req->ExtOpticsTemperature);
        offset += snprintf(p_buf+offset, mlen-offset, "</tth:ExtOpticsTemperature>");
    }

    if (p_req->ExtOpticsTransmittanceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tth:ExtOpticsTransmittance>");
        offset += bdFloatRange_xml(p_buf+offset, mlen-offset, &p_req->ExtOpticsTransmittance);
        offset += snprintf(p_buf+offset, mlen-offset, "</tth:ExtOpticsTransmittance>");
    }
    
    return offset;
}

int bdtth_GetConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    VideoSourceList * p_v_src = g_onvif_cfg.v_src;

	offset += snprintf(p_buf+offset, mlen-offset, "<tth:GetConfigurationsResponse>");
	
    while (p_v_src)
    {
        if (p_v_src->ThermalSupport)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tth:Configurations token=\"%s\"><tth:Configuration>", p_v_src->VideoSource.token);
            offset += bdThermalConfiguration_xml(p_buf+offset, mlen-offset, &p_v_src->ThermalConfiguration);
            offset += snprintf(p_buf+offset, mlen-offset, "</tth:Configuration></tth:Configurations>");
        }

        p_v_src = p_v_src->next;
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</tth:GetConfigurationsResponse>");

	return offset;
}

int bdtth_GetConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tth_GetConfiguration_REQ * p_req = (tth_GetConfiguration_REQ *)argv;
    VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
    if (NULL == p_v_src)
    {
        return ONVIF_ERR_NoScope;
    }

    if (FALSE == p_v_src->ThermalSupport)
    {
        return ONVIF_ERR_NoThermalForSource;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tth:GetConfigurationResponse><tth:Configuration>");
	offset += bdThermalConfiguration_xml(p_buf+offset, mlen-offset, &p_v_src->ThermalConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</tth:Configuration></tth:GetConfigurationResponse>");
    
    return offset;
}

int bdtth_SetConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tth:SetConfigurationResponse />");
	return offset;
}

int bdtth_GetConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    ColorPaletteList * p_ColorPalette;
    NUCTableList * p_NUCTable;
    tth_GetConfigurationOptions_REQ * p_req = (tth_GetConfigurationOptions_REQ *) argv;
    VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
    if (NULL == p_v_src)
    {
        return ONVIF_ERR_NoSource;
    }

    if (FALSE == p_v_src->ThermalSupport)
    {
        return ONVIF_ERR_NoThermalForSource;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tth:GetConfigurationOptionsResponse><tth:ConfigurationOptions>");

    p_ColorPalette = p_v_src->ThermalConfigurationOptions.ColorPalette;
    while (p_ColorPalette)
    {
		offset += bdColorPalette_xml(p_buf+offset, mlen-offset, &p_ColorPalette->ColorPalette);

		p_ColorPalette = p_ColorPalette->next;
    }

	p_NUCTable = p_v_src->ThermalConfigurationOptions.NUCTable;
	while (p_NUCTable)
	{
		offset += bdNUCTable_xml(p_buf+offset, mlen-offset, &p_NUCTable->NUCTable);

		p_NUCTable = p_NUCTable->next;
	}

    if (p_v_src->ThermalConfigurationOptions.CoolerOptionsFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tth:CoolerOptions>"
                "<tth:Enabled>%s</tth:Enabled>"
            "</tth:CoolerOptions>",
            p_v_src->ThermalConfigurationOptions.CoolerOptions.Enabled ? "true" : "false");
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tth:ConfigurationOptions></tth:GetConfigurationOptionsResponse>");
    
    return offset;
}

int bdtth_GetRadiometryConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tth_GetRadiometryConfiguration_REQ * p_req = (tth_GetRadiometryConfiguration_REQ *)argv;
    VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
    if (NULL == p_v_src)
    {
        return ONVIF_ERR_NoSource;
    }

    if (FALSE == p_v_src->ThermalSupport)
    {
        return ONVIF_ERR_NoRadiometryForSource;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tth:GetRadiometryConfigurationResponse><tth:Configuration>");
	offset += bdRadiometryConfiguration_xml(p_buf+offset, mlen-offset, &p_v_src->RadiometryConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</tth:Configuration></tth:GetRadiometryConfigurationResponse>");
    
    return offset;
}

int bdtth_SetRadiometryConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tth:SetRadiometryConfigurationResponse />");
	return offset;
}

int bdtth_GetRadiometryConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tth_GetRadiometryConfigurationOptions_REQ * p_req = (tth_GetRadiometryConfigurationOptions_REQ *) argv;
    VideoSourceList * p_v_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSourceToken);
    if (NULL == p_v_src)
    {
        return ONVIF_ERR_NoSource;
    }

    if (FALSE == p_v_src->ThermalSupport)
    {
        return ONVIF_ERR_NoRadiometryForSource;
    }

	offset += snprintf(p_buf+offset, mlen-offset, "<tth:GetRadiometryConfigurationOptionsResponse><tth:ConfigurationOptions>");

    if (p_v_src->RadiometryConfigurationOptions.RadiometryGlobalParameterOptionsFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tth:RadiometryGlobalParameterOptions>");
        offset += bdRadiometryGlobalParameterOptions_xml(p_buf+offset, mlen-offset, 
            &p_v_src->RadiometryConfigurationOptions.RadiometryGlobalParameterOptions);
        offset += snprintf(p_buf+offset, mlen-offset, "</tth:RadiometryGlobalParameterOptions>");            
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tth:ConfigurationOptions></tth:GetRadiometryConfigurationOptionsResponse>");
    
    return offset;
}

#endif // end of THERMAL_SUPPORT

#ifdef CREDENTIAL_SUPPORT

int bdCredentialInfo_xml(char * p_buf, int mlen, onvif_CredentialInfo * p_res)
{
    int offset = 0;

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:Description>%s</tcr:Description>", 
            p_res->Description);
    }

    offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:CredentialHolderReference>%s</tcr:CredentialHolderReference>", 
            p_res->CredentialHolderReference);

    if (p_res->ValidFromFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:ValidFrom>%s</tcr:ValidFrom>", 
            p_res->ValidFrom);
    }

    if (p_res->ValidToFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:ValidTo>%s</tcr:ValidTo>", 
            p_res->ValidTo);
    }
    
    return offset;
}

int bdCredentialIdentifierType_xml(char * p_buf, int mlen, onvif_CredentialIdentifierType * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tcr:Name>%s</tcr:Name>"
        "<tcr:FormatType>%s</tcr:FormatType>",
        p_res->Name,
        p_res->FormatType);

    return offset;
}

int bdCredentialIdentifier_xml(char * p_buf, int mlen, onvif_CredentialIdentifier * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tcr:Type>");
    offset += bdCredentialIdentifierType_xml(p_buf+offset, mlen-offset, &p_res->Type);   
    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:Type>");

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tcr:ExemptedFromAuthentication>%s</tcr:ExemptedFromAuthentication>"
        "<tcr:Value>%s</tcr:Value>",
        p_res->ExemptedFromAuthentication ? "true" : "false",
        p_res->Value);

    return offset;
}

int bdCredentialIdentifierItem_xml(char * p_buf, int mlen, onvif_CredentialIdentifierItem * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tcr:Type>");
    offset += bdCredentialIdentifierType_xml(p_buf+offset, mlen-offset, &p_res->Type);   
    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:Type>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tcr:Value>%s</tcr:Value>", p_res->Value);

    return offset;
}

int bdCredentialAccessProfile_xml(char * p_buf, int mlen, onvif_CredentialAccessProfile * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tcr:AccessProfileToken>%s</tcr:AccessProfileToken>",
        p_res->AccessProfileToken);

    if (p_res->ValidFromFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:ValidFrom>%s</tcr:ValidFrom>", 
            p_res->ValidFrom);
    }

    if (p_res->ValidToFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:ValidTo>%s</tcr:ValidTo>", 
            p_res->ValidTo);
    }
    
    return offset;        
}

int bdCredential_xml(char * p_buf, int mlen, onvif_Credential * p_res)
{
    int i;
    int offset = 0;

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:Description>%s</tcr:Description>", 
            p_res->Description);
    }

    offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:CredentialHolderReference>%s</tcr:CredentialHolderReference>", 
            p_res->CredentialHolderReference);

    if (p_res->ValidFromFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:ValidFrom>%s</tcr:ValidFrom>", 
            p_res->ValidFrom);
    }

    if (p_res->ValidToFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:ValidTo>%s</tcr:ValidTo>", 
            p_res->ValidTo);
    }

    for (i = 0; i < p_res->sizeCredentialIdentifier; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tcr:CredentialIdentifier>");
        offset += bdCredentialIdentifier_xml(p_buf+offset, mlen-offset, &p_res->CredentialIdentifier[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tcr:CredentialIdentifier>");
    }

    for (i = 0; i < p_res->sizeCredentialAccessProfile; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tcr:CredentialAccessProfile>");
        offset += bdCredentialAccessProfile_xml(p_buf+offset, mlen-offset, &p_res->CredentialAccessProfile[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tcr:CredentialAccessProfile>");
    }

    offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:ExtendedGrantTime>%s</tcr:ExtendedGrantTime>",
            p_res->ExtendedGrantTime ? "true" : "false");
    
    for (i = 0; i < p_res->sizeAttribute; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:Attribute Name=\"%s\"",
            p_res->Attribute[i].Name);

        if (p_res->Attribute[i].ValueFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
                " Value=\"%s\"",
                p_res->Attribute[i].Value);
        }

        offset += snprintf(p_buf+offset, mlen-offset, "/>");
    }
    
    return offset;
}

int bdCredentialIdentifierFormatTypeInfo_xml(char * p_buf, int mlen, onvif_CredentialIdentifierFormatTypeInfo * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tcr:FormatType>%s</tcr:FormatType>"
        "<tcr:Description>%s</tcr:Description>",
        p_res->FormatType,
        p_res->Description);

    return offset;
}

int bdCredentialState_xml(char * p_buf, int mlen, onvif_CredentialState * p_res)
{
    int offset = 0;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tcr:Enabled>%s</tcr:Enabled>", 
	    p_res->Enabled ? "true" : "false");

	if (p_res->ReasonFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tcr:Reason>%s</tcr:Reason>", p_res->Reason);
	}
	
	if (p_res->AntipassbackStateFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	        "<tcr:AntipassbackState>"
	            "<tcr:AntipassbackViolated>%s</tcr:AntipassbackViolated>"
	        "</tcr:AntipassbackState>", 
	        p_res->AntipassbackState.AntipassbackViolated ? "true" : "false");
	}

	return offset;
}

int bdtcr_GetCredentialInfo_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tcr_GetCredentialInfo_RES * p_res = (tcr_GetCredentialInfo_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetCredentialInfoResponse>");

	for (i = 0; i < p_res->sizeCredentialInfo; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tcr:CredentialInfo token=\"%s\">", p_res->CredentialInfo[i].token);
        offset += bdCredentialInfo_xml(p_buf+offset, mlen-offset, &p_res->CredentialInfo[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tcr:CredentialInfo>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetCredentialInfoResponse>");
    
    return offset;
}

int bdtcr_GetCredentialInfoList_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
    int offset = 0;
    tcr_GetCredentialInfoList_RES * p_res = (tcr_GetCredentialInfoList_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetCredentialInfoListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:NextStartReference>%s</tcr:NextStartReference>",
            p_res->NextStartReference);
    }
    
	for (i = 0; i < p_res->sizeCredentialInfo; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tcr:CredentialInfo token=\"%s\">", p_res->CredentialInfo[i].token);
        offset += bdCredentialInfo_xml(p_buf+offset, mlen-offset, &p_res->CredentialInfo[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tcr:CredentialInfo>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetCredentialInfoListResponse>");
    
    return offset;
}

int bdtcr_GetCredentials_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tcr_GetCredentials_RES * p_res = (tcr_GetCredentials_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetCredentialsResponse>");
    
	for (i = 0; i < p_res->sizeCredential; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tcr:Credential token=\"%s\">", p_res->Credential[i].token);
        offset += bdCredential_xml(p_buf+offset, mlen-offset, &p_res->Credential[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tcr:Credential>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetCredentialsResponse>");
    
    return offset;
}

int bdtcr_GetCredentialList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tcr_GetCredentialList_RES * p_res = (tcr_GetCredentialList_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetCredentialListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:NextStartReference>%s</tcr:NextStartReference>",
            p_res->NextStartReference);
    }
    
	for (i = 0; i < p_res->sizeCredential; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tcr:Credential token=\"%s\">", p_res->Credential[i].token);
        offset += bdCredential_xml(p_buf+offset, mlen-offset, &p_res->Credential[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tcr:Credential>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetCredentialListResponse>");
    
    return offset;
}

int bdtcr_CreateCredential_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tcr_CreateCredential_RES * p_res = (tcr_CreateCredential_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tcr:CreateCredentialResponse>"
	        "<tcr:Token>%s</tcr:Token>"
	    "</tcr:CreateCredentialResponse>",
	    p_res->Token);
	    
    return offset;
}

int bdtcr_ModifyCredential_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:ModifyCredentialResponse />");
    return offset;
}

int bdtcr_DeleteCredential_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:DeleteCredentialResponse />");
    return offset;
}

int bdtcr_GetCredentialState_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tcr_GetCredentialState_RES * p_res = (tcr_GetCredentialState_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetCredentialStateResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:State>");
    offset += bdCredentialState_xml(p_buf+offset, mlen-offset, &p_res->State);	
	offset += snprintf(p_buf+offset, mlen-offset, "</tcr:State>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetCredentialStateResponse>");
    
    return offset;
}

int bdtcr_EnableCredential_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:EnableCredentialResponse />");

    return offset;
}

int bdtcr_DisableCredential_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:DisableCredentialResponse />");
    return offset;
}

int bdtcr_SetCredential_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:SetCredentialResponse />");
    return offset;
}

int bdtcr_ResetAntipassbackViolation_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:ResetAntipassbackViolationResponse />");
    return offset;
}

int bdtcr_GetSupportedFormatTypes_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tcr_GetSupportedFormatTypes_RES * p_res = (tcr_GetSupportedFormatTypes_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetSupportedFormatTypesResponse>");

	for (i = 0; i < p_res->sizeFormatTypeInfo; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tcr:FormatTypeInfo>");
	    offset += bdCredentialIdentifierFormatTypeInfo_xml(p_buf+offset, mlen-offset, &p_res->FormatTypeInfo[i]);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:FormatTypeInfo>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetSupportedFormatTypesResponse>");
	    
    return offset;
}

int bdtcr_GetCredentialIdentifiers_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tcr_GetCredentialIdentifiers_RES * p_res = (tcr_GetCredentialIdentifiers_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetCredentialIdentifiersResponse>");

	for (i = 0; i < p_res->sizeCredentialIdentifier; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tcr:CredentialIdentifier>");
	    offset += bdCredentialIdentifier_xml(p_buf+offset, mlen-offset, &p_res->CredentialIdentifier[i]);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:CredentialIdentifier>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetCredentialIdentifiersResponse>");
    
    return offset;
}

int bdtcr_SetCredentialIdentifier_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:SetCredentialIdentifierResponse />");
    return offset;
}

int bdtcr_DeleteCredentialIdentifier_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:DeleteCredentialIdentifierResponse />");
    return offset;
}

int bdtcr_GetCredentialAccessProfiles_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tcr_GetCredentialAccessProfiles_RES * p_res = (tcr_GetCredentialAccessProfiles_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetCredentialAccessProfilesResponse>");

	for (i = 0; i < p_res->sizeCredentialAccessProfile; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tcr:CredentialAccessProfile>");
	    offset += bdCredentialAccessProfile_xml(p_buf+offset, mlen-offset, &p_res->CredentialAccessProfile[i]);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:CredentialAccessProfile>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetCredentialAccessProfilesResponse>");
	    
    return offset;
}

int bdtcr_SetCredentialAccessProfiles_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:SetCredentialAccessProfilesResponse />");
    return offset;
}

int bdtcr_DeleteCredentialAccessProfiles_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:DeleteCredentialAccessProfilesResponse />");
    return offset;
}

int bdtcr_GetWhitelist_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tcr_GetWhitelist_RES * p_res = (tcr_GetWhitelist_RES *)argv;
    CredentialIdentifierItemList * p_item = p_res->Identifier;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetWhitelistResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:NextStartReference>%s</tcr:NextStartReference>",
            p_res->NextStartReference);
    }
    
	while (p_item)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tcr:Identifier>");
	    offset += bdCredentialIdentifierItem_xml(p_buf+offset, mlen-offset, &p_item->Item);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:Identifier>");

	    p_item = p_item->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetWhitelistResponse>");
	    
    return offset;
}

int bdtcr_AddToWhitelist_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:AddToWhitelistResponse />");
    return offset;
}

int bdtcr_RemoveFromWhitelist_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:RemoveFromWhitelistResponse />");
    return offset;
}

int bdtcr_DeleteWhitelist_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:DeleteWhitelistResponse />");
    return offset;
}

int bdtcr_GetBlacklist_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tcr_GetBlacklist_RES * p_res = (tcr_GetBlacklist_RES *)argv;
    CredentialIdentifierItemList * p_item = p_res->Identifier;

	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:GetBlacklistResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tcr:NextStartReference>%s</tcr:NextStartReference>",
            p_res->NextStartReference);
    }
    
	while (p_item)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tcr:Identifier>");
	    offset += bdCredentialIdentifierItem_xml(p_buf+offset, mlen-offset, &p_item->Item);
	    offset += snprintf(p_buf+offset, mlen-offset, "</tcr:Identifier>");

	    p_item = p_item->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tcr:GetBlacklistResponse>");
	    
    return offset;
}

int bdtcr_AddToBlacklist_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:AddToBlacklistResponse />");
    return offset;
}

int bdtcr_RemoveFromBlacklist_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:RemoveFromBlacklistResponse />");
    return offset;
}

int bdtcr_DeleteBlacklist_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tcr:DeleteBlacklistResponse />");
    return offset;
}

#endif // end of CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES

int bdAccessProfileInfo_xml(char * p_buf, int mlen, onvif_AccessProfileInfo * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tar:Name>%s</tar:Name>", p_res->Name);

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tar:Description>%s</tar:Description>", 
            p_res->Description);
    }

    return offset;
}

int bdAccessPolicy_xml(char * p_buf, int mlen, onvif_AccessPolicy * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tar:ScheduleToken>%s</tar:ScheduleToken>"
        "<tar:Entity>%s</tar:Entity>", 
        p_res->ScheduleToken,
        p_res->Entity);

    if (p_res->EntityTypeFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tar:EntityType>%s</tar:EntityType>",
            p_res->EntityType);
    }

    return offset;
}

int bdAccessProfile_xml(char * p_buf, int mlen, onvif_AccessProfile * p_res)
{
    int i;
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tar:Name>%s</tar:Name>", p_res->Name);

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tar:Description>%s</tar:Description>", 
            p_res->Description);
    }

    for (i = 0; i < p_res->sizeAccessPolicy; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tar:AccessPolicy>");
        offset += bdAccessPolicy_xml(p_buf+offset, mlen-offset, &p_res->AccessPolicy[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tar:AccessPolicy>");
    }

    return offset;
}

int bdtar_GetAccessProfileInfo_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tar_GetAccessProfileInfo_RES * p_res = (tar_GetAccessProfileInfo_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tar:GetAccessProfileInfoResponse>");

	for (i = 0; i < p_res->sizeAccessProfileInfo; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tar:AccessProfileInfo token=\"%s\">", p_res->AccessProfileInfo[i].token);
        offset += bdAccessProfileInfo_xml(p_buf+offset, mlen-offset, &p_res->AccessProfileInfo[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tar:AccessProfileInfo>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tar:GetAccessProfileInfoResponse>");
    
    return offset;
}

int bdtar_GetAccessProfileInfoList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tar_GetAccessProfileInfoList_RES * p_res = (tar_GetAccessProfileInfoList_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tar:GetAccessProfileInfoListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tar:NextStartReference>%s</tar:NextStartReference>",
            p_res->NextStartReference);
    }
    
	for (i = 0; i < p_res->sizeAccessProfileInfo; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tar:AccessProfileInfo token=\"%s\">", p_res->AccessProfileInfo[i].token);
        offset += bdAccessProfileInfo_xml(p_buf+offset, mlen-offset, &p_res->AccessProfileInfo[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tar:AccessProfileInfo>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tar:GetAccessProfileInfoListResponse>");
    
    return offset;
}

int bdtar_GetAccessProfiles_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tar_GetAccessProfiles_RES * p_res = (tar_GetAccessProfiles_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tar:GetAccessProfilesResponse>");
    
	for (i = 0; i < p_res->sizeAccessProfile; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tar:AccessProfile token=\"%s\">", p_res->AccessProfile[i].token);
        offset += bdAccessProfile_xml(p_buf+offset, mlen-offset, &p_res->AccessProfile[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tar:AccessProfile>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tar:GetAccessProfilesResponse>");
    
    return offset;
}

int bdtar_GetAccessProfileList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tar_GetAccessProfileList_RES * p_res = (tar_GetAccessProfileList_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tar:GetAccessProfileListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tar:NextStartReference>%s</tar:NextStartReference>",
            p_res->NextStartReference);
    }
    
	for (i = 0; i < p_res->sizeAccessProfile; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tar:AccessProfile token=\"%s\">", p_res->AccessProfile[i].token);
        offset += bdAccessProfile_xml(p_buf+offset, mlen-offset, &p_res->AccessProfile[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tar:AccessProfile>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tar:GetAccessProfileListResponse>");
    
    return offset;
}

int bdtar_CreateAccessProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tar_CreateAccessProfile_RES * p_res = (tar_CreateAccessProfile_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tar:CreateAccessProfileResponse>"
	        "<tar:Token>%s</tar:Token>"
	    "</tar:CreateAccessProfileResponse>",
	    p_res->Token);
	    
    return offset;
}

int bdtar_ModifyAccessProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tar:ModifyAccessProfileResponse />");
    return offset;
}

int bdtar_DeleteAccessProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tar:DeleteAccessProfileResponse />");
    return offset;
}

int bdtar_SetAccessProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tar:SetAccessProfileResponse />");
    return offset;
}

#endif // end of ACCESS_RULES

#ifdef SCHEDULE_SUPPORT

int bdScheduleInfo_xml(char * p_buf, int mlen, onvif_ScheduleInfo * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Name>%s</tsc:Name>", p_res->Name);

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Description>%s</tsc:Description>", p_res->Description);
    }

    return offset;
}

int bdTimePeriod_xml(char * p_buf, int mlen, onvif_TimePeriod * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tsc:From>%s</tsc:From>", p_res->From);

    if (p_res->UntilFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Until>%s</tsc:Until>", p_res->Until);
    }
    
    return offset;
}

int bdSpecialDaysSchedule_xml(char * p_buf, int mlen, onvif_SpecialDaysSchedule * p_res)
{
    int i;
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GroupToken>%s</tsc:GroupToken>", p_res->GroupToken);

    for (i = 0; i < p_res->sizeTimeRange; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:TimeRange>");
        offset += bdTimePeriod_xml(p_buf+offset, mlen-offset, &p_res->TimeRange[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:TimeRange>");
    }
    
    return offset;
}

int bdSchedule_xml(char * p_buf, int mlen, onvif_Schedule * p_res)
{
    int i;
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Name>%s</tsc:Name>", p_res->Name);

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Description>%s</tsc:Description>", p_res->Description);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Standard>%s</tsc:Standard>", p_res->Standard);

    for (i = 0; i < p_res->sizeSpecialDays; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:SpecialDays>");
        offset += bdSpecialDaysSchedule_xml(p_buf+offset, mlen-offset, &p_res->SpecialDays[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:SpecialDays>");
    }
    
    return offset;
}

int bdSpecialDayGroupInfo_xml(char * p_buf, int mlen, onvif_SpecialDayGroupInfo * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Name>%s</tsc:Name>", p_res->Name);

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Description>%s</tsc:Description>", p_res->Description);
    }

    return offset;
}

int bdSpecialDayGroup_xml(char * p_buf, int mlen, onvif_SpecialDayGroup * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Name>%s</tsc:Name>", p_res->Name);

    if (p_res->DescriptionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Description>%s</tsc:Description>", p_res->Description);
    }

    if (p_res->DaysFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Days>%s</tsc:Days>", p_res->Days);
    }

    return offset;
}

int bdtsc_GetScheduleInfo_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tsc_GetScheduleInfo_RES * p_res = (tsc_GetScheduleInfo_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetScheduleInfoResponse>");

	for (i = 0; i < p_res->sizeScheduleInfo; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:ScheduleInfo token=\"%s\">", p_res->ScheduleInfo[i].token);
        offset += bdScheduleInfo_xml(p_buf+offset, mlen-offset, &p_res->ScheduleInfo[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:ScheduleInfo>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tsc:GetScheduleInfoResponse>");
    
    return offset;
}

int bdtsc_GetScheduleInfoList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tsc_GetScheduleInfoList_RES * p_res = (tsc_GetScheduleInfoList_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetScheduleInfoListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tsc:NextStartReference>%s</tsc:NextStartReference>",
            p_res->NextStartReference);
    }
    
	for (i = 0; i < p_res->sizeScheduleInfo; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:ScheduleInfo token=\"%s\">", p_res->ScheduleInfo[i].token);
        offset += bdScheduleInfo_xml(p_buf+offset, mlen-offset, &p_res->ScheduleInfo[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:ScheduleInfo>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tsc:GetScheduleInfoListResponse>");
    
    return offset;
}

int bdtsc_GetSchedules_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tsc_GetSchedules_RES * p_res = (tsc_GetSchedules_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetSchedulesResponse>");

	for (i = 0; i < p_res->sizeSchedule; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Schedule token=\"%s\">", p_res->Schedule[i].token);
        offset += bdSchedule_xml(p_buf+offset, mlen-offset, &p_res->Schedule[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:Schedule>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tsc:GetSchedulesResponse>");
    
    return offset;
}

int bdtsc_GetScheduleList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tsc_GetScheduleList_RES * p_res = (tsc_GetScheduleList_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetScheduleListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tsc:NextStartReference>%s</tsc:NextStartReference>",
            p_res->NextStartReference);
    }
    
	for (i = 0; i < p_res->sizeSchedule; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Schedule token=\"%s\">", p_res->Schedule[i].token);
        offset += bdSchedule_xml(p_buf+offset, mlen-offset, &p_res->Schedule[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:Schedule>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tsc:GetScheduleListResponse>");
    
    return offset;
}

int bdtsc_CreateSchedule_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tsc_CreateSchedule_RES * p_res = (tsc_CreateSchedule_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tsc:CreateScheduleResponse>"
	        "<tsc:Token>%s</tsc:Token>"
	    "</tsc:CreateScheduleResponse>",
	    p_res->Token);
    
    return offset;
}

int bdtsc_ModifySchedule_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:ModifyScheduleResponse />");
    return offset;
}

int bdtsc_DeleteSchedule_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:DeleteScheduleResponse />");
    return offset;
}

int bdtsc_GetSpecialDayGroupInfo_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tsc_GetSpecialDayGroupInfo_RES * p_res = (tsc_GetSpecialDayGroupInfo_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetSpecialDayGroupInfoResponse>");

	for (i = 0; i < p_res->sizeSpecialDayGroupInfo; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:SpecialDayGroupInfo token=\"%s\">", p_res->SpecialDayGroupInfo[i].token);
        offset += bdSpecialDayGroupInfo_xml(p_buf+offset, mlen-offset, &p_res->SpecialDayGroupInfo[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:SpecialDayGroupInfo>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tsc:GetSpecialDayGroupInfoResponse>");
    
    return offset;
}

int bdtsc_GetSpecialDayGroupInfoList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tsc_GetSpecialDayGroupInfoList_RES * p_res = (tsc_GetSpecialDayGroupInfoList_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetSpecialDayGroupInfoListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tsc:NextStartReference>%s</tsc:NextStartReference>",
            p_res->NextStartReference);
    }
    
	for (i = 0; i < p_res->sizeSpecialDayGroupInfo; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:SpecialDayGroupInfo token=\"%s\">", p_res->SpecialDayGroupInfo[i].token);
        offset += bdSpecialDayGroupInfo_xml(p_buf+offset, mlen-offset, &p_res->SpecialDayGroupInfo[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:SpecialDayGroupInfo>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tsc:GetSpecialDayGroupInfoListResponse>");
    
    return offset;
}

int bdtsc_GetSpecialDayGroups_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tsc_GetSpecialDayGroups_RES * p_res = (tsc_GetSpecialDayGroups_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetSpecialDayGroupsResponse>");

	for (i = 0; i < p_res->sizeSpecialDayGroup; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:SpecialDayGroup token=\"%s\">", p_res->SpecialDayGroup[i].token);
        offset += bdSpecialDayGroup_xml(p_buf+offset, mlen-offset, &p_res->SpecialDayGroup[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:SpecialDayGroup>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tsc:GetSpecialDayGroupsResponse>");
    
    return offset;
}

int bdtsc_GetSpecialDayGroupList_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int i;
    int offset = 0;
    tsc_GetSpecialDayGroupList_RES * p_res = (tsc_GetSpecialDayGroupList_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetSpecialDayGroupListResponse>");

    if (p_res->NextStartReferenceFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tsc:NextStartReference>%s</tsc:NextStartReference>",
            p_res->NextStartReference);
    }
    
	for (i = 0; i < p_res->sizeSpecialDayGroup; i++)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:SpecialDayGroup token=\"%s\">", p_res->SpecialDayGroup[i].token);
        offset += bdSpecialDayGroup_xml(p_buf+offset, mlen-offset, &p_res->SpecialDayGroup[i]);
        offset += snprintf(p_buf+offset, mlen-offset, "</tsc:SpecialDayGroup>");
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tsc:GetSpecialDayGroupListResponse>");
    
    return offset;
}

int bdtsc_CreateSpecialDayGroup_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tsc_CreateSpecialDayGroup_RES * p_res = (tsc_CreateSpecialDayGroup_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tsc:CreateSpecialDayGroupResponse>"
	        "<tsc:Token>%s</tsc:Token>"
	    "</tsc:CreateSpecialDayGroupResponse>",
	    p_res->Token);
    
    return offset;
}

int bdtsc_ModifySpecialDayGroup_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:ModifySpecialDayGroupResponse />");
    return offset;
}

int bdtsc_DeleteSpecialDayGroup_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:DeleteSpecialDayGroupResponse />");
    return offset;
}

int bdtsc_GetScheduleState_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tsc_GetScheduleState_RES * p_res = (tsc_GetScheduleState_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:GetScheduleStateResponse><tsc:ScheduleState>");
    offset += snprintf(p_buf+offset, mlen-offset, "<tsc:Active>%s</tsc:Active>", p_res->ScheduleState.Active ? "true" : "false");
    
    if (p_res->ScheduleState.SpecialDayFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tsc:SpecialDay>%s</tsc:SpecialDay>", 
            p_res->ScheduleState.SpecialDay ? "true" : "false");
    }
    
	offset += snprintf(p_buf+offset, mlen-offset, "</tsc:ScheduleState></tsc:GetScheduleStateResponse>");
	    
    return offset;
}

int bdtsc_SetSchedule_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:SetScheduleResponse />");
    return offset;
}

int bdtsc_SetSpecialDayGroup_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, "<tsc:SetSpecialDayGroupResponse />");
    return offset;
}

#endif // end of SCHEDULE_SUPPORT

#ifdef RECEIVER_SUPPORT

int bdStreamSetup_xml(char * p_buf, int mlen, onvif_StreamSetup * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Stream>%s</tt:Stream>"
		"<tt:Transport>"
			"<tt:Protocol>%s</tt:Protocol>"
		"</tt:Transport>",
		onvif_StreamTypeToString(p_res->Stream),
		onvif_TransportProtocolToString(p_res->Transport.Protocol));
		
    return offset;
}

int bdtrv_ReceiverConfiguration_xml(char * p_buf, int mlen, onvif_ReceiverConfiguration * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:Mode>%s</tt:Mode>"
        "<tt:MediaUri>%s</tt:MediaUri>", 
        onvif_ReceiverModeToString(p_res->Mode), 
        p_res->MediaUri);

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:StreamSetup>");
    offset += bdStreamSetup_xml(p_buf+offset, mlen-offset, &p_res->StreamSetup);
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:StreamSetup>");
    
    return offset;        
}

int bdtrv_Receiver_xml(char * p_buf, int mlen, onvif_Receiver * p_res)
{
    int offset = 0;

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Token>%s</tt:Token>", p_res->Token);

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Configuration>");
    offset += bdtrv_ReceiverConfiguration_xml(p_buf+offset, mlen-offset, &p_res->Configuration);
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Configuration>");

    return offset;
}

int bdtrv_GetReceivers_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    trv_GetReceivers_RES * p_res = (trv_GetReceivers_RES *)argv;
    ReceiverList * p_Receiver;

    p_Receiver = p_res->Receivers;
    
	offset += snprintf(p_buf+offset, mlen-offset, "<trv:GetReceiversResponse>");

    while (p_Receiver)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<trv:Receivers>");
        offset += bdtrv_Receiver_xml(p_buf+offset, mlen-offset, &p_Receiver->Receiver);
        offset += snprintf(p_buf+offset, mlen-offset, "</trv:Receivers>");
        
        p_Receiver = p_Receiver->next;
    }
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trv:GetReceiversResponse>");
	    
    return offset;
}

int bdtrv_GetReceiver_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    trv_GetReceiver_RES * p_res = (trv_GetReceiver_RES *)argv;

    offset += snprintf(p_buf+offset, mlen-offset, "<trv:GetReceiverResponse><trv:Receiver>");
    offset += bdtrv_Receiver_xml(p_buf+offset, mlen-offset, &p_res->Receiver);
    offset += snprintf(p_buf+offset, mlen-offset, "</trv:Receiver></trv:GetReceiverResponse>");

    return offset;
}

int bdtrv_CreateReceiver_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    trv_CreateReceiver_RES * p_res = (trv_CreateReceiver_RES *)argv;

    offset += snprintf(p_buf+offset, mlen-offset, "<trv:CreateReceiverResponse><trv:Receiver>");
    offset += bdtrv_Receiver_xml(p_buf+offset, mlen-offset, &p_res->Receiver);
    offset += snprintf(p_buf+offset, mlen-offset, "</trv:Receiver></trv:CreateReceiverResponse>");

    return offset;
}

int bdtrv_DeleteReceiver_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<trv:DeleteReceiverResponse />");
    return offset;
}

int bdtrv_ConfigureReceiver_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<trv:ConfigureReceiverResponse />");
    return offset;
}

int bdtrv_SetReceiverMode_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<trv:SetReceiverModeResponse />");
    return offset;
}

int bdtrv_GetReceiverState_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    trv_GetReceiverState_RES * p_res = (trv_GetReceiverState_RES *)argv;
    
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<trv:GetReceiverStateResponse>"
            "<trv:ReceiverState>"
                "<tt:State>%s</tt:State>"
                "<tt:AutoCreated>%s</tt:AutoCreated>"
            "</trv:ReceiverState>"
        "</trv:GetReceiverStateResponse>",
        onvif_ReceiverStateToString(p_res->ReceiverState.State),
        p_res->ReceiverState.AutoCreated ? "true" : "false");

    return offset;                
}

#endif // end of RECEIVER_SUPPORT

#ifdef PROVISIONING_SUPPORT

int bdtpv_PanMove_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:PanMoveResponse />");
    return offset;
}

int bdtpv_TiltMove_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:TiltMoveResponse />");
    return offset;
}

int bdtpv_ZoomMove_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:ZoomMoveResponse />");
    return offset;
}

int bdtpv_RollMove_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:RollMoveResponse />");
    return offset;
}

int bdtpv_FocusMove_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:FocusMoveResponse />");
    return offset;
}

int bdtpv_Stop_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:StopResponse />");
    return offset;
}

int bdtpv_GetUsage_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
    tpv_GetUsage_RES * p_res = (tpv_GetUsage_RES *)argv;
    
    offset += snprintf(p_buf+offset, mlen-offset, "<tpv:GetUsageResponse><tpv:Usage>");

    if (p_res->Usage.PanFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tpv:Pan>%d</tpv:Pan>", p_res->Usage.Pan);
    }

    if (p_res->Usage.TiltFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tpv:Tilt>%d</tpv:Tilt>", p_res->Usage.Tilt);
    }

    if (p_res->Usage.ZoomFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tpv:Zoom>%d</tpv:Zoom>", p_res->Usage.Zoom);
    }

    if (p_res->Usage.RollFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tpv:Roll>%d</tpv:Roll>", p_res->Usage.Roll);
    }

    if (p_res->Usage.FocusFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tpv:Focus>%d</tpv:Focus>", p_res->Usage.Focus);
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tpv:Usage></tpv:GetUsageResponse>");
    
    return offset;
}

#endif // end of PROVISIONING_SUPPORT




