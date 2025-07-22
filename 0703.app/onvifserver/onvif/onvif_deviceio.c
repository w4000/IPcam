

#ifdef DEVICEIO_SUPPORT

#include "sys_inc.h"
#include "onvif_deviceio.h"
#include "onvif_event.h"
#include "onvif_timer.h"

extern ONVIF_CFG g_onvif_cfg;

/***************************************************************************************/

void onvif_DigitalInputStateNotify(DigitalInputList * p_req)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Device/Trigger/DigitalInput", PropertyOperation_Changed, 
        "InputToken", p_req->DigitalInput.token, NULL, NULL, 
        "LogicalState", (p_req->DigitalInput.IdleState == DigitalIdleState_closed) ? "true" : "false", NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

void onvif_RelayOutputStateNotify(RelayOutputList * p_req, int state)
{
    NotificationMessageList * p_message = onvif_init_NotificationMessage3(
        "tns1:Device/Trigger/Relay", (onvif_PropertyOperation)state, 
        "RelayToken", p_req->RelayOutput.token, NULL, NULL, 
        "LogicalState", onvif_RelayLogicalStateToString(p_req->RelayLogicalState), NULL, NULL);
    if (p_message)
    {
        onvif_put_NotificationMessage(p_message);
    }
}

void onvif_RelayLogicalStateChanged(void * argv)
{
    RelayOutputList * p_output = (RelayOutputList *)argv;

    p_output->RelayLogicalState = RelayLogicalState_inactive;

    // send notify message    
    onvif_RelayOutputStateNotify(p_output, PropertyOperation_Changed);
}

ONVIF_RET onvif_tmd_GetRelayOutputs()
{

    return ONVIF_OK;
}

/**
 * possible retrun value:
 *  ONVIF_OK
 *  ONVIF_ERR_NoVideoOutput
 *  ONVIF_ERR_ConfigModify
 */
ONVIF_RET onvif_tmd_SetVideoOutputConfiguration(tmd_SetVideoOutputConfiguration_REQ * p_req)
{
    VideoOutputList * p_output;
    VideoOutputConfigurationList * p_cfg = onvif_find_VideoOutputConfiguration(g_onvif_cfg.v_output_cfg, p_req->Configuration.token);
    if (NULL == p_cfg)
    {
        return ONVIF_ERR_NoVideoOutput;
    }

    p_output = onvif_find_VideoOutput(g_onvif_cfg.v_output, p_req->Configuration.OutputToken);
    if (NULL == p_output)
    {
        return ONVIF_ERR_NoVideoOutput;
    }

    // todo : here add process code ...

    
    strcpy(p_cfg->Configuration.Name, p_req->Configuration.Name);
    strcpy(p_cfg->Configuration.OutputToken, p_req->Configuration.OutputToken);
    
    return ONVIF_OK;
}

/**
 * possible retrun value:
 *  ONVIF_OK
 *  ONVIF_ERR_NoAudioOutput
 *  ONVIF_ERR_ConfigModify
 */
ONVIF_RET onvif_tmd_SetAudioOutputConfiguration(tmd_SetAudioOutputConfiguration_REQ * p_req)
{
    AudioOutputList * p_output;
    AudioOutputConfigurationList * p_cfg = onvif_find_AudioOutputConfiguration(g_onvif_cfg.a_output_cfg, p_req->Configuration.token);
    if (NULL == p_cfg)
    {
        return ONVIF_ERR_NoAudioOutput;
    }

    p_output = onvif_find_AudioOutput(g_onvif_cfg.a_output, p_req->Configuration.OutputToken);
    if (NULL == p_output)
    {
        return ONVIF_ERR_NoAudioOutput;
    }

    if (p_req->Configuration.OutputLevel < p_cfg->Options.OutputLevelRange.Min || 
        p_req->Configuration.OutputLevel > p_cfg->Options.OutputLevelRange.Max)
    {
        return ONVIF_ERR_ConfigModify;
    }
    
    // todo : here add process code ...

    
    strcpy(p_cfg->Configuration.Name, p_req->Configuration.Name);
    strcpy(p_cfg->Configuration.OutputToken, p_req->Configuration.OutputToken);
    if (p_req->Configuration.SendPrimacyFlag)
    {
        strcpy(p_cfg->Configuration.SendPrimacy, p_req->Configuration.SendPrimacy);
    }
    p_cfg->Configuration.OutputLevel = p_req->Configuration.OutputLevel;

    return ONVIF_OK;
}

/**
 * possible retrun value:
 *  ONVIF_OK
 *  ONVIF_ERR_RelayToken
 *  ONVIF_ERR_ModeError
 */
ONVIF_RET onvif_tmd_SetRelayOutputSettings(tmd_SetRelayOutputSettings_REQ * p_req)
{

    return ONVIF_OK;
}

/**
 * possible retrun value:
 *  ONVIF_OK
 *  ONVIF_ERR_RelayToken
 */
ONVIF_RET onvif_tmd_SetRelayOutputState(tmd_SetRelayOutputState_REQ * p_req)
{
    return -1;
    RelayOutputList * p_output = onvif_find_RelayOutput(g_onvif_cfg.relay_output, p_req->RelayOutputToken);
    if (NULL == p_output)
    {
        return ONVIF_ERR_RelayToken;
    }


    if (p_output->RelayLogicalState != p_req->LogicalState)
    {
        p_output->RelayLogicalState = p_req->LogicalState;

        // send notify message    
        onvif_RelayOutputStateNotify(p_output, PropertyOperation_Changed);

        if (p_output->RelayLogicalState == RelayLogicalState_active)
        {
            timer_add(5, onvif_RelayLogicalStateChanged, p_output, TIMER_MODE_SINGLE);
        }
    }

    return ONVIF_OK;
}

/**
 * possible retrun value:
 *  ONVIF_OK
 *  ONVIF_ERR_NoConfig
 *  ONVIF_ERR_SettingsInvalid
 */
ONVIF_RET onvif_tmd_SetDigitalInputConfigurations(tmd_SetDigitalInputConfigurations_REQ * p_req)
{
    DigitalInputList * p_input = p_req->DigitalInputs;

    
    while (p_input)
    {
        DigitalInputList * p_tmp = onvif_find_DigitalInput(g_onvif_cfg.digit_input, p_input->DigitalInput.token);
        if (p_tmp)
        {
            if (p_input->DigitalInput.IdleStateFlag)
            {
                if (p_tmp->DigitalInput.IdleState != p_input->DigitalInput.IdleState)
                {
                    p_tmp->DigitalInput.IdleState = p_input->DigitalInput.IdleState;

                    // send notify message    
                    onvif_DigitalInputStateNotify(p_tmp);
                }
            }
        }
        
        p_input = p_input->next;
    }

    return ONVIF_OK;
}

/**
 * possible retrun value:
 *  ONVIF_OK
 *  ONVIF_ERR_ConfigModify
 *  ONVIF_ERR_InvalidSerialPort
 */
ONVIF_RET onvif_tmd_SetSerialPortConfiguration(tmd_SetSerialPortConfiguration_REQ * p_req)
{
    SerialPortList * p_port = onvif_find_SerialPort_by_ConfigurationToken(g_onvif_cfg.serial_port, p_req->SerialPortConfiguration.token);
    if (NULL == p_port)
    {
        return ONVIF_ERR_InvalidSerialPort;
    }

    // todo : here add process code ...


    memcpy(&p_port->Configuration, &p_req->SerialPortConfiguration, sizeof(onvif_SerialPortConfiguration));

    return ONVIF_OK;
}

/**
 * possible retrun value:
 *  ONVIF_OK
 *  ONVIF_ERR_InvalidSerialPort
 *  ONVIF_ERR_DataLengthOver
 *  ONVIF_ERR_DelimiterNotSupported
 */
ONVIF_RET onvif_tmd_SendReceiveSerialCommandRx(tmd_SendReceiveSerialCommand_REQ * p_req, tmd_SendReceiveSerialCommand_RES * p_res)
{
    SerialPortList * p_port = onvif_find_SerialPort_by_ConfigurationToken(g_onvif_cfg.serial_port, p_req->token);
    if (NULL == p_port)
    {
        return ONVIF_ERR_InvalidSerialPort;
    }
    
    // todo : here add process code ...

    
    return ONVIF_OK;
}


#endif // DEVICEIO_SUPPORT

