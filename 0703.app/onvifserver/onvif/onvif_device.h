

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "sys_inc.h"
#include "onvif.h"

/***************************************************************************************/
typedef struct 
{
    onvif_CapabilityCategory    Category;               // optional, List of categories to retrieve capability information on
} tds_GetCapabilities_REQ;

typedef struct
{
    BOOL    IncludeCapability;
} tds_GetServices_REQ;

typedef struct 
{
	onvif_DNSInformation	DNSInformation;			    // required, 
} tds_SetDNS_REQ;

typedef struct 
{
    onvif_DynamicDNSInformation DynamicDNSInformation;  // required, 
} tds_SetDynamicDNS_REQ;

typedef struct 
{
	onvif_NTPInformation	NTPInformation;			    // required, 
} tds_SetNTP_REQ;

typedef struct
{
	onvif_NetworkProtocol	NetworkProtocol;		    // required,  
} tds_SetNetworkProtocols_REQ;

typedef struct
{
	char 	IPv4Address[MAX_GATEWAY][32];			    // optional, Sets IPv4 gateway address used as default setting
} tds_SetNetworkDefaultGateway_REQ;

typedef struct
{
    onvif_SystemLogType LogType;	                    // required, Specifies the type of system log to get
} tds_GetSystemLog_REQ;

typedef struct
{
    char    String[2048];                               // optional, Contains the system log information
} tds_GetSystemLog_RES;

typedef struct
{	
	uint32	UTCDateTimeFlag	: 1;					    // Indicates whether the field UTCDateTime is valid
	uint32 	Reserved		: 31;
	
    onvif_SystemDateTime	SystemDateTime;			    // required,     
    onvif_DateTime 			UTCDateTime;			    // optional, Date and time in UTC. If time is obtained via NTP, UTCDateTime has no meaning
} tds_SetSystemDateAndTime_REQ;

typedef struct
{
    uint32  SystemLogUriFlag    : 1;
    uint32  AccessLogUriFlag    : 1;
    uint32  SupportInfoUriFlag  : 1;
    uint32  SystemBackupUriFlag : 1;
    uint32  Reserved            : 28;
    
    char    SystemLogUri[256];                          // optional
    char    AccessLogUri[256];                          // optional
	char    SupportInfoUri[256];	                    // optional
	char    SystemBackupUri[256];	                    // optional
} tds_GetSystemUris_RES;

typedef struct
{
	onvif_NetworkInterface	NetworkInterface;		    // required,  
} tds_SetNetworkInterfaces_REQ;

typedef struct
{
    onvif_FactoryDefaultType    FactoryDefault;	        // required, Specifies the factory default action type
} tds_SetSystemFactoryDefault_REQ;

typedef struct
{
	onvif_DiscoveryMode	DiscoveryMode;				    // required, Indicator of discovery mode: Discoverable, NonDiscoverable
} tds_SetDiscoveryMode_REQ;

typedef struct
{	
	char	UploadUri[256];							    // required, A URL to which the firmware file may be uploaded
	int		UploadDelay;							    // required, An optional delay; the client shall wait for this amount of time before initiating the firmware upload, unit is second
	int		ExpectedDownTime;						    // required, A duration that indicates how long the device expects to be unavailable after the firmware upload is complete, unit is second
} tds_StartFirmwareUpgrade_RES;

typedef struct 
{
    char    UploadUri[256];	                            // required
	int     ExpectedDownTime;	                        // required
} tds_StartSystemRestore_RES;

typedef struct
{
    onvif_NetworkZeroConfiguration ZeroConfiguration;   // Contains the zero-configuration
} tds_GetZeroConfiguration_RES;

typedef struct
{
    char    InterfaceToken[100];            // requied, Unique identifier referencing the physical interface
    BOOL    Enabled;                                    // requied, Specifies if the zero-configuration should be enabled or not
} tds_SetZeroConfiguration_REQ;

typedef struct 
{
    onvif_User User[MAX_USERS];
} tds_CreateUsers_REQ;

typedef struct 
{
    char    Username[MAX_USERS][64];
} tds_DeleteUsers_REQ;

typedef struct 
{
    onvif_User User[MAX_USERS];
} tds_SetUser_REQ;

typedef struct
{
    uint32	RemoteUserFlag	: 1;					    // Indicates whether the field RemoteUser is valid
	uint32 	Reserved		: 31;
	
    onvif_RemoteUser RemoteUser;                        // optional
} tds_GetRemoteUser_RES;

typedef struct
{
    uint32	RemoteUserFlag	: 1;					    // Indicates whether the field RemoteUser is valid
	uint32 	Reserved		: 31;
	
    onvif_RemoteUser RemoteUser;                       // optional     
} tds_SetRemoteUser_REQ;

typedef struct
{
	onvif_IPAddressFilter   IPAddressFilter;	        // required
} tds_GetIPAddressFilter_RES;

typedef struct 
{
	onvif_IPAddressFilter   IPAddressFilter;	        // required
} tds_SetIPAddressFilter_REQ;

typedef struct
{
	onvif_IPAddressFilter   IPAddressFilter;	        // required
} tds_AddIPAddressFilter_REQ;

typedef struct
{
	onvif_IPAddressFilter   IPAddressFilter;	        // required
} tds_RemoveIPAddressFilter_REQ;

typedef struct
{
    char    ScopeItem[100][128];
} tds_AddScopes_REQ;

typedef struct
{
    char    Scopes[100][128];
} tds_SetScopes_REQ;

typedef struct
{
    char    ScopeItem[100][128];
} tds_RemoveScopes_REQ;

typedef struct
{
    char    Name[100];	                                // required, The hostname to set
} tds_SetHostname_REQ;

typedef struct
{
    BOOL    FromDHCP;	                                // required, True if the hostname shall be obtained via DHCP
} tds_SetHostnameFromDHCP_REQ;

typedef struct 
{
	char    InterfaceToken[100];	        // required
} tds_GetDot11Status_REQ;

typedef struct 
{
	onvif_Dot11Status   Status;	                        // required
} tds_GetDot11Status_RES;

typedef struct 
{
	char    InterfaceToken[100];	        // required
} tds_ScanAvailableDot11Networks_REQ;

typedef struct 
{
    int     sizeNetworks;	                            // sequence of elements <Networks>
	onvif_Dot11AvailableNetworks Networks[10];
} tds_ScanAvailableDot11Networks_RES;


#ifdef __cplusplus
extern "C" {
#endif

ONVIF_RET onvif_tds_SetSystemDateAndTime(tds_SetSystemDateAndTime_REQ * p_req);
ONVIF_RET onvif_tds_SetHostname(tds_SetHostname_REQ * p_req);
ONVIF_RET onvif_tds_SetHostnameFromDHCP(tds_SetHostnameFromDHCP_REQ * p_req);
ONVIF_RET onvif_tds_SetNTP(tds_SetNTP_REQ * p_req);
ONVIF_RET onvif_tds_SetDynamicDNS(tds_SetDynamicDNS_REQ * p_req);
ONVIF_RET onvif_tds_SetZeroConfiguration(tds_SetZeroConfiguration_REQ * p_req);
ONVIF_RET onvif_tds_SetNetworkProtocols(tds_SetNetworkProtocols_REQ * p_req);
ONVIF_RET onvif_tds_GetSystemUris(const char * lip, uint32 lport, tds_GetSystemUris_RES * p_res);
ONVIF_RET onvif_tds_SetNetworkDefaultGateway(tds_SetNetworkDefaultGateway_REQ * p_req);
ONVIF_RET onvif_tds_SystemReboot();
ONVIF_RET onvif_tds_SetSystemFactoryDefault(tds_SetSystemFactoryDefault_REQ * p_req);
ONVIF_RET onvif_tds_SetNetworkInterfaces(tds_SetNetworkInterfaces_REQ * p_req);
ONVIF_RET onvif_tds_GetDot11Status(tds_GetDot11Status_REQ * p_req, tds_GetDot11Status_RES * p_res);
ONVIF_RET onvif_tds_ScanAvailableDot11Networks(tds_ScanAvailableDot11Networks_REQ * p_req, tds_ScanAvailableDot11Networks_RES * p_res);

ONVIF_RET onvif_tds_CreateUsers(tds_CreateUsers_REQ * p_req);
ONVIF_RET onvif_tds_DeleteUsers(tds_DeleteUsers_REQ * p_req);
ONVIF_RET onvif_tds_SetUser(tds_SetUser_REQ * p_req);
ONVIF_RET onvif_tds_GetRemoteUser(tds_GetRemoteUser_RES * p_res);
ONVIF_RET onvif_tds_SetRemoteUser(tds_SetRemoteUser_REQ * p_req);

ONVIF_RET onvif_tds_AddScopes(tds_AddScopes_REQ * p_req);
ONVIF_RET onvif_tds_SetScopes(tds_SetScopes_REQ * p_req);
ONVIF_RET onvif_tds_RemoveScopes(tds_RemoveScopes_REQ * p_req);

BOOL	  onvif_tds_StartFirmwareUpgrade(const char * lip, uint32 lport, tds_StartFirmwareUpgrade_RES * p_res);	
BOOL 	  onvif_tds_FirmwareUpgradeCheck(const char * buff, int len);
BOOL 	  onvif_tds_FirmwareUpgrade(const char * buff, int len);
void 	  onvif_tds_FirmwareUpgradePost();

BOOL      onvif_tds_StartSystemRestore(const char * lip, uint32 lport, tds_StartSystemRestore_RES * p_res);
BOOL      onvif_tds_SystemRestoreCheck(const char * buff, int len);
BOOL      onvif_tds_SystemRestore(const char * buff, int len);
void      onvif_tds_SystemRestorePost();


#ifdef __cplusplus
}
#endif

#endif // _DEVICE_H_

