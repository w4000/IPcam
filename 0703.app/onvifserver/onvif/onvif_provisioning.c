

#include "sys_inc.h"
#include "onvif.h"
#include "onvif_provisioning.h"


#ifdef PROVISIONING_SUPPORT

/************************************************************************************/
extern ONVIF_CLS g_onvif_cls;
extern ONVIF_CFG g_onvif_cfg;

/************************************************************************************/

/**
 * Possible return value : 
 *  ONVIF_ERR_NoSource
 *  ONVIF_ERR_NoProvisioning
**/
ONVIF_RET onvif_tpv_PanMove(tpv_PanMove_REQ * p_req)
{
    VideoSourceList * p_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSource);
    if (NULL == p_src)
    {
        return ONVIF_ERR_NoSource;
    }
    
    // todo : add your handler code ...

    return ONVIF_OK;
}

/**
 * Possible return value : 
 *  ONVIF_ERR_NoSource
 *  ONVIF_ERR_NoProvisioning
**/
ONVIF_RET onvif_tpv_TiltMove(tpv_TiltMove_REQ * p_req)
{
    VideoSourceList * p_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSource);
    if (NULL == p_src)
    {
        return ONVIF_ERR_NoSource;
    }
    
    // todo : add your handler code ...

    return ONVIF_OK;
}

/**
 * Possible return value : 
 *  ONVIF_ERR_NoSource
 *  ONVIF_ERR_NoProvisioning
**/
ONVIF_RET onvif_tpv_ZoomMove(tpv_ZoomMove_REQ * p_req)
{
    VideoSourceList * p_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSource);
    if (NULL == p_src)
    {
        return ONVIF_ERR_NoSource;
    }
    
    // todo : add your handler code ...

    return ONVIF_OK;
}

/**
 * Possible return value : 
 *  ONVIF_ERR_NoSource
 *  ONVIF_ERR_NoProvisioning
 *  ONVIF_ERR_NoAutoMode
**/
ONVIF_RET onvif_tpv_RollMove(tpv_RollMove_REQ * p_req)
{
    VideoSourceList * p_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSource);
    if (NULL == p_src)
    {
        return ONVIF_ERR_NoSource;
    }
    
    // todo : add your handler code ...

    return ONVIF_OK;
}

/**
 * Possible return value : 
 *  ONVIF_ERR_NoSource
 *  ONVIF_ERR_NoProvisioning
 *  ONVIF_ERR_NoAutoMode
**/
ONVIF_RET onvif_tpv_FocusMove(tpv_FocusMove_REQ * p_req)
{
    VideoSourceList * p_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSource);
    if (NULL == p_src)
    {
        return ONVIF_ERR_NoSource;
    }
    
    // todo : add your handler code ...

    return ONVIF_OK;
}

/**
 * Possible return value : 
 *  ONVIF_ERR_NoSource
**/
ONVIF_RET onvif_tpv_Stop(tpv_Stop_REQ * p_req)
{
    VideoSourceList * p_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSource);
    if (NULL == p_src)
    {
        return ONVIF_ERR_NoSource;
    }
    
    // todo : add your handler code ...

    return ONVIF_OK;
}

/**
 * Possible return value : 
 *  ONVIF_ERR_NoSource
**/
ONVIF_RET onvif_tpv_GetUsage(tpv_GetUsage_REQ * p_req, tpv_GetUsage_RES * p_res)
{
    VideoSourceList * p_src = onvif_find_VideoSource(g_onvif_cfg.v_src, p_req->VideoSource);
    if (NULL == p_src)
    {
        return ONVIF_ERR_NoSource;
    }
    
    // todo : add your handler code ...
    

    return ONVIF_OK;
}


#endif // end of #ifdef PROVISIONING_SUPPORT



