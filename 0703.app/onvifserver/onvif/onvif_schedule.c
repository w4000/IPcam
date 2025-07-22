

#include "sys_inc.h"
#include "onvif_schedule.h"
#include "onvif.h"
#include "onvif_event.h"

#ifdef LIBICAL
#include "icalvcal.h"
#include "vcc.h"

// link to libical dynamic library
#pragma comment(lib, "libical.lib")
#pragma comment(lib, "libicalvcal.lib")
#endif

#ifdef SCHEDULE_SUPPORT

/********************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;

/********************************************************************************/

void onvif_ScheduleStateNotify(ScheduleList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Schedule/State/Active", PropertyOperation_Changed, 
        "ScheduleToken", p_req->Schedule.token,
        "Name", p_req->Schedule.Name, 
        "Active", p_req->ScheduleState.Active ? "true" : "false", 
        "SpecialDay", p_req->ScheduleState.SpecialDay ? "true" : "false");
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

/*
 * Whenever the configuration data for a schedule is changed (including SpecialDaysSchedule)
 * or if a schedule is added,  the device shall provide the following event
 */
void onvif_ScheduleChangedNotify(ScheduleList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Configuration/Schedule/Changed", PropertyOperation_Changed, 
        "ScheduleToken", p_req->Schedule.token,
        NULL, NULL, 
        NULL, NULL, 
        NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

/*
 * Whenever a schedule is removed, the device shall provide the following event
 */
void onvif_ScheduleRemovedNotify(ScheduleList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Configuration/Schedule/Removed", PropertyOperation_Deleted, 
        "ScheduleToken", p_req->Schedule.token,
        NULL, NULL, 
        NULL, NULL, 
        NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

/*
 * Whenever the configuration data for a SpecialDays item is changed or added, the device shall
 * provide the following event
 */
void onvif_SpecialDayChangedNotify(SpecialDayGroupList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Configuration/SpecialDays/Changed", PropertyOperation_Changed, 
        "SpecialDaysToken", p_req->SpecialDayGroup.token,
        NULL, NULL, 
        NULL, NULL, 
        NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

/*
 * Whenever a SpecialDays item is removed, the device shall provide the following event
 */
void onvif_SpecialDayRemovedNotify(SpecialDayGroupList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Configuration/SpecialDays/Removed", PropertyOperation_Deleted, 
        "SpecialDaysToken", p_req->SpecialDayGroup.token,
        NULL, NULL, 
        NULL, NULL, 
        NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}


/**
 * The possible return values:
 *  ONVIF_ERR_TooManyItems
 **/ 
ONVIF_RET _GetScheduleInfo(tsc_GetScheduleInfo_REQ * p_req, tsc_GetScheduleInfo_RES * p_res)
{
    int i;
    int idx;
    ScheduleList * p_tmp;
    
    for (i = 0; i< p_req->sizeToken; i++)
    {
        p_tmp = onvif_find_Schedule(g_onvif_cfg.schedule, p_req->Token[i]);
        if (p_tmp)
        {
            idx = p_res->sizeScheduleInfo;

            p_res->ScheduleInfo[idx].DescriptionFlag = p_tmp->Schedule.DescriptionFlag;
            
            strcpy(p_res->ScheduleInfo[idx].token, p_tmp->Schedule.token);
            strcpy(p_res->ScheduleInfo[idx].Name, p_tmp->Schedule.Name);
            strcpy(p_res->ScheduleInfo[idx].Description, p_tmp->Schedule.Description);            
            
            p_res->sizeScheduleInfo++;
        }
    }

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_InvalidStartReference
 **/ 
ONVIF_RET _GetScheduleInfoList(tsc_GetScheduleInfoList_REQ * p_req, tsc_GetScheduleInfoList_RES * p_res)
{
    int idx;
    int nums = 0;
    ScheduleList * p_tmp = g_onvif_cfg.schedule;
    
    if (p_req->StartReferenceFlag)
    {
        p_tmp = onvif_find_Schedule(g_onvif_cfg.schedule, p_req->StartReference);
        if (NULL == p_tmp)
        {
            return ONVIF_ERR_InvalidStartReference;
        }
    }

    while (p_tmp)
    {
        idx = p_res->sizeScheduleInfo;

        p_res->ScheduleInfo[idx].DescriptionFlag = p_tmp->Schedule.DescriptionFlag;
        
        strcpy(p_res->ScheduleInfo[idx].token, p_tmp->Schedule.token);
        strcpy(p_res->ScheduleInfo[idx].Name, p_tmp->Schedule.Name);
        strcpy(p_res->ScheduleInfo[idx].Description, p_tmp->Schedule.Description);  
            
        p_res->sizeScheduleInfo++;
        
        p_tmp = p_tmp->next;

        nums++;
        if (p_req->LimitFlag && nums >= p_req->Limit)
        {
            break;
        }
        else if (nums >= ARRAY_SIZE(p_res->ScheduleInfo))
        {
            break;
        }
    }

    if (p_tmp)
    {
        p_res->NextStartReferenceFlag = 1;
        strcpy(p_res->NextStartReference, p_tmp->Schedule.token);
    }    
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_TooManyItems
 **/
ONVIF_RET _GetSchedules(tsc_GetSchedules_REQ * p_req, tsc_GetSchedules_RES * p_res)
{
    int i;
    int idx;
    ScheduleList * p_tmp;
    
    for (i = 0; i< p_req->sizeToken; i++)
    {
        p_tmp = onvif_find_Schedule(g_onvif_cfg.schedule, p_req->Token[i]);
        if (p_tmp)
        {
            idx = p_res->sizeSchedule;

            memcpy(&p_res->Schedule[idx], &p_tmp->Schedule, sizeof(onvif_Schedule));          
            
            p_res->sizeSchedule++;
        }
    }

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_InvalidStartReference
 **/
ONVIF_RET _GetScheduleList(tsc_GetScheduleList_REQ * p_req, tsc_GetScheduleList_RES * p_res)
{
    int idx;
    int nums = 0;
    ScheduleList * p_tmp = g_onvif_cfg.schedule;
    
    if (p_req->StartReferenceFlag)
    {
        p_tmp = onvif_find_Schedule(g_onvif_cfg.schedule, p_req->StartReference);
        if (NULL == p_tmp)
        {
            return ONVIF_ERR_InvalidStartReference;
        }
    }

    while (p_tmp)
    {
        idx = p_res->sizeSchedule;

        memcpy(&p_res->Schedule[idx], &p_tmp->Schedule, sizeof(onvif_Schedule));  
            
        p_res->sizeSchedule++;
        
        p_tmp = p_tmp->next;

        nums++;
        if (p_req->LimitFlag && nums >= p_req->Limit)
        {
            break;
        }
        else if (nums >= ARRAY_SIZE(p_res->Schedule))
        {
            break;
        }
    }

    if (p_tmp)
    {
        p_res->NextStartReferenceFlag = 1;
        strcpy(p_res->NextStartReference, p_tmp->Schedule.token);
    }    
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_MaxSchedules
 *  ONVIF_ERR_MaxSpecialDaysSchedules
 *  ONVIF_ERR_MaxTimePeriodsPerDay
 *  ONVIF_ERR_ReferenceNotFound
 **/
ONVIF_RET _CreateSchedule(tsc_CreateSchedule_REQ * p_req, tsc_CreateSchedule_RES * p_res)
{
    int i, j;
    ScheduleList * p_tmp;
#ifdef LIBICAL
    char * p;
    VObject * vcal = NULL;
    icalcomponent *comp = NULL;
#endif

    // parameter check
    if (strlen(p_req->Schedule.token) > 0)
    {
        return ONVIF_ERR_InvalidArgVal;
    }

#ifdef LIBICAL
    // replace "&#xD;" with "\n"
    p = strstr(p_req->Schedule.Standard, "&#xD;");
    while (p)
	{
		memmove(p+1, p+5, strlen(p+5));
		*p = '\n';
		p = strstr(p+5, "&#xD;");
		p_req->Schedule.Standard[strlen(p_req->Schedule.Standard)-4] = '\0';
	}
	
    vcal = Parse_MIME(p_req->Schedule.Standard, strlen(p_req->Schedule.Standard));
    if (vcal)
    {
        int cnt;
        
        comp = icalvcal_convert(vcal);
        
        cnt = icalcomponent_count_components(comp, ICAL_VEVENT_COMPONENT);
        if (cnt > 10)
        {
            icalcomponent_free(comp);
            return ONVIF_ERR_MaxTimePeriodsPerDay;
        }
    }
#endif // end of LIBICAL

    for (i = 0; i < p_req->Schedule.sizeSpecialDays; i++)
    {
        for (j = 0; j < p_req->Schedule.SpecialDays[i].sizeTimeRange; j++)
        {
            if (p_req->Schedule.SpecialDays[i].TimeRange[j].UntilFlag && 
                strcmp(p_req->Schedule.SpecialDays[i].TimeRange[j].Until, p_req->Schedule.SpecialDays[i].TimeRange[j].From) <= 0)
            {
                return ONVIF_ERR_InvalidArgVal;
            }
        }
    }

    // add new Schedule
    p_tmp = onvif_add_Schedule();
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_MaxSchedules;
    }

    memcpy(&p_tmp->Schedule, &p_req->Schedule, sizeof(onvif_Schedule));

    sprintf(p_tmp->Schedule.token, "ScheduleToken%d", g_onvif_cls.schedule_idx++);
    strcpy(p_res->Token, p_tmp->Schedule.token);

    if (p_tmp->Schedule.sizeSpecialDays > 0)
    {
        p_tmp->ScheduleState.SpecialDayFlag = 1;
        p_tmp->ScheduleState.SpecialDay = 1;
    }
    
#ifdef LIBICAL
    p_tmp->comp = comp;
#endif

    // here add hander code ...

    p_tmp->ScheduleState.Active = TRUE;
    
    // send event notify
    onvif_ScheduleChangedNotify(p_tmp);
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_NotFound
 *  ONVIF_ERR_MaxSpecialDaysSchedules
 *  ONVIF_ERR_MaxTimePeriodsPerDay
 *  ONVIF_ERR_ReferenceNotFound
 **/
ONVIF_RET _ModifySchedule(tsc_ModifySchedule_REQ * p_req)
{
    int i, j;
#ifdef LIBICAL
    char * p;
    VObject * vcal = NULL;
    icalcomponent *comp = NULL;
#endif

    ScheduleList * p_tmp = onvif_find_Schedule(g_onvif_cfg.schedule, p_req->Schedule.token);
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_NotFound;
    }
    
    // parameter check
#ifdef LIBICAL
    // replace "&#xD;" with "\n"
    p = strstr(p_req->Schedule.Standard, "&#xD;");
    while (p)
    {
        memmove(p+1, p+5, strlen(p+5));
        *p = '\n';
        p = strstr(p+5, "&#xD;");
        p_req->Schedule.Standard[strlen(p_req->Schedule.Standard)-4] = '\0';
    }
    
    vcal = Parse_MIME(p_req->Schedule.Standard, strlen(p_req->Schedule.Standard));
    if (vcal)
    {
        int cnt;
        
        comp = icalvcal_convert(vcal);
        
        cnt = icalcomponent_count_components(comp, ICAL_VEVENT_COMPONENT);
        if (cnt > 10)
        {
            icalcomponent_free(comp);
            return ONVIF_ERR_MaxTimePeriodsPerDay;
        }
    }
#endif // end of LIBICAL

    for (i = 0; i < p_req->Schedule.sizeSpecialDays; i++)
    {
        for (j = 0; j < p_req->Schedule.SpecialDays[i].sizeTimeRange; j++)
        {
            if (p_req->Schedule.SpecialDays[i].TimeRange[j].UntilFlag && 
                strcmp(p_req->Schedule.SpecialDays[i].TimeRange[j].Until, p_req->Schedule.SpecialDays[i].TimeRange[j].From) <= 0)
            {
                return ONVIF_ERR_InvalidArgVal;
            }
        }
    }

#ifdef LIBICAL
    if (p_tmp->comp)
    {
        icalcomponent_free(p_tmp->comp);
    }
    p_tmp->comp = comp;
#endif

    // here add hander code ...
    
    // modify the Schedule
    memcpy(&p_tmp->Schedule, &p_req->Schedule, sizeof(onvif_Schedule));

    if (p_tmp->Schedule.sizeSpecialDays > 0)
    {
        p_tmp->ScheduleState.SpecialDayFlag = 1;
        p_tmp->ScheduleState.SpecialDay = 1;
    }

    // send event notify
    onvif_ScheduleChangedNotify(p_tmp);
            
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_NotFound
 *  ONVIF_ERR_ReferenceInUse
 **/
ONVIF_RET _DeleteSchedule(tsc_DeleteSchedule_REQ * p_req)
{
    ScheduleList * p_tmp = onvif_find_Schedule(g_onvif_cfg.schedule, p_req->Token);
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_NotFound;
    }

    // here add hander code ...

    // send event notify
    onvif_ScheduleRemovedNotify(p_tmp);
        
    // delete the Schedule
    onvif_free_Schedule(&g_onvif_cfg.schedule, p_tmp);

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_TooManyItems
 **/
ONVIF_RET _GetSpecialDayGroupInfo(tsc_GetSpecialDayGroupInfo_REQ * p_req, tsc_GetSpecialDayGroupInfo_RES * p_res)
{
    int i;
    int idx;
    SpecialDayGroupList * p_tmp;
    
    for (i = 0; i< p_req->sizeToken; i++)
    {
        p_tmp = onvif_find_SpecialDayGroup(g_onvif_cfg.specialdaygroup, p_req->Token[i]);
        if (p_tmp)
        {
            idx = p_res->sizeSpecialDayGroupInfo;

            p_res->SpecialDayGroupInfo[idx].DescriptionFlag = p_tmp->SpecialDayGroup.DescriptionFlag;
            
            strcpy(p_res->SpecialDayGroupInfo[idx].token, p_tmp->SpecialDayGroup.token);
            strcpy(p_res->SpecialDayGroupInfo[idx].Name, p_tmp->SpecialDayGroup.Name);
            strcpy(p_res->SpecialDayGroupInfo[idx].Description, p_tmp->SpecialDayGroup.Description);            
            
            p_res->sizeSpecialDayGroupInfo++;
        }
    }

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_InvalidStartReference
 **/
ONVIF_RET _GetSpecialDayGroupInfoList(tsc_GetSpecialDayGroupInfoList_REQ * p_req, tsc_GetSpecialDayGroupInfoList_RES * p_res)
{
    int idx;
    int nums = 0;
    SpecialDayGroupList * p_tmp = g_onvif_cfg.specialdaygroup;
    
    if (p_req->StartReferenceFlag)
    {
        p_tmp = onvif_find_SpecialDayGroup(g_onvif_cfg.specialdaygroup, p_req->StartReference);
        if (NULL == p_tmp)
        {
            return ONVIF_ERR_InvalidStartReference;
        }
    }

    while (p_tmp)
    {
        idx = p_res->sizeSpecialDayGroupInfo;

        p_res->SpecialDayGroupInfo[idx].DescriptionFlag = p_tmp->SpecialDayGroup.DescriptionFlag;
        
        strcpy(p_res->SpecialDayGroupInfo[idx].token, p_tmp->SpecialDayGroup.token);
        strcpy(p_res->SpecialDayGroupInfo[idx].Name, p_tmp->SpecialDayGroup.Name);
        strcpy(p_res->SpecialDayGroupInfo[idx].Description, p_tmp->SpecialDayGroup.Description);  
            
        p_res->sizeSpecialDayGroupInfo++;
        
        p_tmp = p_tmp->next;

        nums++;
        if (p_req->LimitFlag && nums >= p_req->Limit)
        {
            break;
        }
        else if (nums >= ARRAY_SIZE(p_res->SpecialDayGroupInfo))
        {
            break;
        }
    }

    if (p_tmp)
    {
        p_res->NextStartReferenceFlag = 1;
        strcpy(p_res->NextStartReference, p_tmp->SpecialDayGroup.token);
    }    
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_TooManyItems
 **/
ONVIF_RET _GetSpecialDayGroups(tsc_GetSpecialDayGroups_REQ * p_req, tsc_GetSpecialDayGroups_RES * p_res)
{
    int i;
    int idx;
    SpecialDayGroupList * p_tmp;
    
    for (i = 0; i< p_req->sizeToken; i++)
    {
        p_tmp = onvif_find_SpecialDayGroup(g_onvif_cfg.specialdaygroup, p_req->Token[i]);
        if (p_tmp)
        {
            idx = p_res->sizeSpecialDayGroup;

            memcpy(&p_res->SpecialDayGroup[idx], &p_tmp->SpecialDayGroup, sizeof(onvif_SpecialDayGroup));                 
            
            p_res->sizeSpecialDayGroup++;
        }
    }

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_InvalidStartReference
 **/
ONVIF_RET _GetSpecialDayGroupList(tsc_GetSpecialDayGroupList_REQ * p_req, tsc_GetSpecialDayGroupList_RES * p_res)
{
    int idx;
    int nums = 0;
    SpecialDayGroupList * p_tmp = g_onvif_cfg.specialdaygroup;
    
    if (p_req->StartReferenceFlag)
    {
        p_tmp = onvif_find_SpecialDayGroup(g_onvif_cfg.specialdaygroup, p_req->StartReference);
        if (NULL == p_tmp)
        {
            return ONVIF_ERR_InvalidStartReference;
        }
    }

    while (p_tmp)
    {
        idx = p_res->sizeSpecialDayGroup;

        memcpy(&p_res->SpecialDayGroup[idx], &p_tmp->SpecialDayGroup, sizeof(onvif_SpecialDayGroup));  
            
        p_res->sizeSpecialDayGroup++;
        
        p_tmp = p_tmp->next;

        nums++;
        if (p_req->LimitFlag && nums >= p_req->Limit)
        {
            break;
        }
        else if (nums >= ARRAY_SIZE(p_res->SpecialDayGroup))
        {
            break;
        }
    }

    if (p_tmp)
    {
        p_res->NextStartReferenceFlag = 1;
        strcpy(p_res->NextStartReference, p_tmp->SpecialDayGroup.token);
    }    
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_MaxSpecialDayGroups
 *  ONVIF_ERR_MaxDaysInSpecialDayGroup
 **/
ONVIF_RET _CreateSpecialDayGroup(tsc_CreateSpecialDayGroup_REQ * p_req, tsc_CreateSpecialDayGroup_RES * p_res)
{
#ifdef LIBICAL
    char * p;
    VObject * vcal = NULL;
    icalcomponent *comp = NULL;
#endif
    SpecialDayGroupList * p_tmp;
    
    // parameter check
    if (strlen(p_req->SpecialDayGroup.token) > 0)
    {
        return ONVIF_ERR_InvalidArgVal;
    }

#ifdef LIBICAL
    if (p_req->SpecialDayGroup.DaysFlag)
    {
        // replace "&#xD;" with "\n"
        p = strstr(p_req->SpecialDayGroup.Days, "&#xD;");
        while (p)
        {
            memmove(p+1, p+5, strlen(p+5));
            *p = '\n';
            p = strstr(p+5, "&#xD;");
            p_req->SpecialDayGroup.Days[strlen(p_req->SpecialDayGroup.Days)-4] = '\0';
        }
        
        vcal = Parse_MIME(p_req->SpecialDayGroup.Days, strlen(p_req->SpecialDayGroup.Days));
        if (vcal)
        {
            comp = icalvcal_convert(vcal);
        }
    }
#endif // end of LIBICAL

    // add new SpecialDayGroup
    p_tmp = onvif_add_SpecialDayGroup();
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_MaxSpecialDayGroups;
    }

    memcpy(&p_tmp->SpecialDayGroup, &p_req->SpecialDayGroup, sizeof(onvif_SpecialDayGroup));

    sprintf(p_tmp->SpecialDayGroup.token, "SpecialDayGroupToken%d", g_onvif_cls.specialdaygroup_idx++);
    strcpy(p_res->Token, p_tmp->SpecialDayGroup.token);

#ifdef LIBICAL
    p_tmp->comp = comp;
#endif

    // here add hander code ...

    // send event notify
    onvif_SpecialDayChangedNotify(p_tmp);
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_NotFound
 *  ONVIF_ERR_MaxDaysInSpecialDayGroup
 **/
ONVIF_RET _ModifySpecialDayGroup(tsc_ModifySpecialDayGroup_REQ * p_req)
{
#ifdef LIBICAL
    char * p;
    VObject * vcal = NULL;
    icalcomponent *comp = NULL;
#endif

    SpecialDayGroupList * p_tmp = onvif_find_SpecialDayGroup(g_onvif_cfg.specialdaygroup, p_req->SpecialDayGroup.token);
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_NotFound;
    }
    
    // parameter check
#ifdef LIBICAL
    if (p_req->SpecialDayGroup.DaysFlag)
    {
        // replace "&#xD;" with "\n"
        p = strstr(p_req->SpecialDayGroup.Days, "&#xD;");
        while (p)
        {
            memmove(p+1, p+5, strlen(p+5));
            *p = '\n';
            p = strstr(p+5, "&#xD;");
            p_req->SpecialDayGroup.Days[strlen(p_req->SpecialDayGroup.Days)-4] = '\0';
        }
        
        vcal = Parse_MIME(p_req->SpecialDayGroup.Days, strlen(p_req->SpecialDayGroup.Days));
        if (vcal)
        {
            comp = icalvcal_convert(vcal);
            if (p_tmp->comp)
            {
                icalcomponent_free(p_tmp->comp);
            }
            p_tmp->comp = comp;
        }
    }
#endif // end of LIBICAL

    // here add hander code ...
    
    // modify the SpecialDayGroup
    memcpy(&p_tmp->SpecialDayGroup, &p_req->SpecialDayGroup, sizeof(onvif_SpecialDayGroup));

    // send event notify
    onvif_SpecialDayChangedNotify(p_tmp);
            
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_NotFound
 *  ONVIF_ERR_ReferenceInUse
 **/
ONVIF_RET _DeleteSpecialDayGroup(tsc_DeleteSpecialDayGroup_REQ * p_req)
{
    SpecialDayGroupList * p_tmp = onvif_find_SpecialDayGroup(g_onvif_cfg.specialdaygroup, p_req->Token);
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_NotFound;
    }

    // here add hander code ...

    // send event notify
    onvif_SpecialDayRemovedNotify(p_tmp);
        
    // delete the SpecialDayGroup
    onvif_free_SpecialDayGroup(&g_onvif_cfg.specialdaygroup, p_tmp);

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_NotFound
 **/
ONVIF_RET _GetScheduleState(tsc_GetScheduleState_REQ * p_req, tsc_GetScheduleState_RES * p_res)
{
    ScheduleList * p_tmp = onvif_find_Schedule(g_onvif_cfg.schedule, p_req->Token);
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_NotFound;
    }

    p_res->ScheduleState.Active = p_tmp->ScheduleState.Active;
    p_res->ScheduleState.SpecialDayFlag = p_tmp->ScheduleState.SpecialDayFlag;
    p_res->ScheduleState.SpecialDay= p_tmp->ScheduleState.SpecialDay;

    return ONVIF_OK;
}

ONVIF_RET _SetSchedule(tsc_SetSchedule_REQ * p_req)
{
    int i, j;
    ScheduleList * p_tmp;
#ifdef LIBICAL
    char * p;
    VObject * vcal = NULL;
    icalcomponent *comp = NULL;
#endif

    if (p_req->Schedule.token[0] == '\0')
    {
        return ONVIF_ERR_InvalidArgs;
    }
    
#ifdef LIBICAL
    // replace "&#xD;" with "\n"
    p = strstr(p_req->Schedule.Standard, "&#xD;");
    while (p)
	{
		memmove(p+1, p+5, strlen(p+5));
		*p = '\n';
		p = strstr(p+5, "&#xD;");
		p_req->Schedule.Standard[strlen(p_req->Schedule.Standard)-4] = '\0';
	}
	
    vcal = Parse_MIME(p_req->Schedule.Standard, strlen(p_req->Schedule.Standard));
    if (vcal)
    {
        int cnt;
        
        comp = icalvcal_convert(vcal);
        
        cnt = icalcomponent_count_components(comp, ICAL_VEVENT_COMPONENT);
        if (cnt > 10)
        {
            icalcomponent_free(comp);
            return ONVIF_ERR_MaxTimePeriodsPerDay;
        }
    }
#endif // end of LIBICAL

    for (i = 0; i < p_req->Schedule.sizeSpecialDays; i++)
    {
        for (j = 0; j < p_req->Schedule.SpecialDays[i].sizeTimeRange; j++)
        {
            if (p_req->Schedule.SpecialDays[i].TimeRange[j].UntilFlag && 
                strcmp(p_req->Schedule.SpecialDays[i].TimeRange[j].Until, p_req->Schedule.SpecialDays[i].TimeRange[j].From) <= 0)
            {
                return ONVIF_ERR_InvalidArgVal;
            }
        }
    }
        
    p_tmp = onvif_find_Schedule(g_onvif_cfg.schedule, p_req->Schedule.token);
    if (NULL == p_tmp)
    {
        // add new Schedule
        p_tmp = onvif_add_Schedule();
        if (NULL == p_tmp)
        {
            return ONVIF_ERR_MaxSchedules;
        }

        memcpy(&p_tmp->Schedule, &p_req->Schedule, sizeof(onvif_Schedule));

        if (p_tmp->Schedule.sizeSpecialDays > 0)
        {
            p_tmp->ScheduleState.SpecialDayFlag = 1;
            p_tmp->ScheduleState.SpecialDay = 1;
        }
        
#ifdef LIBICAL
        p_tmp->comp = comp;
#endif

        // here add hander code ...

        // send event notify
        onvif_ScheduleChangedNotify(p_tmp);
    }
    else
    {
#ifdef LIBICAL
        if (p_tmp->comp)
        {
            icalcomponent_free(p_tmp->comp);
        }
        p_tmp->comp = comp;
#endif

        // here add hander code ...
        
        // modify the Schedule
        memcpy(&p_tmp->Schedule, &p_req->Schedule, sizeof(onvif_Schedule));

        if (p_tmp->Schedule.sizeSpecialDays > 0)
        {
            p_tmp->ScheduleState.SpecialDayFlag = 1;
            p_tmp->ScheduleState.SpecialDay = 1;
        }
        
        // send event notify
        onvif_ScheduleChangedNotify(p_tmp);
    }

    return ONVIF_OK;
}

ONVIF_RET _SetSpecialDayGroup(tsc_SetSpecialDayGroup_REQ * p_req)
{
#ifdef LIBICAL
    char * p;
    VObject * vcal = NULL;
    icalcomponent *comp = NULL;
#endif
    SpecialDayGroupList * p_tmp;

    if (p_req->SpecialDayGroup.token[0] == '\0')
    {
        return ONVIF_ERR_InvalidArgs;
    }
    
#ifdef LIBICAL
    if (p_req->SpecialDayGroup.DaysFlag)
    {
        // replace "&#xD;" with "\n"
        p = strstr(p_req->SpecialDayGroup.Days, "&#xD;");
        while (p)
        {
            memmove(p+1, p+5, strlen(p+5));
            *p = '\n';
            p = strstr(p+5, "&#xD;");
            p_req->SpecialDayGroup.Days[strlen(p_req->SpecialDayGroup.Days)-4] = '\0';
        }
        
        vcal = Parse_MIME(p_req->SpecialDayGroup.Days, strlen(p_req->SpecialDayGroup.Days));
        if (vcal)
        {
            comp = icalvcal_convert(vcal);
        }
    }
#endif // end of LIBICAL

    p_tmp = onvif_find_SpecialDayGroup(g_onvif_cfg.specialdaygroup, p_req->SpecialDayGroup.token);
    if (NULL == p_tmp)
    {
        // add new SpecialDayGroup
        p_tmp = onvif_add_SpecialDayGroup();
        if (NULL == p_tmp)
        {
            return ONVIF_ERR_MaxSpecialDayGroups;
        }

        memcpy(&p_tmp->SpecialDayGroup, &p_req->SpecialDayGroup, sizeof(onvif_SpecialDayGroup));

#ifdef LIBICAL
        p_tmp->comp = comp;
#endif

        // here add hander code ...

        // send event notify
        onvif_SpecialDayChangedNotify(p_tmp);
    }
    else
    {
#ifdef LIBICAL
        if (p_tmp->comp)
        {
            icalcomponent_free(p_tmp->comp);
        }
        p_tmp->comp = comp;
#endif

        // here add hander code ...
    
        // modify the SpecialDayGroup
        memcpy(&p_tmp->SpecialDayGroup, &p_req->SpecialDayGroup, sizeof(onvif_SpecialDayGroup));

        // send event notify
        onvif_SpecialDayChangedNotify(p_tmp);
    }
    
    return ONVIF_OK;
}

// is called in onvif_timer
void onvif_DoSchedule()
{
    ScheduleList * p_sch = NULL;
    
    // schedule hanlder
	p_sch = g_onvif_cfg.schedule;
    while (p_sch)
    {
        // todo : here, add the schedule hanlder code ...

        
        p_sch = p_sch->next;
    }
}

#endif // end of SCHEDULE_SUPPORT




