

#include "sys_inc.h"
#include "onvif_receiver.h"
#include "onvif.h"
#include "onvif_event.h"
    

#ifdef RECEIVER_SUPPORT
    
/********************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;

/********************************************************************************/

void onvif_ReceiverChangeStateNotify(ReceiverList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Receiver/ChangeState", PropertyOperation_Changed, 
        "ReceiverToken", p_req->Receiver.Token, NULL, NULL, 
        "NewState", onvif_ReceiverStateToString(p_req->StateInformation.State), 
        "MediaUri", p_req->Receiver.Configuration.MediaUri);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

void onvif_ReceiverConnectionFailedNotify(ReceiverList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Receiver/ConnectionFailed", PropertyOperation_Changed, 
        "ReceiverToken", p_req->Receiver.Token, NULL, NULL, 
        "MediaUri", p_req->Receiver.Configuration.MediaUri, NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}


ONVIF_RET onvif_trv_GetReceivers(trv_GetReceivers_RES * p_res)
{
    p_res->Receivers = g_onvif_cfg.receiver;

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_UnknownToken
 **/
ONVIF_RET onvif_trv_GetReceiver(trv_GetReceiver_REQ * p_req, trv_GetReceiver_RES * p_res)
{
    ReceiverList * p_receiver = onvif_find_Receiver(g_onvif_cfg.receiver, p_req->ReceiverToken);
    if (NULL == p_receiver)
    {
        return ONVIF_ERR_NotFound;
    }

    memcpy(&p_res->Receiver, &p_receiver->Receiver, sizeof(onvif_Receiver));

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_BadConfiguration
 *  ONVIF_ERR_MaxReceivers
 **/
ONVIF_RET onvif_trv_CreateReceiver(trv_CreateReceiver_REQ * p_req, trv_CreateReceiver_RES * p_res)
{
    int nums;
    ReceiverList * p_receiver;

    nums = onvif_get_Receiver_nums(g_onvif_cfg.receiver);
    if (nums >= g_onvif_cfg.Capabilities.receiver.SupportedReceivers)
    {
        return ONVIF_ERR_MaxReceivers;
    }

    p_receiver = onvif_add_Receiver(&g_onvif_cfg.receiver);
    if (NULL == p_receiver)
    {
        return ONVIF_ERR_MaxReceivers;
    }

    sprintf(p_receiver->Receiver.Token, "ReceiverToken%d", g_onvif_cls.receiver_idx++);
    memcpy(&p_receiver->Receiver.Configuration, &p_req->Configuration, sizeof(onvif_ReceiverConfiguration));

    memcpy(&p_res->Receiver, &p_receiver->Receiver, sizeof(onvif_Receiver));

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_UnknownToken
 *  ONVIF_ERR_CannotDeleteReceiver
 **/
ONVIF_RET onvif_trv_DeleteReceiver(trv_DeleteReceiver_REQ * p_req)
{
    ReceiverList * p_receiver = onvif_find_Receiver(g_onvif_cfg.receiver, p_req->ReceiverToken);
    if (NULL == p_receiver)
    {
        return ONVIF_ERR_UnknownToken;
    }

    onvif_free_Receiver(&g_onvif_cfg.receiver, p_receiver);

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_UnknownToken
 *  ONVIF_ERR_BadConfiguration
 **/
ONVIF_RET onvif_trv_ConfigureReceiver(trv_ConfigureReceiver_REQ * p_req)
{
    onvif_ReceiverState State;
    ReceiverList * p_receiver = onvif_find_Receiver(g_onvif_cfg.receiver, p_req->ReceiverToken);
    if (NULL == p_receiver)
    {
        return ONVIF_ERR_UnknownToken;
    }
    
    memcpy(&p_receiver->Receiver.Configuration, &p_req->Configuration, sizeof(onvif_ReceiverConfiguration));

    // todo : here add your handler code ...


    State = p_receiver->StateInformation.State;
    
    if (ReceiverMode_AlwaysConnect == p_receiver->Receiver.Configuration.Mode)
    {
        p_receiver->StateInformation.State = ReceiverState_Connected;
        p_receiver->StateInformation.AutoCreated = FALSE;
    }
    else if (ReceiverMode_AutoConnect == p_receiver->Receiver.Configuration.Mode)
    {
        p_receiver->StateInformation.State = ReceiverState_Connected;
        p_receiver->StateInformation.AutoCreated = TRUE;
    }
    else 
    {
        p_receiver->StateInformation.State = ReceiverState_NotConnected;
        p_receiver->StateInformation.AutoCreated = FALSE;
    }

    if (State != p_receiver->StateInformation.State)
    {
        onvif_ReceiverChangeStateNotify(p_receiver);
    }    

    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_UnknownToken
 **/
ONVIF_RET onvif_trv_SetReceiverMode(trv_SetReceiverMode_REQ * p_req)
{
    onvif_ReceiverState State;
    ReceiverList * p_receiver = onvif_find_Receiver(g_onvif_cfg.receiver, p_req->ReceiverToken);
    if (NULL == p_receiver)
    {
        return ONVIF_ERR_UnknownToken;
    }

    p_receiver->Receiver.Configuration.Mode = p_req->Mode;

    // todo : here add your handler code ...


    State = p_receiver->StateInformation.State;
    
    if (ReceiverMode_AlwaysConnect == p_receiver->Receiver.Configuration.Mode)
    {
        p_receiver->StateInformation.State = ReceiverState_Connected;
        p_receiver->StateInformation.AutoCreated = FALSE;
    }
    else if (ReceiverMode_AutoConnect == p_receiver->Receiver.Configuration.Mode)
    {
        p_receiver->StateInformation.State = ReceiverState_Connected;
        p_receiver->StateInformation.AutoCreated = TRUE;
    }
    else 
    {
        p_receiver->StateInformation.State = ReceiverState_NotConnected;
        p_receiver->StateInformation.AutoCreated = FALSE;
    }

    if (State != p_receiver->StateInformation.State)
    {
        onvif_ReceiverChangeStateNotify(p_receiver);
    }   
    
    return ONVIF_OK;
}

/**
 * The possible return values:
 *  ONVIF_ERR_UnknownToken
 **/
ONVIF_RET onvif_trv_GetReceiverState(trv_GetReceiverState_REQ * p_req, trv_GetReceiverState_RES * p_res)
{
    ReceiverList * p_receiver = onvif_find_Receiver(g_onvif_cfg.receiver, p_req->ReceiverToken);
    if (NULL == p_receiver)
    {
        return ONVIF_ERR_UnknownToken;
    }

    memcpy(&p_res->ReceiverState, &p_receiver->StateInformation, sizeof(onvif_ReceiverStateInformation));

    return ONVIF_OK;
}

#endif // end of RECEIVER_SUPPORT



