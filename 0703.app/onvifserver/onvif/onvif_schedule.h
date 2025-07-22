

#ifndef ONVIF_SCHEDULE_H
#define ONVIF_SCHEDULE_H

#include "onvif.h"
#include "onvif_cm.h"


typedef struct
{
	int     sizeToken;	                                // sequence of elements <Token>
	char    Token[10][100];   // required, Tokens of ScheduleInfo items to get
} tsc_GetScheduleInfo_REQ;

typedef struct
{
    int     sizeScheduleInfo;	                        // sequence of elements <ScheduleInfo>
	onvif_ScheduleInfo  ScheduleInfo[10];   // optional, List of ScheduleInfo items
} tsc_GetScheduleInfo_RES;

typedef struct
{
    uint32  LimitFlag           : 1;                    // Indicates whether the field Limit is valid
    uint32  StartReferenceFlag  : 1;                    // Indicates whether the field StartReference is valid
    uint32  Reserved            : 30;
    
	int     Limit;	                                    // optional, Maximum number of entries to return. If not specified, less than one
									                    //  or higher than what the device supports, the number of items is
									                    //  determined by the device
	char    StartReference[100];	        // optional, Start returning entries from this start reference.
									                    //  If not specified, entries shall start from the beginning of the dataset
} tsc_GetScheduleInfoList_REQ;

typedef struct 
{
    uint32  NextStartReferenceFlag  : 1;                // Indicates whether the field StartReference is valid
    uint32  Reserved                : 31;
    
    char    NextStartReference[100];        // optional, StartReference to use in next call to get the following items. If
									                    //  absent, no more items to get
    int     sizeScheduleInfo;                           // sequence of elements <ScheduleInfo>

    onvif_ScheduleInfo ScheduleInfo[10];    // optional, List of ScheduleInfo items
} tsc_GetScheduleInfoList_RES;

typedef struct 
{
    int     sizeToken;	                                // sequence of elements <Token>
	char    Token[10][100];   // required, Tokens of Schedule items to get
} tsc_GetSchedules_REQ;

typedef struct 
{
    int     sizeSchedule;	                            // sequence of elements <Schedule>
	onvif_Schedule  Schedule[10];	    // optional, List of schedule items
} tsc_GetSchedules_RES;

typedef struct 
{
    uint32  LimitFlag           : 1;                    // Indicates whether the field Limit is valid
    uint32  StartReferenceFlag  : 1;                    // Indicates whether the field StartReference is valid
    uint32  Reserved            : 30;
    
	int     Limit;	                                    // optional, Maximum number of entries to return. If not specified, less than one
									                    //  or higher than what the device supports, the number of items is
									                    //  determined by the device
	char    StartReference[100];	        // optional, Start returning entries from this start reference.
									                    //  If not specified, entries shall start from the beginning of the dataset
} tsc_GetScheduleList_REQ;

typedef struct 
{
    uint32  NextStartReferenceFlag  : 1;                // Indicates whether the field StartReference is valid
    uint32  Reserved                : 31;
    
    char    NextStartReference[100];        // optional, StartReference to use in next call to get the following items. If
									                    //  absent, no more items to get
    int     sizeSchedule;                               // sequence of elements <Schedule>

    onvif_Schedule Schedule[10];        // optional, List of Schedule items
} tsc_GetScheduleList_RES;

typedef struct 
{
    onvif_Schedule Schedule;                            // required, The Schedule to create
} tsc_CreateSchedule_REQ;

typedef struct 
{
    char    Token[100];                     // required, The token of created Schedule
} tsc_CreateSchedule_RES;

typedef struct 
{
    onvif_Schedule Schedule;                            // required, The Schedule to modify/update
} tsc_ModifySchedule_REQ;

typedef struct 
{
    char    Token[100];                     // required, The token of the schedule to delete
} tsc_DeleteSchedule_REQ;

typedef struct 
{
    int     sizeToken;	                                // sequence of elements <Token>
	char    Token[10][100];   // required, Tokens of SpecialDayGroupInfo items to get
} tsc_GetSpecialDayGroupInfo_REQ;

typedef struct 
{
    int     sizeSpecialDayGroupInfo;	                // sequence of elements <SpecialDayGroupInfo>
	onvif_SpecialDayGroupInfo  SpecialDayGroupInfo[10];   // optional, List of SpecialDayGroupInfo items
} tsc_GetSpecialDayGroupInfo_RES;

typedef struct 
{
    uint32  LimitFlag           : 1;                    // Indicates whether the field Limit is valid
    uint32  StartReferenceFlag  : 1;                    // Indicates whether the field StartReference is valid
    uint32  Reserved            : 30;
    
	int     Limit;	                                    // optional, Maximum number of entries to return. If not specified, less than one
									                    //  or higher than what the device supports, the number of items is
									                    //  determined by the device
	char    StartReference[100];	        // optional, Start returning entries from this start reference.
									                    //  If not specified, entries shall start from the beginning of the dataset
} tsc_GetSpecialDayGroupInfoList_REQ;

typedef struct 
{
    uint32  NextStartReferenceFlag  : 1;                // Indicates whether the field StartReference is valid
    uint32  Reserved                : 31;
    
    char    NextStartReference[100];        // optional, StartReference to use in next call to get the following items. If
									                    //  absent, no more items to get
    int     sizeSpecialDayGroupInfo;                    // sequence of elements <SpecialDayGroupInfo>

    onvif_SpecialDayGroupInfo SpecialDayGroupInfo[10];    // optional, List of SpecialDayGroupInfo items
} tsc_GetSpecialDayGroupInfoList_RES;

typedef struct 
{
    int     sizeToken;	                                // sequence of elements <Token>
	char    Token[10][100];   // required, Tokens of the SpecialDayGroup items to get
} tsc_GetSpecialDayGroups_REQ;

typedef struct 
{
    int     sizeSpecialDayGroup;	                    // sequence of elements <SpecialDayGroup>
	onvif_SpecialDayGroup  SpecialDayGroup[10];   // optional, List of SpecialDayGroup items
} tsc_GetSpecialDayGroups_RES;

typedef struct 
{
    uint32  LimitFlag           : 1;                    // Indicates whether the field Limit is valid
    uint32  StartReferenceFlag  : 1;                    // Indicates whether the field StartReference is valid
    uint32  Reserved            : 30;
    
	int     Limit;	                                    // optional, Maximum number of entries to return. If not specified, less than one
									                    //  or higher than what the device supports, the number of items is
									                    //  determined by the device
	char    StartReference[100];	        // optional, Start returning entries from this start reference.
									                    //  If not specified, entries shall start from the beginning of the dataset
} tsc_GetSpecialDayGroupList_REQ;

typedef struct 
{
    uint32  NextStartReferenceFlag  : 1;                // Indicates whether the field StartReference is valid
    uint32  Reserved                : 31;
    
    char    NextStartReference[100];        // optional, StartReference to use in next call to get the following items. If
									                    //  absent, no more items to get
    int     sizeSpecialDayGroup;                        // sequence of elements <SpecialDayGroup>

    onvif_SpecialDayGroup SpecialDayGroup[10];    // optional, List of SpecialDayGroup items
} tsc_GetSpecialDayGroupList_RES;

typedef struct 
{
    onvif_SpecialDayGroup SpecialDayGroup;              // required, The special day group to create
} tsc_CreateSpecialDayGroup_REQ;

typedef struct 
{
    char    Token[100];                     // required, The token of created special day group
} tsc_CreateSpecialDayGroup_RES;

typedef struct 
{
    onvif_SpecialDayGroup SpecialDayGroup;              // required, The special day group to modify/update
} tsc_ModifySpecialDayGroup_REQ;

typedef struct 
{
    char    Token[100];                     // required, The token of the special day group item to delete
} tsc_DeleteSpecialDayGroup_REQ;

typedef struct 
{
    char    Token[100];                     // required, Token of schedule instance to get ScheduleState
} tsc_GetScheduleState_REQ;

typedef struct 
{
    onvif_ScheduleState ScheduleState;	                // required, ScheduleState item
} tsc_GetScheduleState_RES;

typedef struct
{
    onvif_Schedule Schedule;                            // required, The Schedule to modify/update
} tsc_SetSchedule_REQ;

typedef struct
{
    onvif_SpecialDayGroup SpecialDayGroup;              // required, The special day group to modify/update
} tsc_SetSpecialDayGroup_REQ;


#ifdef __cplusplus
extern "C" {
#endif

ONVIF_RET _GetScheduleInfo(tsc_GetScheduleInfo_REQ * p_req, tsc_GetScheduleInfo_RES * p_res);
ONVIF_RET _GetScheduleInfoList(tsc_GetScheduleInfoList_REQ * p_req, tsc_GetScheduleInfoList_RES * p_res);
ONVIF_RET _GetSchedules(tsc_GetSchedules_REQ * p_req, tsc_GetSchedules_RES * p_res);
ONVIF_RET _GetScheduleList(tsc_GetScheduleList_REQ * p_req, tsc_GetScheduleList_RES * p_res);
ONVIF_RET _CreateSchedule(tsc_CreateSchedule_REQ * p_req, tsc_CreateSchedule_RES * p_res);
ONVIF_RET _ModifySchedule(tsc_ModifySchedule_REQ * p_req);
ONVIF_RET _DeleteSchedule(tsc_DeleteSchedule_REQ * p_req);
ONVIF_RET _GetSpecialDayGroupInfo(tsc_GetSpecialDayGroupInfo_REQ * p_req, tsc_GetSpecialDayGroupInfo_RES * p_res);
ONVIF_RET _GetSpecialDayGroupInfoList(tsc_GetSpecialDayGroupInfoList_REQ * p_req, tsc_GetSpecialDayGroupInfoList_RES * p_res);
ONVIF_RET _GetSpecialDayGroups(tsc_GetSpecialDayGroups_REQ * p_req, tsc_GetSpecialDayGroups_RES * p_res);
ONVIF_RET _GetSpecialDayGroupList(tsc_GetSpecialDayGroupList_REQ * p_req, tsc_GetSpecialDayGroupList_RES * p_res);
ONVIF_RET _CreateSpecialDayGroup(tsc_CreateSpecialDayGroup_REQ * p_req, tsc_CreateSpecialDayGroup_RES * p_res);
ONVIF_RET _ModifySpecialDayGroup(tsc_ModifySpecialDayGroup_REQ * p_req);
ONVIF_RET _DeleteSpecialDayGroup(tsc_DeleteSpecialDayGroup_REQ * p_req);
ONVIF_RET _GetScheduleState(tsc_GetScheduleState_REQ * p_req, tsc_GetScheduleState_RES * p_res);
ONVIF_RET _SetSchedule(tsc_SetSchedule_REQ * p_req);
ONVIF_RET _SetSpecialDayGroup(tsc_SetSpecialDayGroup_REQ * p_req);
void      onvif_DoSchedule();

#ifdef __cplusplus
}
#endif

#endif







