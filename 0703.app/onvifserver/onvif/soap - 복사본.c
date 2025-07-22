

#include "sys_inc.h"
#include "hxml.h"
#include "xml_node.h"
#include "onvif.h"
#include "http.h"
#include "http_parse.h"
#include "soap.h"
#include "onvif_device.h"
#include "onvif_pkt.h"
#include "soap_parser.h"
#include "onvif_event.h"
#include "sha1.h"
#include "onvif_ptz.h"
#include "onvif_err.h"
#include "onvif_image.h"
#include "http_auth.h"
#include "base64.h"
#include "onvif_utils.h"
#include "onvif_probe.h"
#include "onvif_srv.h"

#ifdef MEDIA2_SUPPORT
#include "onvif_media2.h"
#endif

#ifdef PROFILE_G_SUPPORT
#include "onvif_recording.h"
#endif

#ifdef CREDENTIAL_SUPPORT
#include "onvif_credential.h"
#endif

#ifdef ACCESS_RULES
#include "onvif_accessrules.h"
#endif

#ifdef HTTPS
#include "openssl/ssl.h"
#endif

#include "cam_setup.h"

/***************************************************************************************/
HD_AUTH_INFO        g_onvif_auth;

extern ONVIF_CFG    g_onvif_cfg;

/***************************************************************************************/
char xml_hdr[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

char onvif_xmlns[] =
	"<s:Envelope "
    "xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
    "xmlns:e=\"http://www.w3.org/2003/05/soap-encoding\" "
    "xmlns:wsa=\"http://www.w3.org/2005/08/addressing\" "
    "xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
    "xmlns:wsaw=\"http://www.w3.org/2006/05/addressing/wsdl\" "
    "xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\" "
    "xmlns:wstop=\"http://docs.oasis-open.org/wsn/t-1\" "
    "xmlns:wsntw=\"http://docs.oasis-open.org/wsn/bw-2\" "
    "xmlns:wsrf-rw=\"http://docs.oasis-open.org/wsrf/rw-2\" "
    "xmlns:wsrf-r=\"http://docs.oasis-open.org/wsrf/r-2\" "
    "xmlns:wsrf-bf=\"http://docs.oasis-open.org/wsrf/bf-2\" "
    "xmlns:wsdl=\"http://schemas.xmlsoap.org/wsdl\" "
    "xmlns:wsoap12=\"http://schemas.xmlsoap.org/wsdl/soap12\" "
    "xmlns:http=\"http://schemas.xmlsoap.org/wsdl/http\" "
    "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
    "xmlns:wsadis=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
    "xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
    "xmlns:tns1=\"http://www.onvif.org/ver10/topics\" "
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" "
    "xmlns:tev=\"http://www.onvif.org/ver10/events/wsdl\" "
    "xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\" "
    "xmlns:tst=\"http://www.onvif.org/ver10/storage/wsdl\" "
    "xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" "
    "xmlns:pt=\"http://www.onvif.org/ver10/pacs\" "

#ifdef MEDIA2_SUPPORT
    "xmlns:tr2=\"http://www.onvif.org/ver20/media/wsdl\" "
#endif


#ifdef VIDEO_ANALYTICS
	"xmlns:tan=\"http://www.onvif.org/ver20/analytics/wsdl\" "
	"xmlns:axt=\"http://www.onvif.org/ver20/analytics\" "
#endif

#ifdef PROFILE_G_SUPPORT
    "xmlns:trp=\"http://www.onvif.org/ver10/replay/wsdl\" "
    "xmlns:tse=\"http://www.onvif.org/ver10/search/wsdl\" "
    "xmlns:trc=\"http://www.onvif.org/ver10/recording/wsdl\" "
#endif

#ifdef PROFILE_C_SUPPORT
    "xmlns:tac=\"http://www.onvif.org/ver10/accesscontrol/wsdl\" "
    "xmlns:tdc=\"http://www.onvif.org/ver10/doorcontrol/wsdl\" "
#endif

#ifdef DEVICEIO_SUPPORT
	"xmlns:tmd=\"http://www.onvif.org/ver10/deviceIO/wsdl\" "
#endif

#ifdef THERMAL_SUPPORT
    "xmlns:tth=\"http://www.onvif.org/ver10/thermal/wsdl\" "
#endif

#ifdef CREDENTIAL_SUPPORT
    "xmlns:tcr=\"http://www.onvif.org/ver10/credential/wsdl\" "
#endif

#ifdef ACCESS_RULES
    "xmlns:tar=\"http://www.onvif.org/ver10/accessrules/wsdl\" "
#endif

#ifdef SCHEDULE_SUPPORT
    "xmlns:tsc=\"http://www.onvif.org/ver10/schedule/wsdl\" "
#endif

#ifdef RECEIVER_SUPPORT
    "xmlns:trv=\"http://www.onvif.org/ver10/receiver/wsdl\" "
#endif

#ifdef PROVISIONING_SUPPORT
    "xmlns:tpv=\"http://www.onvif.org/ver10/provisioning/wsdl\" "
#endif

	"xmlns:ter=\"http://www.onvif.org/ver10/error\">";

char soap_head[] =
	"<s:Header>"
    	"<wsa:Action>%s</wsa:Action>"
	"</s:Header>";

char soap_body[] =
    "<s:Body>";

char soap_tailer[] =
    "</s:Body></s:Envelope>";


/***************************************************************************************/
int soap_http_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, const char * p_xml, int len)
{
    int tlen;
	char * p_bufs;

	p_bufs = (char *)malloc(len + 1024);
	if (NULL == p_bufs)
	{
		return -1;
	}

	tlen = sprintf(p_bufs,	"HTTP/1.1 200 OK\r\n"
							"Server: IP Camera onvif server %s\r\n"
							"Content-Type: %s\r\n"
							"Content-Length: %d\r\n"
							"Connection: close\r\n\r\n",
							ONVIF_VERSION_STRING,
							rx_msg ? http_get_headline(rx_msg, "Content-Type") : "application/soap+xml", len);

	if (p_xml && len > 0)
	{
		memcpy(p_bufs+tlen, p_xml, len);
		tlen += len;
	}

	p_bufs[tlen] = '\0';

	send(p_user->cfd, p_bufs, tlen, 0);

	free(p_bufs);

	return tlen;
}

int soap_http_err_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, int err_code, const char * err_str, const char * p_xml, int len)
{
    int tlen;
    int buflen = 16 * 1024;
	char * p_bufs;
	char auth[512] = {'\0'};

	p_bufs = (char *)malloc(buflen);
	if (NULL == p_bufs)
	{
		return -1;
	}


	tlen = snprintf(p_bufs,	buflen,
	            "HTTP/1.1 %d %s\r\n"
				"Server: IP Camera onvif server %s\r\n"
				"Content-Type: %s\r\n"
				"Content-Length: %d\r\n"
				"%s"
				"Connection: close\r\n\r\n",
				err_code, err_str, ONVIF_VERSION_STRING,
				rx_msg ? http_get_headline(rx_msg, "Content-Type") : "application/soap+xml", len, auth);

	if (p_xml && len > 0)
	{
		memcpy(p_bufs+tlen, p_xml, len);
		tlen += len;
	}

	p_bufs[tlen] = '\0';

	send(p_user->cfd, p_bufs, tlen, 0);

	free(p_bufs);

	return tlen;
}

int soap_err_rly
(
HTTPCLN * p_user,
HTTPMSG * rx_msg,
const char * code,
const char * subcode,
const char * subcode_ex,
const char * reason,
const char * action,
int http_err_code,
const char * http_err_str
)
{
    int ret = -1, mlen = 1024*16, xlen;
    char * p_xml;

	p_xml = (char *)malloc(mlen);
	if (NULL == p_xml)
	{
		goto soap_rly_err;
	}

	xlen = bderr_rly_xml(p_xml, mlen, code, subcode, subcode_ex, reason, action);
	if (xlen < 0 || xlen >= mlen)
	{
		goto soap_rly_err;
	}

	ret = soap_http_err_rly(p_user, rx_msg, http_err_code, http_err_str, p_xml, xlen);

soap_rly_err:

	if (p_xml)
	{
		free(p_xml);
	}

	return ret;
}

int soap_err_def_rly(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	
	return soap_err_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, NULL, "Action Not Implemented", NULL, 400, "Bad Request");
}

int soap_err_def2_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, const char * code, const char * subcode, const char * subcode_ex, const char * reason)
{
	
	return soap_err_rly(p_user, rx_msg, code, subcode, subcode_ex, reason, NULL, 400, "Bad Request");
}

int soap_err_def3_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, const char * code, const char * subcode, const char * subcode_ex, const char * reason, const char * action)
{
	
	return soap_err_rly(p_user, rx_msg, code, subcode, subcode_ex, reason, action, 400, "Bad Request");
}

int soap_security_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, int errcode)
{
    return soap_err_rly(p_user, rx_msg, ERR_SENDER, ERR_NOTAUTHORIZED, NULL, "Sender not Authorized", NULL, errcode, "Not Authorized");
}

int soap_bderr_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, ONVIF_RET err)
{
	int ret = 0;
	switch (err)
	{
	case ONVIF_ERR_InvalidIPv4Address:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidIPv4Address", "Invalid IPv4 Address");
		break;

	case ONVIF_ERR_InvalidIPv6Address:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidIPv6Address", "Invalid IPv6 Address");
		break;

	case ONVIF_ERR_InvalidDnsName:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter::InvalidDnsName", "Invalid DNS Name");
		break;

	case ONVIF_ERR_ServiceNotSupported:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ServiceNotSupported", "Service Not Supported");
		break;

	case ONVIF_ERR_PortAlreadyInUse:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:PortAlreadyInUse", "Port Already In Use");
		break;

	case ONVIF_ERR_InvalidGatewayAddress:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidGatewayAddress", "Invalid Gateway Address");
		break;

	case ONVIF_ERR_InvalidHostname:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidHostname", "Invalid Hostname");
		break;

	case ONVIF_ERR_MissingAttribute:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
		break;

	case ONVIF_ERR_InvalidDateTime:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidDateTime", "Invalid Datetime");
		break;

	case ONVIF_ERR_InvalidTimeZone:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidTimeZone", "Invalid Timezone");
		break;

	case ONVIF_ERR_ProfileExists:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ProfileExists", "Profile Exist");
		break;

	case ONVIF_ERR_MaxNVTProfiles:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxNVTProfiles", "Max Profiles");
		break;

	case ONVIF_ERR_NoProfile:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoProfile", "Profile Not Exist");
		break;

	case ONVIF_ERR_DeletionOfFixedProfile:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:DeletionOfFixedProfile", "Deleting Fixed Profile");
		break;

	case ONVIF_ERR_NoConfig:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoConfig", "Config Not Exist");
		break;

	case ONVIF_ERR_NoPTZProfile:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoPTZProfile", "PTZ Profile Not Exist");
		break;

	case ONVIF_ERR_NoHomePosition:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:NoHomePosition", "No Home Position");
		break;

	case ONVIF_ERR_NoToken:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:NoToken", "The requested token does not exist.");
		break;

	case ONVIF_ERR_PresetExist:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:PresetExist", "The requested name already exist for another preset.");
		break;

	case ONVIF_ERR_TooManyPresets:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:TooManyPresets", "Maximum number of Presets reached.");
		break;

	case ONVIF_ERR_MovingPTZ:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:MovingPTZ", "Preset cannot be set while PTZ unit is moving.");
		break;

	case ONVIF_ERR_NoEntity:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoEntity", "No such PTZ Node on the device");
		break;

    case ONVIF_ERR_InvalidNetworkInterface:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidNetworkInterface", "The supplied network interface token does not exist");
		break;

    case ONVIF_ERR_InvalidMtuValue:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidMtuValue", "The MTU value is invalid");
		break;

    case ONVIF_ERR_ConfigModify:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ConfigModify", "The configuration parameters are not possible to set");
		break;

	case ONVIF_ERR_ConfigurationConflict:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ConfigurationConflict", "The new settings conflicts with other uses of the configuration");
		break;

	case ONVIF_ERR_InvalidPosition:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidPosition", "Invalid Postion");
		break;

	case ONVIF_ERR_TooManyScopes:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:TooManyScopes", "The requested scope list exceeds the supported number of scopes");
		break;

	case ONVIF_ERR_FixedScope:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:FixedScope", "Trying to Remove fixed scope parameter, command rejected");
		break;

	case ONVIF_ERR_NoScope:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoScope", "Trying to Remove scope which does not exist");
		break;

	case ONVIF_ERR_ScopeOverwrite:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:ScopeOverwrite", "Scope Overwrite");
		break;

    case ONVIF_ERR_ResourceUnknownFault:
        ret = soap_err_def3_rly(p_user, rx_msg, ERR_RECEIVER, "wsrf-rw:ResourceUnknownFault", NULL, "ResourceUnknownFault", "http://www.w3.org/2005/08/addressing/soap/fault");
        break;

	case ONVIF_ERR_NoSource:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoSource", "The requested VideoSource does not exist");
		break;

	case ONVIF_ERR_CannotOverwriteHome:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:CannotOverwriteHome", "The home position is fixed and cannot be overwritten");
		break;

	case ONVIF_ERR_SettingsInvalid:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "The requested settings are incorrect");
		break;

	case ONVIF_ERR_NoImagingForSource:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NoImagingForSource", "The requested VideoSource does not support imaging settings");
		break;

	case ONVIF_ERR_UsernameClash:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:UsernameClash", "Username already exists");
		break;

	case ONVIF_ERR_PasswordTooLong:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:PasswordTooLong", "The password is too long");
		break;

	case ONVIF_ERR_UsernameTooLong:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:UsernameTooLong", "The username is too long");
		break;

	case ONVIF_ERR_Password:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:Password", "Too weak password");
		break;

	case ONVIF_ERR_TooManyUsers:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:TooManyUsers", "Maximum number of supported users exceeded");
		break;

	case ONVIF_ERR_AnonymousNotAllowed:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:AnonymousNotAllowed", "User level anonymous is not allowed");
		break;

	case ONVIF_ERR_UsernameTooShort:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:UsernameTooShort", "The username is too short");
		break;

	case ONVIF_ERR_UsernameMissing:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:UsernameMissing", "Username not recognized");
		break;

	case ONVIF_ERR_FixedUser:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:FixedUser", "Username may not be deleted");
		break;

	case ONVIF_ERR_MaxOSDs:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxOSDs", "The maximum number of supported OSDs has been reached");
		break;

	case ONVIF_ERR_InvalidStreamSetup:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidStreamSetup", "Specification of Stream Type or Transport part in StreamSetup is not supported");
		break;

	case ONVIF_ERR_BadConfiguration:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:BadConfiguration", "The configuration is invalid");
		break;

	case ONVIF_ERR_MaxRecordings:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxRecordings", "Max recordings");
		break;

	case ONVIF_ERR_NoRecording:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoRecording", "The RecordingToken does not reference an existing recording");
		break;

	case ONVIF_ERR_CannotDelete:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:CannotDelete", "Can not delete");
		break;

	case ONVIF_ERR_MaxTracks:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxTracks", "Max tracks");
		break;

	case ONVIF_ERR_NoTrack:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoTrack", "The TrackToken does not reference an existing track of the recording");
		break;

	case ONVIF_ERR_MaxRecordingJobs:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxRecordingJobs", "The maximum number of recording jobs that the device can handle has been reached");
		break;

	case ONVIF_ERR_MaxReceivers:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxReceivers", "Max receivers");
		break;

	case ONVIF_ERR_NoRecordingJob:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoRecordingJob", "The JobToken does not reference an existing job");
		break;

	case ONVIF_ERR_BadMode:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:BadMode", "The Mode is invalid");
		break;

	case ONVIF_ERR_InvalidToken:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidToken", "The Token is not valid");
		break;

    case ONVIF_ERR_InvalidRule:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidRule", "The suggested rules configuration is not valid on the device");
        break;

	case ONVIF_ERR_RuleAlreadyExistent:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:RuleAlreadyExistent", "The same rule name exists already in the configuration");
	    break;

	case ONVIF_ERR_TooManyRules:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:TooManyRules", "There is not enough space in the device to add the rules to the configuration");
	    break;

	case ONVIF_ERR_RuleNotExistent:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:RuleNotExistent", "The rule name or names do not exist");
	    break;

	case ONVIF_ERR_NameAlreadyExistent:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NameAlreadyExistent", "The same analytics module name exists already in the configuration");
	    break;

	case ONVIF_ERR_TooManyModules:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:TooManyModules", "There is not enough space in the device to add the analytics modules to the configuration");
	    break;

	case ONVIF_ERR_InvalidModule:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidModule", "The suggested module configuration is not valid on the device");
	    break;

	case ONVIF_ERR_NameNotExistent:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NameNotExistent", "The analytics module with the requested name does not exist");
        break;

	case ONVIF_ERR_InvalidFilterFault:
	    ret = soap_err_def3_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "wsntw:InvalidFilterFault", "InvalidFilterFault", "http://www.w3.org/2005/08/addressing/soap/fault");
	    break;

	case ONVIF_ERR_InvalidTopicExpressionFault:
	    ret = soap_err_def3_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "wsntw:InvalidTopicExpressionFault", "InvalidTopicExpressionFault", "http://www.w3.org/2005/08/addressing/soap/fault");
	    break;

	case ONVIF_ERR_TopicNotSupportedFault:
	    ret = soap_err_def3_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "wsntw:TopicNotSupportedFault", "TopicNotSupportedFault", "http://www.w3.org/2005/08/addressing/soap/fault");
	    break;

	case ONVIF_ERR_InvalidMessageContentExpressionFault:
	    ret = soap_err_def3_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "wsntw:InvalidMessageContentExpressionFault", "InvalidMessageContentExpressionFault", "http://www.w3.org/2005/08/addressing/soap/fault");
	    break;

    case ONVIF_ERR_InvalidStartReference:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter::InvalidStartReference", "StartReference is invalid");
        break;

    case ONVIF_ERR_TooManyItems:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter::TooManyItems", "Too many items were requested, see MaxLimit capability");
        break;

    case ONVIF_ERR_NotFound:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NotFound", "Not found");
        break;

    case ONVIF_ERR_NotSupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NotSupported", "The operation is not supported");
        break;

    case ONVIF_ERR_Failure:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:Failure", "Failed to go to Accessed state and unlock the door");
        break;

    case ONVIF_ERR_NoVideoOutput:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoVideoOutput", "The requested VideoOutput indicated with VideoOutputToken does not exist");
        break;

    case ONVIF_ERR_NoAudioOutput:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoAudioOutput", "The requested AudioOutput indicated with AudioOutputToken does not exist");
        break;

    case ONVIF_ERR_RelayToken:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:RelayToken", "Unknown relay token reference");
        break;

    case ONVIF_ERR_ModeError:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ModeError", "Monostable delay time not valid");
        break;

    case ONVIF_ERR_InvalidSerialPort:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidSerialPort", "The supplied serial port token does not exist");
        break;

    case ONVIF_ERR_DataLengthOver:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:DataLengthOver", "Number of available bytes exceeded");
        break;

    case ONVIF_ERR_DelimiterNotSupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:DelimiterNotSupported", "Sequence of character (delimiter) is not supported");
        break;

    case ONVIF_ERR_InvalidDot11:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:InvalidDot11", "IEEE 802.11 configuration is not supported");
        break;

    case ONVIF_ERR_NotDot11:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NotDot11", "The interface is not an IEEE 802.11 interface");
        break;

    case ONVIF_ERR_NotConnectedDot11:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:NotConnectedDot11", "IEEE 802.11 network is not connected");
        break;

    case ONVIF_ERR_NotScanAvailable:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NotScanAvailable", "ScanAvailableDot11Networks is not supported");
        break;

    case ONVIF_ERR_NotRemoteUser:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NotRemoteUser", "Remote User handling is not supported");
        break;

    case ONVIF_ERR_NoVideoSource:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoVideoSource", "The requested video source does not exist");
        break;

    case ONVIF_ERR_NoVideoSourceMode:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoVideoSourceMode", "The requested video source mode does not exist");
        break;

    case ONVIF_ERR_NoThermalForSource:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NoThermalForSource", "The requested VideoSource does not support thermal configuration");
        break;

    case ONVIF_ERR_NoRadiometryForSource:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NoRadiometryForSource", "The requested VideoSource does not support radiometry config settings");
        break;

    case ONVIF_ERR_InvalidConfiguration:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidConfiguration", "The requested configuration is incorrect");
        break;

    case ONVIF_ERR_MaxAccessProfilesPerCredential:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:MaxAccessProfilesPerCredential", "There are too many access profiles per credential");
        break;

    case ONVIF_ERR_CredentialValiditySupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:CredentialValiditySupported", "Credential validity is not supported by device");
        break;

    case ONVIF_ERR_CredentialAccessProfileValiditySupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:CredentialAccessProfileValiditySupported", "Credential access profile validity is not supported by the device");
        break;

    case ONVIF_ERR_SupportedIdentifierType:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:SupportedIdentifierType", "Specified identifier type is not supported by device");
        break;

    case ONVIF_ERR_DuplicatedIdentifierType:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:DuplicatedIdentifierType", "The same identifier type was used more than once");
        break;

    case ONVIF_ERR_InvalidFormatType:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidFormatType", "Specified identifier format type is not supported by the device");
        break;

    case ONVIF_ERR_InvalidIdentifierValue:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidIdentifierValue", "Specified identifier value is not as per FormatType definition");
        break;

    case ONVIF_ERR_DuplicatedIdentifierValue:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:DuplicatedIdentifierValue", "The same combination of identifier type, format and value was used more than once");
        break;

    case ONVIF_ERR_ReferenceNotFound:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ReferenceNotFound", "A referred entity token is not found");
        break;

    case ONVIF_ERR_ExemptFromAuthenticationSupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ExemptFromAuthenticationSupported", "Exempt from authentication is not supported by the device");
        break;

    case ONVIF_ERR_MaxCredentials:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_CAPABILITYVIOLATED, "ter:MaxCredentials", "There is not enough space to create a new credential");
        break;

    case ONVIF_ERR_ReferenceInUse:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ReferenceInUse", "Failed to delete, credential token is in use");
        break;

    case ONVIF_ERR_MinIdentifiersPerCredential:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_CONSTRAINTVIOLATED, "ter:MinIdentifiersPerCredential", "At least one credential identifier is required");
        break;

    case ONVIF_ERR_InvalidArgs:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGS, NULL, "InvalidArgs");
        break;

    case ONVIF_ERR_MaxAccessProfiles:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_CAPABILITYVIOLATED, "ter:MaxAccessProfiles", "There is not enough space to add new AccessProfile, see the MaxAccessProfiles capability");
        break;

    case ONVIF_ERR_MaxAccessPoliciesPerAccessProfile:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:MaxAccessPoliciesPerAccessProfile", "There are too many AccessPolicies in anAccessProfile, see MaxAccessPoliciesPerAccessProfile capability");
        break;

    case ONVIF_ERR_MultipleSchedulesPerAccessPointSupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:MultipleSchedulesPerAccessPointSupported", "Multiple AccessPoints are not supported for the same schedule, see MultipleSchedulesPerAccessPointSupported capability");
        break;

    case ONVIF_ERR_InvalidArgVal:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, NULL, "InvalidArgVal");
        break;

    case ONVIF_ERR_MaxSchedules:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_CAPABILITYVIOLATED, "ter:MaxSchedules", "There is not enough space to add new schedule, see MaxSchedules capability");
        break;

    case ONVIF_ERR_MaxSpecialDaysSchedules:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:MaxSpecialDaysSchedule", "There are too many SpecialDaysSchedule entities referred in this schedule, see MaxSpecialDaysSchedules capability");
        break;

    case ONVIF_ERR_MaxTimePeriodsPerDay:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:MaxTimePeriodsPerDay", "There are too many time periods in a day schedule, see MaxTimePeriodsPerDay capability");
        break;

    case ONVIF_ERR_MaxSpecialDayGroups:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_CAPABILITYVIOLATED, "ter:MaxSpecialDayGroups", "There is not enough space to add new SpecialDayGroup items, see the MaxSpecialDayGroups capabilit");
        break;

    case ONVIF_ERR_MaxDaysInSpecialDayGroup:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_CAPABILITYVIOLATED, "ter:MaxDaysInSpecialDayGroup", "There are too many special days in a SpecialDayGroup, see MaxDaysInSpecialDayGroup capability");
        break;

    case ONVIF_ERR_UnknownToken:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:UnknownToken", "The receiver indicated by ReceiverToken does not exist");
        break;

    case ONVIF_ERR_CannotDeleteReceiver:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:CannotDeleteReceiver", "It is not possible to delete the specified receiver");
        break;

    case ONVIF_ERR_MaxMasks:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxMasks", "The maximum number of supported masks by the specific VideoSourceConfiguration has been reached");
        break;

    case ONVIF_ERR_IPFilterListIsFull:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:IPFilterListIsFull", "It is not possible to add more IP filters since the IP filter list is full");
        break;

    case ONVIF_ERR_NoIPv4Address:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoIPv4Address", "The IPv4 address to be removed does not exist. ");
        break;

    case ONVIF_ERR_NoIPv6Address:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoIPv6Address", "Cannot set position automatically");
        break;

    case ONVIF_ERR_NoProvisioning:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTIONNOTSUPPORTED, "ter:NoProvisioning", "Provisioning is not supported for this operation on the given video source");
        break;

    case ONVIF_ERR_NoAutoMode:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTIONNOTSUPPORTED, "ter:NoAutoMode", "Cannot set position automatically");
        break;

    case ONVIF_ERR_TooManyPresetTours:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:TooManyPresetTours", "There is not enough space in the device to create the new preset tour to the profile");
        break;

    case ONVIF_ERR_InvalidPresetTour:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidPresetTour", "The requested PresetTour includes invalid parameter(s)");
        break;

    case ONVIF_ERR_SpaceNotSupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SpaceNotSupported", "A space is referenced in an argument which is not supported by the PTZ Node");
        break;

    case ONVIF_ERR_ActivationFailed:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_INVALIDARGVAL, "ter:ActivationFailed", "The requested preset tour cannot be activated while PTZ unit is moving or another preset tour is now activated");
        break;

    case ONVIF_ERR_MaxDoors:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_CAPABILITYVIOLATED, "ter:MaxDoors", "There is not enough space to add the new door");
        break;

    case ONVIF_ERR_ClientSuppliedTokenSupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_CAPABILITYVIOLATED, "ter:ClientSuppliedTokenSupported", "The device does not support that the client supplies the token");
        break;

    case ONVIF_ERR_GeoMoveNotSupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:GeoMoveNotSupported", "The device does not support geo move");
        break;

    case ONVIF_ERR_UnreachablePosition:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:UnreachablePosition", "The requested translation is out of bounds");
        break;

    case ONVIF_ERR_TimeoutNotSupported:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:TimeoutNotSupported", "The specified timeout argument is not within the supported timeout range");
        break;

    case ONVIF_ERR_GeoLocationUnknown:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:GeoLocationUnknown", "The unit is not able to perform GeoMove because its geolocation is not configured or available");
        break;

    case ONVIF_ERR_InvalidSpeed:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidSpeed", "The requested speed is out of bounds");
        break;

    case ONVIF_ERR_InvalidTranslation:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidTranslation", "The requested translation is out of bounds");
        break;

    case ONVIF_ERR_InvalidVelocity:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidVelocity", "The requested speed is out of bounds");
        break;

    case ONVIF_ERR_NoStatus:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:NoStatus", "Status is unavailable in the requested Media Profile");
        break;

    case ONVIF_ERR_AccesslogUnavailable:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGS, "ter:AccesslogUnavailable", "There is no access log information available");
        break;

    case ONVIF_ERR_SystemlogUnavailable:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGS, "ter:SystemlogUnavailable", "There is no system log information available");
        break;

    case ONVIF_ERR_NtpServerUndefined:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NtpServerUndefined", "Cannot switch DateTimeType to NTP because no NTP server is defined");
        break;

    case ONVIF_ERR_InvalidInterfaceSpeed:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidInterfaceSpeed", "The suggested speed is not supported");
        break;

    case ONVIF_ERR_InvalidInterfaceType:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidInterfaceType", "The suggested network interface type is not supported");
        break;

    case ONVIF_ERR_StreamConflict:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:StreamConflict", "Specification of StreamType or Transport part in causes conflict with other streams");
        break;

    case ONVIF_ERR_IncompleteConfiguration:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:IncompleteConfiguration", "The specified media profile does not have the minimum amount of configurations to have streams");
        break;

    case ONVIF_ERR_InvalidMulticastSettings:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidMulticastSettings", "No configuration is configured for multicast");
        break;

    case ONVIF_ERR_InvalidPolygon:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_INVALIDARGVAL, "ter:InvalidPolygon", "The provided polygon is not supported");
        break;

    case ONVIF_ERR_MaxAccessPoints:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_CAPABILITYVIOLATED, "ter:MaxAccessPoints", "There is not enough space to add new AccessPoint");
        break;

    case ONVIF_ERR_MaxAreas:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_CAPABILITYVIOLATED, "ter:MaxAreas", "There is not enough space to add new Area");
        break;

	default:
		

		
		ret = soap_err_def_rly(p_user, rx_msg);
		break;
	}

	return ret;
}

int soap_bdheader(char * p_xml, int mlen, const char * action, XMLN * p_header)
{
    int offset = 0;

    offset += snprintf(p_xml, mlen, "%s", xml_hdr);
	offset += snprintf(p_xml+offset, mlen-offset, "%s", onvif_xmlns);

	if (p_header)
	{
	    XMLN * p_MessageID;
	    XMLN * p_ReplyTo;

        offset += snprintf(p_xml+offset, mlen-offset, "<s:Header>");

	    p_MessageID = _node_soap_get(p_header, "MessageID");
	    if (p_MessageID && p_MessageID->data)
	    {
	        offset += snprintf(p_xml+offset, mlen-offset,
	            "<wsa:MessageID>%s</wsa:MessageID>",
	            p_MessageID->data);
	    }

	    p_ReplyTo = _node_soap_get(p_header, "ReplyTo");
	    if (p_ReplyTo)
	    {
	        XMLN * p_Address;

	        p_Address = _node_soap_get(p_ReplyTo, "Address");
	        if (p_Address && p_Address->data)
	        {
	            offset += snprintf(p_xml+offset, mlen-offset,
    	            "<wsa:To>%s</wsa:To>",
    	            p_Address->data);
	        }
	    }

	    if (action)
    	{
    	    offset += snprintf(p_xml+offset, mlen-offset, "<wsa:Action>%s</wsa:Action>", action);
    	}

	    offset += snprintf(p_xml+offset, mlen-offset, "</s:Header>");
	}
	else if (action)
	{
	    offset += snprintf(p_xml+offset, mlen-offset, soap_head, action);
	}

	return offset;
}

ONVIF_RET _bdsend_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, ap_bd bdxml, const char * argv, const char * action, XMLN * p_header)
{
    int offset = 0;
	int ret = -1, mlen = 1024*80, xlen;


	char * p_xml = (char *)malloc(mlen);
	if (NULL == p_xml)
	{
		return (ONVIF_RET)-1;
	}

    offset += soap_bdheader(p_xml, mlen, action, p_header);

	offset += snprintf(p_xml+offset, mlen-offset, "%s", soap_body);

	xlen = bdxml(p_xml+offset, mlen-offset, argv);
	if (xlen < 0)
	{
		ret = soap_bderr_rly(p_user, rx_msg, (ONVIF_RET)xlen);
	}
    else
    {
        offset += xlen;
	    offset += snprintf(p_xml+offset, mlen-offset, "%s", soap_tailer);

		


	    ret = soap_http_rly(p_user, rx_msg, p_xml, offset);
	}


	free(p_xml);

	return (ONVIF_RET)ret;
}

/***************************************************************************************/

int soap_tds_GetDeviceInformation(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtds_GetDeviceInformation_rly_xml, NULL, NULL, p_header);
}

int soap_tds_GetSystemUris(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    tds_GetSystemUris_RES res;

    

    memset(&res, 0, sizeof(res));

    ret = onvif_tds_GetSystemUris(p_user->lip, p_user->lport, &res);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtds_GetSystemUris_rly_xml, (char*)&res, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetCapabilities;
    tds_GetCapabilities_REQ req;

    

    p_GetCapabilities = _node_soap_get(p_body, "GetCapabilities");
    assert(p_GetCapabilities);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_GetCapabilities(p_GetCapabilities, &req);
    if (ONVIF_OK == ret)
    {
	    if (CapabilityCategory_Invalid       == req.Category
			|| (CapabilityCategory_Events    == req.Category && g_onvif_cfg.Capabilities.events.support == 0)
            || (CapabilityCategory_PTZ       == req.Category)
#ifdef MEDIA_SUPPORT
	    	|| (CapabilityCategory_Media     == req.Category && g_onvif_cfg.Capabilities.media.support == 0)
#else
			|| (CapabilityCategory_Media     == req.Category)
#endif
#ifdef IMAGE_SUPPORT
	    	|| (CapabilityCategory_Imaging   == req.Category && g_onvif_cfg.Capabilities.image.support == 0)
#else
			|| (CapabilityCategory_Imaging   == req.Category)
#endif
#ifdef VIDEO_ANALYTICS
			|| (CapabilityCategory_Analytics == req.Category && g_onvif_cfg.Capabilities.analytics.support == 0)
#else
            || (CapabilityCategory_Analytics == req.Category)
#endif
	    	)
	    {
	    	return soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NoSuchService", "No Such Service");
	    }
	    else
	    {
	        return _bdsend_rly(p_user, rx_msg, bdtds_GetCapabilities_rly_xml, (char *)&req, NULL, p_header);
	    }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetSystemDateAndTime(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtds_GetSystemDateAndTime_rly_xml, NULL, NULL, p_header);
}

int soap_tds_GetNetworkInterfaces(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtds_GetNetworkInterfaces_rly_xml, NULL, NULL, p_header);
}

int soap_tds_SetNetworkInterfaces(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_SetNetworkInterfaces;
    tds_SetNetworkInterfaces_REQ req;

	

	p_SetNetworkInterfaces = _node_soap_get(p_body, "SetNetworkInterfaces");
    assert(p_SetNetworkInterfaces);

	memset(&req, 0, sizeof(req));

	ret = parse_tds_SetNetworkInterfaces(p_SetNetworkInterfaces, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tds_SetNetworkInterfaces(&req);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtds_SetNetworkInterfaces_rly_xml, NULL, NULL, p_header);
		}
	}

    if (ret > 0)
    {
	    // todo : send onvif hello message
	    sleep(3);
        onvif_hello();
        return ret;
    }

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_SystemReboot(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;

    

	ret = _bdsend_rly(p_user, rx_msg, bdtds_SystemReboot_rly_xml, NULL, NULL, p_header);

	onvif_tds_SystemReboot();

    return ret;
}

int soap_tds_SetSystemFactoryDefault(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetSystemFactoryDefault;
	tds_SetSystemFactoryDefault_REQ req;

	

    p_SetSystemFactoryDefault = _node_soap_get(p_body, "SetSystemFactoryDefault");
    assert(p_SetSystemFactoryDefault);

    memset(&req, 0, sizeof(req));

	ret = parse_tds_SetSystemFactoryDefault(p_SetSystemFactoryDefault, &req);
	if (ONVIF_OK == ret)
	{
	    ret = _bdsend_rly(p_user, rx_msg, bdtds_SetSystemFactoryDefault_rly_xml, NULL, NULL, p_header);
	}
	
	onvif_tds_SetSystemFactoryDefault(&req);

    return ret;
}

int soap_tds_SetSystemDateAndTime(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetSystemDateAndTime;
	tds_SetSystemDateAndTime_REQ req;
	manager_tz_dt dt;

    
	



    p_SetSystemDateAndTime = _node_soap_get(p_body, "SetSystemDateAndTime");
    assert(p_SetSystemDateAndTime);
	
	memset(&req, 0, sizeof(tds_SetSystemDateAndTime_REQ));
	
	ret = parse_tds_SetSystemDateAndTime(p_SetSystemDateAndTime, &req, &dt);
    if (ONVIF_OK == ret)
    {
		
    	ret = onvif_tds_SetSystemDateAndTime(&req);
		
        if (ONVIF_OK == ret)
        {
			
        	return _bdsend_rly(p_user, rx_msg, bdtds_SetSystemDateAndTime_rly_xml, NULL, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetServices(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetServices;
	tds_GetServices_REQ req;

    

    p_GetServices = _node_soap_get(p_body, "GetServices");
    assert(p_GetServices);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_GetServices(p_GetServices, &req);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtds_GetServices_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetScopes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

    return _bdsend_rly(p_user, rx_msg, bdtds_GetScopes_rly_xml, NULL, NULL, p_header);
}

int soap_tds_AddScopes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_AddScopes;
	tds_AddScopes_REQ req;

    

    p_AddScopes = _node_soap_get(p_body, "AddScopes");
    assert(p_AddScopes);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_AddScopes(p_AddScopes, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_tds_AddScopes(&req);
        if (ONVIF_OK == ret)
        {
    		ret = _bdsend_rly(p_user, rx_msg, bdtds_AddScopes_rly_xml, NULL, NULL, p_header);

    		onvif_hello();
    		return ret;
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_SetScopes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetScopes;
	tds_SetScopes_REQ req;

	

    p_SetScopes = _node_soap_get(p_body, "SetScopes");
    assert(p_SetScopes);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_SetScopes(p_SetScopes, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_tds_SetScopes(&req);
        if (ONVIF_OK == ret)
        {
    		ret = _bdsend_rly(p_user, rx_msg, bdtds_SetScopes_rly_xml, NULL, NULL, p_header);

    		onvif_hello();
    		return ret;
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_RemoveScopes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_RemoveScopes;
	tds_RemoveScopes_REQ req;

	

    p_RemoveScopes = _node_soap_get(p_body, "RemoveScopes");
    assert(p_RemoveScopes);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_RemoveScopes(p_RemoveScopes, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_tds_RemoveScopes(&req);
        if (ONVIF_OK == ret)
        {
    		ret = _bdsend_rly(p_user, rx_msg, bdtds_RemoveScopes_rly_xml, (char *)&req, NULL, p_header);

    		onvif_hello();
    		return ret;
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetHostname(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

    return _bdsend_rly(p_user, rx_msg, bdtds_GetHostname_rly_xml, NULL, NULL, p_header);
}

int soap_tds_SetHostname(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetHostname;
	tds_SetHostname_REQ req;

    

    p_SetHostname = _node_soap_get(p_body, "SetHostname");
    assert(p_SetHostname);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_SetHostname(p_SetHostname, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_tds_SetHostname(&req);
        if (ONVIF_OK == ret)
        {
            return _bdsend_rly(p_user, rx_msg, bdtds_SetHostname_rly_xml, NULL, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_SetHostnameFromDHCP(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetHostnameFromDHCP;
	tds_SetHostnameFromDHCP_REQ req;

    

    p_SetHostnameFromDHCP = _node_soap_get(p_body, "SetHostnameFromDHCP");
    assert(p_SetHostnameFromDHCP);

    ret = parse_tds_SetHostnameFromDHCP(p_SetHostnameFromDHCP, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_tds_SetHostnameFromDHCP(&req);
        if (ONVIF_OK == ret)
        {
            return _bdsend_rly(p_user, rx_msg, bdtds_SetHostnameFromDHCP_rly_xml, NULL, NULL, p_header);
        }
    }

	
	
    return soap_err_def_rly(p_user, rx_msg);
}


int soap_tds_GetNetworkProtocols(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

    return _bdsend_rly(p_user, rx_msg, bdtds_GetNetworkProtocols_rly_xml, NULL, NULL, p_header);
}

int soap_tds_SetNetworkProtocols(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetNetworkProtocols;
	tds_SetNetworkProtocols_REQ req;

    

    p_SetNetworkProtocols = _node_soap_get(p_body, "SetNetworkProtocols");
    assert(p_SetNetworkProtocols);

	memset(&req, 0, sizeof(req));

	ret = parse_tds_SetNetworkProtocols(p_SetNetworkProtocols, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_tds_SetNetworkProtocols(&req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtds_SetNetworkProtocols_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetNetworkDefaultGateway(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

    return _bdsend_rly(p_user, rx_msg, bdtds_GetNetworkDefaultGateway_rly_xml, NULL, NULL, p_header);
}

int soap_tds_SetNetworkDefaultGateway(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetNetworkDefaultGateway;
	tds_SetNetworkDefaultGateway_REQ req;

    

	p_SetNetworkDefaultGateway = _node_soap_get(p_body, "SetNetworkDefaultGateway");
    assert(p_SetNetworkDefaultGateway);

	memset(&req, 0, sizeof(tds_SetNetworkDefaultGateway_REQ));

	ret = parse_tds_SetNetworkDefaultGateway(p_SetNetworkDefaultGateway, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_tds_SetNetworkDefaultGateway(&req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtds_SetNetworkDefaultGateway_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetDiscoveryMode(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

    return _bdsend_rly(p_user, rx_msg, bdtds_GetDiscoveryMode_rly_xml, NULL, NULL, p_header);
}

int soap_tds_SetDiscoveryMode(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetDiscoveryMode;
	tds_SetDiscoveryMode_REQ req;

    

    p_SetDiscoveryMode = _node_soap_get(p_body, "SetDiscoveryMode");
    assert(p_SetDiscoveryMode);

	memset(&req, 0, sizeof(tds_SetDiscoveryMode_REQ));

	ret = parse_tds_SetDiscoveryMode(p_SetDiscoveryMode, &req);

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetDNS(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	
	return _bdsend_rly(p_user, rx_msg, bdtds_GetDNS_rly_xml, NULL, NULL, p_header);
}

int soap_tds_SetDNS(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	
	return 0;
}

int soap_tds_GetDynamicDNS(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    
    return 0;
}

int soap_tds_SetDynamicDNS(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	
	return 0;
}

int soap_tds_GetNTP(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

    return _bdsend_rly(p_user, rx_msg, bdtds_GetNTP_rly_xml, NULL, NULL, p_header);
}

int soap_tds_SetNTP(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetNTP;
	tds_SetNTP_REQ req;

	

	p_SetNTP = _node_soap_get(p_body, "SetNTP");
    assert(p_SetNTP);

    memset(&req, 0, sizeof(tds_SetNTP_REQ));

	ret = parse_tds_SetNTP(p_SetNTP, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_tds_SetNTP(&req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtds_SetNTP_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetZeroConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

    return _bdsend_rly(p_user, rx_msg, bdtds_GetZeroConfiguration_rly_xml, NULL, NULL, p_header);
}

int soap_tds_SetZeroConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetZeroConfiguration;
	tds_SetZeroConfiguration_REQ req;

	

	p_SetZeroConfiguration = _node_soap_get(p_body, "SetZeroConfiguration");
    assert(p_SetZeroConfiguration);

    memset(&req, 0, sizeof(tds_SetZeroConfiguration_REQ));

	ret = parse_tds_SetZeroConfiguration(p_SetZeroConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_tds_SetZeroConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtds_SetZeroConfiguration_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetDot11Capabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

    return _bdsend_rly(p_user, rx_msg, bdtds_GetDot11Capabilities_rly_xml, NULL, NULL, p_header);
}

int soap_tds_GetDot11Status(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetDot11Status;
	tds_GetDot11Status_REQ req;

	

	p_GetDot11Status = _node_soap_get(p_body, "GetDot11Status");
    assert(p_GetDot11Status);

    memset(&req, 0, sizeof(req));

	ret = parse_tds_GetDot11Status(p_GetDot11Status, &req);
    if (ONVIF_OK == ret)
    {
        tds_GetDot11Status_RES res;
        memset(&res, 0, sizeof(res));

    	ret = onvif_tds_GetDot11Status(&req, &res);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtds_GetDot11Status_rly_xml, (char *)&res, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_ScanAvailableDot11Networks(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_ScanAvailableDot11Networks;
	tds_ScanAvailableDot11Networks_REQ req;

	

	p_ScanAvailableDot11Networks = _node_soap_get(p_body, "ScanAvailableDot11Networks");
    assert(p_ScanAvailableDot11Networks);

    memset(&req, 0, sizeof(req));

	ret = parse_tds_ScanAvailableDot11Networks(p_ScanAvailableDot11Networks, &req);
    if (ONVIF_OK == ret)
    {
        tds_ScanAvailableDot11Networks_RES res;
        memset(&res, 0, sizeof(res));

    	ret = onvif_tds_ScanAvailableDot11Networks(&req, &res);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtds_ScanAvailableDot11Networks_rly_xml, (char *)&res, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Device;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tds_GetWsdlUrl(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtds_GetWsdlUrl_rly_xml, NULL, NULL, p_header);
}

int soap_tds_GetEndpointReference(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

	return _bdsend_rly(p_user, rx_msg, bdtds_GetEndpointReference_rly_xml, NULL, NULL, p_header);
}

int soap_tds_GetUsers(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtds_GetUsers_rly_xml, NULL, NULL, p_header);
}

int soap_tds_CreateUsers(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_CreateUsers;
	tds_CreateUsers_REQ req;

	

	p_CreateUsers = _node_soap_get(p_body, "CreateUsers");
	assert(p_CreateUsers);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_CreateUsers(p_CreateUsers, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_tds_CreateUsers(&req);
        if (ONVIF_OK == ret)
        {
    		return _bdsend_rly(p_user, rx_msg, bdtds_CreateUsers_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_DeleteUsers(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_DeleteUsers;
	tds_DeleteUsers_REQ req;

	

	p_DeleteUsers = _node_soap_get(p_body, "DeleteUsers");
	assert(p_DeleteUsers);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_DeleteUsers(p_DeleteUsers, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_tds_DeleteUsers(&req);
        if (ONVIF_OK == ret)
        {
    		return _bdsend_rly(p_user, rx_msg, bdtds_DeleteUsers_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_SetUser(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetUser;
	tds_SetUser_REQ req;

	

	p_SetUser = _node_soap_get(p_body, "SetUser");
	assert(p_SetUser);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_SetUser(p_SetUser, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_tds_SetUser(&req);
        if (ONVIF_OK == ret)
        {
    		return _bdsend_rly(p_user, rx_msg, bdtds_SetUser_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_GetRemoteUser(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    tds_GetRemoteUser_RES res;

	

    memset(&res, 0, sizeof(res));

	ret = onvif_tds_GetRemoteUser(&res);
    if (ONVIF_OK == ret)
    {
		return _bdsend_rly(p_user, rx_msg, bdtds_GetRemoteUser_rly_xml, (char *)&res, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_SetRemoteUser(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetRemoteUser;
	tds_SetRemoteUser_REQ req;

	

	p_SetRemoteUser = _node_soap_get(p_body, "SetRemoteUser");
	assert(p_SetRemoteUser);

    memset(&req, 0, sizeof(req));

    ret = parse_tds_SetRemoteUser(p_SetRemoteUser, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_tds_SetRemoteUser(&req);
        if (ONVIF_OK == ret)
        {
    		return _bdsend_rly(p_user, rx_msg, bdtds_SetRemoteUser_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_StartSystemRestore(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    tds_StartSystemRestore_RES res;

	

	memset(&res, 0, sizeof(res));

	if (onvif_tds_StartSystemRestore(p_user->lip, p_user->lport, &res))
	{
		return _bdsend_rly(p_user, rx_msg, bdtds_StartSystemRestore_rly_xml, (char *)&res, NULL, p_header);
	}

	
	return soap_err_def_rly(p_user, rx_msg);
}

#ifdef DEVICEIO_SUPPORT

int soap_tds_SetRelayOutputSettings(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_SetRelayOutputSettings;
	tmd_SetRelayOutputSettings_REQ req;

	

	p_SetRelayOutputSettings = _node_soap_get(p_body, "SetRelayOutputSettings");
	assert(p_SetRelayOutputSettings);

	memset(&req, 0, sizeof(req));

    ret = parse_tmd_SetRelayOutputSettings(p_SetRelayOutputSettings, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tmd_SetRelayOutputSettings(&req);
	    if (ONVIF_OK == ret)
	    {
            return _bdsend_rly(p_user, rx_msg, bdtds_SetRelayOutputSettings_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tds_SetRelayOutputState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_SetRelayOutputState;
	tmd_SetRelayOutputState_REQ req;

	

	p_SetRelayOutputState = _node_soap_get(p_body, "SetRelayOutputState");
	assert(p_SetRelayOutputState);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_SetRelayOutputState(p_SetRelayOutputState, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tmd_SetRelayOutputState(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtds_SetRelayOutputState_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // DEVICEIO_SUPPORT


void SystemRestore(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    char * p_buff = http_get_ctt(rx_msg);

	if (onvif_tds_SystemRestoreCheck(p_buff, rx_msg->ctt_len))
	{
		if (onvif_tds_SystemRestore(p_buff, rx_msg->ctt_len))
		{
			soap_http_rly(p_user, rx_msg, NULL, 0);

			onvif_tds_SystemRestorePost();
		}
		else
		{
			soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
		}
	}
	else
	{
		soap_http_err_rly(p_user, rx_msg, 415, "Unsupported Media Type", NULL, 0);
	}
}

void GetSnapshot(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
	return;
}

void GetHttpSystemLog(HTTPCLN * p_user, HTTPMSG * rx_msg)
{

	soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
	return;
}

void GetHttpAccessLog(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
	return;
}

void GetSupportInfo(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
	return;
}

void GetSystemBackup(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
	return;
}

int soap_tev_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Events;

    return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category,
            "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetServiceCapabilitiesResponse", p_header);
}

int soap_tev_GetEventProperties(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtev_GetEventProperties_rly_xml, NULL,
		"http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesResponse", p_header);
}

int soap_tev_Subscribe(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_Subscribe;
	tev_Subscribe_REQ req;

    

    p_Subscribe = _node_soap_get(p_body, "Subscribe");
    assert(p_Subscribe);

	memset(&req, 0, sizeof(req));

	ret = parse_tev_Subscribe(p_Subscribe, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tev_Subscribe(p_user->lip, p_user->lport, &req);
	    if (ONVIF_OK == ret)
	    {
			ret = _bdsend_rly(p_user, rx_msg, bdtev_Subscribe_rly_xml, (char *)req.p_eua,
				"http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/SubscribeResponse", p_header);
		}
	}

    if (ret > 0)
    {
		onvifserver_init_NotificationMessage((char *)req.p_eua, (req.FiltersFlag)?req.Filters.TopicExpression[0]:NULL);
#if 0
    	if (g_onvif_cfg.evt_sim_flag)
    	{
    	    NotificationMessageList * p_message;

    	    // todo : generate event, just for test

    	    p_message = onvif_init_NotificationMessage1();
    		if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}

    		p_message = onvif_init_NotificationMessage2();
    		if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}
    	}
#endif
    	return ret;
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tev_Unsubscribe(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_Unsubscribe;
	XMLN * p_To;

    

    p_Unsubscribe = _node_soap_get(p_body, "Unsubscribe");
    assert(p_Unsubscribe);

    p_To = _node_soap_get(p_header, "To");
	if (p_To && p_To->data)
	{
		ONVIF_RET ret = onvif_tev_Unsubscribe(p_To->data);
	    if (ONVIF_OK == ret)
	    {
	        return _bdsend_rly(p_user, rx_msg, bdtev_Unsubscribe_rly_xml, NULL,
				"http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeResponse", p_header);
	    }
	    else
	    {
	    	return soap_bderr_rly(p_user, rx_msg, ret);
	    }
	}

	return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attibute");
}

int soap_tev_Renew(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_Renew;
	XMLN * p_To;
	tev_Renew_REQ req;

	

	p_Renew = _node_soap_get(p_body, "Renew");
	assert(p_Renew);

	memset(&req, 0, sizeof(req));

	p_To = _node_soap_get(p_header, "To");
	if (p_To && p_To->data)
	{
		strncpy(req.ProducterReference, p_To->data, sizeof(req.ProducterReference)-1);
	}

	ret = parse_tev_Renew(p_Renew, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tev_Renew(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtev_Renew_rly_xml, NULL,
				"http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewResponse", p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tev_CreatePullPointSubscription(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_CreatePullPointSubscription;
	tev_CreatePullPointSubscription_REQ req;

	

	p_CreatePullPointSubscription = _node_soap_get(p_body, "CreatePullPointSubscription");
	assert(p_CreatePullPointSubscription);

	memset(&req, 0, sizeof(req));

	ret = parse_tev_CreatePullPointSubscription(p_CreatePullPointSubscription, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tev_CreatePullPointSubscription(p_user->lip, p_user->lport, &req);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtev_CreatePullPointSubscription_rly_xml, (char *)req.p_eua,
				"http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionResponse", p_header);
		}
	}

    if (ret > 0)
    {
		onvifserver_init_NotificationMessage((char *)req.p_eua, (req.FiltersFlag)?req.Filters.TopicExpression[0]:NULL);
#if 1
    	// todo : generate event, just for test

    	if (g_onvif_cfg.evt_sim_flag)
    	{
            if (req.FiltersFlag)
            {
                char * p;
                char * tmp;
                char topic[256];

                tmp = req.Filters.TopicExpression[0];
                p = strchr(tmp, '|');
                while (p)
                {
                    memset(topic, 0, sizeof(topic));
                    strncpy(topic, tmp, p-tmp);

                    onvif_send_simulate_events(topic);

                    tmp = p+1;
                    p = strchr(tmp, '|');
                }

                if (tmp)
                {
                    onvif_send_simulate_events(tmp);
                }
            }
            else
            {
                onvif_send_simulate_events("");
            }
    	}
#endif
    	return ret;
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tev_PullMessages(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_PullMessages;
	tev_PullMessages_REQ req;

	

	p_PullMessages = _node_soap_get(p_body, "PullMessages");
	assert(p_PullMessages);

	memset(&req, 0, sizeof(req));

    // get the event agent entity index
	sscanf(rx_msg->first_line.value_string, "/event_service/%u", &req.eua_idx);

	ret = parse_tev_PullMessages(p_PullMessages, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tev_PullMessages(&req);
		if (ONVIF_OK == ret)
		{
		    EUA * p_eua = onvif_get_eua_by_index(req.eua_idx);
		    if (p_eua)
		    {
		        p_user->use_count++;
		        p_eua->pullmsg.http_cln = (void *)p_user;
		        return ONVIF_OK;
		    }

		    usleep(5*1000);

			return _bdsend_rly(p_user, rx_msg, bdtev_PullMessages_rly_xml, (char *)&req,
				"http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesResponse", p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tev_SetSynchronizationPoint(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret;
	//EUA * p_eua = NULL;

	

	onvif_tev_SetSynchronizationPoint();

    ret = _bdsend_rly(p_user, rx_msg, bdtev_SetSynchronizationPoint_rly_xml, NULL,
		"http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/SetSynchronizationPointResponse", p_header);

#if 0
    // todo : generate event, just for test
    if (g_onvif_cfg.evt_sim_flag)
	{
	    NotificationMessageList * p_message = onvif_init_NotificationMessage();
		if (p_message)
		{
			onvif_put_NotificationMessage(p_message);
		}

		p_message = onvif_init_NotificationMessage1();
		if (p_message)
		{
			onvif_put_NotificationMessage(p_message);
		}
	}
#endif

    return ret;
}

#ifdef IMAGE_SUPPORT

int soap_img_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Imaging;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_img_GetImagingSettings(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetImagingSettings;
	img_GetImagingSettings_REQ req;

	

    p_GetImagingSettings = _node_soap_get(p_body, "GetImagingSettings");
	assert(p_GetImagingSettings);

    memset(&req, 0, sizeof(img_GetImagingSettings_REQ));

	ret = parse_img_GetImagingSettings(p_GetImagingSettings, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdimg_GetImagingSettings_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_img_SetImagingSettings(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetImagingSettings;
	img_SetImagingSettings_REQ req;

	

    p_SetImagingSettings = _node_soap_get(p_body, "SetImagingSettings");
	assert(p_SetImagingSettings);

	memset(&req, 0, sizeof(img_SetImagingSettings_REQ));

	ret = parse_img_SetImagingSettings(p_SetImagingSettings, &req);
	if (ONVIF_OK == ret)
	{
		ret = _SetImagingSettings(&req);
		if (ONVIF_OK == ret)
		{
			_bdsend_rly(p_user, rx_msg, bdimg_SetImagingSettings_rly_xml, NULL, NULL, p_header);
			//_bdsend_rly(p_user, rx_msg, bdimg_SetImagingSettings_rly_xml, NULL, NULL, p_header);
		}
	}


	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_img_GetOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetOptions;
	img_GetOptions_REQ req;

	

    p_GetOptions = _node_soap_get(p_body, "GetOptions");
	assert(p_GetOptions);

    memset(&req, 0, sizeof(img_GetOptions_REQ));

	ret = parse_img_GetOptions(p_GetOptions, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdimg_GetOptions_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_img_GetMoveOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetMoveOptions;
	img_GetMoveOptions_REQ req;

	

    p_GetMoveOptions = _node_soap_get(p_body, "GetMoveOptions");
	assert(p_GetMoveOptions);

    memset(&req, 0, sizeof(img_GetMoveOptions_REQ));

	ret = parse_img_GetMoveOptions(p_GetMoveOptions, &req);
	if (ONVIF_OK == ret)
	{
	    img_GetMoveOptions_RES res;
	    memset(&res, 0, sizeof(img_GetMoveOptions_RES));

	    ret = _GetMoveOptions(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdimg_GetMoveOptions_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}


int soap_img_GetStatus(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetStatus;
	img_GetStatus_REQ req;

	

	p_GetStatus = _node_soap_get(p_body, "GetStatus");
	assert(p_GetStatus);

    memset(&req, 0, sizeof(img_GetStatus_REQ));

	ret = parse_img_GetStatus(p_GetStatus, &req);
	if (ONVIF_OK == ret)
	{
	    img_GetStatus_RES res;
	    memset(&res, 0, sizeof(img_GetStatus_RES));

	    ret = _GetStatus(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdimg_GetStatus_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	
	return soap_err_def_rly(p_user, rx_msg);
}


int soap_img_GetPresets(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetPresets;
	img_GetPresets_REQ req;

	

	p_GetPresets = _node_soap_get(p_body, "GetPresets");
	assert(p_GetPresets);

    memset(&req, 0, sizeof(img_GetPresets_REQ));

    ret = parse_img_GetPresets(p_GetPresets, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdimg_GetPresets_rly_xml, (char *)&req, NULL, p_header);
	}

	
	return soap_err_def_rly(p_user, rx_msg);
}

int soap_img_GetCurrentPreset(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetPresets;
	img_GetCurrentPreset_REQ req;

	

	p_GetPresets = _node_soap_get(p_body, "GetCurrentPreset");
	assert(p_GetPresets);

    ret = parse_img_GetCurrentPreset(p_GetPresets, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdimg_GetCurrentPreset_rly_xml, (char *)&req, NULL, p_header);
	}

	
	return soap_err_def_rly(p_user, rx_msg);
}

int soap_img_SetCurrentPreset(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetPresets;
	img_SetCurrentPreset_REQ req;

	

	p_GetPresets = _node_soap_get(p_body, "SetCurrentPreset");
	assert(p_GetPresets);

    ret = parse_img_SetCurrentPreset(p_GetPresets, &req);
	if (ONVIF_OK == ret)
	{
	    ret = _SetCurrentPreset(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdimg_SetCurrentPreset_rly_xml, NULL, NULL, p_header);
		}
	}

	
	return soap_err_def_rly(p_user, rx_msg);
}

#endif // IMAGE_SUPPORT

#ifdef MEDIA_SUPPORT

int soap_trt_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Media;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_trt_GetProfiles(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtrt_GetProfiles_rly_xml, NULL, NULL, p_header);
}

int soap_trt_GetProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetProfile;
    trt_GetProfile_REQ req;

	

    p_GetProfile = _node_soap_get(p_body, "GetProfile");
    assert(p_GetProfile);

    memset(&req, 0, sizeof(req));

    ret = _trt_GetProfile(p_GetProfile, &req);
    if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetProfile_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_CreateProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_CreateProfile;
    trt_CreateProfile_REQ req;

	

	p_CreateProfile = _node_soap_get(p_body, "CreateProfile");
    assert(p_CreateProfile);

    memset(&req, 0, sizeof(req));

    ret = _trt_CreateProfile(p_CreateProfile, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_CreateProfile(p_user->lip, p_user->lport, &req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtrt_CreateProfile_rly_xml, req.Token, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_DeleteProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_DeleteProfile;
    trt_DeleteProfile_REQ req;

	

	p_DeleteProfile = _node_soap_get(p_body, "DeleteProfile");
    assert(p_DeleteProfile);

	memset(&req, 0, sizeof(req));

    ret = _trt_DeleteProfile(p_DeleteProfile, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_DeleteProfile(&req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtrt_DeleteProfile_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_AddVideoSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_AddVideoSourceConfiguration;
    trt_AddVideoSourceConfiguration_REQ req;

	

	p_AddVideoSourceConfiguration = _node_soap_get(p_body, "AddVideoSourceConfiguration");
    assert(p_AddVideoSourceConfiguration);

	memset(&req, 0, sizeof(req));

    ret = _trt_AddVideoSourceConfiguration(p_AddVideoSourceConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_AddVideoSourceConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtrt_AddVideoSourceConfiguration_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_RemoveVideoSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_RemoveVideoSourceConfiguration;
    trt_RemoveVideoSourceConfiguration_REQ req;

	

	p_RemoveVideoSourceConfiguration = _node_soap_get(p_body, "RemoveVideoSourceConfiguration");
	assert(p_RemoveVideoSourceConfiguration);

	memset(&req, 0, sizeof(req));

    ret = _trt_RemoveVideoSourceConfiguration(p_RemoveVideoSourceConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_RemoveVideoSourceConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtrt_RemoveVideoSourceConfiguration_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_AddVideoEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_AddVideoEncoderConfiguration;
    trt_AddVideoEncoderConfiguration_REQ req;

	

	p_AddVideoEncoderConfiguration = _node_soap_get(p_body, "AddVideoEncoderConfiguration");
	assert(p_AddVideoEncoderConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_AddVideoEncoderConfiguration(p_AddVideoEncoderConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = trt_AddVideoEncoderConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrt_AddVideoEncoderConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_RemoveVideoEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_RemoveVideoEncoderConfiguration;
    trt_RemoveVideoEncoderConfiguration_REQ req;

	

	p_RemoveVideoEncoderConfiguration = _node_soap_get(p_body, "RemoveVideoEncoderConfiguration");
	assert(p_RemoveVideoEncoderConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_RemoveVideoEncoderConfiguration(p_RemoveVideoEncoderConfiguration, &req);
	if (ONVIF_OK == ret)
    {
    	ret = trt_RemoveVideoEncoderConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtrt_RemoveVideoEncoderConfiguration_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetVideoSources(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoSources_rly_xml, NULL, NULL, p_header);
}

int soap_trt_GetVideoEncoderConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoEncoderConfigurations_rly_xml, NULL, NULL, p_header);
}

int soap_trt_GetCompatibleVideoEncoderConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetCompatibleVideoEncoderConfigurations;
    XMLN * p_ProfileToken;

	

	p_GetCompatibleVideoEncoderConfigurations = _node_soap_get(p_body, "GetCompatibleVideoEncoderConfigurations");
    assert(p_GetCompatibleVideoEncoderConfigurations);

    p_ProfileToken = _node_soap_get(p_GetCompatibleVideoEncoderConfigurations, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
    	return _bdsend_rly(p_user, rx_msg, bdtrt_GetCompatibleVideoEncoderConfigurations_rly_xml, p_ProfileToken->data, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ONVIF_ERR_MissingAttribute);
}

int soap_trt_GetVideoEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetVideoEncoderConfiguration;
    XMLN * p_ConfigurationToken;

	

    p_GetVideoEncoderConfiguration = _node_soap_get(p_body, "GetVideoEncoderConfiguration");
    assert(p_GetVideoEncoderConfiguration);

	p_ConfigurationToken = _node_soap_get(p_GetVideoEncoderConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoEncoderConfiguration_rly_xml, p_ConfigurationToken->data, NULL, p_header);
	}
	else
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_trt_GetVideoSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoSourceConfigurations_rly_xml, NULL, NULL, p_header);
}

int soap_trt_GetVideoSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetVideoSourceConfiguration;
    XMLN * p_ConfigurationToken;

    

    p_GetVideoSourceConfiguration = _node_soap_get(p_body, "GetVideoSourceConfiguration");
    assert(p_GetVideoSourceConfiguration);

	p_ConfigurationToken = _node_soap_get(p_GetVideoSourceConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoSourceConfiguration_rly_xml, p_ConfigurationToken->data, NULL, p_header);
	}
	else
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_trt_SetVideoSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_SetVideoSourceConfiguration;
    trt_SetVideoSourceConfiguration_REQ req;

	

    p_SetVideoSourceConfiguration = _node_soap_get(p_body, "SetVideoSourceConfiguration");
    assert(p_SetVideoSourceConfiguration);

    memset(&req, 0, sizeof(req));

    ret = _trt_SetVideoSourceConfiguration(p_SetVideoSourceConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_SetVideoSourceConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
    		return _bdsend_rly(p_user, rx_msg, bdtrt_SetVideoSourceConfiguration_rly_xml, NULL, NULL, p_header);
    	}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetVideoSourceConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetVideoSourceConfigurationOptions;
    trt_GetVideoSourceConfigurationOptions_REQ req;

    

    p_GetVideoSourceConfigurationOptions = _node_soap_get(p_body, "GetVideoSourceConfigurationOptions");
    assert(p_GetVideoSourceConfigurationOptions);

	memset(&req, 0, sizeof(req));

	ret = _trt_GetVideoSourceConfigurationOptions(p_GetVideoSourceConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoSourceConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetVideoEncoderConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_GetVideoEncoderConfigurationOptions;
	trt_GetVideoEncoderConfigurationOptions_REQ req;

	const int CH1 = 0;
	const int CH2 = 1;
	const int CH3 = 2;

    

	p_GetVideoEncoderConfigurationOptions = _node_soap_get(p_body, "GetVideoEncoderConfigurationOptions");
    assert(p_GetVideoEncoderConfigurationOptions);

    memset(&req, 0, sizeof(req));
//ConfigurationToken
	ret = _trt_GetVideoEncoderConfigurationOptions(p_GetVideoEncoderConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
	    trt_GetVideoEncoderConfigurationOptions_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = trt_GetVideoEncoderConfigurationOptions(&req, &res);

        if (ONVIF_OK == ret)
        {
			return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoEncoderConfigurationOptions_rly_xml, (char *)&res, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetCompatibleVideoSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetCompatibleVideoSourceConfigurations;
	XMLN * p_ProfileToken;

	

    p_GetCompatibleVideoSourceConfigurations = _node_soap_get(p_body, "GetCompatibleVideoSourceConfigurations");
    assert(p_GetCompatibleVideoSourceConfigurations);

	p_ProfileToken = _node_soap_get(p_GetCompatibleVideoSourceConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtrt_GetCompatibleVideoSourceConfigurations_rly_xml, p_ProfileToken->data, NULL, p_header);
	}
	else
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_trt_SetVideoEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetVideoEncoderConfiguration;
	trt_SetVideoEncoderConfiguration_REQ req;

    

    p_SetVideoEncoderConfiguration = _node_soap_get(p_body, "SetVideoEncoderConfiguration");
    assert(p_SetVideoEncoderConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_SetVideoEncoderConfiguration(p_SetVideoEncoderConfiguration, &req);

	
    if (ONVIF_OK == ret)
    {
        ret = trt_SetVideoEncoderConfiguration(&req);
		
        if (ONVIF_OK == ret)
        {
        	return _bdsend_rly(p_user, rx_msg, bdtrt_SetVideoEncoderConfiguration_rly_xml, NULL, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_SetSynchronizationPoint(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_SetSynchronizationPoint;
    trt_SetSynchronizationPoint_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_SetSynchronizationPoint = _node_soap_get(p_body, "SetSynchronizationPoint");
    assert(p_SetSynchronizationPoint);

    ret = _trt_SetSynchronizationPoint(p_SetSynchronizationPoint, &req);
    if (ONVIF_OK == ret)
    {
        ret = trt_SetSynchronizationPoint(&req);
        if (ONVIF_OK == ret)
        {
            return _bdsend_rly(p_user, rx_msg, bdtrt_SetSynchronizationPoint_rly_xml, (char *)&req, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetStreamUri(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetStreamUri;
    trt_GetStreamUri_REQ req;

	

    p_GetStreamUri = _node_soap_get(p_body, "GetStreamUri");
    assert(p_GetStreamUri);

	memset(&req, 0, sizeof(req));

	ret = _trt_GetStreamUri(p_GetStreamUri, &req);
	if (ONVIF_OK == ret)
	{
	    trt_GetStreamUri_RES res;
	    memset(&res, 0, sizeof(res));

		ret = trt_GetStreamUri(p_user->lip, p_user->lport, &req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrt_GetStreamUri_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetOSDs(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_GetOSDs;
	trt_GetOSDs_REQ req;

	

	p_GetOSDs = _node_soap_get(p_body, "GetOSDs");
	assert(p_GetOSDs);

	memset(&req, 0, sizeof(req));

    ret = _trt_GetOSDs(p_GetOSDs, &req);
    if (ONVIF_OK == ret)
    {
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetOSDs_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_GetOSD;
	trt_GetOSD_REQ req;

	

	p_GetOSD = _node_soap_get(p_body, "GetOSD");
	assert(p_GetOSD);

	memset(&req, 0, sizeof(req));

    ret = _trt_GetOSD(p_GetOSD, &req);
    if (ONVIF_OK == ret)
    {
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetOSD_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_SetOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetOSD;
	trt_SetOSD_REQ req;

	

	p_SetOSD = _node_soap_get(p_body, "SetOSD");
	assert(p_SetOSD);

	memset(&req, 0, sizeof(req));

    ret = _trt_SetOSD(p_SetOSD, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_SetOSD(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtrt_SetOSD_rly_xml, NULL, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetOSDOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtrt_GetOSDOptions_rly_xml, NULL, NULL, p_header);
}

int soap_trt_CreateOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_CreateOSD;
	trt_CreateOSD_REQ req;

	

	p_CreateOSD = _node_soap_get(p_body, "CreateOSD");
	assert(p_CreateOSD);

	memset(&req, 0, sizeof(req));

    ret = _trt_CreateOSD(p_CreateOSD, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_CreateOSD(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtrt_CreateOSD_rly_xml, req.OSD.token, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_DeleteOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_DeleteOSD;
	trt_DeleteOSD_REQ req;

	

	p_DeleteOSD = _node_soap_get(p_body, "DeleteOSD");
	assert(p_DeleteOSD);

	memset(&req, 0, sizeof(req));

    ret = _trt_DeleteOSD(p_DeleteOSD, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_DeleteOSD(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtrt_DeleteOSD_rly_xml, NULL, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_StartMulticastStreaming(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_StartMulticastStreaming;
	XMLN * p_ProfileToken;
	ONVIF_RET ret = ONVIF_ERR_MissingAttribute;

	

    p_StartMulticastStreaming = _node_soap_get(p_body, "StartMulticastStreaming");
	assert(p_StartMulticastStreaming);

	p_ProfileToken = _node_soap_get(p_StartMulticastStreaming, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		ret = trt_StartMulticastStreaming(p_ProfileToken->data);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrt_StartMulticastStreaming_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_StopMulticastStreaming(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_StopMulticastStreaming;
	XMLN * p_ProfileToken;
	ONVIF_RET ret = ONVIF_ERR_MissingAttribute;

	

    p_StopMulticastStreaming = _node_soap_get(p_body, "StopMulticastStreaming");
	assert(p_StopMulticastStreaming);

	p_ProfileToken = _node_soap_get(p_StopMulticastStreaming, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		ret = trt_StopMulticastStreaming(p_ProfileToken->data);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrt_StopMulticastStreaming_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetMetadataConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtrt_GetMetadataConfigurations_rly_xml, NULL, NULL, p_header);
}

int soap_trt_GetMetadataConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetMetadataConfiguration;
	XMLN * p_ConfigurationToken;
	ONVIF_RET ret = ONVIF_ERR_MissingAttribute;

	

    p_GetMetadataConfiguration = _node_soap_get(p_body, "GetMetadataConfiguration");
	assert(p_GetMetadataConfiguration);

	p_ConfigurationToken = _node_soap_get(p_GetMetadataConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetMetadataConfiguration_rly_xml, p_ConfigurationToken->data, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetCompatibleMetadataConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetCompatibleMetadataConfigurations;
	XMLN * p_ProfileToken;
	ONVIF_RET ret = ONVIF_ERR_MissingAttribute;

	

    p_GetCompatibleMetadataConfigurations = _node_soap_get(p_body, "GetCompatibleMetadataConfigurations");
	assert(p_GetCompatibleMetadataConfigurations);

	p_ProfileToken = _node_soap_get(p_GetCompatibleMetadataConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetCompatibleMetadataConfigurations_rly_xml, p_ProfileToken->data, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetMetadataConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_GetMetadataConfigurationOptions;
	trt_GetMetadataConfigurationOptions_REQ req;

	

	p_GetMetadataConfigurationOptions = _node_soap_get(p_body, "GetMetadataConfigurationOptions");
	assert(p_GetMetadataConfigurationOptions);

	memset(&req, 0, sizeof(req));

    ret = _trt_GetMetadataConfigurationOptions(p_GetMetadataConfigurationOptions, &req);
    if (ONVIF_OK == ret)
    {
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetMetadataConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_SetMetadataConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetMetadataConfiguration;
	trt_SetMetadataConfiguration_REQ req;

	

	p_SetMetadataConfiguration = _node_soap_get(p_body, "SetMetadataConfiguration");
	assert(p_SetMetadataConfiguration);

	memset(&req, 0, sizeof(req));

    ret = _trt_SetMetadataConfiguration(p_SetMetadataConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_SetMetadataConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtrt_SetMetadataConfiguration_rly_xml, NULL, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_AddMetadataConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_AddMetadataConfiguration;
	trt_AddMetadataConfiguration_REQ req;

	

	p_AddMetadataConfiguration = _node_soap_get(p_body, "AddMetadataConfiguration");
	assert(p_AddMetadataConfiguration);

	memset(&req, 0, sizeof(req));

    ret = _trt_AddMetadataConfiguration(p_AddMetadataConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_AddMetadataConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtrt_AddMetadataConfiguration_rly_xml, NULL, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_RemoveMetadataConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_RemoveMetadataConfiguration;
	XMLN * p_ProfileToken;
	ONVIF_RET ret = ONVIF_ERR_MissingAttribute;

	

    p_RemoveMetadataConfiguration = _node_soap_get(p_body, "RemoveMetadataConfiguration");
	assert(p_RemoveMetadataConfiguration);

	p_ProfileToken = _node_soap_get(p_RemoveMetadataConfiguration, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		ret = trt_RemoveMetadataConfiguration(p_ProfileToken->data);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrt_RemoveMetadataConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetVideoSourceModes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetVideoSourceModes;
    trt_GetVideoSourceModes_REQ req;

	

	p_GetVideoSourceModes = _node_soap_get(p_body, "GetVideoSourceModes");
    assert(p_GetVideoSourceModes);

	memset(&req, 0, sizeof(req));

    ret = _trt_GetVideoSourceModes(p_GetVideoSourceModes, &req);
    if (ONVIF_OK == ret)
    {
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoSourceModes_rly_xml, NULL, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_SetVideoSourceMode(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_SetVideoSourceMode;
    trt_SetVideoSourceMode_REQ req;

	

	p_SetVideoSourceMode = _node_soap_get(p_body, "SetVideoSourceMode");
    assert(p_SetVideoSourceMode);

	memset(&req, 0, sizeof(req));

    ret = _trt_SetVideoSourceMode(p_SetVideoSourceMode, &req);
    if (ONVIF_OK == ret)
    {
        trt_SetVideoSourceMode_RES res;
        memset(&res, 0, sizeof(res));

        ret = trt_SetVideoSourceMode(&req, &res);
    	if (ONVIF_OK == ret)
    	{
		    return _bdsend_rly(p_user, rx_msg, bdtrt_SetVideoSourceMode_rly_xml, (char*)&res, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetGuaranteedNumberOfVideoEncoderInstances(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetGuaranteedNumberOfVideoEncoderInstances;
	XMLN * p_ConfigurationToken;

	

    p_GetGuaranteedNumberOfVideoEncoderInstances = _node_soap_get(p_body, "GetGuaranteedNumberOfVideoEncoderInstances");
	assert(p_GetGuaranteedNumberOfVideoEncoderInstances);

	p_ConfigurationToken = _node_soap_get(p_GetGuaranteedNumberOfVideoEncoderInstances, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetGuaranteedNumberOfVideoEncoderInstances_rly_xml, p_ConfigurationToken->data, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ONVIF_ERR_MissingAttribute);
}


int soap_trt_GetAudioSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	return _bdsend_rly(p_user, rx_msg, bdtrt_GetAudioSourceConfigurations_rly_xml, NULL, NULL, p_header);
}


int soap_trt_GetCompatibleAudioSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetCompatibleAudioSourceConfigurations;
	XMLN * p_ProfileToken;

	

	p_GetCompatibleAudioSourceConfigurations = _node_soap_get(p_body, "GetCompatibleAudioSourceConfigurations");
    assert(p_GetCompatibleAudioSourceConfigurations);

    p_ProfileToken = _node_soap_get(p_GetCompatibleAudioSourceConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtrt_GetCompatibleAudioSourceConfigurations_rly_xml, p_ProfileToken->data, NULL, p_header);
	}
	else
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}


int soap_trt_GetAudioSourceConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_GetAudioSourceConfigurationOptions;
	trt_GetAudioSourceConfigurationOptions_REQ req;

	

	p_GetAudioSourceConfigurationOptions = _node_soap_get(p_body, "GetAudioSourceConfigurationOptions");
    assert(p_GetAudioSourceConfigurationOptions);

	memset(&req, 0, sizeof(req));

	ret = _trt_GetAudioSourceConfigurationOptions(p_GetAudioSourceConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetAudioSourceConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}


int soap_trt_GetAudioSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetAudioSourceConfiguration;
	XMLN * p_ConfigurationToken;

    

    p_GetAudioSourceConfiguration = _node_soap_get(p_body, "GetAudioSourceConfiguration");
    assert(p_GetAudioSourceConfiguration);

	p_ConfigurationToken = _node_soap_get(p_GetAudioSourceConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtrt_GetAudioSourceConfiguration_rly_xml, p_ConfigurationToken->data, NULL, p_header);
	}
	else
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_trt_GetAudioEncoderConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_GetAudioEncoderConfigurationOptions;
	trt_GetAudioEncoderConfigurationOptions_REQ req;

    

	p_GetAudioEncoderConfigurationOptions = _node_soap_get(p_body, "GetAudioEncoderConfigurationOptions");
    assert(p_GetAudioEncoderConfigurationOptions);

    memset(&req, 0, sizeof(req));

	ret = _trt_GetAudioEncoderConfigurationOptions(p_GetAudioEncoderConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetAudioEncoderConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
	}

    return soap_bderr_rly(p_user, rx_msg, ret);
}
#ifdef DEVICEIO_SUPPORT

int soap_trt_GetAudioOutputConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

	return _bdsend_rly(p_user, rx_msg, bdtrt_GetAudioOutputConfigurations_rly_xml, NULL, NULL, p_header);
}

int soap_trt_SetAudioOutputConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_SetAudioOutputConfiguration;
	tmd_SetAudioOutputConfiguration_REQ req;
	ONVIF_RET ret;

	

	p_SetAudioOutputConfiguration = _node_soap_get(p_body, "SetAudioOutputConfiguration");
	assert(p_SetAudioOutputConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_SetAudioOutputConfiguration(p_SetAudioOutputConfiguration, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tmd_SetAudioOutputConfiguration(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrt_SetAudioOutputConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}


int soap_trt_GetAudioOutputConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetAudioOutputConfiguration;
    trt_GetAudioOutputConfiguration_REQ req;

	

	p_GetAudioOutputConfiguration = _node_soap_get(p_body, "GetAudioOutputConfiguration");
	assert(p_GetAudioOutputConfiguration);

    memset(&req, 0, sizeof(req));

    ret = _trt_GetAudioOutputConfiguration(p_GetAudioOutputConfiguration, &req);
	if (ONVIF_OK == ret)
	{
        return _bdsend_rly(p_user, rx_msg, bdtrt_GetAudioOutputConfiguration_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_AddAudioOutputConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_AddAudioOutputConfiguration;
	trt_AddAudioOutputConfiguration_REQ req;
	ONVIF_RET ret;

	

	p_AddAudioOutputConfiguration = _node_soap_get(p_body, "AddAudioOutputConfiguration");
	assert(p_AddAudioOutputConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_AddAudioOutputConfiguration(p_AddAudioOutputConfiguration, &req);
	if (ONVIF_OK == ret)
	{
	    ret = trt_AddAudioOutputConfiguration(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrt_AddAudioOutputConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_RemoveAudioOutputConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_RemoveAudioOutputConfiguration;
	trt_RemoveAudioOutputConfiguration_REQ req;
	ONVIF_RET ret;

	

	p_RemoveAudioOutputConfiguration = _node_soap_get(p_body, "RemoveAudioOutputConfiguration");
	assert(p_RemoveAudioOutputConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_RemoveAudioOutputConfiguration(p_RemoveAudioOutputConfiguration, &req);
	if (ONVIF_OK == ret)
	{
	    ret = trt_RemoveAudioOutputConfiguration(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrt_RemoveAudioOutputConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetCompatibleAudioOutputConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetCompatibleAudioOutputConfigurations;
	trt_GetCompatibleAudioOutputConfigurations_REQ req;
	ONVIF_RET ret;

	

	p_GetCompatibleAudioOutputConfigurations = _node_soap_get(p_body, "GetCompatibleAudioOutputConfigurations");
	assert(p_GetCompatibleAudioOutputConfigurations);

	memset(&req, 0, sizeof(req));

	ret = _trt_GetCompatibleAudioOutputConfigurations(p_GetCompatibleAudioOutputConfigurations, &req);
	if (ONVIF_OK == ret)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtrt_GetCompatibleAudioOutputConfigurations_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // end of DEVICEIO_SUPPORT

#ifdef VIDEO_ANALYTICS

int soap_trt_GetAnalyticsConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

	return _bdsend_rly(p_user, rx_msg, bdtrt_GetAnalyticsConfigurations_rly_xml, NULL, NULL, p_header);
}

int soap_trt_GetVideoAnalyticsConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoAnalyticsConfigurations_rly_xml, NULL, NULL, p_header);
}

int soap_trt_AddVideoAnalyticsConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_AddVideoAnalyticsConfiguration;
	trt_AddVideoAnalyticsConfiguration_REQ req;
	ONVIF_RET ret;

	

    p_AddVideoAnalyticsConfiguration = _node_soap_get(p_body, "AddVideoAnalyticsConfiguration");
	assert(p_AddVideoAnalyticsConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_AddVideoAnalyticsConfiguration(p_AddVideoAnalyticsConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = trt_AddVideoAnalyticsConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrt_AddVideoAnalyticsConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetVideoAnalyticsConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetVideoAnalyticsConfiguration;
	trt_GetVideoAnalyticsConfiguration_REQ req;
	ONVIF_RET ret;

	

    p_GetVideoAnalyticsConfiguration = _node_soap_get(p_body, "GetVideoAnalyticsConfiguration");
	assert(p_GetVideoAnalyticsConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_GetVideoAnalyticsConfiguration(p_GetVideoAnalyticsConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetVideoAnalyticsConfiguration_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_RemoveVideoAnalyticsConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_RemoveVideoAnalyticsConfiguration;
	trt_RemoveVideoAnalyticsConfiguration_REQ req;
	ONVIF_RET ret;

	

    p_RemoveVideoAnalyticsConfiguration = _node_soap_get(p_body, "RemoveVideoAnalyticsConfiguration");
	assert(p_RemoveVideoAnalyticsConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_RemoveVideoAnalyticsConfiguration(p_RemoveVideoAnalyticsConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = trt_RemoveVideoAnalyticsConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrt_RemoveVideoAnalyticsConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_SetVideoAnalyticsConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_SetVideoAnalyticsConfiguration;
	trt_SetVideoAnalyticsConfiguration_REQ req;
	ONVIF_RET ret;

	

    p_SetVideoAnalyticsConfiguration = _node_soap_get(p_body, "SetVideoAnalyticsConfiguration");
	assert(p_SetVideoAnalyticsConfiguration);

	memset(&req, 0, sizeof(req));

	ret = _trt_SetVideoAnalyticsConfiguration(p_SetVideoAnalyticsConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = trt_SetVideoAnalyticsConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrt_SetVideoAnalyticsConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trt_GetCompatibleVideoAnalyticsConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetCompatibleVideoAnalyticsConfigurations;
	trt_GetCompatibleVideoAnalyticsConfigurations_REQ req;
	ONVIF_RET ret;

	

    p_GetCompatibleVideoAnalyticsConfigurations = _node_soap_get(p_body, "GetCompatibleVideoAnalyticsConfigurations");
	assert(p_GetCompatibleVideoAnalyticsConfigurations);

	memset(&req, 0, sizeof(req));

	ret = _trt_GetCompatibleVideoAnalyticsConfigurations(p_GetCompatibleVideoAnalyticsConfigurations, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrt_GetCompatibleVideoAnalyticsConfigurations_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // VIDEO_ANALYTICS

#endif // MEDIA_SUPPORT

#ifdef VIDEO_ANALYTICS

int soap_tan_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Analytics;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tan_GetSupportedRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetSupportedRules;
	tan_GetSupportedRules_REQ req;
	ONVIF_RET ret;

	

    p_GetSupportedRules = _node_soap_get(p_body, "GetSupportedRules");
	assert(p_GetSupportedRules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_GetSupportedRules(p_GetSupportedRules, &req);
	if (ONVIF_OK == ret)
	{
		tan_GetSupportedRules_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tan_GetSupportedRules(&req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtan_GetSupportedRules_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_CreateRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_CreateRules;
	tan_CreateRules_REQ req;
	ONVIF_RET ret;

	

    p_CreateRules = _node_soap_get(p_body, "CreateRules");
	assert(p_CreateRules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_CreateRules(p_CreateRules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tan_CreateRules(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtan_CreateRules_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_DeleteRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_DeleteRules;
	tan_DeleteRules_REQ req;
	ONVIF_RET ret;

	

    p_DeleteRules = _node_soap_get(p_body, "DeleteRules");
	assert(p_DeleteRules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_DeleteRules(p_DeleteRules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tan_DeleteRules(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtan_DeleteRules_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_GetRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetRules;
	tan_GetRules_REQ req;
	ONVIF_RET ret;

	

    p_GetRules = _node_soap_get(p_body, "GetRules");
	assert(p_GetRules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_GetRules(p_GetRules, &req);
	if (ONVIF_OK == ret)
	{
		tan_GetRules_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tan_GetRules(&req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtan_GetRules_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_ModifyRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_ModifyRules;
	tan_ModifyRules_REQ req;
	ONVIF_RET ret;

	

    p_ModifyRules = _node_soap_get(p_body, "ModifyRules");
	assert(p_ModifyRules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_ModifyRules(p_ModifyRules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tan_ModifyRules(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtan_ModifyRules_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_CreateAnalyticsModules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_CreateAnalyticsModules;
	tan_CreateAnalyticsModules_REQ req;
	ONVIF_RET ret;

	

    p_CreateAnalyticsModules = _node_soap_get(p_body, "CreateAnalyticsModules");
	assert(p_CreateAnalyticsModules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_CreateAnalyticsModules(p_CreateAnalyticsModules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tan_CreateAnalyticsModules(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtan_CreateAnalyticsModules_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_DeleteAnalyticsModules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_DeleteAnalyticsModules;
	tan_DeleteAnalyticsModules_REQ req;
	ONVIF_RET ret;

	

    p_DeleteAnalyticsModules = _node_soap_get(p_body, "DeleteAnalyticsModules");
	assert(p_DeleteAnalyticsModules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_DeleteAnalyticsModules(p_DeleteAnalyticsModules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tan_DeleteAnalyticsModules(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtan_DeleteAnalyticsModules_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_GetAnalyticsModules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetAnalyticsModules;
	tan_GetAnalyticsModules_REQ req;
	ONVIF_RET ret;

	

    p_GetAnalyticsModules = _node_soap_get(p_body, "GetAnalyticsModules");
	assert(p_GetAnalyticsModules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_GetAnalyticsModules(p_GetAnalyticsModules, &req);
	if (ONVIF_OK == ret)
	{
		tan_GetAnalyticsModules_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tan_GetAnalyticsModules(&req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtan_GetAnalyticsModules_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_ModifyAnalyticsModules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_ModifyAnalyticsModules;
	tan_ModifyAnalyticsModules_REQ req;
	ONVIF_RET ret;

	

    p_ModifyAnalyticsModules = _node_soap_get(p_body, "ModifyAnalyticsModules");
	assert(p_ModifyAnalyticsModules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_ModifyAnalyticsModules(p_ModifyAnalyticsModules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tan_ModifyAnalyticsModules(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtan_ModifyAnalyticsModules_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_GetRuleOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetRuleOptions;
	tan_GetRuleOptions_REQ req;
	ONVIF_RET ret;

	

    p_GetRuleOptions = _node_soap_get(p_body, "GetRuleOptions");
	assert(p_GetRuleOptions);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_GetRuleOptions(p_GetRuleOptions, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtan_GetRuleOptions_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_GetSupportedAnalyticsModules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetSupportedAnalyticsModules;
	tan_GetSupportedAnalyticsModules_REQ req;
	ONVIF_RET ret;

	

    p_GetSupportedAnalyticsModules = _node_soap_get(p_body, "GetSupportedAnalyticsModules");
	assert(p_GetSupportedAnalyticsModules);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_GetSupportedAnalyticsModules(p_GetSupportedAnalyticsModules, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtan_GetSupportedAnalyticsModules_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tan_GetAnalyticsModuleOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAnalyticsModuleOptions;
	tan_GetAnalyticsModuleOptions_REQ req;
	ONVIF_RET ret;

	

    p_GetAnalyticsModuleOptions = _node_soap_get(p_body, "GetAnalyticsModuleOptions");
	assert(p_GetAnalyticsModuleOptions);

	memset(&req, 0, sizeof(req));

	ret = parse_tan_GetAnalyticsModuleOptions(p_GetAnalyticsModuleOptions, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtan_GetAnalyticsModuleOptions_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // end of VIDEO_ANALYTICS

#ifdef PROFILE_G_SUPPORT

int soap_tse_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Search;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tse_GetRecordingSummary(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	tse_GetRecordingSummary_RES res;

	

	memset(&res, 0, sizeof(res));

	ret = onvif_tse_GetRecordingSummary(&res);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtse_GetRecordingSummary_rly_xml, (char *)&res, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_GetRecordingInformation(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetRecordingInformation;
	tse_GetRecordingInformation_REQ req;
	ONVIF_RET ret = ONVIF_ERR_MissingAttribute;

	

	p_GetRecordingInformation = _node_soap_get(p_body, "GetRecordingInformation");
	assert(p_GetRecordingInformation);

    memset(&req, 0, sizeof(req));

    ret = parse_tse_GetRecordingInformation(p_GetRecordingInformation, &req);
	if (ONVIF_OK == ret)
	{
		tse_GetRecordingInformation_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_GetRecordingInformation(req.RecordingToken, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtse_GetRecordingInformation_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_GetMediaAttributes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetMediaAttributes;
	tse_GetMediaAttributes_REQ req;
	ONVIF_RET ret;

	

	p_GetMediaAttributes = _node_soap_get(p_body, "GetMediaAttributes");
	assert(p_GetMediaAttributes);

	memset(&req, 0, sizeof(req));

	ret = parse_tse_GetMediaAttributes(p_GetMediaAttributes, &req);
	if (ONVIF_OK == ret)
	{
		tse_GetMediaAttributes_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_GetMediaAttributes(&req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtse_GetMediaAttributes_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_FindRecordings(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_FindRecordings;
	tse_FindRecordings_REQ req;
	ONVIF_RET ret;

	

	p_FindRecordings = _node_soap_get(p_body, "FindRecordings");
	assert(p_FindRecordings);

	memset(&req, 0, sizeof(req));

	ret = parse_tse_FindRecordings(p_FindRecordings, &req);
	if (ONVIF_OK == ret)
	{
		tse_FindRecordings_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_FindRecordings(&req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtse_FindRecordings_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_GetRecordingSearchResults(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetRecordingSearchResults;
	tse_GetRecordingSearchResults_REQ req;
	ONVIF_RET ret;

	

	p_GetRecordingSearchResults = _node_soap_get(p_body, "GetRecordingSearchResults");
	assert(p_GetRecordingSearchResults);

	memset(&req, 0, sizeof(req));

	ret = parse_tse_GetRecordingSearchResults(p_GetRecordingSearchResults, &req);
	if (ONVIF_OK == ret)
	{
		tse_GetRecordingSearchResults_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_GetRecordingSearchResults(&req, &res);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtse_GetRecordingSearchResults_rly_xml, (char *)&res, NULL, p_header);

			onvif_free_RecordingInformations(&res.ResultList.RecordInformation);

			return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_FindEvents(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_FindEvents;
	tse_FindEvents_REQ req;
	ONVIF_RET ret;

	

	p_FindEvents = _node_soap_get(p_body, "FindEvents");
	assert(p_FindEvents);

	memset(&req, 0, sizeof(req));

	ret = parse_tse_FindEvents(p_FindEvents, &req);
	if (ONVIF_OK == ret)
	{
		tse_FindEvents_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_FindEvents(&req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtse_FindEvents_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_GetEventSearchResults(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetEventSearchResults;
	tse_GetEventSearchResults_REQ req;
	ONVIF_RET ret;

	

	p_GetEventSearchResults = _node_soap_get(p_body, "GetEventSearchResults");
	assert(p_GetEventSearchResults);

	memset(&req, 0, sizeof(req));

	ret = parse_tse_GetEventSearchResults(p_GetEventSearchResults, &req);
	if (ONVIF_OK == ret)
	{
		tse_GetEventSearchResults_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_GetEventSearchResults(&req, &res);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtse_GetEventSearchResults_rly_xml, (char *)&res, NULL, p_header);

			onvif_free_FindEventResults(&res.ResultList.Result);

			return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_FindMetadata(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_FindMetadata;
	tse_FindMetadata_REQ req;
	ONVIF_RET ret;

	

	p_FindMetadata = _node_soap_get(p_body, "FindMetadata");
	assert(p_FindMetadata);

	memset(&req, 0, sizeof(req));

	ret = parse_tse_FindMetadata(p_FindMetadata, &req);
	if (ONVIF_OK == ret)
	{
		tse_FindMetadata_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_FindMetadata(&req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtse_FindMetadata_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_GetMetadataSearchResults(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetMetadataSearchResults;
	tse_GetMetadataSearchResults_REQ req;
	ONVIF_RET ret;

	

	p_GetMetadataSearchResults = _node_soap_get(p_body, "GetMetadataSearchResults");
	assert(p_GetMetadataSearchResults);

	memset(&req, 0, sizeof(req));

	ret = parse_tse_GetMetadataSearchResults(p_GetMetadataSearchResults, &req);
	if (ONVIF_OK == ret)
	{
		tse_GetMetadataSearchResults_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_GetMetadataSearchResults(&req, &res);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtse_GetMetadataSearchResults_rly_xml, (char *)&res, NULL, p_header);

			onvif_free_FindMetadataResults(&res.ResultList.Result);

			return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_EndSearch(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_EndSearch;
	tse_EndSearch_REQ req;
	ONVIF_RET ret;

	

	p_EndSearch = _node_soap_get(p_body, "EndSearch");
	assert(p_EndSearch);

	memset(&req, 0, sizeof(req));

	ret = parse_tse_EndSearch(p_EndSearch, &req);
	if (ONVIF_OK == ret)
	{
		tse_EndSearch_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_EndSearch(&req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtse_EndSearch_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tse_GetSearchState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetSearchState;
	tse_GetSearchState_REQ req;
	ONVIF_RET ret;

	

	p_GetSearchState = _node_soap_get(p_body, "GetSearchState");
	assert(p_GetSearchState);

	memset(&req, 0, sizeof(req));

	ret = parse_tse_GetSearchState(p_GetSearchState, &req);
	if (ONVIF_OK == ret)
	{
		tse_GetSearchState_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_tse_GetSearchState(&req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtse_GetSearchState_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Recording;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_trc_CreateRecording(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_CreateRecording;
	trc_CreateRecording_REQ req;

	

	p_CreateRecording = _node_soap_get(p_body, "CreateRecording");
	assert(p_CreateRecording);

	memset(&req, 0, sizeof(req));

	ret = parse_trc_CreateRecording(p_CreateRecording, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_trc_CreateRecording(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_CreateRecording_rly_xml, req.RecordingToken, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_DeleteRecording(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_DeleteRecording;
	XMLN * p_RecordingToken;
	ONVIF_RET ret = ONVIF_ERR_NoRecording;

	

	p_DeleteRecording = _node_soap_get(p_body, "DeleteRecording");
	assert(p_DeleteRecording);

	p_RecordingToken = _node_soap_get(p_DeleteRecording, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		ret = onvif_trc_DeleteRecording(p_RecordingToken->data);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_DeleteRecording_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_GetRecordings(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtrc_GetRecordings_rly_xml, NULL, NULL, p_header);
}

int soap_trc_SetRecordingConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetRecordingConfiguration;
	trc_SetRecordingConfiguration_REQ req;

	

	p_SetRecordingConfiguration = _node_soap_get(p_body, "SetRecordingConfiguration");
	assert(p_SetRecordingConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_trc_SetRecordingConfiguration(p_SetRecordingConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_trc_SetRecordingConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_SetRecordingConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_GetRecordingConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetRecordingConfiguration;
	XMLN * p_RecordingToken;
	ONVIF_RET ret = ONVIF_ERR_NoRecording;

	

	p_GetRecordingConfiguration = _node_soap_get(p_body, "GetRecordingConfiguration");
	assert(p_GetRecordingConfiguration);

	p_RecordingToken = _node_soap_get(p_GetRecordingConfiguration, "RecordingToken");
	if (p_RecordingToken)
	{
	    if (p_RecordingToken->data)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrc_GetRecordingConfiguration_rly_xml, p_RecordingToken->data, NULL, p_header);
		}
		else
		{
		    return _bdsend_rly(p_user, rx_msg, bdtrc_GetRecordingConfiguration_rly_xml, "", NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_CreateTrack(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_CreateTrack;
	trc_CreateTrack_REQ req;

	

	p_CreateTrack = _node_soap_get(p_body, "CreateTrack");
	assert(p_CreateTrack);

	memset(&req, 0, sizeof(req));

	ret = parse_trc_CreateTrack(p_CreateTrack, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_trc_CreateTrack(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_CreateTrack_rly_xml, req.TrackToken, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_DeleteTrack(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_DeleteTrack;
	trc_DeleteTrack_REQ req;

	

	p_DeleteTrack = _node_soap_get(p_body, "DeleteTrack");
	assert(p_DeleteTrack);

	memset(&req, 0, sizeof(req));

	ret = parse_trc_DeleteTrack(p_DeleteTrack, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_trc_DeleteTrack(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_DeleteTrack_rly_xml, req.TrackToken, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_GetTrackConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_GetTrackConfiguration;
	trc_GetTrackConfiguration_REQ req;

	

	p_GetTrackConfiguration = _node_soap_get(p_body, "GetTrackConfiguration");
	assert(p_GetTrackConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_trc_GetTrackConfiguration(p_GetTrackConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrc_GetTrackConfiguration_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_SetTrackConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetTrackConfiguration;
	trc_SetTrackConfiguration_REQ req;

	

	p_SetTrackConfiguration = _node_soap_get(p_body, "SetTrackConfiguration");
	assert(p_SetTrackConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_trc_SetTrackConfiguration(p_SetTrackConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_trc_SetTrackConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_SetTrackConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_CreateRecordingJob(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_CreateRecordingJob;
	trc_CreateRecordingJob_REQ req;

	

	p_CreateRecordingJob = _node_soap_get(p_body, "CreateRecordingJob");
	assert(p_CreateRecordingJob);

	memset(&req, 0, sizeof(req));

	ret = parse_trc_CreateRecordingJob(p_CreateRecordingJob, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_trc_CreateRecordingJob(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_CreateRecordingJob_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_DeleteRecordingJob(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_DeleteRecordingJob;
	XMLN * p_JobToken;
	ONVIF_RET ret = ONVIF_ERR_NoRecordingJob;

	

	p_DeleteRecordingJob = _node_soap_get(p_body, "DeleteRecordingJob");
	assert(p_DeleteRecordingJob);

	p_JobToken = _node_soap_get(p_DeleteRecordingJob, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{
		ret = onvif_trc_DeleteRecordingJob(p_JobToken->data);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_DeleteRecordingJob_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_GetRecordingJobs(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtrc_GetRecordingJobs_rly_xml, NULL, NULL, p_header);
}

int soap_trc_SetRecordingJobConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetRecordingJobConfiguration;
	trc_SetRecordingJobConfiguration_REQ req;

	

	p_SetRecordingJobConfiguration = _node_soap_get(p_body, "SetRecordingJobConfiguration");
	assert(p_SetRecordingJobConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_trc_SetRecordingJobConfiguration(p_SetRecordingJobConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_trc_SetRecordingJobConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_SetRecordingJobConfiguration_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_GetRecordingJobConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetRecordingJobConfiguration;
	XMLN * p_JobToken;
	ONVIF_RET ret = ONVIF_ERR_NoRecordingJob;

	

	p_GetRecordingJobConfiguration = _node_soap_get(p_body, "GetRecordingJobConfiguration");
	assert(p_GetRecordingJobConfiguration);

	p_JobToken = _node_soap_get(p_GetRecordingJobConfiguration, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrc_GetRecordingJobConfiguration_rly_xml, p_JobToken->data, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_SetRecordingJobMode(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetRecordingJobMode;
	trc_SetRecordingJobMode_REQ req;

	

	p_SetRecordingJobMode = _node_soap_get(p_body, "SetRecordingJobMode");
	assert(p_SetRecordingJobMode);

	memset(&req, 0, sizeof(req));

	ret = parse_trc_SetRecordingJobMode(p_SetRecordingJobMode, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_trc_SetRecordingJobMode(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_SetRecordingJobMode_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_GetRecordingJobState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetRecordingJobState;
	XMLN * p_JobToken;
	ONVIF_RET ret = ONVIF_ERR_NoRecordingJob;

	

	p_GetRecordingJobState = _node_soap_get(p_body, "GetRecordingJobState");
	assert(p_GetRecordingJobState);

	p_JobToken = _node_soap_get(p_GetRecordingJobState, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{
		onvif_RecordingJobStateInformation state;
		memset(&state, 0, sizeof(state));

		ret = onvif_trc_GetRecordingJobState(p_JobToken->data, &state);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_GetRecordingJobState_rly_xml, (char*)&state, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trc_GetRecordingOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetRecordingOptions;
	XMLN * p_RecordingToken;
	ONVIF_RET ret = ONVIF_ERR_NoRecording;

	

	p_GetRecordingOptions = _node_soap_get(p_body, "GetRecordingOptions");
	assert(p_GetRecordingOptions);

	p_RecordingToken = _node_soap_get(p_GetRecordingOptions, "RecordingToken");
	if (p_RecordingToken)
	{
		onvif_RecordingOptions options;
		memset(&options, 0, sizeof(options));

		if (p_RecordingToken->data)
		{
		    ret = onvif_trc_GetRecordingOptions(p_RecordingToken->data, &options);
		}
		else
		{
		    ret = onvif_trc_GetRecordingOptions("", &options);
		}

		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrc_GetRecordingOptions_rly_xml, (char *)&options, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trp_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Replay;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_trp_GetReplayUri(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetReplayUri;
	trp_GetReplayUri_REQ req;
	ONVIF_RET ret;

	

	p_GetReplayUri = _node_soap_get(p_body, "GetReplayUri");
	assert(p_GetReplayUri);

	memset(&req, 0, sizeof(req));

	ret = parse_trp_GetReplayUri(p_GetReplayUri, &req);
	if (ONVIF_OK == ret)
	{
		trp_GetReplayUri_RES res;
		memset(&res, 0, sizeof(res));

		ret = onvif_trp_GetReplayUri(p_user->lip, p_user->lport, &req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrp_GetReplayUri_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trp_GetReplayConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	trp_GetReplayConfiguration_RES res;

	

	memset(&res, 0, sizeof(res));

	ret = onvif_trp_GetReplayConfiguration(&res);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtrp_GetReplayConfiguration_rly_xml, (char *)&res, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trp_SetReplayConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_SetReplayConfiguration;
	trp_SetReplayConfiguration_REQ req;
	ONVIF_RET ret;

	

	p_SetReplayConfiguration = _node_soap_get(p_body, "SetReplayConfiguration");
	assert(p_SetReplayConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_trp_SetReplayConfiguration(p_SetReplayConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_trp_SetReplayConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtrp_SetReplayConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif	// end of PROFILE_G_SUPPORT

#ifdef PROFILE_C_SUPPORT

int soap_tac_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_AccessControl;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tac_GetAccessPointList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAccessPointList;
	tac_GetAccessPointList_REQ req;
	ONVIF_RET ret;

	

	p_GetAccessPointList = _node_soap_get(p_body, "GetAccessPointList");
	assert(p_GetAccessPointList);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_GetAccessPointList(p_GetAccessPointList, &req);
	if (ONVIF_OK == ret)
	{
	    tac_GetAccessPointList_RES res;
	    memset(&res, 0, sizeof(res));

		ret = onvif_tac_GetAccessPointList(&req, &res);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtac_GetAccessPointList_rly_xml, (char *)&res, NULL, p_header);
			onvif_free_AccessPoints(&res.AccessPoint);
			return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_GetAccessPoints(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAccessPoints;
	tac_GetAccessPoints_REQ req;
	ONVIF_RET ret;

	

	p_GetAccessPoints = _node_soap_get(p_body, "GetAccessPoints");
	assert(p_GetAccessPoints);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_GetAccessPoints(p_GetAccessPoints, &req);
	if (ONVIF_OK == ret)
	{
        return _bdsend_rly(p_user, rx_msg, bdtac_GetAccessPoints_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_CreateAccessPoint(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_CreateAccessPoint;
	tac_CreateAccessPoint_REQ req;
	ONVIF_RET ret;

	

	p_CreateAccessPoint = _node_soap_get(p_body, "CreateAccessPoint");
	assert(p_CreateAccessPoint);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_CreateAccessPoint(p_CreateAccessPoint, &req);
	if (ONVIF_OK == ret)
	{
	    tac_CreateAccessPoint_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tac_CreateAccessPoint(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_CreateAccessPoint_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_SetAccessPoint(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_SetAccessPoint;
	tac_SetAccessPoint_REQ req;
	ONVIF_RET ret;

	

	p_SetAccessPoint = _node_soap_get(p_body, "SetAccessPoint");
	assert(p_SetAccessPoint);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_SetAccessPoint(p_SetAccessPoint, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tac_SetAccessPoint(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_SetAccessPoint_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_ModifyAccessPoint(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_ModifyAccessPoint;
	tac_ModifyAccessPoint_REQ req;
	ONVIF_RET ret;

	

	p_ModifyAccessPoint = _node_soap_get(p_body, "ModifyAccessPoint");
	assert(p_ModifyAccessPoint);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_ModifyAccessPoint(p_ModifyAccessPoint, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tac_ModifyAccessPoint(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_ModifyAccessPoint_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_DeleteAccessPoint(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_DeleteAccessPoint;
	tac_DeleteAccessPoint_REQ req;
	ONVIF_RET ret;

	

	p_DeleteAccessPoint = _node_soap_get(p_body, "DeleteAccessPoint");
	assert(p_DeleteAccessPoint);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_DeleteAccessPoint(p_DeleteAccessPoint, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tac_DeleteAccessPoint(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_DeleteAccessPoint_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_GetAccessPointInfoList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAccessPointInfoList;
	tac_GetAccessPointInfoList_REQ req;
	ONVIF_RET ret;

	

	p_GetAccessPointInfoList = _node_soap_get(p_body, "GetAccessPointInfoList");
	assert(p_GetAccessPointInfoList);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_GetAccessPointInfoList(p_GetAccessPointInfoList, &req);
	if (ONVIF_OK == ret)
	{
	    tac_GetAccessPointInfoList_RES res;
	    memset(&res, 0, sizeof(res));

		ret = onvif_tac_GetAccessPointInfoList(&req, &res);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtac_GetAccessPointInfoList_rly_xml, (char *)&res, NULL, p_header);
			onvif_free_AccessPoints(&res.AccessPointInfo);
			return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_GetAccessPointInfo(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAccessPointInfo;
	tac_GetAccessPointInfo_REQ req;
	ONVIF_RET ret;

	

	p_GetAccessPointInfo = _node_soap_get(p_body, "GetAccessPointInfo");
	assert(p_GetAccessPointInfo);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_GetAccessPointInfo(p_GetAccessPointInfo, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtac_GetAccessPointInfo_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_GetAreaList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAreaList;
	tac_GetAreaList_REQ req;
	ONVIF_RET ret;

	

	p_GetAreaList = _node_soap_get(p_body, "GetAreaList");
	assert(p_GetAreaList);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_GetAreaList(p_GetAreaList, &req);
	if (ONVIF_OK == ret)
	{
	    tac_GetAreaList_RES res;
	    memset(&res, 0, sizeof(res));

		ret = onvif_tac_GetAreaList(&req, &res);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtac_GetAreaList_rly_xml, (char *)&res, NULL, p_header);
			onvif_free_Areas(&res.Area);
			return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_GetAreas(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAreas;
	tac_GetAreas_REQ req;
	ONVIF_RET ret;

	

	p_GetAreas = _node_soap_get(p_body, "GetAreas");
	assert(p_GetAreas);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_GetAreas(p_GetAreas, &req);
	if (ONVIF_OK == ret)
	{
        return _bdsend_rly(p_user, rx_msg, bdtac_GetAreas_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_CreateArea(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_CreateArea;
	tac_CreateArea_REQ req;
	ONVIF_RET ret;

	

	p_CreateArea = _node_soap_get(p_body, "CreateArea");
	assert(p_CreateArea);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_CreateArea(p_CreateArea, &req);
	if (ONVIF_OK == ret)
	{
	    tac_CreateArea_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tac_CreateArea(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_CreateArea_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_SetArea(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_SetArea;
	tac_SetArea_REQ req;
	ONVIF_RET ret;

	

	p_SetArea = _node_soap_get(p_body, "SetArea");
	assert(p_SetArea);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_SetArea(p_SetArea, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tac_SetArea(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_SetArea_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_ModifyArea(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_ModifyArea;
	tac_ModifyArea_REQ req;
	ONVIF_RET ret;

	

	p_ModifyArea = _node_soap_get(p_body, "ModifyArea");
	assert(p_ModifyArea);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_ModifyArea(p_ModifyArea, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tac_ModifyArea(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_ModifyArea_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_DeleteArea(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_DeleteArea;
	tac_DeleteArea_REQ req;
	ONVIF_RET ret;

	

	p_DeleteArea = _node_soap_get(p_body, "DeleteArea");
	assert(p_DeleteArea);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_DeleteArea(p_DeleteArea, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tac_DeleteArea(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_DeleteArea_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_GetAreaInfoList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAreaInfoList;
	tac_GetAreaInfoList_REQ req;
	ONVIF_RET ret;

	

	p_GetAreaInfoList = _node_soap_get(p_body, "GetAreaInfoList");
	assert(p_GetAreaInfoList);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_GetAreaInfoList(p_GetAreaInfoList, &req);
	if (ONVIF_OK == ret)
	{
	    tac_GetAreaInfoList_RES res;
	    memset(&res, 0, sizeof(res));

		ret = onvif_tac_GetAreaInfoList(&req, &res);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtac_GetAreaInfoList_rly_xml, (char *)&res, NULL, p_header);
			onvif_free_Areas(&res.AreaInfo);
			return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_GetAreaInfo(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAreaInfo;
	tac_GetAreaInfo_REQ req;
	ONVIF_RET ret;

	

	p_GetAreaInfo = _node_soap_get(p_body, "GetAreaInfo");
	assert(p_GetAreaInfo);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_GetAreaInfo(p_GetAreaInfo, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtac_GetAreaInfo_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_GetAccessPointState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetAccessPointState;
	tac_GetAccessPointState_REQ req;
	ONVIF_RET ret;

	

	p_GetAccessPointState = _node_soap_get(p_body, "GetAccessPointState");
	assert(p_GetAccessPointState);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_GetAccessPointState(p_GetAccessPointState, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtac_GetAccessPointState_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_EnableAccessPoint(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_EnableAccessPoint;
	tac_EnableAccessPoint_REQ req;
	ONVIF_RET ret;

	

	p_EnableAccessPoint = _node_soap_get(p_body, "EnableAccessPoint");
	assert(p_EnableAccessPoint);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_EnableAccessPoint(p_EnableAccessPoint, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tac_EnableAccessPoint(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_EnableAccessPoint_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tac_DisableAccessPoint(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_DisableAccessPoint;
	tac_DisableAccessPoint_REQ req;
	ONVIF_RET ret;

	

	p_DisableAccessPoint = _node_soap_get(p_body, "DisableAccessPoint");
	assert(p_DisableAccessPoint);

	memset(&req, 0, sizeof(req));

	ret = parse_tac_DisableAccessPoint(p_DisableAccessPoint, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tac_DisableAccessPoint(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtac_DisableAccessPoint_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_DoorControl;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tdc_GetDoorList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetDoorList;
	tdc_GetDoorList_REQ req;
	ONVIF_RET ret;

	

	p_GetDoorList = _node_soap_get(p_body, "GetDoorList");
	assert(p_GetDoorList);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_GetDoorList(p_GetDoorList, &req);
	if (ONVIF_OK == ret)
	{
	    tdc_GetDoorList_RES res;
	    memset(&res, 0, sizeof(res));

		ret = onvif_tdc_GetDoorList(&req, &res);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtdc_GetDoorList_rly_xml, (char *)&res, NULL, p_header);
			onvif_free_Doors(&res.Door);
			return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_GetDoors(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetDoors;
	tdc_GetDoors_REQ req;
	ONVIF_RET ret;

	

	p_GetDoors = _node_soap_get(p_body, "GetDoors");
	assert(p_GetDoors);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_GetDoors(p_GetDoors, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtdc_GetDoors_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_CreateDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_CreateDoor;
	tdc_CreateDoor_REQ req;
	ONVIF_RET ret;

	

	p_CreateDoor = _node_soap_get(p_body, "CreateDoor");
	assert(p_CreateDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_CreateDoor(p_CreateDoor, &req);
	if (ONVIF_OK == ret)
	{
	    tdc_CreateDoor_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tdc_CreateDoor(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_CreateDoor_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_SetDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_SetDoor;
	tdc_SetDoor_REQ req;
	ONVIF_RET ret;

	

	p_SetDoor = _node_soap_get(p_body, "SetDoor");
	assert(p_SetDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_SetDoor(p_SetDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_SetDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_SetDoor_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_ModifyDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_ModifyDoor;
	tdc_ModifyDoor_REQ req;
	ONVIF_RET ret;

	

	p_ModifyDoor = _node_soap_get(p_body, "ModifyDoor");
	assert(p_ModifyDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_ModifyDoor(p_ModifyDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_ModifyDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_ModifyDoor_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_DeleteDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_DeleteDoor;
	tdc_DeleteDoor_REQ req;
	ONVIF_RET ret;

	

	p_DeleteDoor = _node_soap_get(p_body, "DeleteDoor");
	assert(p_DeleteDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_DeleteDoor(p_DeleteDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_DeleteDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_DeleteDoor_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_GetDoorInfoList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetDoorInfoList;
	tdc_GetDoorInfoList_REQ req;
	ONVIF_RET ret;

	

	p_GetDoorInfoList = _node_soap_get(p_body, "GetDoorInfoList");
	assert(p_GetDoorInfoList);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_GetDoorInfoList(p_GetDoorInfoList, &req);
	if (ONVIF_OK == ret)
	{
	    tdc_GetDoorInfoList_RES res;
	    memset(&res, 0, sizeof(res));

		ret = onvif_tdc_GetDoorInfoList(&req, &res);
		if (ONVIF_OK == ret)
		{
			ret = _bdsend_rly(p_user, rx_msg, bdtdc_GetDoorInfoList_rly_xml, (char *)&res, NULL, p_header);
			onvif_free_DoorInfos(&res.DoorInfo);
			return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_GetDoorInfo(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetDoorInfo;
	tdc_GetDoorInfo_REQ req;
	ONVIF_RET ret;

	

	p_GetDoorInfo = _node_soap_get(p_body, "GetDoorInfo");
	assert(p_GetDoorInfo);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_GetDoorInfo(p_GetDoorInfo, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtdc_GetDoorInfo_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_GetDoorState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetDoorState;
	tdc_GetDoorState_REQ req;
	ONVIF_RET ret;

	

	p_GetDoorState = _node_soap_get(p_body, "GetDoorState");
	assert(p_GetDoorState);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_GetDoorState(p_GetDoorState, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtdc_GetDoorState_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_AccessDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_AccessDoor;
	tdc_AccessDoor_REQ req;
	ONVIF_RET ret;

	

	p_AccessDoor = _node_soap_get(p_body, "AccessDoor");
	assert(p_AccessDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_AccessDoor(p_AccessDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_AccessDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_AccessDoor_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_LockDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_LockDoor;
	tdc_LockDoor_REQ req;
	ONVIF_RET ret;

	

	p_LockDoor = _node_soap_get(p_body, "LockDoor");
	assert(p_LockDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_LockDoor(p_LockDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_LockDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_LockDoor_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_UnlockDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_UnlockDoor;
	tdc_UnlockDoor_REQ req;
	ONVIF_RET ret;

	

	p_UnlockDoor = _node_soap_get(p_body, "UnlockDoor");
	assert(p_UnlockDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_UnlockDoor(p_UnlockDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_UnlockDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_UnlockDoor_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_DoubleLockDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_DoubleLockDoor;
	tdc_DoubleLockDoor_REQ req;
	ONVIF_RET ret;

	

	p_DoubleLockDoor = _node_soap_get(p_body, "DoubleLockDoor");
	assert(p_DoubleLockDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_DoubleLockDoor(p_DoubleLockDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_DoubleLockDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_DoubleLockDoor_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_BlockDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_BlockDoor;
	tdc_BlockDoor_REQ req;
	ONVIF_RET ret;

	

	p_BlockDoor = _node_soap_get(p_body, "BlockDoor");
	assert(p_BlockDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_BlockDoor(p_BlockDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_BlockDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_BlockDoor_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_LockDownDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_LockDownDoor;
	tdc_LockDownDoor_REQ req;
	ONVIF_RET ret;

	

	p_LockDownDoor = _node_soap_get(p_body, "LockDownDoor");
	assert(p_LockDownDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_LockDownDoor(p_LockDownDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_LockDownDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_LockDownDoor_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_LockDownReleaseDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_LockDownReleaseDoor;
	tdc_LockDownReleaseDoor_REQ req;
	ONVIF_RET ret;

	

	p_LockDownReleaseDoor = _node_soap_get(p_body, "LockDownReleaseDoor");
	assert(p_LockDownReleaseDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_LockDownReleaseDoor(p_LockDownReleaseDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_LockDownReleaseDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_LockDownReleaseDoor_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_LockOpenDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_LockOpenDoor;
	tdc_LockOpenDoor_REQ req;
	ONVIF_RET ret;

	

	p_LockOpenDoor = _node_soap_get(p_body, "LockOpenDoor");
	assert(p_LockOpenDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_LockOpenDoor(p_LockOpenDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_LockOpenDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_LockOpenDoor_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tdc_LockOpenReleaseDoor(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_LockOpenReleaseDoor;
	tdc_LockOpenReleaseDoor_REQ req;
	ONVIF_RET ret;

	

	p_LockOpenReleaseDoor = _node_soap_get(p_body, "LockOpenReleaseDoor");
	assert(p_LockOpenReleaseDoor);

	memset(&req, 0, sizeof(req));

	ret = parse_tdc_LockOpenReleaseDoor(p_LockOpenReleaseDoor, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tdc_LockOpenReleaseDoor(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtdc_LockOpenReleaseDoor_rly_xml, (char *)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // end of PROFILE_C_SUPPORT

#ifdef DEVICEIO_SUPPORT

int soap_tmd_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_DeviceIO;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tmd_GetVideoOutputs(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

    return _bdsend_rly(p_user, rx_msg, bdtmd_GetVideoOutputs_rly_xml, NULL, NULL, p_header);
}

int soap_tmd_GetVideoOutputConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetVideoOutputConfiguration;
	tmd_GetVideoOutputConfiguration_REQ req;
	ONVIF_RET ret;

	

	p_GetVideoOutputConfiguration = _node_soap_get(p_body, "GetVideoOutputConfiguration");
	assert(p_GetVideoOutputConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_GetVideoOutputConfiguration(p_GetVideoOutputConfiguration, &req);
	if (ONVIF_OK == ret)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtmd_GetVideoOutputConfiguration_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tmd_SetVideoOutputConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_SetVideoOutputConfiguration;
	tmd_SetVideoOutputConfiguration_REQ req;
	ONVIF_RET ret;

	

	p_SetVideoOutputConfiguration = _node_soap_get(p_body, "SetVideoOutputConfiguration");
	assert(p_SetVideoOutputConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_SetVideoOutputConfiguration(p_SetVideoOutputConfiguration, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tmd_SetVideoOutputConfiguration(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtmd_SetVideoOutputConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tmd_GetVideoOutputConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetVideoOutputConfigurationOptions;
	tmd_GetVideoOutputConfigurationOptions_REQ req;
	ONVIF_RET ret;

	

	p_GetVideoOutputConfigurationOptions = _node_soap_get(p_body, "GetVideoOutputConfigurationOptions");
	assert(p_GetVideoOutputConfigurationOptions);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_GetVideoOutputConfigurationOptions(p_GetVideoOutputConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtmd_GetVideoOutputConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}


int soap_tmd_GetDigitalInputConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetDigitalInputConfigurationOptions;
	tmd_GetDigitalInputConfigurationOptions_REQ req;
	ONVIF_RET ret;

	

	p_GetDigitalInputConfigurationOptions = _node_soap_get(p_body, "GetDigitalInputConfigurationOptions");
	assert(p_GetDigitalInputConfigurationOptions);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_GetDigitalInputConfigurationOptions(p_GetDigitalInputConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtmd_GetDigitalInputConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tmd_SetDigitalInputConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_SetDigitalInputConfigurations;
	tmd_SetDigitalInputConfigurations_REQ req;
	ONVIF_RET ret;

	

	p_SetDigitalInputConfigurations = _node_soap_get(p_body, "SetDigitalInputConfigurations");
	assert(p_SetDigitalInputConfigurations);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_SetDigitalInputConfigurations(p_SetDigitalInputConfigurations, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tmd_SetDigitalInputConfigurations(&req);
        onvif_free_DigitalInputs(&req.DigitalInputs);

	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtmd_SetDigitalInputConfigurations_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tmd_GetSerialPorts(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

	return _bdsend_rly(p_user, rx_msg, bdtmd_GetSerialPorts_rly_xml, NULL, NULL, p_header);
}

int soap_tmd_GetSerialPortConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetSerialPortConfiguration;
	tmd_GetSerialPortConfiguration_REQ req;
	ONVIF_RET ret;

	

	p_GetSerialPortConfiguration = _node_soap_get(p_body, "GetSerialPortConfiguration");
	assert(p_GetSerialPortConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_GetSerialPortConfiguration(p_GetSerialPortConfiguration, &req);
	if (ONVIF_OK == ret)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtmd_GetSerialPortConfiguration_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tmd_GetSerialPortConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_GetSerialPortConfigurationOptions;
	tmd_GetSerialPortConfigurationOptions_REQ req;
	ONVIF_RET ret;

	

	p_GetSerialPortConfigurationOptions = _node_soap_get(p_body, "GetSerialPortConfigurationOptions");
	assert(p_GetSerialPortConfigurationOptions);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_GetSerialPortConfigurationOptions(p_GetSerialPortConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
	    return _bdsend_rly(p_user, rx_msg, bdtmd_GetSerialPortConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tmd_SetSerialPortConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_SetSerialPortConfiguration;
	tmd_SetSerialPortConfiguration_REQ req;
	ONVIF_RET ret;

	

	p_SetSerialPortConfiguration = _node_soap_get(p_body, "SetSerialPortConfiguration");
	assert(p_SetSerialPortConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_SetSerialPortConfiguration(p_SetSerialPortConfiguration, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tmd_SetSerialPortConfiguration(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtmd_SetSerialPortConfiguration_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tmd_SendReceiveSerialCommand(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    XMLN * p_SendReceiveSerialCommand;
	tmd_SendReceiveSerialCommand_REQ req;
	ONVIF_RET ret;

	

	p_SendReceiveSerialCommand = _node_soap_get(p_body, "SendReceiveSerialCommand");
	assert(p_SendReceiveSerialCommand);

	memset(&req, 0, sizeof(req));

	ret = parse_tmd_SendReceiveSerialCommand(p_SendReceiveSerialCommand, &req);
	if (ONVIF_OK == ret)
	{
	    tmd_SendReceiveSerialCommand_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tmd_SendReceiveSerialCommandRx(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    ret = _bdsend_rly(p_user, rx_msg, bdtmd_SendReceiveSerialCommand_rly_xml, (char *)&res, NULL, p_header);
		}

        if (req.Command.SerialDataFlag)
        {
            onvif_free_SerialData(&req.Command.SerialData);
        }
        if (res.SerialDataFlag)
        {
            onvif_free_SerialData(&res.SerialData);
        }

		return ret;
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}


int soap_tmd_GetVideoSources(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    return _bdsend_rly(p_user, rx_msg, bdtmd_GetVideoSources_rly_xml, NULL, NULL, p_header);
}

#endif // end of DEVICEIO_SUPPORT

#ifdef MEDIA2_SUPPORT

int soap_tr2_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Media2;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tr2_GetProfiles(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetProfiles;
	tr2_GetProfiles_REQ req;

    

    p_GetProfiles = _node_soap_get(p_body, "GetProfiles");
    assert(p_GetProfiles);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_GetProfiles(p_GetProfiles, &req);
    if (ONVIF_OK == ret)
    {
    	return _bdsend_rly(p_user, rx_msg, bdtr2_GetProfiles_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_CreateProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_CreateProfile;
	tr2_CreateProfile_REQ req;

    

    p_CreateProfile = _node_soap_get(p_body, "CreateProfile");
    assert(p_CreateProfile);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_CreateProfile(p_CreateProfile, &req);
    if (ONVIF_OK == ret)
    {
        tr2_CreateProfile_RES res;
        memset(&res, 0, sizeof(res));

        ret = _CreateProfile(p_user->lip, p_user->lport, &req, &res);
        if (ONVIF_OK == ret)
        {
        	return _bdsend_rly(p_user, rx_msg, bdtr2_CreateProfile_rly_xml, (char *)&res, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_DeleteProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteProfile;
	tr2_DeleteProfile_REQ req;

    

    p_DeleteProfile = _node_soap_get(p_body, "DeleteProfile");
    assert(p_DeleteProfile);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_DeleteProfile(p_DeleteProfile, &req);
    if (ONVIF_OK == ret)
    {
        ret = _DeleteProfile(&req);
        if (ONVIF_OK == ret)
        {
        	return _bdsend_rly(p_user, rx_msg, bdtr2_DeleteProfile_rly_xml, (char *)&req, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_AddConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_AddConfiguration;
	tr2_AddConfiguration_REQ req;

    

    p_AddConfiguration = _node_soap_get(p_body, "AddConfiguration");
    assert(p_AddConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_AddConfiguration(p_AddConfiguration, &req);
    if (ONVIF_OK == ret)
    {
        ret = _AddConfiguration(&req);
        if (ONVIF_OK == ret)
        {
        	return _bdsend_rly(p_user, rx_msg, bdtr2_AddConfiguration_rly_xml, (char *)&req, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_RemoveConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_RemoveConfiguration;
	tr2_RemoveConfiguration_REQ req;

    

    p_RemoveConfiguration = _node_soap_get(p_body, "RemoveConfiguration");
    assert(p_RemoveConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_RemoveConfiguration(p_RemoveConfiguration, &req);
    if (ONVIF_OK == ret)
    {
        ret = _RemoveConfiguration(&req);
        if (ONVIF_OK == ret)
        {
        	return _bdsend_rly(p_user, rx_msg, bdtr2_RemoveConfiguration_rly_xml, (char *)&req, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetVideoEncoderConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetVideoEncoderConfigurations;
    tr2_GetVideoEncoderConfigurations_REQ req;


    memset(&req, 0, sizeof(req));

    p_GetVideoEncoderConfigurations = _node_soap_get(p_body, "GetVideoEncoderConfigurations");
    assert(p_GetVideoEncoderConfigurations);

    ret = parse_tr2_GetConfiguration(p_GetVideoEncoderConfigurations, &req.GetConfiguration);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtr2_GetVideoEncoderConfigurations_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetVideoSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetVideoSourceConfigurations;
    tr2_GetVideoSourceConfigurations_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetVideoSourceConfigurations = _node_soap_get(p_body, "GetVideoSourceConfigurations");
    assert(p_GetVideoSourceConfigurations);

    ret = parse_tr2_GetConfiguration(p_GetVideoSourceConfigurations, &req.GetConfiguration);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtr2_GetVideoSourceConfigurations_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetMetadataConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetMetadataConfigurations;
    tr2_GetMetadataConfigurations_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetMetadataConfigurations = _node_soap_get(p_body, "GetMetadataConfigurations");
    assert(p_GetMetadataConfigurations);

    ret = parse_tr2_GetConfiguration(p_GetMetadataConfigurations, &req.GetConfiguration);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtr2_GetMetadataConfigurations_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_SetVideoEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetVideoEncoderConfiguration;
	tr2_SetVideoEncoderConfiguration_REQ req;

    

    p_SetVideoEncoderConfiguration = _node_soap_get(p_body, "SetVideoEncoderConfiguration");
    assert(p_SetVideoEncoderConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_SetVideoEncoderConfiguration(p_SetVideoEncoderConfiguration, &req);
    if (ONVIF_OK == ret)
    {
        ret = _SetVideoEncoderConfiguration(&req);
        if (ONVIF_OK == ret)
        {
        	return _bdsend_rly(p_user, rx_msg, bdtr2_SetVideoEncoderConfiguration_rly_xml, NULL, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_SetVideoSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetVideoSourceConfiguration;
	tr2_SetVideoSourceConfiguration_REQ req;

    

    p_SetVideoSourceConfiguration = _node_soap_get(p_body, "SetVideoSourceConfiguration");
    assert(p_SetVideoSourceConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_SetVideoSourceConfiguration(p_SetVideoSourceConfiguration, &req);
    if (ONVIF_OK == ret)
    {
        ret = _SetVideoSourceConfiguration(&req);
        if (ONVIF_OK == ret)
        {
        	return _bdsend_rly(p_user, rx_msg, bdtr2_SetVideoSourceConfiguration_rly_xml, NULL, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_SetMetadataConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetMetadataConfiguration;
	tr2_SetMetadataConfiguration_REQ req;

    

    p_SetMetadataConfiguration = _node_soap_get(p_body, "SetMetadataConfiguration");
    assert(p_SetMetadataConfiguration);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_SetMetadataConfiguration(p_SetMetadataConfiguration, &req);
    if (ONVIF_OK == ret)
    {
        ret = _SetMetadataConfiguration(&req);
        if (ONVIF_OK == ret)
        {
        	return _bdsend_rly(p_user, rx_msg, bdtr2_SetMetadataConfiguration_rly_xml, NULL, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetVideoSourceConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetVideoSourceConfigurationOptions;
    tr2_GetVideoSourceConfigurationOptions_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetVideoSourceConfigurationOptions = _node_soap_get(p_body, "GetVideoSourceConfigurationOptions");
    assert(p_GetVideoSourceConfigurationOptions);

    ret = parse_tr2_GetConfiguration(p_GetVideoSourceConfigurationOptions, &req.GetConfiguration);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtr2_GetVideoSourceConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetVideoEncoderConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetVideoEncoderConfigurationOptions;
    tr2_GetVideoEncoderConfigurationOptions_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetVideoEncoderConfigurationOptions = _node_soap_get(p_body, "GetVideoEncoderConfigurationOptions");
    assert(p_GetVideoEncoderConfigurationOptions);

    ret = parse_tr2_GetConfiguration(p_GetVideoEncoderConfigurationOptions, &req.GetConfiguration);
    if (ONVIF_OK == ret)
    {
        tr2_GetVideoEncoderConfigurationOptions_RES res;
        memset(&res, 0, sizeof(res));

        ret = _GetVideoEncoderConfigurationOptions(&req, &res);
        if (ONVIF_OK == ret)
        {
            ret = _bdsend_rly(p_user, rx_msg, bdtr2_GetVideoEncoderConfigurationOptions_rly_xml, (char *)&res, NULL, p_header);

            onvif_free_VideoEncoder2ConfigurationOptions(&res.Options);
            return ret;
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetMetadataConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetMetadataConfigurationOptions;
    tr2_GetMetadataConfigurationOptions_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetMetadataConfigurationOptions = _node_soap_get(p_body, "GetMetadataConfigurationOptions");
    assert(p_GetMetadataConfigurationOptions);

    ret = parse_tr2_GetConfiguration(p_GetMetadataConfigurationOptions, &req.GetConfiguration);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtr2_GetMetadataConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetVideoEncoderInstances(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetVideoEncoderInstances;
    tr2_GetVideoEncoderInstances_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetVideoEncoderInstances = _node_soap_get(p_body, "GetVideoEncoderInstances");
    assert(p_GetVideoEncoderInstances);

    ret = parse_tr2_GetVideoEncoderInstances(p_GetVideoEncoderInstances, &req);
    if (ONVIF_OK == ret)
    {
        tr2_GetVideoEncoderInstances_RES res;
        memset(&res, 0, sizeof(res));

        ret = _GetVideoEncoderInstances(&req, &res);
        if (ONVIF_OK == ret)
        {
            return _bdsend_rly(p_user, rx_msg, bdtr2_GetVideoEncoderInstances_rly_xml,(char *)&res, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetStreamUri(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetStreamUri;
    tr2_GetStreamUri_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetStreamUri = _node_soap_get(p_body, "GetStreamUri");
    assert(p_GetStreamUri);

    ret = parse_tr2_GetStreamUri(p_GetStreamUri, &req);
    if (ONVIF_OK == ret)
    {
        tr2_GetStreamUri_RES res;
	    memset(&res, 0, sizeof(res));

		ret = _GetStreamUri(p_user->lip, p_user->lport, &req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtr2_GetStreamUri_rly_xml, (char *)&res, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_SetSynchronizationPoint(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_SetSynchronizationPoint;
    tr2_SetSynchronizationPoint_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_SetSynchronizationPoint = _node_soap_get(p_body, "SetSynchronizationPoint");
    assert(p_SetSynchronizationPoint);

    ret = parse_tr2_SetSynchronizationPoint(p_SetSynchronizationPoint, &req);
    if (ONVIF_OK == ret)
    {
        ret = _SetSynchronizationPoint(&req);
        if (ONVIF_OK == ret)
        {
            return _bdsend_rly(p_user, rx_msg, bdtr2_SetSynchronizationPoint_rly_xml, (char *)&req, NULL, p_header);
        }
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetVideoSourceModes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetVideoSourceModes;
    tr2_GetVideoSourceModes_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetVideoSourceModes = _node_soap_get(p_body, "GetVideoSourceModes");
    assert(p_GetVideoSourceModes);

    ret = parse_tr2_GetVideoSourceModes(p_GetVideoSourceModes, &req);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtr2_GetVideoSourceModes_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_SetVideoSourceMode(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_SetVideoSourceMode;
    tr2_SetVideoSourceMode_REQ req;

	

	p_SetVideoSourceMode = _node_soap_get(p_body, "SetVideoSourceMode");
    assert(p_SetVideoSourceMode);

	memset(&req, 0, sizeof(req));

    ret = parse_tr2_SetVideoSourceMode(p_SetVideoSourceMode, &req);
    if (ONVIF_OK == ret)
    {
        tr2_SetVideoSourceMode_RES res;
        memset(&res, 0, sizeof(res));

        ret = _SetVideoSourceMode(&req, &res);
    	if (ONVIF_OK == ret)
    	{
		    return _bdsend_rly(p_user, rx_msg, bdtr2_SetVideoSourceMode_rly_xml, (char*)&res, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetSnapshotUri(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetSnapshotUri;
    tr2_GetSnapshotUri_REQ req;

    

    p_GetSnapshotUri = _node_soap_get(p_body, "GetSnapshotUri");
    assert(p_GetSnapshotUri);

    memset(&req, 0, sizeof(req));

    ret = parse_tr2_GetSnapshotUri(p_GetSnapshotUri, &req);
	if (ONVIF_OK == ret)
	{
	    tr2_GetSnapshotUri_RES res;
	    memset(&res, 0, sizeof(res));

		int port = g_onvif_cfg.network.NetworkProtocol.HTTPPort[0];
		if(g_onvif_cfg.https_enable) {
			port = g_onvif_cfg.network.NetworkProtocol.HTTPSPort[0];
		}
		ret = _GetSnapshotUri(p_user->lip, port, &req, &res);
		//ret = _GetSnapshotUri(p_user->lip, p_user->lport, &req, &res);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtr2_GetSnapshotUri_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);

}

int soap_tr2_SetOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetOSD;
	trt_SetOSD_REQ req;

	

	p_SetOSD = _node_soap_get(p_body, "SetOSD");
	assert(p_SetOSD);

	memset(&req, 0, sizeof(req));

    ret = _trt_SetOSD(p_SetOSD, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_SetOSD(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtr2_SetOSD_rly_xml, NULL, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetOSDOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    

	return _bdsend_rly(p_user, rx_msg, bdtr2_GetOSDOptions_rly_xml, NULL, NULL, p_header);
}

int soap_tr2_GetOSDs(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetOSDs;
	tr2_GetOSDs_REQ req;

	

	p_GetOSDs = _node_soap_get(p_body, "GetOSDs");
	assert(p_GetOSDs);

	memset(&req, 0, sizeof(req));

    ret = parse_tr2_GetOSDs(p_GetOSDs, &req);
    if (ONVIF_OK == ret)
    {
		return _bdsend_rly(p_user, rx_msg, bdtr2_GetOSDs_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_CreateOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_CreateOSD;
	trt_CreateOSD_REQ req;

	

	p_CreateOSD = _node_soap_get(p_body, "CreateOSD");
	assert(p_CreateOSD);

	memset(&req, 0, sizeof(req));

    ret = _trt_CreateOSD(p_CreateOSD, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_CreateOSD(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtr2_CreateOSD_rly_xml, req.OSD.token, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_DeleteOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteOSD;
	trt_DeleteOSD_REQ req;

	

	p_DeleteOSD = _node_soap_get(p_body, "DeleteOSD");
	assert(p_DeleteOSD);

	memset(&req, 0, sizeof(req));

    ret = _trt_DeleteOSD(p_DeleteOSD, &req);
    if (ONVIF_OK == ret)
    {
    	ret = trt_DeleteOSD(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtr2_DeleteOSD_rly_xml, NULL, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_CreateMask(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_CreateMask;
	tr2_CreateMask_REQ req;

	

	p_CreateMask = _node_soap_get(p_body, "CreateMask");
	assert(p_CreateMask);

	memset(&req, 0, sizeof(req));

    ret = parse_tr2_CreateMask(p_CreateMask, &req);
    if (ONVIF_OK == ret)
    {
    	ret = _CreateMask(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtr2_CreateMask_rly_xml, req.Mask.token, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_DeleteMask(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteMask;
	tr2_DeleteMask_REQ req;

	

	p_DeleteMask = _node_soap_get(p_body, "DeleteMask");
	assert(p_DeleteMask);

	memset(&req, 0, sizeof(req));

    ret = parse_tr2_DeleteMask(p_DeleteMask, &req);
    if (ONVIF_OK == ret)
    {
    	ret = _DeleteMask(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtr2_DeleteMask_rly_xml, NULL, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}


int soap_tr2_StartMulticastStreaming(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_StartMulticastStreaming;
	tr2_StartMulticastStreaming_REQ req;

	

    p_StartMulticastStreaming = _node_soap_get(p_body, "StartMulticastStreaming");
	assert(p_StartMulticastStreaming);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_StartMulticastStreaming(p_StartMulticastStreaming, &req);
    if (ONVIF_OK == ret)
	{
		ret = _StartMulticastStreaming(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtr2_StartMulticastStreaming_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_StopMulticastStreaming(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_StopMulticastStreaming;
	tr2_StopMulticastStreaming_REQ req;

	

    p_StopMulticastStreaming = _node_soap_get(p_body, "StopMulticastStreaming");
	assert(p_StopMulticastStreaming);

	memset(&req, 0, sizeof(req));

	ret = parse_tr2_StopMulticastStreaming(p_StopMulticastStreaming, &req);
    if (ONVIF_OK == ret)
	{
		ret = _StopMulticastStreaming(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtr2_StopMulticastStreaming_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tr2_GetAudioSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetAudioSourceConfigurations;
    tr2_GetAudioSourceConfigurations_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetAudioSourceConfigurations = _node_soap_get(p_body, "GetAudioSourceConfigurations");
    assert(p_GetAudioSourceConfigurations);

    ret = parse_tr2_GetConfiguration(p_GetAudioSourceConfigurations, &req.GetConfiguration);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtr2_GetAudioSourceConfigurations_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}


int soap_tr2_GetAudioEncoderConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetAudioEncoderConfigurationOptions;
    tr2_GetAudioEncoderConfigurationOptions_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetAudioEncoderConfigurationOptions = _node_soap_get(p_body, "GetAudioEncoderConfigurationOptions");
    assert(p_GetAudioEncoderConfigurationOptions);

    ret = parse_tr2_GetConfiguration(p_GetAudioEncoderConfigurationOptions, &req.GetConfiguration);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtr2_GetAudioEncoderConfigurationOptions_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

#ifdef VIDEO_ANALYTICS

int soap_tr2_GetAnalyticsConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    XMLN * p_GetAnalyticsConfigurations;
    tr2_GetAnalyticsConfigurations_REQ req;

    

    memset(&req, 0, sizeof(req));

    p_GetAnalyticsConfigurations = _node_soap_get(p_body, "GetAnalyticsConfigurations");
    assert(p_GetAnalyticsConfigurations);

    ret = parse_tr2_GetConfiguration(p_GetAnalyticsConfigurations, &req.GetConfiguration);
    if (ONVIF_OK == ret)
    {
        return _bdsend_rly(p_user, rx_msg, bdtr2_GetAnalyticsConfigurations_rly_xml, (char *)&req, NULL, p_header);
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // end of VIDEO_ANALYTICS

#endif // end of MEDIA2_SUPPORT

#ifdef THERMAL_SUPPORT

int soap_tth_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Thermal;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tth_GetConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	

	return _bdsend_rly(p_user, rx_msg, bdtth_GetConfigurations_rly_xml, NULL, NULL, p_header);
}

int soap_tth_GetConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetConfiguration;
	tth_GetConfiguration_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetConfiguration = _node_soap_get(p_body, "GetConfiguration");
	assert(p_GetConfiguration);

	ret = parse_tth_GetConfiguration(p_GetConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtth_GetConfiguration_rly_xml, (char*)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tth_SetConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_SetConfiguration;
	tth_SetConfiguration_REQ req;

	

	p_SetConfiguration = _node_soap_get(p_body, "SetConfiguration");
	assert(p_SetConfiguration);

	memset(&req, 0, sizeof(req));

    ret = parse_tth_SetConfiguration(p_SetConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_tth_SetConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtth_SetConfiguration_rly_xml, NULL, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tth_GetConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_GetConfigurationOptions;
	tth_GetConfigurationOptions_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetConfigurationOptions = _node_soap_get(p_body, "GetConfigurationOptions");
	assert(p_GetConfigurationOptions);

	ret = parse_tth_GetConfigurationOptions(p_GetConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtth_GetConfigurationOptions_rly_xml, (char*)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tth_GetRadiometryConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetRadiometryConfiguration;
	tth_GetRadiometryConfiguration_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetRadiometryConfiguration = _node_soap_get(p_body, "GetRadiometryConfiguration");
	assert(p_GetRadiometryConfiguration);

	ret = parse_tth_GetRadiometryConfiguration(p_GetRadiometryConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtth_GetRadiometryConfiguration_rly_xml, (char*)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tth_SetRadiometryConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetRadiometryConfiguration;
	tth_SetRadiometryConfiguration_REQ req;

	

	p_SetRadiometryConfiguration = _node_soap_get(p_body, "SetRadiometryConfiguration");
	assert(p_SetRadiometryConfiguration);

	memset(&req, 0, sizeof(req));

    ret = parse_tth_SetRadiometryConfiguration(p_SetRadiometryConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_tth_SetRadiometryConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
			return _bdsend_rly(p_user, rx_msg, bdtth_SetRadiometryConfiguration_rly_xml, NULL, NULL, p_header);
		}
    }

    return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tth_GetRadiometryConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetRadiometryConfigurationOptions;
	tth_GetRadiometryConfigurationOptions_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetRadiometryConfigurationOptions = _node_soap_get(p_body, "GetRadiometryConfigurationOptions");
	assert(p_GetRadiometryConfigurationOptions);

	ret = parse_tth_GetRadiometryConfigurationOptions(p_GetRadiometryConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
		return _bdsend_rly(p_user, rx_msg, bdtth_GetRadiometryConfigurationOptions_rly_xml, (char*)&req, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // end of THERMAL_SUPPORT

#ifdef CREDENTIAL_SUPPORT

int soap_tcr_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Credential;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tcr_GetCredentialInfo(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetCredentialInfo;
	tcr_GetCredentialInfo_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetCredentialInfo = _node_soap_get(p_body, "GetCredentialInfo");
	assert(p_GetCredentialInfo);

	ret = parse_tcr_GetCredentialInfo(p_GetCredentialInfo, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetCredentialInfo_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetCredentialInfo(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_GetCredentialInfo_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_GetCredentialInfoList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetCredentialInfoList;
	tcr_GetCredentialInfoList_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetCredentialInfoList = _node_soap_get(p_body, "GetCredentialInfoList");
	assert(p_GetCredentialInfoList);

	ret = parse_tcr_GetCredentialInfoList(p_GetCredentialInfoList, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetCredentialInfoList_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetCredentialInfoList(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_GetCredentialInfoList_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_GetCredentials(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetCredentials;
	tcr_GetCredentials_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetCredentials = _node_soap_get(p_body, "GetCredentials");
	assert(p_GetCredentials);

	ret = parse_tcr_GetCredentials(p_GetCredentials, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetCredentials_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetCredentials(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_GetCredentials_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_GetCredentialList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetCredentialList;
	tcr_GetCredentialList_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetCredentialList = _node_soap_get(p_body, "GetCredentialList");
	assert(p_GetCredentialList);

	ret = parse_tcr_GetCredentialList(p_GetCredentialList, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetCredentialList_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetCredentialList(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_GetCredentialList_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_CreateCredential(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_CreateCredential;
	tcr_CreateCredential_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_CreateCredential = _node_soap_get(p_body, "CreateCredential");
	assert(p_CreateCredential);

	ret = parse_tcr_CreateCredential(p_CreateCredential, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_CreateCredential_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_CreateCredential(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_CreateCredential_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_ModifyCredential(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_ModifyCredential;
	tcr_ModifyCredential_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_ModifyCredential = _node_soap_get(p_body, "ModifyCredential");
	assert(p_ModifyCredential);

	ret = parse_tcr_ModifyCredential(p_ModifyCredential, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_ModifyCredential(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_ModifyCredential_rly_xml, (char*)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_DeleteCredential(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteCredential;
	tcr_DeleteCredential_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_DeleteCredential = _node_soap_get(p_body, "DeleteCredential");
	assert(p_DeleteCredential);

	ret = parse_tcr_DeleteCredential(p_DeleteCredential, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_DeleteCredential(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_DeleteCredential_rly_xml, (char*)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_GetCredentialState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetCredentialState;
	tcr_GetCredentialState_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetCredentialState = _node_soap_get(p_body, "GetCredentialState");
	assert(p_GetCredentialState);

	ret = parse_tcr_GetCredentialState(p_GetCredentialState, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetCredentialState_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetCredentialState(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_GetCredentialState_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_EnableCredential(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_EnableCredential;
	tcr_EnableCredential_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_EnableCredential = _node_soap_get(p_body, "EnableCredential");
	assert(p_EnableCredential);

	ret = parse_tcr_EnableCredential(p_EnableCredential, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_EnableCredential(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_EnableCredential_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_DisableCredential(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DisableCredential;
	tcr_DisableCredential_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_DisableCredential = _node_soap_get(p_body, "DisableCredential");
	assert(p_DisableCredential);

	ret = parse_tcr_DisableCredential(p_DisableCredential, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_DisableCredential(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_DisableCredential_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_SetCredential(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetCredential;
	tcr_SetCredential_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_SetCredential = _node_soap_get(p_body, "SetCredential");
	assert(p_SetCredential);

	ret = parse_tcr_SetCredential(p_SetCredential, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_SetCredential(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_SetCredential_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_ResetAntipassbackViolation(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_ResetAntipassbackViolation;
	tcr_ResetAntipassbackViolation_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_ResetAntipassbackViolation = _node_soap_get(p_body, "ResetAntipassbackViolation");
	assert(p_ResetAntipassbackViolation);

	ret = parse_tcr_ResetAntipassbackViolation(p_ResetAntipassbackViolation, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_ResetAntipassbackViolation(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_ResetAntipassbackViolation_rly_xml, (char*)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_GetSupportedFormatTypes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetSupportedFormatTypes;
	tcr_GetSupportedFormatTypes_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetSupportedFormatTypes = _node_soap_get(p_body, "GetSupportedFormatTypes");
	assert(p_GetSupportedFormatTypes);

	ret = parse_tcr_GetSupportedFormatTypes(p_GetSupportedFormatTypes, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetSupportedFormatTypes_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetSupportedFormatTypes(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_GetSupportedFormatTypes_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_GetCredentialIdentifiers(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetCredentialIdentifiers;
	tcr_GetCredentialIdentifiers_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetCredentialIdentifiers = _node_soap_get(p_body, "GetCredentialIdentifiers");
	assert(p_GetCredentialIdentifiers);

	ret = parse_tcr_GetCredentialIdentifiers(p_GetCredentialIdentifiers, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetCredentialIdentifiers_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetCredentialIdentifiers(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_GetCredentialIdentifiers_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_SetCredentialIdentifier(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetCredentialIdentifier;
	tcr_SetCredentialIdentifier_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_SetCredentialIdentifier = _node_soap_get(p_body, "SetCredentialIdentifier");
	assert(p_SetCredentialIdentifier);

	ret = parse_tcr_SetCredentialIdentifier(p_SetCredentialIdentifier, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_SetCredentialIdentifier(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_SetCredentialIdentifier_rly_xml, (char*)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_DeleteCredentialIdentifier(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteCredentialIdentifier;
	tcr_DeleteCredentialIdentifier_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_DeleteCredentialIdentifier = _node_soap_get(p_body, "DeleteCredentialIdentifier");
	assert(p_DeleteCredentialIdentifier);

	ret = parse_tcr_DeleteCredentialIdentifier(p_DeleteCredentialIdentifier, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_DeleteCredentialIdentifier(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_DeleteCredentialIdentifier_rly_xml, (char*)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_GetCredentialAccessProfiles(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetCredentialAccessProfiles;
	tcr_GetCredentialAccessProfiles_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetCredentialAccessProfiles = _node_soap_get(p_body, "GetCredentialAccessProfiles");
	assert(p_GetCredentialAccessProfiles);

	ret = parse_tcr_GetCredentialAccessProfiles(p_GetCredentialAccessProfiles, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetCredentialAccessProfiles_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetCredentialAccessProfiles(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_GetCredentialAccessProfiles_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_SetCredentialAccessProfiles(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetCredentialAccessProfiles;
	tcr_SetCredentialAccessProfiles_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_SetCredentialAccessProfiles = _node_soap_get(p_body, "SetCredentialAccessProfiles");
	assert(p_SetCredentialAccessProfiles);

	ret = parse_tcr_SetCredentialAccessProfiles(p_SetCredentialAccessProfiles, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_SetCredentialAccessProfiles(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_SetCredentialAccessProfiles_rly_xml, (char*)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_DeleteCredentialAccessProfiles(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteCredentialAccessProfiles;
	tcr_DeleteCredentialAccessProfiles_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_DeleteCredentialAccessProfiles = _node_soap_get(p_body, "DeleteCredentialAccessProfiles");
	assert(p_DeleteCredentialAccessProfiles);

	ret = parse_tcr_DeleteCredentialAccessProfiles(p_DeleteCredentialAccessProfiles, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_DeleteCredentialAccessProfiles(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_DeleteCredentialAccessProfiles_rly_xml, (char*)&req, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_GetWhitelist(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetWhitelist;
	tcr_GetWhitelist_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetWhitelist = _node_soap_get(p_body, "GetWhitelist");
	assert(p_GetWhitelist);

	ret = parse_tcr_GetWhitelist(p_GetWhitelist, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetWhitelist_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetWhitelist(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    ret = _bdsend_rly(p_user, rx_msg, bdtcr_GetWhitelist_rly_xml, (char*)&res, NULL, p_header);

            onvif_free_CredentialIdentifierItems(&res.Identifier);
		    return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_AddToWhitelist(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_AddToWhitelist;
	tcr_AddToWhitelist_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_AddToWhitelist = _node_soap_get(p_body, "AddToWhitelist");
	assert(p_AddToWhitelist);

	ret = parse_tcr_AddToWhitelist(p_AddToWhitelist, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_AddToWhitelist(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_AddToWhitelist_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_RemoveFromWhitelist(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_RemoveFromWhitelist;
	tcr_RemoveFromWhitelist_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_RemoveFromWhitelist = _node_soap_get(p_body, "RemoveFromWhitelist");
	assert(p_RemoveFromWhitelist);

	ret = parse_tcr_RemoveFromWhitelist(p_RemoveFromWhitelist, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_RemoveFromWhitelist(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_RemoveFromWhitelist_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_DeleteWhitelist(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteWhitelist;

	

	p_DeleteWhitelist = _node_soap_get(p_body, "DeleteWhitelist");
	assert(p_DeleteWhitelist);

    ret = onvif_tcr_DeleteWhitelist();
    if (ONVIF_OK == ret)
    {
	    return _bdsend_rly(p_user, rx_msg, bdtcr_DeleteWhitelist_rly_xml, NULL, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_GetBlacklist(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetBlacklist;
	tcr_GetBlacklist_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetBlacklist = _node_soap_get(p_body, "GetBlacklist");
	assert(p_GetBlacklist);

	ret = parse_tcr_GetBlacklist(p_GetBlacklist, &req);
	if (ONVIF_OK == ret)
	{
	    tcr_GetBlacklist_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tcr_GetBlacklist(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    ret = _bdsend_rly(p_user, rx_msg, bdtcr_GetBlacklist_rly_xml, (char*)&res, NULL, p_header);

            onvif_free_CredentialIdentifierItems(&res.Identifier);
		    return ret;
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_AddToBlacklist(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)

{
    ONVIF_RET ret;
	XMLN * p_AddToBlacklist;
	tcr_AddToBlacklist_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_AddToBlacklist = _node_soap_get(p_body, "AddToBlacklist");
	assert(p_AddToBlacklist);

	ret = parse_tcr_AddToBlacklist(p_AddToBlacklist, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_AddToBlacklist(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_AddToBlacklist_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_RemoveFromBlacklist(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_RemoveFromBlacklist;
	tcr_RemoveFromBlacklist_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_RemoveFromBlacklist = _node_soap_get(p_body, "RemoveFromBlacklist");
	assert(p_RemoveFromBlacklist);

	ret = parse_tcr_RemoveFromBlacklist(p_RemoveFromBlacklist, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tcr_RemoveFromBlacklist(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtcr_RemoveFromBlacklist_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tcr_DeleteBlacklist(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteBlacklist;

	

	p_DeleteBlacklist = _node_soap_get(p_body, "DeleteBlacklist");
	assert(p_DeleteBlacklist);

    ret = onvif_tcr_DeleteBlacklist();
    if (ONVIF_OK == ret)
    {
	    return _bdsend_rly(p_user, rx_msg, bdtcr_DeleteBlacklist_rly_xml, NULL, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // end of CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES

int soap_tar_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_AccessRules;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tar_GetAccessProfileInfo(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetAccessProfileInfo;
	tar_GetAccessProfileInfo_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetAccessProfileInfo = _node_soap_get(p_body, "GetAccessProfileInfo");
	assert(p_GetAccessProfileInfo);

	ret = parse_tar_GetAccessProfileInfo(p_GetAccessProfileInfo, &req);
	if (ONVIF_OK == ret)
	{
	    tar_GetAccessProfileInfo_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tar_GetAccessProfileInfo(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtar_GetAccessProfileInfo_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tar_GetAccessProfileInfoList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetAccessProfileInfoList;
	tar_GetAccessProfileInfoList_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetAccessProfileInfoList = _node_soap_get(p_body, "GetAccessProfileInfoList");
	assert(p_GetAccessProfileInfoList);

	ret = parse_tar_GetAccessProfileInfoList(p_GetAccessProfileInfoList, &req);
	if (ONVIF_OK == ret)
	{
	    tar_GetAccessProfileInfoList_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tar_GetAccessProfileInfoList(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtar_GetAccessProfileInfoList_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tar_GetAccessProfiles(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetAccessProfiles;
	tar_GetAccessProfiles_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetAccessProfiles = _node_soap_get(p_body, "GetAccessProfiles");
	assert(p_GetAccessProfiles);

	ret = parse_tar_GetAccessProfiles(p_GetAccessProfiles, &req);
	if (ONVIF_OK == ret)
	{
	    tar_GetAccessProfiles_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tar_GetAccessProfiles(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtar_GetAccessProfiles_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tar_GetAccessProfileList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetAccessProfileList;
	tar_GetAccessProfileList_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetAccessProfileList = _node_soap_get(p_body, "GetAccessProfileList");
	assert(p_GetAccessProfileList);

	ret = parse_tar_GetAccessProfileList(p_GetAccessProfileList, &req);
	if (ONVIF_OK == ret)
	{
	    tar_GetAccessProfileList_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tar_GetAccessProfileList(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtar_GetAccessProfileList_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tar_CreateAccessProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_CreateAccessProfile;
	tar_CreateAccessProfile_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_CreateAccessProfile = _node_soap_get(p_body, "CreateAccessProfile");
	assert(p_CreateAccessProfile);

	ret = parse_tar_CreateAccessProfile(p_CreateAccessProfile, &req);
	if (ONVIF_OK == ret)
	{
	    tar_CreateAccessProfile_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tar_CreateAccessProfile(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtar_CreateAccessProfile_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tar_ModifyAccessProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_ModifyAccessProfile;
	tar_ModifyAccessProfile_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_ModifyAccessProfile = _node_soap_get(p_body, "ModifyAccessProfile");
	assert(p_ModifyAccessProfile);

	ret = parse_tar_ModifyAccessProfile(p_ModifyAccessProfile, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tar_ModifyAccessProfile(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtar_ModifyAccessProfile_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tar_DeleteAccessProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteAccessProfile;
	tar_DeleteAccessProfile_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_DeleteAccessProfile = _node_soap_get(p_body, "DeleteAccessProfile");
	assert(p_DeleteAccessProfile);

	ret = parse_tar_DeleteAccessProfile(p_DeleteAccessProfile, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tar_DeleteAccessProfile(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtar_DeleteAccessProfile_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tar_SetAccessProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetAccessProfile;
	tar_SetAccessProfile_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_SetAccessProfile = _node_soap_get(p_body, "SetAccessProfile");
	assert(p_SetAccessProfile);

	ret = parse_tar_SetAccessProfile(p_SetAccessProfile, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tar_SetAccessProfile(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtar_SetAccessProfile_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}
#endif // end of ACCESS_RULES

#ifdef SCHEDULE_SUPPORT

int _GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Schedule;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int _GetScheduleInfo(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetScheduleInfo;
	tsc_GetScheduleInfo_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetScheduleInfo = _node_soap_get(p_body, "GetScheduleInfo");
	assert(p_GetScheduleInfo);

	ret = parse_tsc_GetScheduleInfo(p_GetScheduleInfo, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_GetScheduleInfo_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _GetScheduleInfo(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_GetScheduleInfo_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _GetScheduleInfoList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetScheduleInfoList;
	tsc_GetScheduleInfoList_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetScheduleInfoList = _node_soap_get(p_body, "GetScheduleInfoList");
	assert(p_GetScheduleInfoList);

	ret = parse_tsc_GetScheduleInfoList(p_GetScheduleInfoList, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_GetScheduleInfoList_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _GetScheduleInfoList(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_GetScheduleInfoList_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _GetSchedules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetSchedules;
	tsc_GetSchedules_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetSchedules = _node_soap_get(p_body, "GetSchedules");
	assert(p_GetSchedules);

	ret = parse_tsc_GetSchedules(p_GetSchedules, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_GetSchedules_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _GetSchedules(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_GetSchedules_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _GetScheduleList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetScheduleList;
	tsc_GetScheduleList_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetScheduleList = _node_soap_get(p_body, "GetScheduleList");
	assert(p_GetScheduleList);

	ret = parse_tsc_GetScheduleList(p_GetScheduleList, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_GetScheduleList_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _GetScheduleList(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_GetScheduleList_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _CreateSchedule(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_CreateSchedule;
	tsc_CreateSchedule_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_CreateSchedule = _node_soap_get(p_body, "CreateSchedule");
	assert(p_CreateSchedule);

	ret = parse_tsc_CreateSchedule(p_CreateSchedule, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_CreateSchedule_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _CreateSchedule(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_CreateSchedule_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _ModifySchedule(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_ModifySchedule;
	tsc_ModifySchedule_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_ModifySchedule = _node_soap_get(p_body, "ModifySchedule");
	assert(p_ModifySchedule);

	ret = parse_tsc_ModifySchedule(p_ModifySchedule, &req);
	if (ONVIF_OK == ret)
	{
	    ret = _ModifySchedule(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_ModifySchedule_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _DeleteSchedule(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteSchedule;
	tsc_DeleteSchedule_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_DeleteSchedule = _node_soap_get(p_body, "DeleteSchedule");
	assert(p_DeleteSchedule);

	ret = parse_tsc_DeleteSchedule(p_DeleteSchedule, &req);
	if (ONVIF_OK == ret)
	{
	    ret = _DeleteSchedule(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_DeleteSchedule_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _GetSpecialDayGroupInfo(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetSpecialDayGroupInfo;
	tsc_GetSpecialDayGroupInfo_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetSpecialDayGroupInfo = _node_soap_get(p_body, "GetSpecialDayGroupInfo");
	assert(p_GetSpecialDayGroupInfo);

	ret = parse_tsc_GetSpecialDayGroupInfo(p_GetSpecialDayGroupInfo, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_GetSpecialDayGroupInfo_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _GetSpecialDayGroupInfo(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_GetSpecialDayGroupInfo_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _GetSpecialDayGroupInfoList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetSpecialDayGroupInfoList;
	tsc_GetSpecialDayGroupInfoList_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetSpecialDayGroupInfoList = _node_soap_get(p_body, "GetSpecialDayGroupInfoList");
	assert(p_GetSpecialDayGroupInfoList);

	ret = parse_tsc_GetSpecialDayGroupInfoList(p_GetSpecialDayGroupInfoList, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_GetSpecialDayGroupInfoList_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _GetSpecialDayGroupInfoList(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_GetSpecialDayGroupInfoList_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _GetSpecialDayGroups(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetSpecialDayGroups;
	tsc_GetSpecialDayGroups_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetSpecialDayGroups = _node_soap_get(p_body, "GetSpecialDayGroups");
	assert(p_GetSpecialDayGroups);

	ret = parse_tsc_GetSpecialDayGroups(p_GetSpecialDayGroups, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_GetSpecialDayGroups_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _GetSpecialDayGroups(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_GetSpecialDayGroups_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _GetSpecialDayGroupList(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetSpecialDayGroupList;
	tsc_GetSpecialDayGroupList_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetSpecialDayGroupList = _node_soap_get(p_body, "GetSpecialDayGroupList");
	assert(p_GetSpecialDayGroupList);

	ret = parse_tsc_GetSpecialDayGroupList(p_GetSpecialDayGroupList, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_GetSpecialDayGroupList_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _GetSpecialDayGroupList(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_GetSpecialDayGroupList_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _CreateSpecialDayGroup(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_CreateSpecialDayGroup;
	tsc_CreateSpecialDayGroup_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_CreateSpecialDayGroup = _node_soap_get(p_body, "CreateSpecialDayGroup");
	assert(p_CreateSpecialDayGroup);

	ret = parse_tsc_CreateSpecialDayGroup(p_CreateSpecialDayGroup, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_CreateSpecialDayGroup_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _CreateSpecialDayGroup(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_CreateSpecialDayGroup_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _ModifySpecialDayGroup(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_ModifySpecialDayGroup;
	tsc_ModifySpecialDayGroup_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_ModifySpecialDayGroup = _node_soap_get(p_body, "ModifySpecialDayGroup");
	assert(p_ModifySpecialDayGroup);

	ret = parse_tsc_ModifySpecialDayGroup(p_ModifySpecialDayGroup, &req);
	if (ONVIF_OK == ret)
	{
	    ret = _ModifySpecialDayGroup(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_ModifySpecialDayGroup_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _DeleteSpecialDayGroup(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteSpecialDayGroup;
	tsc_DeleteSpecialDayGroup_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_DeleteSpecialDayGroup = _node_soap_get(p_body, "DeleteSpecialDayGroup");
	assert(p_DeleteSpecialDayGroup);

	ret = parse_tsc_DeleteSpecialDayGroup(p_DeleteSpecialDayGroup, &req);
	if (ONVIF_OK == ret)
	{
	    ret = _DeleteSpecialDayGroup(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_DeleteSpecialDayGroup_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _GetScheduleState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetScheduleState;
	tsc_GetScheduleState_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetScheduleState = _node_soap_get(p_body, "GetScheduleState");
	assert(p_GetScheduleState);

	ret = parse_tsc_GetScheduleState(p_GetScheduleState, &req);
	if (ONVIF_OK == ret)
	{
	    tsc_GetScheduleState_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = _GetScheduleState(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_GetScheduleState_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _SetSchedule(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetSchedule;
	tsc_SetSchedule_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_SetSchedule = _node_soap_get(p_body, "SetSchedule");
	assert(p_SetSchedule);

	ret = parse_tsc_SetSchedule(p_SetSchedule, &req);
	if (ONVIF_OK == ret)
	{
	    ret = _SetSchedule(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_SetSchedule_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int _SetSpecialDayGroup(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetSpecialDayGroup;
	tsc_SetSpecialDayGroup_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_SetSpecialDayGroup = _node_soap_get(p_body, "SetSpecialDayGroup");
	assert(p_SetSpecialDayGroup);

	ret = parse_tsc_SetSpecialDayGroup(p_SetSpecialDayGroup, &req);
	if (ONVIF_OK == ret)
	{
	    ret = _SetSpecialDayGroup(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtsc_SetSpecialDayGroup_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}
#endif // end of SCHEDULE_SUPPORT

#ifdef RECEIVER_SUPPORT

int soap_trv_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Receiver;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_trv_GetReceivers(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
    trv_GetReceivers_RES res;

    memset(&res, 0, sizeof(res));

    ret = onvif_trv_GetReceivers(&res);
    if (ONVIF_OK == ret)
    {
	    return _bdsend_rly(p_user, rx_msg, bdtrv_GetReceivers_rly_xml, (char*)&res, NULL, p_header);
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trv_GetReceiver(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetReceiver;
	trv_GetReceiver_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetReceiver = _node_soap_get(p_body, "GetReceiver");
	assert(p_GetReceiver);

	ret = parse_trv_GetReceiver(p_GetReceiver, &req);
	if (ONVIF_OK == ret)
	{
	    trv_GetReceiver_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_trv_GetReceiver(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrv_GetReceiver_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trv_CreateReceiver(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_CreateReceiver;
	trv_CreateReceiver_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_CreateReceiver = _node_soap_get(p_body, "CreateReceiver");
	assert(p_CreateReceiver);

	ret = parse_trv_CreateReceiver(p_CreateReceiver, &req);
	if (ONVIF_OK == ret)
	{
	    trv_CreateReceiver_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_trv_CreateReceiver(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrv_CreateReceiver_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trv_DeleteReceiver(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_DeleteReceiver;
	trv_DeleteReceiver_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_DeleteReceiver = _node_soap_get(p_body, "DeleteReceiver");
	assert(p_DeleteReceiver);

	ret = parse_trv_DeleteReceiver(p_DeleteReceiver, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_trv_DeleteReceiver(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrv_DeleteReceiver_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trv_ConfigureReceiver(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_ConfigureReceiver;
	trv_ConfigureReceiver_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_ConfigureReceiver = _node_soap_get(p_body, "ConfigureReceiver");
	assert(p_ConfigureReceiver);

	ret = parse_trv_ConfigureReceiver(p_ConfigureReceiver, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_trv_ConfigureReceiver(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrv_ConfigureReceiver_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trv_SetReceiverMode(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_SetReceiverMode;
	trv_SetReceiverMode_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_SetReceiverMode = _node_soap_get(p_body, "SetReceiverMode");
	assert(p_SetReceiverMode);

	ret = parse_trv_SetReceiverMode(p_SetReceiverMode, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_trv_SetReceiverMode(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrv_SetReceiverMode_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_trv_GetReceiverState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetReceiverState;
	trv_GetReceiverState_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetReceiverState = _node_soap_get(p_body, "GetReceiverState");
	assert(p_GetReceiverState);

	ret = parse_trv_GetReceiverState(p_GetReceiverState, &req);
	if (ONVIF_OK == ret)
	{
	    trv_GetReceiverState_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_trv_GetReceiverState(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtrv_GetReceiverState_rly_xml, (char*)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // end of RECEIVER_SUPPORT

#ifdef PROVISIONING_SUPPORT

int soap_tpv_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;

	

    p_GetServiceCapabilities = _node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);

    category = CapabilityCategory_Provisioning;

	return _bdsend_rly(p_user, rx_msg, bdGetServiceCapabilities_rly_xml, (char *)&category, NULL, p_header);
}

int soap_tpv_PanMove(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_PanMove;
	tpv_PanMove_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_PanMove = _node_soap_get(p_body, "PanMove");
	assert(p_PanMove);

	ret = parse_tpv_PanMove(p_PanMove, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tpv_PanMove(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtpv_PanMove_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tpv_TiltMove(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_TiltMove;
	tpv_TiltMove_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_TiltMove = _node_soap_get(p_body, "TiltMove");
	assert(p_TiltMove);

	ret = parse_tpv_TiltMove(p_TiltMove, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tpv_TiltMove(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtpv_TiltMove_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tpv_ZoomMove(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_ZoomMove;
	tpv_ZoomMove_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_ZoomMove = _node_soap_get(p_body, "ZoomMove");
	assert(p_ZoomMove);

	ret = parse_tpv_ZoomMove(p_ZoomMove, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tpv_ZoomMove(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtpv_ZoomMove_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tpv_RollMove(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_RollMove;
	tpv_RollMove_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_RollMove = _node_soap_get(p_body, "RollMove");
	assert(p_RollMove);

	ret = parse_tpv_RollMove(p_RollMove, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tpv_RollMove(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtpv_RollMove_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tpv_FocusMove(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_FocusMove;
	tpv_FocusMove_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_FocusMove = _node_soap_get(p_body, "FocusMove");
	assert(p_FocusMove);

	ret = parse_tpv_FocusMove(p_FocusMove, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_tpv_FocusMove(&req);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtpv_FocusMove_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tpv_GetUsage(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    ONVIF_RET ret;
	XMLN * p_GetUsage;
	tpv_GetUsage_REQ req;

	

    memset(&req, 0, sizeof(req));

	p_GetUsage = _node_soap_get(p_body, "GetUsage");
	assert(p_GetUsage);

	ret = parse_tpv_GetUsage(p_GetUsage, &req);
	if (ONVIF_OK == ret)
	{
	    tpv_GetUsage_RES res;
	    memset(&res, 0, sizeof(res));

	    ret = onvif_tpv_GetUsage(&req, &res);
	    if (ONVIF_OK == ret)
	    {
		    return _bdsend_rly(p_user, rx_msg, bdtpv_GetUsage_rly_xml, (char *)&res, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

int soap_tpv_Stop(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_Stop;
	ONVIF_RET ret;
	tpv_Stop_REQ req;

	

	p_Stop = _node_soap_get(p_body, "Stop");
	assert(p_Stop);

	memset(&req, 0, sizeof(req));

	ret = parse_tpv_Stop(p_Stop, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_tpv_Stop(&req);
		if (ONVIF_OK == ret)
		{
			return _bdsend_rly(p_user, rx_msg, bdtpv_Stop_rly_xml, NULL, NULL, p_header);
		}
	}

	return soap_bderr_rly(p_user, rx_msg, ret);
}

#endif // end of PROVISIONING_SUPPORT

/*********************************************************/
void soap_calc_digest(const char *created, uint8 *nonce, int noncelen, const char *password, uint8 hash[20])
{
	sha1_context ctx;

	sha1_starts(&ctx);
	sha1_update(&ctx, (uint8 *)nonce, noncelen);
	sha1_update(&ctx, (uint8 *)created, (uint32)strlen(created));
	sha1_update(&ctx, (uint8 *)password, (uint32)strlen(password));
	sha1_finish(&ctx, (uint8 *)hash);
}

BOOL soap_auth_process(XMLN * p_Security, int oplevel)
{
	return FALSE;
}

/**
 * if the request not need auth, return TRUE, or FALSE
 */
BOOL soap_IsReqNotNeedAuth(const char * request)
{
#ifdef PROFILE_Q_SUPPORT
    // A device shall provide full anonymous access to all ONVIF commands
    //  while the device operates in Factory Default State

    if (0 == g_onvif_cfg.device_state)
    {
        return TRUE;
    }
#endif

    if (soap_strcmp(request, "GetCapabilities") == 0)
    {
        return TRUE;
    }
    else if (soap_strcmp(request, "GetServices") == 0)
    {
        return TRUE;
    }
    else if (soap_strcmp(request, "GetServiceCapabilities") == 0)
    {
        return TRUE;
    }
    else if (soap_strcmp(request, "GetSystemDateAndTime") == 0)
    {
        return TRUE;
    }
    else if (soap_strcmp(request, "GetEndpointReference") == 0)
    {
        return TRUE;
    }
    else if (soap_strcmp(request, "GetWsdlUrl") == 0)
    {
        return TRUE;
    }
    else if (soap_strcmp(request, "GetHostname") == 0)
    {
        return TRUE;
    }

    return FALSE;
}

onvif_UserLevel soap_getUserOpLevel(const char * request)
{
    onvif_UserLevel level = UserLevel_User;

    if (soap_strcmp(request, "SetScopes") == 0)
    {
        level = UserLevel_Administrator;
    }
    else if (soap_strcmp(request, "SetDiscoveryMode") == 0)
    {
        level = UserLevel_Administrator;
    }
    else if (soap_strcmp(request, "GetAccessPolicy") == 0)
    {
        level = UserLevel_Administrator;
    }
    else if (soap_strcmp(request, "CreateUsers") == 0)
    {
        level = UserLevel_Administrator;
    }
    else if (soap_strcmp(request, "SetSystemDateAndTime") == 0)
    {
        level = UserLevel_Administrator;
    }

    return level;
}

BOOL soap_matchNamespace(XMLN * p_node, const char * ns, const char * prefix)
{
    XMLN * p_attr;

    p_attr = p_node->f_attrib;
	while (p_attr != NULL)
	{
		if (NTYPE_ATTRIB == p_attr->type)
		{
		    if (NULL == prefix)
		    {
    	        if (strcasecmp(p_attr->data, ns) == 0)
    	        {
    		        return TRUE;
    		    }
		    }
		    else
		    {
		        const char * ptr1;

		        ptr1 = strchr(p_attr->name, ':');
		        if (ptr1)
		        {
		            if (strcasecmp(ptr1+1, prefix) == 0 &&
		                strcasecmp(p_attr->data, ns) == 0)
		            {
		                return TRUE;
		            }
		        }
		    }
        }

		p_attr = p_attr->next;
	}

	return FALSE;
}

BOOL soap_isDeviceServiceNamespace(XMLN * p_body)
{
    const char * ptr1;
    XMLN * p_name = p_body->f_child;
    char prefix[32] = {'\0'};

    ptr1 = strchr(p_name->name, ':');
	if (ptr1)
	{
	    memcpy(prefix, p_name->name, ptr1 - p_name->name);
	}

    if (prefix[0] == '\0')
    {
        return soap_matchNamespace(p_name, "http://www.onvif.org/ver10/device/wsdl", NULL);
	}
    else
    {
        while (p_name)
        {
            if (soap_matchNamespace(p_name, "http://www.onvif.org/ver10/device/wsdl", prefix))
            {
                return TRUE;
            }

            p_name = p_name->parent;
        }
    }

	return FALSE;
}

int soap_process_device_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tds_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetDeviceInformation") == 0)
	{
		soap_tds_GetDeviceInformation(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetSystemUris") == 0)
	{
		soap_tds_GetSystemUris(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCapabilities") == 0)
	{
        soap_tds_GetCapabilities(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetSystemDateAndTime") == 0)
	{
		soap_tds_GetSystemDateAndTime(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetSystemDateAndTime") == 0)
	{
        soap_tds_SetSystemDateAndTime(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetNetworkInterfaces") == 0)
	{
		soap_tds_GetNetworkInterfaces(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetNetworkInterfaces") == 0)
	{
		soap_tds_SetNetworkInterfaces(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SystemReboot") == 0)
	{
		soap_tds_SystemReboot(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetSystemFactoryDefault") == 0)
	{
		soap_tds_SetSystemFactoryDefault(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetServices") == 0)
	{
		soap_tds_GetServices(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetScopes") == 0)
	{
		soap_tds_GetScopes(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "AddScopes") == 0)
	{
		soap_tds_AddScopes(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetScopes") == 0)
	{
		soap_tds_SetScopes(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "RemoveScopes") == 0)
	{
		soap_tds_RemoveScopes(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetHostname") == 0)
	{
		soap_tds_GetHostname(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetHostname") == 0)
	{
		soap_tds_SetHostname(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetHostnameFromDHCP") == 0)
	{
		soap_tds_SetHostnameFromDHCP(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetNetworkProtocols") == 0)
	{
		soap_tds_GetNetworkProtocols(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetNetworkProtocols") == 0)
	{
		soap_tds_SetNetworkProtocols(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetNetworkDefaultGateway") == 0)
	{
		soap_tds_GetNetworkDefaultGateway(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetNetworkDefaultGateway") == 0)
	{
		soap_tds_SetNetworkDefaultGateway(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetDiscoveryMode") == 0)
	{
		soap_tds_GetDiscoveryMode(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetDiscoveryMode") == 0)
	{
		soap_tds_SetDiscoveryMode(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetDNS") == 0)
	{
		soap_tds_GetDNS(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetNTP") == 0)
	{
		soap_tds_GetNTP(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetNTP") == 0)
	{
		soap_tds_SetNTP(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetZeroConfiguration") == 0)
	{
		soap_tds_GetZeroConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetZeroConfiguration") == 0)
	{
		soap_tds_SetZeroConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetDot11Capabilities") == 0)
	{
		soap_tds_GetDot11Capabilities(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetDot11Status") == 0)
	{
		soap_tds_GetDot11Status(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "ScanAvailableDot11Networks") == 0)
	{
		soap_tds_ScanAvailableDot11Networks(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetWsdlUrl") == 0)
	{
		soap_tds_GetWsdlUrl(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetEndpointReference") == 0)
	{
	    soap_tds_GetEndpointReference(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetUsers") == 0)
	{
		soap_tds_GetUsers(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreateUsers") == 0)
	{
		soap_tds_CreateUsers(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteUsers") == 0)
	{
		soap_tds_DeleteUsers(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetUser") == 0)
	{
		soap_tds_SetUser(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetRemoteUser") == 0)
	{
		soap_tds_GetRemoteUser(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetRemoteUser") == 0)
	{
		soap_tds_SetRemoteUser(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "StartSystemRestore") == 0)
	{
		soap_tds_StartSystemRestore(p_user, rx_msg, p_body, p_header);
	}
#ifdef DEVICEIO_SUPPORT
    else if (soap_strcmp(p_name, "SetRelayOutputSettings") == 0)
    {
        soap_tds_SetRelayOutputSettings(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetRelayOutputState") == 0)
    {
        soap_tds_SetRelayOutputState(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetDigitalInputConfigurationOptions") == 0)
    {
        soap_tmd_GetDigitalInputConfigurationOptions(p_user, rx_msg, p_body, p_header);
    }
	else if (soap_strcmp(p_name, "SetDigitalInputConfigurations") == 0)
	{
		soap_tmd_SetDigitalInputConfigurations(p_user, rx_msg, p_body, p_header);
	}
#endif //  DEVICEIO_SUPPORT
	else
	{
	    ret = 0;
	}

	return ret;
}

int soap_process_event_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
	int prev_port = 0;
    const char * p_name;
    p_name = p_body->f_child->name;

	prev_port = p_user->lport;
	p_user->lport = g_onvif_cfg.servs[0].web_port;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tev_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetEventProperties") == 0)
	{
		soap_tev_GetEventProperties(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "Subscribe") == 0)
	{
	    soap_tev_Subscribe(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "Unsubscribe") == 0)
	{
		soap_tev_Unsubscribe(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "Renew") == 0)
	{
		soap_tev_Renew(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreatePullPointSubscription") == 0)
	{
		soap_tev_CreatePullPointSubscription(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "PullMessages") == 0)
	{
		soap_tev_PullMessages(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetSynchronizationPoint") == 0)
	{
        soap_tev_SetSynchronizationPoint(p_user, rx_msg, p_body, p_header);
	}
	else
	{
	    ret = 0;
	}

	p_user->lport = prev_port;

    return ret;
}

#ifdef IMAGE_SUPPORT

int soap_process_image_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;
	
    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_img_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetImagingSettings") == 0)
	{
		soap_img_GetImagingSettings(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetImagingSettings") == 0)
	{
		soap_img_SetImagingSettings(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetOptions") == 0)
	{
		soap_img_GetOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetMoveOptions") == 0)
	{
		soap_img_GetMoveOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetStatus") == 0)
	{
		soap_img_GetStatus(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetPresets") == 0)
	{
		soap_img_GetPresets(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCurrentPreset") == 0)
	{
		soap_img_GetCurrentPreset(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetCurrentPreset") == 0)
	{
		soap_img_SetCurrentPreset(p_user, rx_msg, p_body, p_header);
	}
    else
	{
	    ret = 0;
	}

    return ret;
}

#endif // IMAGE_SUPPORT

#ifdef MEDIA_SUPPORT

int soap_process_media_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;


    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_trt_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetProfiles") == 0)
	{
		soap_trt_GetProfiles(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetProfile") == 0)
	{
		soap_trt_GetProfile(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreateProfile") == 0)
	{
		soap_trt_CreateProfile(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteProfile") == 0)
	{
		soap_trt_DeleteProfile(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoSourceModes") == 0)
	{
		soap_trt_GetVideoSourceModes(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetVideoSourceMode") == 0)
	{
		soap_trt_SetVideoSourceMode(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "AddVideoSourceConfiguration") == 0)
	{
		soap_trt_AddVideoSourceConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "RemoveVideoSourceConfiguration") == 0)
	{
		soap_trt_RemoveVideoSourceConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "AddVideoEncoderConfiguration") == 0)
	{
		soap_trt_AddVideoEncoderConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "RemoveVideoEncoderConfiguration") == 0)
	{
		soap_trt_RemoveVideoEncoderConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetStreamUri") == 0)
	{
		soap_trt_GetStreamUri(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoSources") == 0)
	{
	    soap_trt_GetVideoSources(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoEncoderConfigurations") == 0)
	{
		soap_trt_GetVideoEncoderConfigurations(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCompatibleVideoEncoderConfigurations") == 0)
	{
		soap_trt_GetCompatibleVideoEncoderConfigurations(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoSourceConfigurations") == 0)
	{
		soap_trt_GetVideoSourceConfigurations(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoSourceConfiguration") == 0)
	{
		soap_trt_GetVideoSourceConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoSourceConfigurationOptions") == 0)
	{
		soap_trt_GetVideoSourceConfigurationOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetVideoSourceConfiguration") == 0)
	{
		soap_trt_SetVideoSourceConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoEncoderConfiguration") == 0)
	{
	    soap_trt_GetVideoEncoderConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetVideoEncoderConfiguration") == 0)
	{
        soap_trt_SetVideoEncoderConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoEncoderConfigurationOptions") == 0)
	{
		soap_trt_GetVideoEncoderConfigurationOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCompatibleVideoSourceConfigurations") == 0)
	{
		soap_trt_GetCompatibleVideoSourceConfigurations(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetSynchronizationPoint") == 0)
	{
        soap_trt_SetSynchronizationPoint(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetGuaranteedNumberOfVideoEncoderInstances") == 0)
	{
		soap_trt_GetGuaranteedNumberOfVideoEncoderInstances(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetOSDs") == 0)
	{
		soap_trt_GetOSDs(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetOSD") == 0)
	{
		soap_trt_GetOSD(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetOSD") == 0)
	{
		soap_trt_SetOSD(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetOSDOptions") == 0)
	{
		soap_trt_GetOSDOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreateOSD") == 0)
	{
		soap_trt_CreateOSD(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteOSD") == 0)
	{
		soap_trt_DeleteOSD(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "StartMulticastStreaming") == 0)
	{
		soap_trt_StartMulticastStreaming(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "StopMulticastStreaming") == 0)
	{
		soap_trt_StopMulticastStreaming(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetMetadataConfigurations") == 0)
	{
		soap_trt_GetMetadataConfigurations(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetMetadataConfiguration") == 0)
	{
		soap_trt_GetMetadataConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCompatibleMetadataConfigurations") == 0)
	{
		soap_trt_GetCompatibleMetadataConfigurations(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetMetadataConfigurationOptions") == 0)
	{
		soap_trt_GetMetadataConfigurationOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetMetadataConfiguration") == 0)
	{
		soap_trt_SetMetadataConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "AddMetadataConfiguration") == 0)
	{
		soap_trt_AddMetadataConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "RemoveMetadataConfiguration") == 0)
	{
		soap_trt_RemoveMetadataConfiguration(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetAudioEncoderConfigurationOptions") == 0)
    {
        soap_trt_GetAudioEncoderConfigurationOptions(p_user, rx_msg, p_body, p_header);
    }
	#ifdef DEVICEIO_SUPPORT
	// else if (soap_strcmp(p_name, "GetAudioOutputConfigurations") == 0)
	// {
	//     soap_trt_GetAudioOutputConfigurations(p_user, rx_msg, p_body, p_header);
	// }
	// else if (soap_strcmp(p_name, "SetAudioOutputConfiguration") == 0)
	// {
	// 	soap_trt_SetAudioOutputConfiguration(p_user, rx_msg, p_body, p_header);
	// }
#endif // end of DEVICEIO_SUPPORT
#ifdef VIDEO_ANALYTICS
    else if (soap_strcmp(p_name, "GetVideoAnalyticsConfigurations") == 0)
    {
        soap_trt_GetVideoAnalyticsConfigurations(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "AddVideoAnalyticsConfiguration") == 0)
    {
        soap_trt_AddVideoAnalyticsConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetVideoAnalyticsConfiguration") == 0)
    {
        soap_trt_GetVideoAnalyticsConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "RemoveVideoAnalyticsConfiguration") == 0)
    {
        soap_trt_RemoveVideoAnalyticsConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetVideoAnalyticsConfiguration") == 0)
    {
        soap_trt_SetVideoAnalyticsConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAnalyticsConfigurations") == 0)
    {
        soap_trt_GetAnalyticsConfigurations(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetCompatibleVideoAnalyticsConfigurations") == 0)
    {
        soap_trt_GetCompatibleVideoAnalyticsConfigurations(p_user, rx_msg, p_body, p_header);
    }
#endif	// endif of VIDEO_ANALYTICS
    else if (soap_strcmp(p_name, "GetAudioSourceConfigurations") == 0)
    {
        soap_trt_GetAudioSourceConfigurations(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetCompatibleAudioSourceConfigurations") == 0)
    {
        soap_trt_GetCompatibleAudioSourceConfigurations(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAudioSourceConfigurationOptions") == 0)
    {
        soap_trt_GetAudioSourceConfigurationOptions(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAudioSourceConfiguration") == 0)
    {
        soap_trt_GetAudioSourceConfiguration(p_user, rx_msg, p_body, p_header);
    }


	else
	{
	    ret = 0;
	}

	return ret;
}

#endif // MEDIA_SUPPORT

#ifdef MEDIA2_SUPPORT

int soap_process_media2_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tr2_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetProfiles") == 0)
	{
        soap_tr2_GetProfiles(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreateProfile") == 0)
	{
        soap_tr2_CreateProfile(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteProfile") == 0)
	{
        soap_tr2_DeleteProfile(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoSourceModes") == 0)
	{
        soap_tr2_GetVideoSourceModes(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetVideoSourceMode") == 0)
	{
        soap_tr2_SetVideoSourceMode(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetStreamUri") == 0)
	{
        soap_tr2_GetStreamUri(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoEncoderConfigurations") == 0)
	{
        soap_tr2_GetVideoEncoderConfigurations(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoSourceConfigurations") == 0)
	{
        soap_tr2_GetVideoSourceConfigurations(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoSourceConfigurationOptions") == 0)
	{
        soap_tr2_GetVideoSourceConfigurationOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetVideoSourceConfiguration") == 0)
	{
        soap_tr2_SetVideoSourceConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetVideoEncoderConfiguration") == 0)
	{
        soap_tr2_SetVideoEncoderConfiguration(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetVideoEncoderConfigurationOptions") == 0)
	{
        soap_tr2_GetVideoEncoderConfigurationOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetSnapshotUri") == 0)
	{
        soap_tr2_GetSnapshotUri(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetSynchronizationPoint") == 0)
	{
        soap_tr2_SetSynchronizationPoint(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetOSDs") == 0)
	{
        soap_tr2_GetOSDs(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetOSD") == 0)
	{
        soap_tr2_SetOSD(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetOSDOptions") == 0)
	{
        soap_tr2_GetOSDOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreateOSD") == 0)
	{
        soap_tr2_CreateOSD(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteOSD") == 0)
	{
        soap_tr2_DeleteOSD(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetMetadataConfigurations") == 0)
	{
        soap_tr2_GetMetadataConfigurations(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetMetadataConfigurationOptions") == 0)
	{
        soap_tr2_GetMetadataConfigurationOptions(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetMetadataConfiguration") == 0)
	{
        soap_tr2_SetMetadataConfiguration(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "AddConfiguration") == 0)
    {
        soap_tr2_AddConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "RemoveConfiguration") == 0)
    {
        soap_tr2_RemoveConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetVideoEncoderInstances") == 0)
    {
        soap_tr2_GetVideoEncoderInstances(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "CreateMask") == 0)
    {
        soap_tr2_CreateMask(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteMask") == 0)
    {
        soap_tr2_DeleteMask(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "StartMulticastStreaming") == 0)
	{
		soap_tr2_StartMulticastStreaming(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "StopMulticastStreaming") == 0)
	{
		soap_tr2_StopMulticastStreaming(p_user, rx_msg, p_body, p_header);
	}

	else if (soap_strcmp(p_name, "GetAudioSourceConfigurations") == 0)
    {
        soap_tr2_GetAudioSourceConfigurations(p_user, rx_msg, p_body, p_header);
    }

    else if (soap_strcmp(p_name, "GetAudioEncoderConfigurationOptions") == 0)
    {
        soap_tr2_GetAudioEncoderConfigurationOptions(p_user, rx_msg, p_body, p_header);
    }
#ifdef VIDEO_ANALYTICS
    else if (soap_strcmp(p_name, "GetAnalyticsConfigurations") == 0)
    {
        soap_tr2_GetAnalyticsConfigurations(p_user, rx_msg, p_body, p_header);
    }
#endif	// endif of VIDEO_ANALYTICS
	else
	{
	    ret = 0;
	}

	return ret;
}

#endif // MEDIA2_SUPPORT

#ifdef DEVICEIO_SUPPORT

int soap_process_deviceio_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tmd_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetVideoSources") == 0)
	{
	    soap_tmd_GetVideoSources(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetVideoOutputs") == 0)
    {
        soap_tmd_GetVideoOutputs(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetVideoOutputConfiguration") == 0)
    {
        soap_tmd_GetVideoOutputConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetVideoOutputConfiguration") == 0)
    {
        soap_tmd_SetVideoOutputConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetVideoOutputConfigurationOptions") == 0)
    {
        soap_tmd_GetVideoOutputConfigurationOptions(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetDigitalInputConfigurationOptions") == 0)
    {
        soap_tmd_GetDigitalInputConfigurationOptions(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetDigitalInputConfigurations") == 0)
    {
        soap_tmd_SetDigitalInputConfigurations(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetSerialPorts") == 0)
    {
        soap_tmd_GetSerialPorts(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetSerialPortConfiguration") == 0)
    {
        soap_tmd_GetSerialPortConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetSerialPortConfigurationOptions") == 0)
    {
        soap_tmd_GetSerialPortConfigurationOptions(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetSerialPortConfiguration") == 0)
    {
        soap_tmd_SetSerialPortConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SendReceiveSerialCommand") == 0)
    {
        soap_tmd_SendReceiveSerialCommand(p_user, rx_msg, p_body, p_header);
    }
    else
	{
	    ret = 0;
	}

	return ret;
}

#endif // DEVICEIO_SUPPORT


#ifdef VIDEO_ANALYTICS

int soap_process_analytics_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tan_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetSupportedRules") == 0)
    {
        soap_tan_GetSupportedRules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "CreateRules") == 0)
    {
        soap_tan_CreateRules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteRules") == 0)
    {
        soap_tan_DeleteRules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRules") == 0)
    {
        soap_tan_GetRules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "ModifyRules") == 0)
    {
        soap_tan_ModifyRules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "CreateAnalyticsModules") == 0)
    {
        soap_tan_CreateAnalyticsModules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteAnalyticsModules") == 0)
    {
        soap_tan_DeleteAnalyticsModules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAnalyticsModules") == 0)
    {
        soap_tan_GetAnalyticsModules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "ModifyAnalyticsModules") == 0)
    {
        soap_tan_ModifyAnalyticsModules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRuleOptions") == 0)
    {
        soap_tan_GetRuleOptions(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetSupportedAnalyticsModules") == 0)
    {
        soap_tan_GetSupportedAnalyticsModules(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAnalyticsModuleOptions") == 0)
    {
        soap_tan_GetAnalyticsModuleOptions(p_user, rx_msg, p_body, p_header);
    }
    else
	{
	    ret = 0;
	}

	return ret;
}

#endif // VIDEO_ANALYTICS

#ifdef PROFILE_G_SUPPORT

int soap_process_recording_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_trc_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "CreateRecording") == 0)
    {
        soap_trc_CreateRecording(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteRecording") == 0)
    {
        soap_trc_DeleteRecording(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRecordings") == 0)
    {
        soap_trc_GetRecordings(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetRecordingConfiguration") == 0)
    {
        soap_trc_SetRecordingConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRecordingConfiguration") == 0)
    {
        soap_trc_GetRecordingConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "CreateTrack") == 0)
    {
        soap_trc_CreateTrack(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteTrack") == 0)
    {
        soap_trc_DeleteTrack(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetTrackConfiguration") == 0)
    {
        soap_trc_GetTrackConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetTrackConfiguration") == 0)
    {
        soap_trc_SetTrackConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "CreateRecordingJob") == 0)
    {
        soap_trc_CreateRecordingJob(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteRecordingJob") == 0)
    {
        soap_trc_DeleteRecordingJob(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRecordingJobs") == 0)
    {
        soap_trc_GetRecordingJobs(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetRecordingJobConfiguration") == 0)
    {
        soap_trc_SetRecordingJobConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRecordingJobConfiguration") == 0)
    {
        soap_trc_GetRecordingJobConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetRecordingJobMode") == 0)
    {
        soap_trc_SetRecordingJobMode(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRecordingJobState") == 0)
    {
        soap_trc_GetRecordingJobState(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRecordingOptions") == 0)
    {
        soap_trc_GetRecordingOptions(p_user, rx_msg, p_body, p_header);
    }
    else
	{
	    ret = 0;
	}

	return ret;
}

int soap_process_search_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tse_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetRecordingSummary") == 0)
    {
        soap_tse_GetRecordingSummary(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRecordingInformation") == 0)
    {
        soap_tse_GetRecordingInformation(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetMediaAttributes") == 0)
    {
        soap_tse_GetMediaAttributes(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "FindRecordings") == 0)
    {
        soap_tse_FindRecordings(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetRecordingSearchResults") == 0)
    {
        soap_tse_GetRecordingSearchResults(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "FindEvents") == 0)
    {
        soap_tse_FindEvents(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetEventSearchResults") == 0)
    {
        soap_tse_GetEventSearchResults(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "FindMetadata") == 0)
    {
        soap_tse_FindMetadata(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetMetadataSearchResults") == 0)
    {
        soap_tse_GetMetadataSearchResults(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "EndSearch") == 0)
    {
        soap_tse_EndSearch(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetSearchState") == 0)
    {
        soap_tse_GetSearchState(p_user, rx_msg, p_body, p_header);
    }
    else
	{
	    ret = 0;
	}

	return ret;
}

int soap_process_replay_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_trp_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetReplayUri") == 0)
    {
        soap_trp_GetReplayUri(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetReplayConfiguration") == 0)
    {
        soap_trp_GetReplayConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetReplayConfiguration") == 0)
    {
        soap_trp_SetReplayConfiguration(p_user, rx_msg, p_body, p_header);
    }
    else
	{
	    ret = 0;
	}

	return ret;
}

#endif // PROFILE_G_SUPPORT

#ifdef PROFILE_C_SUPPORT

int soap_process_accesscontrol_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tac_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetAccessPointList") == 0)
    {
        soap_tac_GetAccessPointList(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "CreateAccessPoint") == 0)
    {
        soap_tac_CreateAccessPoint(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetAccessPoint") == 0)
    {
        soap_tac_SetAccessPoint(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "ModifyAccessPoint") == 0)
    {
        soap_tac_ModifyAccessPoint(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteAccessPoint") == 0)
    {
        soap_tac_DeleteAccessPoint(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAccessPoints") == 0)
    {
        soap_tac_GetAccessPoints(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAccessPointInfoList") == 0)
    {
        soap_tac_GetAccessPointInfoList(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAccessPointInfo") == 0)
    {
        soap_tac_GetAccessPointInfo(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAreaList") == 0)
    {
        soap_tac_GetAreaList(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "CreateArea") == 0)
    {
        soap_tac_CreateArea(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetArea") == 0)
    {
        soap_tac_SetArea(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "ModifyArea") == 0)
    {
        soap_tac_ModifyArea(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteArea") == 0)
    {
        soap_tac_DeleteArea(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAreas") == 0)
    {
        soap_tac_GetAreas(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAreaInfoList") == 0)
    {
        soap_tac_GetAreaInfoList(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAreaInfo") == 0)
    {
        soap_tac_GetAreaInfo(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetAccessPointState") == 0)
    {
        soap_tac_GetAccessPointState(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "EnableAccessPoint") == 0)
    {
        soap_tac_EnableAccessPoint(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DisableAccessPoint") == 0)
    {
        soap_tac_DisableAccessPoint(p_user, rx_msg, p_body, p_header);
    }
    else
	{
	    ret = 0;
	}

	return ret;
}

int soap_process_doorcontrol_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tdc_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetDoorList") == 0)
    {
        soap_tdc_GetDoorList(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetDoors") == 0)
    {
        soap_tdc_GetDoors(p_user, rx_msg, p_body, p_header);
    }
	else if (soap_strcmp(p_name, "CreateDoor") == 0)
    {
        soap_tdc_CreateDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetDoor") == 0)
    {
        soap_tdc_SetDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "ModifyDoor") == 0)
    {
        soap_tdc_ModifyDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteDoor") == 0)
    {
        soap_tdc_DeleteDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetDoorInfoList") == 0)
    {
        soap_tdc_GetDoorInfoList(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetDoorInfo") == 0)
    {
        soap_tdc_GetDoorInfo(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetDoorState") == 0)
    {
        soap_tdc_GetDoorState(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "AccessDoor") == 0)
    {
        soap_tdc_AccessDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "LockDoor") == 0)
    {
        soap_tdc_LockDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "UnlockDoor") == 0)
    {
        soap_tdc_UnlockDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DoubleLockDoor") == 0)
    {
        soap_tdc_DoubleLockDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "BlockDoor") == 0)
    {
        soap_tdc_BlockDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "LockDownDoor") == 0)
    {
        soap_tdc_LockDownDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "LockDownReleaseDoor") == 0)
    {
        soap_tdc_LockDownReleaseDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "LockOpenDoor") == 0)
    {
        soap_tdc_LockOpenDoor(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "LockOpenReleaseDoor") == 0)
    {
        soap_tdc_LockOpenReleaseDoor(p_user, rx_msg, p_body, p_header);
    }
    else
	{
	    ret = 0;
	}

	return ret;
}

#endif // PROFILE_C_SUPPORT

#ifdef CREDENTIAL_SUPPORT

int soap_process_credential_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tcr_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetCredentialInfo") == 0)
	{
		soap_tcr_GetCredentialInfo(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCredentialInfoList") == 0)
	{
		soap_tcr_GetCredentialInfoList(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCredentials") == 0)
	{
		soap_tcr_GetCredentials(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCredentialList") == 0)
	{
		soap_tcr_GetCredentialList(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreateCredential") == 0)
	{
		soap_tcr_CreateCredential(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "ModifyCredential") == 0)
	{
		soap_tcr_ModifyCredential(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteCredential") == 0)
	{
		soap_tcr_DeleteCredential(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCredentialState") == 0)
	{
		soap_tcr_GetCredentialState(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "EnableCredential") == 0)
	{
		soap_tcr_EnableCredential(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DisableCredential") == 0)
	{
		soap_tcr_DisableCredential(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetCredential") == 0)
	{
		soap_tcr_SetCredential(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "ResetAntipassbackViolation") == 0)
	{
		soap_tcr_ResetAntipassbackViolation(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetSupportedFormatTypes") == 0)
	{
		soap_tcr_GetSupportedFormatTypes(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCredentialIdentifiers") == 0)
	{
		soap_tcr_GetCredentialIdentifiers(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetCredentialIdentifier") == 0)
	{
		soap_tcr_SetCredentialIdentifier(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteCredentialIdentifier") == 0)
	{
		soap_tcr_DeleteCredentialIdentifier(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetCredentialAccessProfiles") == 0)
	{
		soap_tcr_GetCredentialAccessProfiles(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetCredentialAccessProfiles") == 0)
	{
		soap_tcr_SetCredentialAccessProfiles(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteCredentialAccessProfiles") == 0)
	{
		soap_tcr_DeleteCredentialAccessProfiles(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetWhitelist") == 0)
	{
		soap_tcr_GetWhitelist(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "AddToWhitelist") == 0)
	{
		soap_tcr_AddToWhitelist(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "RemoveFromWhitelist") == 0)
	{
		soap_tcr_RemoveFromWhitelist(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteWhitelist") == 0)
	{
		soap_tcr_DeleteWhitelist(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetBlacklist") == 0)
	{
		soap_tcr_GetBlacklist(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "AddToBlacklist") == 0)
	{
		soap_tcr_AddToBlacklist(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "RemoveFromBlacklist") == 0)
	{
		soap_tcr_RemoveFromBlacklist(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteBlacklist") == 0)
	{
		soap_tcr_DeleteBlacklist(p_user, rx_msg, p_body, p_header);
	}
	else
	{
	    ret = 0;
	}

	return ret;
}

#endif // CREDENTIAL_SUPPORT

#ifdef ACCESS_RULES

int soap_process_accessrules_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_tar_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetAccessProfileInfo") == 0)
	{
		soap_tar_GetAccessProfileInfo(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetAccessProfileInfoList") == 0)
	{
		soap_tar_GetAccessProfileInfoList(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetAccessProfiles") == 0)
	{
		soap_tar_GetAccessProfiles(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetAccessProfileList") == 0)
	{
		soap_tar_GetAccessProfileList(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreateAccessProfile") == 0)
	{
		soap_tar_CreateAccessProfile(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "ModifyAccessProfile") == 0)
	{
		soap_tar_ModifyAccessProfile(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteAccessProfile") == 0)
	{
		soap_tar_DeleteAccessProfile(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetAccessProfile") == 0)
	{
		soap_tar_SetAccessProfile(p_user, rx_msg, p_body, p_header);
	}
	else
	{
	    ret = 0;
	}

	return ret;
}

#endif // ACCESS_RULES

#ifdef SCHEDULE_SUPPORT

int soap_process_schedule_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetScheduleInfo") == 0)
	{
		_GetScheduleInfo(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetScheduleInfoList") == 0)
	{
		_GetScheduleInfoList(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetSchedules") == 0)
	{
		_GetSchedules(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetScheduleList") == 0)
	{
		_GetScheduleList(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreateSchedule") == 0)
	{
		_CreateSchedule(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "ModifySchedule") == 0)
	{
		_ModifySchedule(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteSchedule") == 0)
	{
		_DeleteSchedule(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetSpecialDayGroupInfo") == 0)
	{
		_GetSpecialDayGroupInfo(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetSpecialDayGroupInfoList") == 0)
	{
		_GetSpecialDayGroupInfoList(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetSpecialDayGroups") == 0)
	{
		_GetSpecialDayGroups(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetSpecialDayGroupList") == 0)
	{
		_GetSpecialDayGroupList(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "CreateSpecialDayGroup") == 0)
	{
		_CreateSpecialDayGroup(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "ModifySpecialDayGroup") == 0)
	{
		_ModifySpecialDayGroup(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "DeleteSpecialDayGroup") == 0)
	{
		_DeleteSpecialDayGroup(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "GetScheduleState") == 0)
	{
		_GetScheduleState(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "SetSchedule") == 0)
	{
		_SetSchedule(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_name, "SetSpecialDayGroup") == 0)
	{
		_SetSpecialDayGroup(p_user, rx_msg, p_body, p_header);
	}
	else
	{
	    ret = 0;
	}

	return ret;
}

#endif // SCHEDULE_SUPPORT

#ifdef RECEIVER_SUPPORT

int soap_process_receiver_request(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
    int ret = 1;
    const char * p_name;

    p_name = p_body->f_child->name;

    if (soap_strcmp(p_name, "GetServiceCapabilities") == 0)
	{
		soap_trv_GetServiceCapabilities(p_user, rx_msg, p_body, p_header);
	}
    else if (soap_strcmp(p_name, "GetReceivers") == 0)
    {
        soap_trv_GetReceivers(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetReceiver") == 0)
    {
        soap_trv_GetReceiver(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "CreateReceiver") == 0)
    {
        soap_trv_CreateReceiver(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "DeleteReceiver") == 0)
    {
        soap_trv_DeleteReceiver(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "ConfigureReceiver") == 0)
    {
        soap_trv_ConfigureReceiver(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "SetReceiverMode") == 0)
    {
        soap_trv_SetReceiverMode(p_user, rx_msg, p_body, p_header);
    }
    else if (soap_strcmp(p_name, "GetReceiverState") == 0)
    {
        soap_trv_GetReceiverState(p_user, rx_msg, p_body, p_header);
    }
    else
	{
	    ret = 0;
	}

	return ret;
}

#endif // RECEIVER_SUPPORT


void process_request(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    int errcode = 401;
	BOOL auth = FALSE;
	char * p_xml;
	XMLN * p_node;
	XMLN * p_header;
	XMLN * p_body;
    char * p_post;
    onvif_UserLevel oplevel = UserLevel_Anonymous;

	p_xml = http_get_ctt(rx_msg);
	if (NULL == p_xml)
	{
		return;
	}


	p_node = _hxml_parse(p_xml, (int)strlen(p_xml));
	if (NULL == p_node || NULL == p_node->name)
	{
		return;
	}

	if (soap_strcmp(p_node->name, "Envelope") != 0)
	{
		_node_del(p_node);
		return;
	}

    p_body = _node_soap_get(p_node, "Body");
	if (NULL == p_body)
	{
		_node_del(p_node);
		return;
	}

	if (NULL == p_body->f_child)
	{
		_node_del(p_node);
		return;
	}
	else if (NULL == p_body->f_child->name)
	{
		_node_del(p_node);
		return;
	}
	else
	{
	}


    p_post = rx_msg->first_line.value_string;


    if (strstr(p_post, "event"))
    {
    	if (soap_process_event_request(p_user, rx_msg, p_body, p_header))
    	{
    	    _node_del(p_node);
            return;
    	}
	}
    else if (strstr(p_post, "device"))
    {
    	if (soap_process_device_request(p_user, rx_msg, p_body, p_header))
    	{
    	    _node_del(p_node);
            return;
    	}
	} 
#ifdef IMAGE_SUPPORT
	else if (strstr(p_post, "image"))
    {
    	if (soap_process_image_request(p_user, rx_msg, p_body, p_header))
    	{
    	    _node_del(p_node);
            return;
    	}
	}
#endif
#ifdef MEDIA2_SUPPORT
    else if (strstr(p_post, "media2"))
    {
        if (soap_process_media2_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
#endif
#ifdef MEDIA_SUPPORT
    else if (strstr(p_post, "media"))
    {
        if (soap_process_media_request(p_user, rx_msg, p_body, p_header))
    	{
    	    _node_del(p_node);
            return;
    	}
    }
#endif
#ifdef VIDEO_ANALYTICS
    else if (strstr(p_post, "analytics"))
    {
        if (soap_process_analytics_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
#endif
#ifdef PROFILE_G_SUPPORT
    else if (strstr(p_post, "recording"))
    {
        if (soap_process_recording_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
    else if (strstr(p_post, "search"))
    {
        if (soap_process_search_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
    else if (strstr(p_post, "replay"))
    {
        if (soap_process_replay_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
#endif
#ifdef PROFILE_C_SUPPORT
    else if (strstr(p_post, "accesscontrol"))
    {
        if (soap_process_accesscontrol_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
#endif
#ifdef CREDENTIAL_SUPPORT
    else if (strstr(p_post, "credential"))
    {
        if (soap_process_credential_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
#endif
#ifdef ACCESS_RULES
    else if (strstr(p_post, "accessrules"))
    {
        if (soap_process_accessrules_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
#endif
#ifdef SCHEDULE_SUPPORT
    else if (strstr(p_post, "schedule"))
    {
        if (soap_process_schedule_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
#endif
#ifdef RECEIVER_SUPPORT
    else if (strstr(p_post, "receiver"))
    {
        if (soap_process_receiver_request(p_user, rx_msg, p_body, p_header))
        {
            _node_del(p_node);
            return;
        }
    }
#endif

    soap_err_def_rly(p_user, rx_msg);
	_node_del(p_node);
}



