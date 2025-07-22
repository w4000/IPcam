

#include "sys_inc.h"
#include "onvif.h"
#include "onvif_event.h"
#include "http_cln.h"
#include "http_srv.h"
#include "onvif_pkt.h"
#include "onvif_utils.h"
#include "hqueue.h"
#include "xml_node.h"
#include "soap.h"
#ifdef PROFILE_G_SUPPORT
#include "onvif_recording.h"
#endif


/***************************************************************************************/
extern ONVIF_CFG 	g_onvif_cfg;
extern ONVIF_CLS	g_onvif_cls;

/***************************************************************************************/
void _eua_init()
{
	g_onvif_cls.eua_fl = pps_ctx_fl_init(MAX_NUM_EUA, sizeof(EUA), TRUE);
	g_onvif_cls.eua_ul = pps_ctx_ul_init(g_onvif_cls.eua_fl, TRUE);
}

void onvif_eua_deinit()
{
	if (g_onvif_cls.eua_ul)
	{
		pps_ul_free(g_onvif_cls.eua_ul);
		g_onvif_cls.eua_ul = NULL;
	}

	if (g_onvif_cls.eua_fl)
	{
		pps_fl_free(g_onvif_cls.eua_fl);
		g_onvif_cls.eua_fl = NULL;
	}
}

EUA * onvif_get_idle_eua()
{
	EUA * p_eua = (EUA *)pps_fl_pop(g_onvif_cls.eua_fl);
	if (p_eua)
	{
		memset(p_eua, 0, sizeof(EUA));

		pps_ctx_ul_add(g_onvif_cls.eua_ul, p_eua);
	}

	return p_eua;
}

void onvif_set_idle_eua(EUA * p_eua)
{
    pps_ctx_ul_del(g_onvif_cls.eua_ul, p_eua);

    if (p_eua->pullmsg.used_flag)
    {
        HTTPCLN * p_cln = (HTTPCLN *)p_eua->pullmsg.http_cln;

        http_free_used_cln((HTTPSRV *)p_cln->p_srv, p_cln);
    }

    if (p_eua->msg_list)
    {
		// clear notify messeage list
		while (h_list_get_number_of_nodes(p_eua->msg_list) > 0)
		{
			LINKED_NODE * p_node = h_list_get_from_front(p_eua->msg_list);
			onvif_free_NotificationMessage((NotificationMessageList *) p_node->p_data);

			h_list_remove_from_front(p_eua->msg_list);
		}

        h_list_free_container(p_eua->msg_list);
    }

	memset(p_eua, 0, sizeof(EUA));

	pps_fl_push(g_onvif_cls.eua_fl, p_eua);
}

void onvif_free_used_eua(EUA * p_eua)
{
	if (pps_safe_node(g_onvif_cls.eua_fl, p_eua) == FALSE)
		return;

	onvif_set_idle_eua(p_eua);
}


uint32 onvif_get_eua_index(EUA * p_eua)
{
	return pps_get_index(g_onvif_cls.eua_fl, p_eua);
}

EUA * onvif_get_eua_by_index(uint32 index)
{
	return (EUA *)pps_get_node_by_index(g_onvif_cls.eua_fl, index);
}

EUA * onvif_eua_lookup_start()
{
	return (EUA *)pps_lookup_start(g_onvif_cls.eua_ul);
}

EUA * onvif_eua_lookup_next(void * p_eua)
{
	return (EUA *)pps_lookup_next(g_onvif_cls.eua_ul, (EUA *)p_eua);
}

void onvif_eua_lookup_stop()
{
	pps_lookup_end(g_onvif_cls.eua_ul);
}

static int onvif_compare_addr_except_port(const char *addr1, const char *addr2)
{
	char host1[32], host2[32];
	char url1[256], url2[256];
	int port1, port2;
	int https1, https2;

	onvif_parse_xaddr(addr1, host1, url1, &port1, &https1);
	onvif_parse_xaddr(addr2, host2, url2, &port2, &https2);

	if(strncmp(host1, host2, sizeof(host1)) == 0 ||	strncmp(url1, url2, sizeof(url1)) == 0) {
		return 0;
	}

	return -1;
}

EUA * onvif_eua_lookup_by_addr(const char * addr)
{
	EUA * p_eua = onvif_eua_lookup_start();
	while (p_eua)
	{
		if (strcmp(p_eua->producter_addr, addr) == 0 || onvif_compare_addr_except_port(p_eua->producter_addr, addr) == 0)
			break;

		p_eua = onvif_eua_lookup_next((void *)p_eua);
	}
	onvif_eua_lookup_stop();

	return p_eua;
}

BOOL onvif_put_NotificationMessage(NotificationMessageList * p_message)
{
    EUA * p_eua;

	p_eua = onvif_eua_lookup_start();
	while (p_eua)
	{
	    if (p_eua->pollMode)
	    {
	        if (h_list_get_number_of_nodes(p_eua->msg_list) >= 20) // max 20 notify message
    		{
    			LINKED_NODE * p_node = h_list_get_from_front(p_eua->msg_list);
    			onvif_free_NotificationMessage((NotificationMessageList *) p_node->p_data);

    			h_list_remove_from_front(p_eua->msg_list);
    		}

    		p_message->refcnt++;

    		h_list_add_at_back(p_eua->msg_list, (void *)p_message);
	    }
	    else
	    {
	        onvif_notify(p_eua, p_message);
	    }

	    p_eua = onvif_eua_lookup_next(p_eua);
	}
    onvif_eua_lookup_stop();

	onvif_free_NotificationMessage(p_message);

	return TRUE;
}

BOOL onvif_put_InitNotificationMessage(EUA *p_eua, NotificationMessageList * p_message)
{
	if (p_eua)
	{
	    if (p_eua->pollMode)
	    {
	        if (h_list_get_number_of_nodes(p_eua->msg_list) >= 20) // max 20 notify message
    		{
    			LINKED_NODE * p_node = h_list_get_from_front(p_eua->msg_list);
    			onvif_free_NotificationMessage((NotificationMessageList *) p_node->p_data);

    			h_list_remove_from_front(p_eua->msg_list);
    		}

    		p_message->refcnt++;

    		h_list_add_at_back(p_eua->msg_list, (void *)p_message);
	    }
	    else
	    {
	        onvif_notify(p_eua, p_message);
	    }
	}

	onvif_free_NotificationMessage(p_message);

	return TRUE;
}

HQUEUE * onvif_xpath_parse(char * xpath)
{
    BOOL ret = TRUE;
    HQUEUE * queue = hqCreate(100, 128, 0);

    int i = 0, fpush = 0;
    int quote_nums = 0;
	char fbuf[100];
	char buff[128];
    char * p = xpath;

    while (*p != '\0')
    {
        switch (*p)
        {
        case ' ':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
            break;

        case '(':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}

			fbuf[fpush++] = '(';
            break;

        case '/':
            if (*(p+1) == '/') p++;
            break;

        case ':':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
            break;

        case '[':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}

			fbuf[fpush++] = '[';
            break;

        case '=':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
            break;

        case ']':
			if (fpush == 0)
			{
				return FALSE;
			}
			if (fbuf[--fpush] != '[')
			{
				ret = FALSE;
				break;
			}

			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
            break;

		case '"':
			if (quote_nums == 0)
			{
				quote_nums++;
				i = 0;
			}
			else if (quote_nums == 1)
			{
				quote_nums = 0;

				if (i > 0)
				{
					buff[i] = '\0';
					hqBufPut(queue, buff);
					i = 0;
				}
				else
				{
					ret = FALSE;
				    break;
				}
			}
			break;

        case ')':
			if (fpush == 0)
			{
				ret = FALSE;
				break;
			}
			if (fbuf[--fpush] != '(')
			{
				ret = FALSE;
				break;
			}

			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
            break;

        default:
            buff[i++] = *p;
            break;
        }

        p++;
    }

    if (ret == FALSE || fpush != 0)
    {
        hqDelete(queue);
        queue = NULL;
    }

    return queue;
}

ONVIF_RET onvif_check_filters(ONVIF_FILTER * p_filter)
{
    int i;

    for (i = 0; i < MAX_FILTER_NUMS; i++)
    {
        if (p_filter->TopicExpression[i][0] != '\0')
        {
            // todo : check if support the topic ...
            if (strlen(p_filter->TopicExpression[i]) <= 5)
            {
                return ONVIF_ERR_InvalidTopicExpressionFault;
            }
        }

        if (p_filter->MessageContent[i][0] != '\0')
        {
            // todo : check the message content is valid ...

            HQUEUE * queue = onvif_xpath_parse(p_filter->MessageContent[i]);
            if (NULL == queue)
            {
                return ONVIF_ERR_InvalidMessageContentExpressionFault;
            }
            hqDelete(queue);
        }
    }

    return ONVIF_OK;
}

/***************************************************************************************/

ONVIF_RET onvif_tev_Subscribe(const char * lip, uint32 lport, tev_Subscribe_REQ * p_req)
{
    EUA * p_eua;

    if (p_req->FiltersFlag)
    {
        ONVIF_RET ret = onvif_check_filters(&p_req->Filters);
        if (ONVIF_OK != ret)
        {
            return ret;
        }
    }

	p_eua = onvif_get_idle_eua();
	if (p_eua)
	{
		if (p_req->InitialTerminationTimeFlag)
		{
			p_eua->init_term_time = p_req->InitialTerminationTime;
		}
		else
		{
		    p_eua->init_term_time = g_onvif_cfg.evt_renew_time;
		}

		strcpy(p_eua->consumer_addr, p_req->ConsumerReference);
		onvif_parse_xaddr(p_eua->consumer_addr, p_eua->host, p_eua->url, &p_eua->port, &p_eua->https);


		if (lport == 80)
		{
			sprintf(p_eua->producter_addr, "http://%s/event_service/%u", lip, onvif_get_eua_index(p_eua));
		}
		else
		{
			sprintf(p_eua->producter_addr, "http://%s:%d/event_service/%u", lip, lport, onvif_get_eua_index(p_eua));
		}


		p_eua->subscibe_time = time(NULL);
		p_eua->last_renew_time = time(NULL);

		p_eua->used = 1;

		p_eua->FiltersFlag = p_req->FiltersFlag;
        memcpy(&p_eua->Filters, &p_req->Filters, sizeof(ONVIF_FILTER));

		p_req->p_eua = p_eua;
	}
	else
	{
	    return ONVIF_ERR_ResourceUnknownFault;
	}

	return ONVIF_OK;
}

ONVIF_RET onvif_tev_Renew(tev_Renew_REQ * p_req)
{
	EUA * p_eua = onvif_eua_lookup_by_addr(p_req->ProducterReference);
	if (p_eua)
	{
		p_eua->last_renew_time = time(NULL);

		return ONVIF_OK;
	}

	return ONVIF_ERR_ResourceUnknownFault;
}

ONVIF_RET onvif_tev_Unsubscribe(const char * addr)
{
	EUA * p_eua = onvif_eua_lookup_by_addr(addr);
	if (p_eua)
	{
		onvif_free_used_eua(p_eua);

		return ONVIF_OK;
	}

	return ONVIF_ERR_ResourceUnknownFault;
}

ONVIF_RET onvif_tev_CreatePullPointSubscription(const char * lip, uint32 lport, tev_CreatePullPointSubscription_REQ * p_req)
{
    EUA * p_eua;

    if (p_req->FiltersFlag)
    {
        ONVIF_RET ret = onvif_check_filters(&p_req->Filters);
        if (ONVIF_OK != ret)
        {
            return ret;
        }
    }

	p_eua = onvif_get_idle_eua();
	if (p_eua)
	{
		if (p_req->InitialTerminationTimeFlag)
		{
			p_eua->init_term_time = p_req->InitialTerminationTime;
		}
        else
		{
		    p_eua->init_term_time = g_onvif_cfg.evt_renew_time;
		}
		if (lport == 80)
		{
			sprintf(p_eua->producter_addr, "http://%s/onvif/event_service/%u", lip, onvif_get_eua_index(p_eua));
		}
		else
		{
			sprintf(p_eua->producter_addr, "http://%s:%d/onvif/event_service/%u", lip, lport, onvif_get_eua_index(p_eua));
		}

		p_eua->subscibe_time = time(NULL);
		p_eua->last_renew_time = time(NULL);

		p_eua->pollMode = TRUE;
		p_eua->used = 1;

		p_eua->msg_list = h_list_create(TRUE);

		p_eua->FiltersFlag = p_req->FiltersFlag;
        memcpy(&p_eua->Filters, &p_req->Filters, sizeof(ONVIF_FILTER));

		p_req->p_eua = p_eua;
	}
	else
	{
	    return ONVIF_ERR_ResourceUnknownFault;
	}

	return ONVIF_OK;
}

ONVIF_RET onvif_tev_SetSynchronizationPoint()
{
    // todo : here add handler code ...

    return ONVIF_OK;
}

ONVIF_RET onvif_tev_PullMessages(tev_PullMessages_REQ * p_req)
{
	EUA * p_eua = onvif_get_eua_by_index(p_req->eua_idx);
	if (p_eua)
	{
		p_eua->last_renew_time = time(NULL);

        p_eua->pullmsg.used_flag = 1;
        p_eua->pullmsg.req_time = time(NULL);
        memcpy(&p_eua->pullmsg.req, p_req, sizeof(tev_PullMessages_REQ));

		return ONVIF_OK;
	}

	return ONVIF_ERR_ResourceUnknownFault;
}

void onvif_event_renew()
{
    uint32 i;
	time_t cur_time = time(NULL);
	EUA * p_eua = NULL;
	int renew_time;

	// event agent renew handler

	for (i=0; i<MAX_NUM_EUA; i++)
	{
		p_eua = (EUA *) onvif_get_eua_by_index(i);
		if (p_eua == NULL || p_eua->used == 0)
		{
			continue;
		}

		renew_time = (p_eua->init_term_time > g_onvif_cfg.evt_renew_time ? g_onvif_cfg.evt_renew_time : p_eua->init_term_time);

		/* timer check */
		if ((cur_time - p_eua->last_renew_time) >= renew_time)
		{
			onvif_free_used_eua(p_eua);
			continue;
		}
		else if (p_eua->pullmsg.used_flag)
		{
		    int msgnums = h_list_get_number_of_nodes(p_eua->msg_list);
#if 0
		    if (cur_time - p_eua->pullmsg.req_time + 2 > p_eua->pullmsg.req.Timeout ||
		        msgnums >= p_eua->pullmsg.req.MessageLimit)
#else
		    if (cur_time - p_eua->pullmsg.req_time + 2 > p_eua->pullmsg.req.Timeout ||
		        msgnums > 0)
#endif
		    {
		        HTTPCLN * p_cln = (HTTPCLN *)p_eua->pullmsg.http_cln;

		        _bdsend_rly(p_cln, NULL, bdtev_PullMessages_rly_xml, (char *)&p_eua->pullmsg.req,
				    "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesResponse", NULL);

				http_free_used_cln((HTTPSRV *)p_cln->p_srv, p_cln);

                p_eua->pullmsg.http_cln = NULL;
				p_eua->pullmsg.used_flag = 0;
		    }
		}
	}
}

void onvif_notify(EUA * p_eua, NotificationMessageList * p_message)
{
    int len;
	int buflen = 10*1024;
	char * bufs;
	HTTPREQ	req;

	memset(&req, 0, sizeof(HTTPREQ));

	strcpy(req.host, p_eua->host);
	strcpy(req.url, p_eua->url);
	strcpy(req.action, "http://docs.oasis-open.org/wsn/bw-2/NotificationConsumer/Notify");
	req.port = p_eua->port;
	req.https = p_eua->https;

    if (!onvif_event_filter(p_message, p_eua))
    {
		return;
	}

	bufs = (char *)malloc(buflen);
	if (NULL == bufs)
	{
		return;
	}

	len = bdNotify_xml(bufs, buflen-1, (char *)p_message);

	http_onvif_trans(&req, 3000, bufs, len);

	free(bufs);
}

BOOL onvif_simpleitem_filter(SimpleItemList * p_item, char * name, char * value, BOOL flag)
{
    BOOL nameflag = FALSE;
    BOOL valueflag = FALSE;

    while (p_item)
    {
        if (strcmp(p_item->SimpleItem.Name, name) == 0)
        {
            nameflag = TRUE;
        }

        if (value[0] != '\0')
        {
            if (strcmp(p_item->SimpleItem.Value, value) == 0)
            {
                valueflag = TRUE;
            }

            if (flag == 1)
            {
                if (nameflag && valueflag)
                {
                    return TRUE;
                }
            }
            else if (flag == 2)
            {
                if (nameflag || valueflag)
                {
                    return TRUE;
                }
            }
        }
        else if (nameflag)
        {
            return TRUE;
        }

        p_item = p_item->next;
    }

    return FALSE;
}

BOOL onvif_elementitem_filter(ElementItemList * p_item, char * name)
{
    while (p_item)
    {
        if (strcmp(p_item->ElementItem.Name, name) == 0)
        {
            return TRUE;
        }

        p_item = p_item->next;
    }

    return FALSE;
}


BOOL onvif_message_content_filter_ex(HQUEUE  * queue, NotificationMessageList * p_message)
{
	int  flag = 0;
	int  itemflag = 0;
	char buff[128];
	char name[100];
	char value[100] = {'\0'};

	if (hqBufGet(queue, buff) == FALSE) // prefix
	{
		return FALSE;
	}

	if (hqBufGet(queue, buff) == FALSE) // SimpleItem or ElementItem
	{
		return FALSE;
	}

	if (strcasecmp(buff, "simpleitem") == 0)
	{
		itemflag = 0;
	}
	else if (strcasecmp(buff, "ElementItem") == 0)
	{
		itemflag = 1;
	}
	else
	{
		return FALSE;
	}

	if (hqBufGet(queue, buff) == FALSE) // @Name
	{
		return FALSE;
	}
	if (strcasecmp(buff, "@name") != 0)
	{
		return FALSE;
	}

	if (hqBufGet(queue, buff) == FALSE)
	{
		return FALSE;
	}
	strcpy(name, buff);

	if (hqBufPeek(queue, buff))
	{
		if (strcasecmp(buff, "and") == 0)
		{
			flag = 1;
		}
		else if (strcasecmp(buff, "or") == 0)
		{
			flag = 2;
		}
		else
		{
			goto finish;
		}

		hqBufGet(queue, buff);

		if (hqBufGet(queue, buff) == FALSE)
		{
			return FALSE;
		}
		if (strcasecmp(buff, "@value") != 0)
		{
			return FALSE;
		}

		if (hqBufGet(queue, buff) == FALSE)
		{
			return FALSE;
		}
		strcpy(value, buff);
	}

finish:

    if (itemflag == 0)
    {
        if (p_message->NotificationMessage.Message.SourceFlag && onvif_simpleitem_filter(p_message->NotificationMessage.Message.Source.SimpleItem, name, value, flag))
        {
            return TRUE;
        }

        if (p_message->NotificationMessage.Message.KeyFlag && onvif_simpleitem_filter(p_message->NotificationMessage.Message.Key.SimpleItem, name, value, flag))
        {
            return TRUE;
        }

        if (p_message->NotificationMessage.Message.DataFlag && onvif_simpleitem_filter(p_message->NotificationMessage.Message.Data.SimpleItem, name, value, flag))
        {
            return TRUE;
        }
    }
    else if (itemflag == 1)
    {
        if (p_message->NotificationMessage.Message.SourceFlag && onvif_elementitem_filter(p_message->NotificationMessage.Message.Source.ElementItem, name))
        {
            return TRUE;
        }

        if (p_message->NotificationMessage.Message.KeyFlag && onvif_elementitem_filter(p_message->NotificationMessage.Message.Key.ElementItem, name))
        {
            return TRUE;
        }

        if (p_message->NotificationMessage.Message.DataFlag && onvif_elementitem_filter(p_message->NotificationMessage.Message.Data.ElementItem, name))
        {
            return TRUE;
        }
    }

	return FALSE;
}

BOOL onvif_message_content_filter(char * filter, NotificationMessageList * p_message)
{
    BOOL notflag = FALSE;
    char buff[128];
    BOOL ret = FALSE;
    int  flag = 0;

    HQUEUE * queue = onvif_xpath_parse(filter);
    if (NULL == queue)
    {
        return FALSE;
    }

    while (!hqBufIsEmpty(queue))
    {
        hqBufGet(queue, buff);

        if (strcmp(buff, "not") == 0)
        {
            notflag = TRUE;
        }
        else if (strcmp(buff, "boolean") == 0)
        {
            BOOL ret1 = onvif_message_content_filter_ex(queue, p_message);
			if (notflag)
			{
				ret1 = !ret1;
				notflag = FALSE;
			}

			if (flag == 1)
			{
			    ret = ret && ret1;
			}
			else if (flag == 2)
			{
			    ret = ret || ret1;
			}
			else
			{
			    ret = ret1;
			}
        }
		else if (strcmp(buff, "and") == 0)
		{
			flag = 1;
		}
		else if (strcmp(buff, "or") == 0)
		{
			flag = 2;
		}
    }

    hqDelete(queue);

    return ret;
}

BOOL onvif_topic_filter(NotificationMessageList * p_message, char * topic)
{
    int len;

    len = (int)strlen(topic);

    if (topic[len-1] == '*')
    {
        topic[len-1] = '\0';

        if (memcpy(p_message->NotificationMessage.Topic, topic, strlen(topic)) == 0)
        {
            return TRUE;
        }
    }
    else if (len > 4 && topic[len-3] == '/' && topic[len-2] == '/' && topic[len-1] == '.')
    {
        topic[len-3] = '\0';

        if (memcmp(p_message->NotificationMessage.Topic, topic, strlen(topic)) == 0)
        {
            return TRUE;
        }
    }
    else if (strcmp(p_message->NotificationMessage.Topic, topic) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL onvif_event_filter(NotificationMessageList * p_message, EUA * p_eua)
{
    int i;

    if (p_eua->FiltersFlag == 0)
    {
        return TRUE;
    }

    for (i = 0; i < MAX_FILTER_NUMS; i++)
    {
        if (p_eua->Filters.TopicExpression[i][0] != '\0')
        {
            char * p;
            char * tmp;
            char topic[256];

            // todo : check the message is required ...

            tmp = p_eua->Filters.TopicExpression[i];
            p = strchr(tmp, '|');
            while (p)
            {
                memset(topic, 0, sizeof(topic));
                strncpy(topic, tmp, p-tmp);

                if (onvif_topic_filter(p_message, topic))
                {
                    return TRUE;
                }

                tmp = p+1;
                p = strchr(tmp, '|');
            }

            if (tmp)
            {
                strcpy(topic, tmp);

                if (onvif_topic_filter(p_message, topic))
                {
                    return TRUE;
                }
            }
        }

        if (p_eua->Filters.MessageContent[i][0] != '\0')
        {
            // todo : check the message is required ...

            if (onvif_message_content_filter(p_eua->Filters.MessageContent[i], p_message))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL onvif_init_Message(onvif_Message * p_req, onvif_PropertyOperation op, const char * sname1, const char * svalue1, const char * sname2, const char * svalue2, const char *dname1, const char * dvalue1, const char * dname2, const char * dvalue2, const char *dname3, const char * dvalue3, const char * dname4, const char * dvalue4)
{
	SimpleItemList * p_simpleitem;

	if (op != PropertyOperation_Invalid)
	{
    	p_req->PropertyOperationFlag = 1;
    	p_req->PropertyOperation = op;
	}

	p_req->UtcTime = time(NULL);

    if ((sname1 && svalue1) || (sname2 && svalue2))
    {
        p_req->SourceFlag = 1;

        if (sname1 && svalue1)
        {
    		p_simpleitem = onvif_add_SimpleItem(&p_req->Source.SimpleItem);
    		if (p_simpleitem)
    		{
    			strcpy(p_simpleitem->SimpleItem.Name, sname1);
    			strcpy(p_simpleitem->SimpleItem.Value, svalue1);
    		}
		}

		if (sname2 && svalue2)
        {
    		p_simpleitem = onvif_add_SimpleItem(&p_req->Source.SimpleItem);
    		if (p_simpleitem)
    		{
    			strcpy(p_simpleitem->SimpleItem.Name, sname2);
    			strcpy(p_simpleitem->SimpleItem.Value, svalue2);
    		}
		}
	}

	if ((dname1 && dvalue1) || (dname2 && dvalue2))
    {
        p_req->DataFlag = 1;

        if (dname1 && dvalue1)
        {
    		p_simpleitem = onvif_add_SimpleItem(&p_req->Data.SimpleItem);
    		if (p_simpleitem)
    		{
    			strcpy(p_simpleitem->SimpleItem.Name, dname1);
    			strcpy(p_simpleitem->SimpleItem.Value, dvalue1);
    		}
		}

		if (dname2 && dvalue2)
        {
    		p_simpleitem = onvif_add_SimpleItem(&p_req->Data.SimpleItem);
    		if (p_simpleitem)
    		{
    			strcpy(p_simpleitem->SimpleItem.Name, dname2);
    			strcpy(p_simpleitem->SimpleItem.Value, dvalue2);
    		}
		}

		if (dname3 && dvalue3)
        {
    		p_simpleitem = onvif_add_SimpleItem(&p_req->Data.SimpleItem);
    		if (p_simpleitem)
    		{
    			strcpy(p_simpleitem->SimpleItem.Name, dname3);
    			strcpy(p_simpleitem->SimpleItem.Value, dvalue3);
    		}
		}

		if (dname4 && dvalue4)
        {
    		p_simpleitem = onvif_add_SimpleItem(&p_req->Data.SimpleItem);
    		if (p_simpleitem)
    		{
    			strcpy(p_simpleitem->SimpleItem.Name, dname4);
    			strcpy(p_simpleitem->SimpleItem.Value, dvalue4);
    		}
		}
	}

	return TRUE;
}

/**
 Generate notify message, just for test
*/
NotificationMessageList * onvif_init_NotificationMessage()
{
	NotificationMessageList * p_message = onvif_add_NotificationMessage(NULL);
	if (p_message)
	{
		SimpleItemList * p_simpleitem;

		strcpy(p_message->NotificationMessage.Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
		strcpy(p_message->NotificationMessage.Topic, "tns1:RuleEngine/MotionRegionDetector/Motion");
		p_message->NotificationMessage.Message.PropertyOperationFlag = 1;
		p_message->NotificationMessage.Message.PropertyOperation = PropertyOperation_Initialized;
		p_message->NotificationMessage.Message.UtcTime = time(NULL);

        p_message->NotificationMessage.Message.SourceFlag = 1;

		p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Source.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "VideoSource");
			strcpy(p_simpleitem->SimpleItem.Value, "V_SRC_000");
		}

		p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Source.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "RuleName");
			strcpy(p_simpleitem->SimpleItem.Value, "MotionRegion");
		}

        p_message->NotificationMessage.Message.DataFlag = 1;

		p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "State");
			strcpy(p_simpleitem->SimpleItem.Value, "true");
		}
	}

	return p_message;
}

/**
 Generate notify message, just for test
*/
NotificationMessageList * onvif_init_NotificationMessage1()
{
	NotificationMessageList * p_message = onvif_add_NotificationMessage(NULL);
	if (p_message)
	{
		SimpleItemList * p_simpleitem;

		strcpy(p_message->NotificationMessage.Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
#ifdef DEVICEIO_SUPPORT
        strcpy(p_message->NotificationMessage.Topic, "tns1:Device/Trigger/DigitalInput");
#endif
		p_message->NotificationMessage.Message.PropertyOperationFlag = 1;
		p_message->NotificationMessage.Message.PropertyOperation = PropertyOperation_Initialized;
		p_message->NotificationMessage.Message.UtcTime = time(NULL);

        p_message->NotificationMessage.Message.SourceFlag = 1;

		p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Source.SimpleItem);
		if (p_simpleitem)
		{
#ifdef DEVICEIO_SUPPORT
            strcpy(p_simpleitem->SimpleItem.Name, "InputToken");
			strcpy(p_simpleitem->SimpleItem.Value, "DIGIT_INPUT_000");
#else
			strcpy(p_simpleitem->SimpleItem.Name, "Source");
			strcpy(p_simpleitem->SimpleItem.Value, "VideoSourceToken");
#endif
		}

		p_message->NotificationMessage.Message.DataFlag = 1;

		p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
#ifdef DEVICEIO_SUPPORT
            strcpy(p_simpleitem->SimpleItem.Name, "LogicalState");
			strcpy(p_simpleitem->SimpleItem.Value, "true");
#else
			strcpy(p_simpleitem->SimpleItem.Name, "State");
			strcpy(p_simpleitem->SimpleItem.Value, "true");
#endif
		}
	}

	return p_message;
}

/**
 Generate notify message, just for test
*/
NotificationMessageList * onvif_init_NotificationMessage2()
{
	NotificationMessageList * p_message = onvif_add_NotificationMessage(NULL);
	if (p_message)
	{
		SimpleItemList * p_simpleitem;

		strcpy(p_message->NotificationMessage.Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
#ifdef DEVICEIO_SUPPORT
        strcpy(p_message->NotificationMessage.Topic, "tns1:Device/Trigger/Relay");
#else
        strcpy(p_message->NotificationMessage.Topic, "tns1:VideoSource/ImageTooDark/ImagingService");
#endif
		p_message->NotificationMessage.Message.PropertyOperationFlag = 1;
		p_message->NotificationMessage.Message.PropertyOperation = PropertyOperation_Initialized;
		p_message->NotificationMessage.Message.UtcTime = time(NULL);

        p_message->NotificationMessage.Message.SourceFlag = 1;

		p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Source.SimpleItem);
		if (p_simpleitem)
		{
#ifdef DEVICEIO_SUPPORT
            strcpy(p_simpleitem->SimpleItem.Name, "RelayToken");

            if (g_onvif_cfg.relay_output)
            {
			    strcpy(p_simpleitem->SimpleItem.Value, g_onvif_cfg.relay_output->RelayOutput.token);
			}
			else
			{
			    strcpy(p_simpleitem->SimpleItem.Value, "RELAY_OUTPUT_000");
			}
#else
			strcpy(p_simpleitem->SimpleItem.Name, "Source");
			strcpy(p_simpleitem->SimpleItem.Value, "VideoSourceToken");
#endif
		}

		p_message->NotificationMessage.Message.DataFlag = 1;

		p_simpleitem = onvif_add_SimpleItem(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
#ifdef DEVICEIO_SUPPORT
            strcpy(p_simpleitem->SimpleItem.Name, "LogicalState");

            if (g_onvif_cfg.relay_output)
            {
			    strcpy(p_simpleitem->SimpleItem.Value, onvif_RelayLogicalStateToString(g_onvif_cfg.relay_output->RelayLogicalState));
			}
			else
			{
			    strcpy(p_simpleitem->SimpleItem.Value, "inactive");
			}
#else
			strcpy(p_simpleitem->SimpleItem.Name, "State");
			strcpy(p_simpleitem->SimpleItem.Value, "true");
#endif
		}
	}

	return p_message;
}

NotificationMessageList * onvif_init_NotificationMessage3(const char * topic, onvif_PropertyOperation op, const char * sname1, const char * svalue1, const char * sname2, const char * svalue2, const char *dname1, const char * dvalue1, const char * dname2, const char * dvalue2)
{
    NotificationMessageList * p_message = onvif_add_NotificationMessage(NULL);
	if (p_message)
	{
		strcpy(p_message->NotificationMessage.Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
        strcpy(p_message->NotificationMessage.Topic, topic);

        onvif_init_Message(&p_message->NotificationMessage.Message, op,
            sname1, svalue1, sname2, svalue2,
            dname1, dvalue1, dname2, dvalue2,
            NULL, NULL, NULL, NULL);
	}

	return p_message;
}

NotificationMessageList * onvif_init_NotificationMessage4(const char * topic, onvif_PropertyOperation op, const char * sname1, const char * svalue1, const char * sname2, const char * svalue2, const char *dname1, const char * dvalue1, const char * dname2, const char * dvalue2, const char * dname3, const char * dvalue3, const char * dname4, const char * dvalue4)
{
    NotificationMessageList * p_message = onvif_add_NotificationMessage(NULL);
	if (p_message)
	{
		strcpy(p_message->NotificationMessage.Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
        strcpy(p_message->NotificationMessage.Topic, topic);

        onvif_init_Message(&p_message->NotificationMessage.Message, op,
            sname1, svalue1, sname2, svalue2,
            dname1, dvalue1, dname2, dvalue2,
            dname3, dvalue3, dname4, dvalue4);
	}

	return p_message;
}

void onvif_send_simulate_events(const char * topic)
{
	
    if (soap_strcmp(topic, "VideoSource/ImageTooBlurry/ImagingService")== 0 ||
        soap_strcmp(topic, "VideoSource/ImageTooDark/ImagingService")== 0 ||
        soap_strcmp(topic, "VideoSource/ImageTooBright/ImagingService")== 0 ||
        soap_strcmp(topic, "VideoSource/GlobalSceneChange/ImagingService")== 0 ||
        soap_strcmp(topic, "VideoSource/MotionAlarm")== 0)
    {
        NotificationMessageList * p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "Source", "VideoSourceToken", NULL, NULL, "State", "true", NULL, NULL);
		if (p_message)
		{
			onvif_put_NotificationMessage(p_message);
		}
    }
    else if (soap_strcmp(topic, "RuleEngine/MotionRegionDetector/Motion") == 0)
    {
        NotificationMessageList * p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "VideoSource", "V_SRC_000", "RuleName", "MotionRegion", "State", "true", NULL, NULL);
		if (p_message)
		{
			onvif_put_NotificationMessage(p_message);
		}
    }
    else if (soap_strcmp(topic, "AccessPoint/State/Enabled") == 0)
    {
#ifdef PROFILE_C_SUPPORT
        AccessPointList * p_access_points = g_onvif_cfg.access_points;
        while (p_access_points)
        {
	        NotificationMessageList * p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AccessPointToken", p_access_points->AccessPointInfo.token, NULL, NULL, "State", "true", NULL, NULL);
    		if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}

    		p_access_points = p_access_points->next;
		}
#endif
    }
    else if (soap_strcmp(topic, "Configuration/Area/Changed") == 0 ||
        soap_strcmp(topic, "Configuration/Area/Removed") == 0)
    {
#ifdef PROFILE_C_SUPPORT
        AreaList * p_area = g_onvif_cfg.areas;
        while (p_area)
        {
            NotificationMessageList * p_message;

            if (soap_strcmp(topic, "Configuration/Area/Removed") == 0)
            {
	            p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AreaToken", "testtoken", NULL, NULL, NULL, NULL, NULL, NULL);
            }
            else
            {
                p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AreaToken", p_area->AreaInfo.token, NULL, NULL, NULL, NULL, NULL, NULL);
            }

    		if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}

    		if (soap_strcmp(topic, "Configuration/Area/Removed") == 0)
    		{
    		    break;
    		}

			p_area = p_area->next;
		}
#endif
    }
    else if (soap_strcmp(topic, "Configuration/AccessPoint/Changed") == 0 ||
        soap_strcmp(topic, "Configuration/AccessPoint/Removed") == 0)
    {
#ifdef PROFILE_C_SUPPORT
        AccessPointList * p_access_points = g_onvif_cfg.access_points;
        while (p_access_points)
        {
            NotificationMessageList * p_message;

            if (soap_strcmp(topic, "Configuration/AccessPoint/Removed") == 0)
            {
	            p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AccessPointToken", "testtoken", NULL, NULL, NULL, NULL, NULL, NULL);
            }
            else
            {
                p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AccessPointToken", p_access_points->AccessPointInfo.token, NULL, NULL, NULL, NULL, NULL, NULL);
            }

    		if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}

    		if (soap_strcmp(topic, "Configuration/AccessPoint/Removed") == 0)
    		{
    		    break;
    		}

    		p_access_points = p_access_points->next;
		}
#endif
    }
    else if (soap_strcmp(topic, "AccessControl/AccessGranted/Anonymous") == 0 ||
        soap_strcmp(topic, "AccessControl/AccessTaken/Anonymous") == 0 ||
        soap_strcmp(topic, "AccessControl/AccessNotTaken/Anonymous") == 0 ||
        soap_strcmp(topic, "AccessControl/Denied/Anonymous") == 0 ||
        soap_strcmp(topic, "AccessControl/Duress") == 0)
    {
#ifdef PROFILE_C_SUPPORT
        if (g_onvif_cfg.access_points)
        {
            NotificationMessageList * p_message;

            if (soap_strcmp(topic, "AccessControl/Denied/Anonymous") == 0 ||
                soap_strcmp(topic, "AccessControl/Duress") == 0)
            {
                p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AccessPointToken", g_onvif_cfg.access_points->AccessPointInfo.token, NULL, NULL, "Reason", "CredentialNotEnabled", NULL, NULL);
            }
            else
            {
                p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AccessPointToken", g_onvif_cfg.access_points->AccessPointInfo.token, NULL, NULL, NULL, NULL, NULL, NULL);
            }

    		if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}
        }
#endif
    }
    else if (soap_strcmp(topic, "AccessControl/AccessGranted/Credential") == 0 ||
        soap_strcmp(topic, "AccessControl/AccessTaken/Credential") == 0 ||
        soap_strcmp(topic, "AccessControl/AccessNotTaken/Credential") == 0 ||
        soap_strcmp(topic, "AccessControl/Denied/Credential") == 0 ||
        soap_strcmp(topic, "AccessControl/Denied/CredentialNotFound") == 0 ||
        soap_strcmp(topic, "AccessControl/Denied/CredentialNotFound/Card") == 0 ||
        soap_strcmp(topic, "AccessControl/AccessGranted/Identifier") == 0 ||
        soap_strcmp(topic, "AccessControl/Denied/Identifier") == 0)
    {
#if (defined(PROFILE_C_SUPPORT) && defined(CREDENTIAL_SUPPORT))
        if (g_onvif_cfg.access_points && g_onvif_cfg.credential)
        {
            NotificationMessageList * p_message;

            if (soap_strcmp(topic, "AccessControl/Denied/Credential") == 0)
            {
                p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AccessPointToken", g_onvif_cfg.access_points->AccessPointInfo.token, NULL, NULL, "CredentialToken", g_onvif_cfg.credential->Credential.token, "Reason", "CredentialNotActive");
            }
            else if (soap_strcmp(topic, "AccessControl/Denied/Identifier") == 0)
            {
                p_message = onvif_init_NotificationMessage4(topic, PropertyOperation_Invalid, "AccessPointToken", g_onvif_cfg.access_points->AccessPointInfo.token, NULL, NULL, "IdentifierType", "pt:Card", "FormatType", "GUID", "IdentifierValue", "1A2B3C4D", "Reason", "CredentialNotActive");
            }
            else if (soap_strcmp(topic, "AccessControl/Denied/CredentialNotFound") == 0)
            {
                p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AccessPointToken", g_onvif_cfg.access_points->AccessPointInfo.token, NULL, NULL, "IdentifierType", "pt:Card", "IdentifierValue", "1A2B3C4D");
            }
            else if (soap_strcmp(topic, "AccessControl/Denied/CredentialNotFound/Card") == 0)
            {
                p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AccessPointToken", g_onvif_cfg.access_points->AccessPointInfo.token, NULL, NULL, "Card", "123234534534534", NULL, NULL);
            }
            else if (soap_strcmp(topic, "AccessControl/AccessGranted/Identifier") == 0)
            {
                p_message = onvif_init_NotificationMessage4(topic, PropertyOperation_Invalid, "AccessPointToken", g_onvif_cfg.access_points->AccessPointInfo.token, NULL, NULL, "IdentifierType", "pt:Card", "FormatType", "GUID", "IdentifierValue", "1A2B3C4D", NULL, NULL);
            }
            else
            {
                p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "AccessPointToken", g_onvif_cfg.access_points->AccessPointInfo.token, NULL, NULL, "CredentialToken", g_onvif_cfg.credential->Credential.token, NULL, NULL);
            }

    		if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}
        }
#endif
    }
    else if (soap_strcmp(topic, "Configuration/Door/Changed") == 0 ||
        soap_strcmp(topic, "Configuration/Door/Removed") == 0)
    {
#ifdef PROFILE_C_SUPPORT
        DoorList * p_door = g_onvif_cfg.doors;
        while (p_door)
        {
            NotificationMessageList * p_message;

            if (soap_strcmp(topic, "Configuration/Door/Removed") == 0)
            {
                p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "DoorToken", "testtoken", NULL, NULL, NULL, NULL, NULL, NULL);
            }
            else
            {
	            p_message = onvif_init_NotificationMessage3(topic, PropertyOperation_Initialized, "DoorToken", p_door->Door.DoorInfo.token, NULL, NULL, NULL, NULL, NULL, NULL);
            }

    		if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}

    		if (soap_strcmp(topic, "Configuration/Door/Removed") == 0)
    		{
    		    break;
    		}

    		p_door = p_door->next;
		}
#endif
    }
    else if (soap_strcmp(topic, "Door/State/DoorMode") == 0 ||
        soap_strcmp(topic, "Door/State/DoorPhysicalState") == 0 ||
        soap_strcmp(topic, "Door/State/LockPhysicalState") == 0 ||
        soap_strcmp(topic, "Door/State/DoubleLockPhysicalState") == 0 ||
        soap_strcmp(topic, "Door/State/DoorAlarm") == 0 ||
        soap_strcmp(topic, "Door/State/DoorTamper") == 0 ||
        soap_strcmp(topic, "Door/State/DoorFault") == 0)
    {
#ifdef PROFILE_C_SUPPORT
        DoorList * p_door = g_onvif_cfg.doors;
        while (p_door)
        {
            static int DoorPhysicalState = 1;
            static int LockPhysicalState = 1;
            static int DoubleLockPhysicalState = 1;
            static int DoorAlarm = 1;
            static int DoorTamper = 1;
            static int DoorFault = 1;
            const char * state = "";
            onvif_PropertyOperation op = PropertyOperation_Initialized;
            NotificationMessageList * p_message = NULL;

            if (soap_strcmp(topic, "Door/State/DoorMode") == 0)
            {
                state = onvif_DoorModeToString(p_door->DoorState.DoorMode);
            }
            else if (soap_strcmp(topic, "Door/State/DoorPhysicalState") == 0)
            {
                if (DoorPhysicalState)
                {
                    DoorPhysicalState = 0;
                    op = PropertyOperation_Changed;
                }

                state = onvif_DoorPhysicalStateToString(p_door->DoorState.DoorPhysicalState);
            }
            else if (soap_strcmp(topic, "Door/State/LockPhysicalState") == 0)
            {
                if (LockPhysicalState)
                {
                    LockPhysicalState = 0;
                    op = PropertyOperation_Changed;
                }

                state = onvif_LockPhysicalStateToString(p_door->DoorState.LockPhysicalState);
            }
            else if (soap_strcmp(topic, "Door/State/DoubleLockPhysicalState") == 0)
            {
                if (DoubleLockPhysicalState)
                {
                    DoubleLockPhysicalState = 0;
                    op = PropertyOperation_Changed;
                }

                state = onvif_LockPhysicalStateToString(p_door->DoorState.DoubleLockPhysicalState);
            }
            else if (soap_strcmp(topic, "Door/State/DoorAlarm") == 0)
            {
                if (DoorAlarm)
                {
                    DoorAlarm = 0;
                    op = PropertyOperation_Changed;
                }

                state = onvif_DoorAlarmStateToString(p_door->DoorState.Alarm);
            }
            else if (soap_strcmp(topic, "Door/State/DoorTamper") == 0)
            {
                if (DoorTamper)
                {
                    DoorTamper = 0;
                    op = PropertyOperation_Changed;
                }

                state = onvif_DoorTamperStateToString(p_door->DoorState.Tamper.State);
            }
            else if (soap_strcmp(topic, "Door/State/DoorFault") == 0)
            {
                if (DoorFault)
                {
                    DoorFault = 0;
                    op = PropertyOperation_Changed;
                }

                state = onvif_DoorFaultStateToString(p_door->DoorState.Fault.State);
            }

            if (soap_strcmp(topic, "Door/State/DoorFault") == 0)
            {
                p_message = onvif_init_NotificationMessage3(topic, op, "DoorToken", p_door->Door.DoorInfo.token, NULL, NULL, "State", state, "Reason", p_door->DoorState.Fault.Reason);
            }
            else
            {
	            p_message = onvif_init_NotificationMessage3(topic, op, "DoorToken", p_door->Door.DoorInfo.token, NULL, NULL, "State", state, NULL, NULL);
            }

    		if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}

    		if (soap_strcmp(topic, "Door/State/DoorMode") != 0 &&
    		    op == PropertyOperation_Changed)
            {
                break;
            }

    		p_door = p_door->next;
		}
#endif
    }
    else if (soap_strcmp(topic, "Schedule/State/Active") == 0)
    {
#ifdef SCHEDULE_SUPPORT
        ScheduleList * p_schedule = g_onvif_cfg.schedule;
        while (p_schedule)
        {
            onvif_PropertyOperation op = PropertyOperation_Initialized;
            NotificationMessageList * p_message = NULL;

            if (p_schedule->ScheduleState.Active)
            {
                op = PropertyOperation_Changed;
            }
            else
            {
                op = PropertyOperation_Initialized;
            }

            p_message = onvif_init_NotificationMessage3(topic, op,
                "ScheduleToken", p_schedule->Schedule.token,
                "Name", p_schedule->Schedule.Name,
                "Active", p_schedule->ScheduleState.Active ? "true" : "false",
                "SpecialDay", p_schedule->ScheduleState.SpecialDay ? "true" : "false");
            if (p_message)
    		{
    			onvif_put_NotificationMessage(p_message);
    		}

            p_schedule = p_schedule->next;
        }
#endif
    }
    else if (soap_strcmp(topic, "RecordingConfig/JobState") == 0)
    {
#ifdef PROFILE_G_SUPPORT
        RecordingJobList * p_recordingjob = g_onvif_cfg.recording_jobs;
        while (p_recordingjob)
        {
            onvif_RecordingJobStateNotify(p_recordingjob, PropertyOperation_Initialized);

            p_recordingjob = p_recordingjob->next;
        }
#endif
    }
    else
    {
	    NotificationMessageList * p_message = onvif_init_NotificationMessage1();
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
}




