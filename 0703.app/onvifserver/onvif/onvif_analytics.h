

#ifndef ONVIF_ANALYTICS_H
#define ONVIF_ANALYTICS_H


/***************************************************************************************/

typedef struct
{
	char 	ConfigurationToken[100];			// required, References an existing Video Analytics configuration. The list of available tokens can be obtained
															//	via the Media service GetVideoAnalyticsConfigurations method
} tan_GetSupportedRules_REQ;

typedef struct
{
	onvif_SupportedRules 	SupportedRules;					// required 
} tan_GetSupportedRules_RES;

typedef struct
{
	char 	ConfigurationToken[100];			// required, Reference to an existing VideoAnalyticsConfiguration

	ConfigList * Rule;									    // required
} tan_CreateRules_REQ;

typedef struct 
{
	char 	ConfigurationToken[100];			// required, Reference to an existing VideoAnalyticsConfiguration

	int 	sizeRuleName;
	char 	RuleName[10][100];					// required, References the specific rule to be deleted (e.g. "MyLineDetector"). 
} tan_DeleteRules_REQ;

typedef struct 
{
	char	ConfigurationToken[100];			// required, Reference to an existing VideoAnalyticsConfiguration
} tan_GetRules_REQ;

typedef struct 
{
	ConfigList * Rule;									    // optional
} tan_GetRules_RES;

typedef struct 
{
	char 	ConfigurationToken[100];			// required, Reference to an existing VideoAnalyticsConfiguration

	ConfigList * Rule;									    // required 
} tan_ModifyRules_REQ;

typedef struct 
{
	char 	ConfigurationToken[100];			// required, Reference to an existing VideoAnalyticsConfiguration

	ConfigList * AnalyticsModule;                           // required 
} tan_CreateAnalyticsModules_REQ;

typedef struct 
{
	char 	ConfigurationToken[100];			// required, Reference to an existing Video Analytics configuration

	int 	sizeAnalyticsModuleName;
	char 	AnalyticsModuleName[10][100];		//required, name of the AnalyticsModule to be deleted
} tan_DeleteAnalyticsModules_REQ;

typedef struct 
{
	char 	ConfigurationToken[100];			// required, Reference to an existing VideoAnalyticsConfiguration
} tan_GetAnalyticsModules_REQ;

typedef struct
{
	ConfigList * AnalyticsModule;                           // optional
} tan_GetAnalyticsModules_RES;

typedef struct 
{
	char 	ConfigurationToken[100];			// required, Reference to an existing VideoAnalyticsConfiguration
	
	ConfigList * AnalyticsModule;                           // required 
} tan_ModifyAnalyticsModules_REQ;

typedef struct 
{
	char    RuleType[100];	                                // optional, Reference to an SupportedRule Type returned from GetSupportedRules
	char    ConfigurationToken[100];	        // required, Reference to an existing analytics configuration
} tan_GetRuleOptions_REQ;

typedef struct 
{
    char    ConfigurationToken[100];	        // required, Reference to an existing VideoAnalyticsConfiguration
} tan_GetSupportedAnalyticsModules_REQ;

typedef struct
{
    char    Type[128];	                                    // required, Reference to an SupportedAnalyticsModule Type returned from GetSupportedAnalyticsModules
	char    ConfigurationToken[100];	        // required, Reference to an existing AnalyticsConfiguration
} tan_GetAnalyticsModuleOptions_REQ;


#ifdef __cplusplus
extern "C" {
#endif

ONVIF_RET onvif_tan_GetSupportedRules(tan_GetSupportedRules_REQ * p_req, tan_GetSupportedRules_RES * p_res);
ONVIF_RET onvif_tan_CreateRules(tan_CreateRules_REQ * p_req);
ONVIF_RET onvif_tan_DeleteRules(tan_DeleteRules_REQ * p_req);
ONVIF_RET onvif_tan_GetRules(tan_GetRules_REQ * p_req, tan_GetRules_RES * p_res);
ONVIF_RET onvif_tan_ModifyRules(tan_ModifyRules_REQ * p_req);
ONVIF_RET onvif_tan_CreateAnalyticsModules(tan_CreateAnalyticsModules_REQ * p_req);
ONVIF_RET onvif_tan_DeleteAnalyticsModules(tan_DeleteAnalyticsModules_REQ * p_req);
ONVIF_RET onvif_tan_GetAnalyticsModules(tan_GetAnalyticsModules_REQ * p_req, tan_GetAnalyticsModules_RES * p_res);
ONVIF_RET onvif_tan_ModifyAnalyticsModules(tan_ModifyAnalyticsModules_REQ * p_req);

#ifdef __cplusplus
}
#endif

#endif 	// end of ONVIF_ANALYTICS_H



