

#include "sys_inc.h"
#include "onvif_accessrules.h"
#include "onvif.h"
#include "onvif_event.h"


#ifdef ACCESS_RULES

/********************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;

/********************************************************************************/

void onvif_AccessProfileChangedNotify(AccessProfileList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Configuration/AccessProfile/Changed", PropertyOperation_Changed, 
        "AccessProfileToken", p_req->AccessProfile.token,
        NULL, NULL, NULL, NULL, NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

void onvif_AccessProfileRemovedNotify(AccessProfileList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Configuration/AccessProfile/Removed", PropertyOperation_Deleted, 
        "AccessProfileToken", p_req->AccessProfile.token,
        NULL, NULL, NULL, NULL, NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}


/**
 * The possible return values:
 *	ONVIF_ERR_TooManyItems
 **/
ONVIF_RET onvif_tar_GetAccessProfileInfo(tar_GetAccessProfileInfo_REQ * p_req, tar_GetAccessProfileInfo_RES * p_res)
{
    int i;
    int idx;
    AccessProfileList * p_tmp;
    
    for (i = 0; i< p_req->sizeToken; i++)
    {
        p_tmp = onvif_find_AccessProfile(g_onvif_cfg.access_rules, p_req->Token[i]);
        if (p_tmp)
        {
            idx = p_res->sizeAccessProfileInfo;

            p_res->AccessProfileInfo[idx].DescriptionFlag = p_tmp->AccessProfile.DescriptionFlag;
            
            strcpy(p_res->AccessProfileInfo[idx].token, p_tmp->AccessProfile.token);
            strcpy(p_res->AccessProfileInfo[idx].Name, p_tmp->AccessProfile.Name);
            strcpy(p_res->AccessProfileInfo[idx].Description, p_tmp->AccessProfile.Description);            
            
            p_res->sizeAccessProfileInfo++;
        }
    }

    return ONVIF_OK;
}

/**
 * The possible return values:
 *	ONVIF_ERR_InvalidStartReference
 **/
ONVIF_RET onvif_tar_GetAccessProfileInfoList(tar_GetAccessProfileInfoList_REQ * p_req, tar_GetAccessProfileInfoList_RES * p_res)
{
    int idx;
    int nums = 0;
    AccessProfileList * p_tmp = g_onvif_cfg.access_rules;
    
    if (p_req->StartReferenceFlag)
    {
        p_tmp = onvif_find_AccessProfile(g_onvif_cfg.access_rules, p_req->StartReference);
        if (NULL == p_tmp)
        {
            return ONVIF_ERR_InvalidStartReference;
        }
    }

    while (p_tmp)
    {
        idx = p_res->sizeAccessProfileInfo;

        p_res->AccessProfileInfo[idx].DescriptionFlag = p_tmp->AccessProfile.DescriptionFlag;
        
        strcpy(p_res->AccessProfileInfo[idx].token, p_tmp->AccessProfile.token);
        strcpy(p_res->AccessProfileInfo[idx].Name, p_tmp->AccessProfile.Name);
        strcpy(p_res->AccessProfileInfo[idx].Description, p_tmp->AccessProfile.Description);  
            
        p_res->sizeAccessProfileInfo++;
        
        p_tmp = p_tmp->next;

        nums++;
        if (p_req->LimitFlag && nums >= p_req->Limit)
        {
            break;
        }
        else if (nums >= ARRAY_SIZE(p_res->AccessProfileInfo))
        {
            break;
        }
    }

    if (p_tmp)
    {
        p_res->NextStartReferenceFlag = 1;
        strcpy(p_res->NextStartReference, p_tmp->AccessProfile.token);
    }    
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *	ONVIF_ERR_TooManyItems
 **/
ONVIF_RET onvif_tar_GetAccessProfiles(tar_GetAccessProfiles_REQ * p_req, tar_GetAccessProfiles_RES * p_res)
{
    int i;
    int idx;
    AccessProfileList * p_tmp;
    
    for (i = 0; i< p_req->sizeToken; i++)
    {
        p_tmp = onvif_find_AccessProfile(g_onvif_cfg.access_rules, p_req->Token[i]);
        if (p_tmp)
        {
            idx = p_res->sizeAccessProfile;

            memcpy(&p_res->AccessProfile[idx], &p_tmp->AccessProfile, sizeof(onvif_AccessProfile));
            
            p_res->sizeAccessProfile++;
        }
    }

    return ONVIF_OK;
}

/**
 * The possible return values:
 *	ONVIF_ERR_InvalidStartReference
 **/
ONVIF_RET onvif_tar_GetAccessProfileList(tar_GetAccessProfileList_REQ * p_req, tar_GetAccessProfileList_RES * p_res)
{
    int idx;
    int nums = 0;
    AccessProfileList * p_tmp = g_onvif_cfg.access_rules;
    
    if (p_req->StartReferenceFlag)
    {
        p_tmp = onvif_find_AccessProfile(g_onvif_cfg.access_rules, p_req->StartReference);
        if (NULL == p_tmp)
        {
            return ONVIF_ERR_InvalidStartReference;
        }
    }

    while (p_tmp)
    {
        idx = p_res->sizeAccessProfile;

        memcpy(&p_res->AccessProfile[idx], &p_tmp->AccessProfile, sizeof(onvif_AccessProfile));
        
        p_res->sizeAccessProfile++;
        
        p_tmp = p_tmp->next;

        nums++;
        if (p_req->LimitFlag && nums >= p_req->Limit)
        {
            break;
        }
        else if (nums >= ARRAY_SIZE(p_res->AccessProfile))
        {
            break;
        }
    }

    if (p_tmp)
    {
        p_res->NextStartReferenceFlag = 1;
        strcpy(p_res->NextStartReference, p_tmp->AccessProfile.token);
    }    
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_MaxAccessProfiles
 *  ONVIF_ERR_MaxAccessPoliciesPerAccessProfile
 *  ONVIF_ERR_MultipleSchedulesPerAccessPointSupported
 *  ONVIF_ERR_ReferenceNotFound
 **/
ONVIF_RET onvif_tar_CreateAccessProfile(tar_CreateAccessProfile_REQ * p_req, tar_CreateAccessProfile_RES * p_res)
{
    AccessProfileList * p_tmp;
    
    // parameter check
    if (strlen(p_req->AccessProfile.token) > 0)
    {
        return ONVIF_ERR_InvalidArgVal;
    }

    // add new AccessProfile
    p_tmp = onvif_add_AccessProfile();
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_MaxAccessProfiles;
    }

    memcpy(&p_tmp->AccessProfile, &p_req->AccessProfile, sizeof(onvif_AccessProfile));

    sprintf(p_tmp->AccessProfile.token, "AccessProfileToken%d", g_onvif_cls.accessrule_idx++);
    strcpy(p_res->Token, p_tmp->AccessProfile.token);
    
    // here add hander code ...

    // send event notify
    onvif_AccessProfileChangedNotify(p_tmp);
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_NotFound
 *  ONVIF_ERR_MaxAccessPoliciesPerAccessProfile
 *  ONVIF_ERR_MultipleSchedulesPerAccessPointSupported
 *  ONVIF_ERR_ReferenceNotFound
 **/
ONVIF_RET onvif_tar_ModifyAccessProfile(tar_ModifyAccessProfile_REQ * p_req)
{
    AccessProfileList * p_tmp = onvif_find_AccessProfile(g_onvif_cfg.access_rules, p_req->AccessProfile.token);
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_NotFound;
    }
    
    // parameter check

    // here add hander code ...
    
    // modify the credential
    memcpy(&p_tmp->AccessProfile, &p_req->AccessProfile, sizeof(onvif_AccessProfile));

    // send event notify
    onvif_AccessProfileChangedNotify(p_tmp);
            
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_NotFound
 *  ONVIF_ERR_ReferenceInUse
 **/
ONVIF_RET onvif_tar_DeleteAccessProfile(tar_DeleteAccessProfile_REQ * p_req)
{
    AccessProfileList * p_tmp = onvif_find_AccessProfile(g_onvif_cfg.access_rules, p_req->Token);
    if (NULL == p_tmp)
    {
        return ONVIF_ERR_NotFound;
    }

    // here add hander code ...

    // send event notify
    onvif_AccessProfileRemovedNotify(p_tmp);
        
    // delete the credential
    onvif_free_AccessProfile(&g_onvif_cfg.access_rules, p_tmp);

    return ONVIF_OK;
}

ONVIF_RET onvif_tar_SetAccessProfile(tar_SetAccessProfile_REQ * p_req)
{
    AccessProfileList * p_tmp;
    
    if (p_req->AccessProfile.token[0] == '\0')
    {
        return ONVIF_ERR_InvalidArgs;
    }
    
    p_tmp = onvif_find_AccessProfile(g_onvif_cfg.access_rules, p_req->AccessProfile.token);
    if (NULL == p_tmp)
    {
        // add new AccessProfile
        p_tmp = onvif_add_AccessProfile();
        if (NULL == p_tmp)
        {
            return ONVIF_ERR_MaxAccessProfiles;
        }

        memcpy(&p_tmp->AccessProfile, &p_req->AccessProfile, sizeof(onvif_AccessProfile));
        
        // here add hander code ...

        // send event notify
        onvif_AccessProfileChangedNotify(p_tmp);
    }
    else
    {
        // parameter check

        // here add hander code ...
        
        // modify the credential
        memcpy(&p_tmp->AccessProfile, &p_req->AccessProfile, sizeof(onvif_AccessProfile));

        // send event notify
        onvif_AccessProfileChangedNotify(p_tmp);
    }

    return ONVIF_OK;
}

#endif // end of ACCESS_RULES



