

#ifndef ONVIF_RECORDING_H
#define ONVIF_RECORDING_H

#include "sys_inc.h"
#include "onvif_cm.h"


#define SEARCH_TYPE_EVENTS      1
#define SEARCH_TYPE_RECORDING   2
#define SEARCH_TYPE_PTZPOS      3
#define SEARCH_TYPE_METADATA    4

typedef struct
{
    // if type is SEARCH_TYPE_EVENTS, req -> tse_FindEvents_REQ
    // if type is SEARCH_TYPE_RECORDING, req -> tse_FindRecordings_REQ
    // if type is SEARCH_TYPE_PTZPOS, req -> tse_FindPTZPosition_REQ
    // if type is SEARCH_TYPE_METADATA, req -> tse_FindMetadata_REQ
    
    int     type;                                               //  request type
    void *  req;                                                // request
    char    token[100];                             // request token
    time_t  reqtime;                                            // request time
} SEARCH_SUA;

typedef struct 
{
	char	RecordingToken[100];					// filled by onvif server, return to client
	
	onvif_RecordingConfiguration	RecordingConfiguration;		// required, Initial configuration for the recording
} trc_CreateRecording_REQ;

typedef struct
{
	char	RecordingToken[100];					// required, Token of the recording that shall be changed
	
	onvif_RecordingConfiguration	RecordingConfiguration;		// required, The new configuration
} trc_SetRecordingConfiguration_REQ;

typedef struct
{
	char	RecordingToken[100];					// required, Identifies the recording to which a track shall be added
	char	TrackToken[100];						// filled by onvif server, return to client
	
	onvif_TrackConfiguration	TrackConfiguration;				// required, The configuration of the new track
} trc_CreateTrack_REQ;

typedef struct
{
	char 	RecordingToken[100];					// required, Token of the recording the track belongs to
	char 	TrackToken[100];						// required, Token of the track to be deleted
} trc_DeleteTrack_REQ;

typedef struct
{
	char 	RecordingToken[100];					// required, Token of the recording the track belongs to
	char 	TrackToken[100];						// required, Token of the track
} trc_GetTrackConfiguration_REQ;

typedef struct
{
	char 	RecordingToken[100];					// required, Token of the recording the track belongs to
	char 	TrackToken[100];						// required, Token of the track to be modified
	
	onvif_TrackConfiguration	TrackConfiguration;				// required, New configuration for the track
} trc_SetTrackConfiguration_REQ;

typedef struct
{
	char	JobToken[100];							// filled by onvif server, return to client
	
	onvif_RecordingJobConfiguration	JobConfiguration;			// required, The initial configuration of the new recording job
} trc_CreateRecordingJob_REQ;

typedef struct
{
	char	JobToken[100];							// required, Token of the job to be modified
	
	onvif_RecordingJobConfiguration	JobConfiguration;			// required, New configuration of the recording job
} trc_SetRecordingJobConfiguration_REQ;

typedef struct
{
	char 	JobToken[100];							// required, Token of the recording job
	char 	Mode[16];											// required, The new mode for the recording job, The only valid values for Mode shall be "Idle" or "Active"
} trc_SetRecordingJobMode_REQ;

typedef struct 
{
	onvif_RecordingSummary	Summary;							// required, 
} tse_GetRecordingSummary_RES;

typedef struct
{
    char    RecordingToken[100];                    // required, 
} tse_GetRecordingInformation_REQ;

typedef struct
{
	onvif_RecordingInformation	RecordingInformation;			// required, 
} tse_GetRecordingInformation_RES;

typedef struct
{
	int 	sizeRecordingTokens;
	char 	RecordingTokens[10][100];				// optional, 
	
	time_t 	Time;												// required, 
} tse_GetMediaAttributes_REQ;

typedef struct
{
	int 	sizeMediaAttributes;	
	onvif_MediaAttributes	MediaAttributes[10];				// optional, 
} tse_GetMediaAttributes_RES;

typedef struct 
{
	uint32  MaxMatchesFlag	: 1;								// Indicates whether the field MaxMatches is valid
	uint32  Reserved		: 31;
	
	onvif_SearchScope	Scope;									// required, Scope defines the dataset to consider for this search
	
	int 	MaxMatches;											// optional, The search will be completed after this many matches. If not specified, the search will continue until reaching the endpoint or until the session expires
	int 	KeepAliveTime;										// required, The time the search session will be kept alive after responding to this and subsequent requests. A device shall support at least values up to ten seconds
} tse_FindRecordings_REQ;

typedef struct 
{
	char 	SearchToken[100];						// required
} tse_FindRecordings_RES;

typedef struct 
{
	uint32  MinResultsFlag	: 1;								// Indicates whether the field MinResults is valid
	uint32  MaxResultsFlag	: 1;								// Indicates whether the field MaxResults is valid
	uint32  WaitTimeFlag	: 1;								// Indicates whether the field WaitTime is valid
	uint32  Reserved		: 29;
	
	char 	SearchToken[100];						// required, The search session to get results from
	int 	MinResults;											// optional, The minimum number of results to return in one response
	int 	MaxResults;											// optional, The maximum number of results to return in one response
	int	 	WaitTime;											// optional, The maximum time before responding to the request, even if the MinResults parameter is not fulfilled
} tse_GetRecordingSearchResults_REQ;

typedef struct
{
	onvif_FindRecordingResultList	ResultList;					// required
} tse_GetRecordingSearchResults_RES;

typedef struct
{
	uint32  EndPointFlag		: 1;							// Indicates whether the field EndPoint is valid
	uint32  MaxMatchesFlag		: 1;							// Indicates whether the field MaxMatches is valid
	uint32  KeepAliveTimeFlag	: 1;							// Indicates whether the field KeepAliveTime is valid
	uint32  Reserved			: 29;
	
	time_t 	StartPoint;											// required, The point of time where the search will start
	time_t	EndPoint;											// optional, The point of time where the search will stop. This can be a time before the StartPoint, in which case the search is performed backwards in time

	onvif_SearchScope	Scope;									// required,

	BOOL 	IncludeStartState;									// required, 
	int 	MaxMatches;											// optional, The search will be completed after this many matches. If not specified, the search will continue until reaching the endpoint or until the session expires
	int	 	KeepAliveTime;										// optional, The time the search session will be kept alive after responding to this and subsequent requests. A device shall support at least values up to ten seconds
} tse_FindEvents_REQ;

typedef struct 
{
	char	SearchToken[100];						// required, A unique reference to the search session created by this request
} tse_FindEvents_RES;

typedef struct 
{
	uint32  MinResultsFlag	: 1;								// Indicates whether the field MinResults is valid
	uint32  MaxResultsFlag	: 1;								// Indicates whether the field MaxResults is valid
	uint32  WaitTimeFlag	: 1;								// Indicates whether the field WaitTime is valid
	uint32  Reserved		: 29;
	
	char 	SearchToken[100];						// required, The search session to get results from
	int 	MinResults;											// optional, The minimum number of results to return in one response
	int 	MaxResults;											// optional, The maximum number of results to return in one response
	int	 	WaitTime;											// optional, The maximum time before responding to the request, even if the MinResults parameter is not fulfilled
} tse_GetEventSearchResults_REQ;

typedef struct 
{
	onvif_FindEventResultList	ResultList;						// required
} tse_GetEventSearchResults_RES;

typedef struct
{
    uint32  EndPointFlag		: 1;							// Indicates whether the field EndPoint is valid
	uint32  MaxMatchesFlag		: 1;							// Indicates whether the field MaxMatches is valid
	uint32  KeepAliveTimeFlag	: 1;							// Indicates whether the field KeepAliveTime is valid
	uint32  Reserved			: 29;
	
	time_t 	StartPoint;											// required, The point of time where the search will start
	time_t	EndPoint;											// optional, The point of time where the search will stop. This can be a time before the StartPoint, in which case the search is performed backwards in time

	onvif_SearchScope	    Scope;						        // required,
    onvif_PTZPositionFilter SearchFilter;                       // required,

	int 	MaxMatches;											// optional, The search will be completed after this many matches. If not specified, the search will continue until reaching the endpoint or until the session expires
	int	 	KeepAliveTime;										// optional, The time the search session will be kept alive after responding to this and subsequent requests. A device shall support at least values up to ten seconds
} tse_FindPTZPosition_REQ;

typedef struct
{
    char	SearchToken[100];						// required, A unique reference to the search session created by this request
} tse_FindPTZPosition_RES;

typedef struct
{
    uint32  MinResultsFlag	: 1;								// Indicates whether the field MinResults is valid
	uint32  MaxResultsFlag	: 1;								// Indicates whether the field MaxResults is valid
	uint32  WaitTimeFlag	: 1;								// Indicates whether the field WaitTime is valid
	uint32  Reserved		: 29;
	
	char 	SearchToken[100];						// required, The search session to get results from
	int 	MinResults;											// optional, The minimum number of results to return in one response
	int 	MaxResults;											// optional, The maximum number of results to return in one response
	int	 	WaitTime;											// optional, The maximum time before responding to the request, even if the MinResults parameter is not fulfilled
} tse_GetPTZPositionSearchResults_REQ;

typedef struct
{
    onvif_FindPTZPositionResultList	ResultList;				    // required
} tse_GetPTZPositionSearchResults_RES;

typedef struct
{
    uint32  EndPointFlag		: 1;							// Indicates whether the field EndPoint is valid
	uint32  MaxMatchesFlag		: 1;							// Indicates whether the field MaxMatches is valid
	uint32  KeepAliveTimeFlag	: 1;							// Indicates whether the field KeepAliveTime is valid
	uint32  Reserved			: 29;
	
	time_t 	StartPoint;											// required, The point of time where the search will start
	time_t	EndPoint;											// optional, The point of time where the search will stop. This can be a time before the StartPoint, in which case the search is performed backwards in time

	onvif_SearchScope	    Scope;						        // required,
    onvif_MetadataFilter    MetadataFilter;                     // required,

	int 	MaxMatches;											// optional, The search will be completed after this many matches. If not specified, the search will continue until reaching the endpoint or until the session expires
	int	 	KeepAliveTime;										// optional, The time the search session will be kept alive after responding to this and subsequent requests. A device shall support at least values up to ten seconds
} tse_FindMetadata_REQ;

typedef struct
{
    char	SearchToken[100];						// required, A unique reference to the search session created by this request
} tse_FindMetadata_RES;

typedef struct
{
    uint32  MinResultsFlag	: 1;								// Indicates whether the field MinResults is valid
	uint32  MaxResultsFlag	: 1;								// Indicates whether the field MaxResults is valid
	uint32  WaitTimeFlag	: 1;								// Indicates whether the field WaitTime is valid
	uint32  Reserved		: 29;
	
	char 	SearchToken[100];						// required, The search session to get results from
	int 	MinResults;											// optional, The minimum number of results to return in one response
	int 	MaxResults;											// optional, The maximum number of results to return in one response
	int	 	WaitTime;											// optional, The maximum time before responding to the request, even if the MinResults parameter is not fulfilled
} tse_GetMetadataSearchResults_REQ;

typedef struct
{
    onvif_FindMetadataResultList    ResultList;				    // required
} tse_GetMetadataSearchResults_RES;

typedef struct
{
	char	SearchToken[100];						// required, The search session to end
} tse_EndSearch_REQ;

typedef struct
{
	time_t 	Endpoint;											// required, The point of time the search had reached when it was ended. It is equal to the EndPoint specified in Find-operation if the search was completed
} tse_EndSearch_RES;

typedef struct
{
	char	SearchToken[100];						// required, The search session to get the state from
} tse_GetSearchState_REQ;

typedef struct
{
	onvif_SearchState	State;									// required, 
} tse_GetSearchState_RES;

typedef struct 
{
	onvif_StreamSetup	StreamSetup;							// required, Specifies the connection parameters to be used for the stream. The URI that is returned may depend on these parameters
	
	char 	RecordingToken[100];					// required, The identifier of the recording to be streamed
} trp_GetReplayUri_REQ;

typedef struct 
{
	char 	Uri[256];											// required, The URI to which the client should connect in order to stream the recording
} trp_GetReplayUri_RES;

typedef struct
{
	int 	SessionTimeout;										// required, The RTSP session timeout, unit is second
} trp_GetReplayConfiguration_RES;

typedef struct
{
	int 	SessionTimeout;										// required, The RTSP session timeout, unit is second
} trp_SetReplayConfiguration_REQ;


#ifdef __cplusplus
extern "C" {
#endif

void      onvif_RecordingJobStateNotify(RecordingJobList * p_recordingjob, onvif_PropertyOperation op);
ONVIF_RET onvif_trc_CreateRecording(trc_CreateRecording_REQ * p_req);
ONVIF_RET onvif_trc_DeleteRecording(const char * p_RecordingToken);
ONVIF_RET onvif_trc_SetRecordingConfiguration(trc_SetRecordingConfiguration_REQ * p_req);
ONVIF_RET onvif_trc_CreateTrack(trc_CreateTrack_REQ * p_req);
ONVIF_RET onvif_trc_DeleteTrack(trc_DeleteTrack_REQ * p_req);
ONVIF_RET onvif_trc_SetTrackConfiguration(trc_SetTrackConfiguration_REQ * p_req);
ONVIF_RET onvif_trc_CreateRecordingJob(trc_CreateRecordingJob_REQ  * p_req);
ONVIF_RET onvif_trc_DeleteRecordingJob(const char * p_JobToken);
ONVIF_RET onvif_trc_SetRecordingJobConfiguration(trc_SetRecordingJobConfiguration_REQ * p_req);
ONVIF_RET onvif_trc_SetRecordingJobMode(trc_SetRecordingJobMode_REQ * p_req);
ONVIF_RET onvif_trc_GetRecordingJobState(const char * p_JobToken, onvif_RecordingJobStateInformation * p_res);
ONVIF_RET onvif_trc_GetRecordingOptions(const char * p_RecordingToken, onvif_RecordingOptions * p_res);

ONVIF_RET onvif_tse_GetRecordingSummary(tse_GetRecordingSummary_RES * p_summary);
ONVIF_RET onvif_tse_GetRecordingInformation(const char * p_RecordingToken, tse_GetRecordingInformation_RES * p_res);
ONVIF_RET onvif_tse_GetMediaAttributes(tse_GetMediaAttributes_REQ * p_req, tse_GetMediaAttributes_RES * p_res);
ONVIF_RET onvif_tse_FindRecordings(tse_FindRecordings_REQ * p_req, tse_FindRecordings_RES * p_res);
ONVIF_RET onvif_tse_GetRecordingSearchResults(tse_GetRecordingSearchResults_REQ * p_req, tse_GetRecordingSearchResults_RES * p_res);
ONVIF_RET onvif_tse_FindEvents(tse_FindEvents_REQ * p_req, tse_FindEvents_RES * p_res);
ONVIF_RET onvif_tse_GetEventSearchResults(tse_GetEventSearchResults_REQ * p_req, tse_GetEventSearchResults_RES * p_res);
ONVIF_RET onvif_tse_FindMetadata(tse_FindMetadata_REQ * p_req, tse_FindMetadata_RES * p_res);
ONVIF_RET onvif_tse_GetMetadataSearchResults(tse_GetMetadataSearchResults_REQ * p_req, tse_GetMetadataSearchResults_RES * p_res);
ONVIF_RET onvif_tse_FindPTZPosition(tse_FindPTZPosition_REQ * p_req, tse_FindPTZPosition_RES * p_res);
ONVIF_RET onvif_tse_GetPTZPositionSearchResults(tse_GetPTZPositionSearchResults_REQ * p_req, tse_GetPTZPositionSearchResults_RES * p_res);
ONVIF_RET onvif_tse_EndSearch(tse_EndSearch_REQ * p_req, tse_EndSearch_RES * p_res);
ONVIF_RET onvif_tse_GetSearchState(tse_GetSearchState_REQ * p_req, tse_GetSearchState_RES * p_res);
void      onvif_FreeSearchs();
void      onvif_SearchTimeoutCheck();

ONVIF_RET onvif_trp_GetReplayUri(const char * lip, uint32 lport, trp_GetReplayUri_REQ * p_req, trp_GetReplayUri_RES * p_res);
ONVIF_RET onvif_trp_GetReplayConfiguration(trp_GetReplayConfiguration_RES * p_res);
ONVIF_RET onvif_trp_SetReplayConfiguration(trp_SetReplayConfiguration_REQ * p_req);

#ifdef __cplusplus
}
#endif


#endif	// end of ONVIF_RECORDING_H


