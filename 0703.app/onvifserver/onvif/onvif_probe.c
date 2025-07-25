

#include "sys_inc.h"
#include "hxml.h"
#include "xml_node.h"
#include "onvif_probe.h"
#include "onvif_device.h"
#include "onvif.h"
#include "onvif_utils.h"

/***************************************************************************************/
extern ONVIF_CFG g_onvif_cfg;
extern ONVIF_CLS g_onvif_cls;

#define ScopeMatchByExact   "ScopeMatchByExact"
#define ScopeMatchByPrefix  "ScopeMatchByPrefix"
#define ScopeMatchByLdap    "ScopeMatchByLdap"
#define ScopeMatchByUuid    "ScopeMatchByUuid"
#define ScopeMatchByNone    "ScopeMatchByNone"

#define ONVIF_GRP_ADDR      "239.255.255.250"
#define ONVIF_GRP_PORT      3702

/***************************************************************************************/
SOCKET onvif_probe_init()
{
    int opt = 1;
    int loop = 0;
    int ttl = 64;
    int addr_len;
	int len = 65535;
	SOCKET fd;
    struct sockaddr_in addr;
    struct ip_mreq mcast;
    
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd <= 0)
	{
		printf("socket SOCK_DGRAM error!\r\n");
		return 0;
	}	

	addr_len = sizeof(addr);

	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(ONVIF_GRP_PORT);

	/* reuse socket addr */  
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))) 
    {  
        printf("setsockopt SO_REUSEADDR error!\n");
    }
    
	if (bind(fd,(struct sockaddr *)&addr,sizeof(addr)) == -1)
	{
		printf("Bind udp socket fail,error = %s\r\n", sys_os_get_socket_error());
		closesocket(fd);
		return 0;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&len, sizeof(int)))
	{
		printf("setsockopt SO_SNDBUF error!\n");
	}

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&len, sizeof(int)))
	{
		printf("setsockopt SO_RCVBUF error!\n");
	}
    
    // setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loop, sizeof(loop));

    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));
    
	mcast.imr_multiaddr.s_addr = inet_addr(ONVIF_GRP_ADDR);
    mcast.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
		printf("setsockopt IP_ADD_MEMBERSHIP error!%s\n", sys_os_get_socket_error());
		return 0;
	}

	return fd;
}

int onvif_bdscopes_text(char * pbuf, int buflen)
{
    int i; 
    int offset = 0;
    
    for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.scopes); i++)
	{
		if (g_onvif_cfg.scopes[i].ScopeItem[0] != '\0')
		{
		    if (i == 0)
		    {
		        offset += snprintf(pbuf+offset, buflen-offset, "%s", g_onvif_cfg.scopes[i].ScopeItem);
		    }
		    else
		    {
			    offset += snprintf(pbuf+offset, buflen-offset, " %s", g_onvif_cfg.scopes[i].ScopeItem);
			}
		}
	}

#ifdef PROFILE_Q_SUPPORT
    offset += snprintf(pbuf+offset, buflen-offset, " onvif://www.onvif.org/Profile/Q/");
    
    if (g_onvif_cfg.device_state)
    {
        offset += snprintf(pbuf+offset, buflen-offset, "Operational");
    }
    else
    {
        offset += snprintf(pbuf+offset, buflen-offset, "FactoryDefault");
    }
#endif

    return offset;
}

BOOL onvif_scope_match(const char * matchby, char * scope)
{
    int i;
    
    for (i = 0; i < ARRAY_SIZE(g_onvif_cfg.scopes); i++)
	{
		if (g_onvif_cfg.scopes[i].ScopeItem[0] != '\0')
		{
		    if (strcmp(matchby, ScopeMatchByExact) == 0)
		    {
		        if (strcmp(scope, g_onvif_cfg.scopes[i].ScopeItem) == 0)
		        {
		            return TRUE;
		        }
		    }
		    else if (memcmp(scope, g_onvif_cfg.scopes[i].ScopeItem, strlen(scope)) == 0)
			{
			    return TRUE;
			}
		}
	}

#ifdef PROFILE_Q_SUPPORT    
    if (g_onvif_cfg.device_state)
    {
        if (strcmp(matchby, ScopeMatchByExact) == 0)
        {
            if (strcmp(scope, "onvif://www.onvif.org/Profile/Q/Operational") == 0)
	        {
	            return TRUE;
	        }
        }
        else if (memcmp(scope, "onvif://www.onvif.org/Profile/Q/Operational", strlen(scope)) == 0)
        {
            return TRUE;
        }
    }
    else
    {
        if (strcmp(matchby, ScopeMatchByExact) == 0)
        {
            if (strcmp(scope, "onvif://www.onvif.org/Profile/Q/FactoryDefault") == 0)
	        {
	            return TRUE;
	        }
        }
        else if (memcmp(scope, "onvif://www.onvif.org/Profile/Q/FactoryDefault", strlen(scope)) == 0)
        {
            return TRUE;
        }
    }
#endif

    return FALSE;
}

BOOL onvif_scopes_match(const char * matchby, char * scopes)
{
    int i = 0;
    const char * p_buf = scopes;
    char scope[256] = {'\0'};

    // remove space
    while (*p_buf != '\0') 
    {
        if (*p_buf == ' ') p_buf++;
        else break;
    }
    
    while (*p_buf != '\0') 
    {
        if (*p_buf == ' ')
        {
            if (i > 0)
            {
                scope[i] = '\0';
                
                if (!onvif_scope_match(matchby, scope))
                {
                    return FALSE;
                }
            }
            
            i = 0;            
        }
        else if (i < 255)
        {
            scope[i++] = *p_buf;
        }

        p_buf++;
    }

    if (i > 0)
    {
        scope[i] = '\0';
        
        if (!onvif_scope_match(matchby, scope))
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

void onvif_probe_err_rly(SOCKET vfd, uint32 rip, uint16 rport, char * p_msg_id, const char * code, const char * subcode, const char * reason)
{
    int rlen;
    int offset = 0;
    char buff[2048] = {'\0'};
    int buff_len = sizeof(buff);
    struct sockaddr_in addr;

    offset = snprintf(buff, buff_len, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "
		"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
		"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
		"xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" "
		"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
		"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">");
	
	offset += snprintf(buff+offset, buff_len-offset, "<s:Header>"
		"<wsa:MessageID>uuid:%s</wsa:MessageID>"
		"<wsa:RelatesTo>%s</wsa:RelatesTo>"
		"<wsa:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To>"
		"<wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action>"
		"</s:Header>", onvif_uuid_create(), p_msg_id);

    offset += snprintf(buff+offset, buff_len-offset, "<s:Body>");

	offset += snprintf(buff+offset, buff_len-offset, 
	    "<s:Fault>"
	        "<s:Code>"
	            "<s:Value>%s</s:Value>"
	            "<s:Subcode>"
	                "<s:Value>%s</s:Value>"
	            "</s:Subcode>"
	        "</s:Code>"
	        "<s:Reason>"
	            "<s:Text xml:lang=\"en\">%s</s:Text>"
	        "</s:Reason>"
	    "</s:Fault>",
        code, subcode, reason);
        
	offset += snprintf(buff+offset, buff_len-offset, "</s:Body></s:Envelope>");
	

    // send to received addr
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = rip;
	addr.sin_port = rport;
	
	rlen = sendto(vfd, buff, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
}

int onvif_probe_rly(char * p_msg_id, SOCKET vfd, uint32 rip, uint16 rport, const char * matchby, char * scopes)
{
    int rlen;
    int offset;
    int buff_len;
    char send_buffer[1024 * 10];    
    struct sockaddr_in addr;
   
	if (DiscoveryMode_NonDiscoverable == g_onvif_cfg.network.DiscoveryMode)
	{
		return -1;
	}

    if (strlen(matchby) > 0 && 
        strcmp(matchby, ScopeMatchByExact) && 
        strcmp(matchby, ScopeMatchByPrefix) && 
        strcmp(matchby, ScopeMatchByLdap) && 
        strcmp(matchby, ScopeMatchByUuid) && 
        strcmp(matchby, ScopeMatchByNone))
    {
        onvif_probe_err_rly(vfd, rip, rport, p_msg_id, "s:Sender", "d:MatchingRuleNotSupported", "MatchingRuleNotSupported");
        return -1;
    }

	if (!onvif_scopes_match(matchby, scopes))
	{
	    return -1;
	}
	
	buff_len = sizeof(send_buffer);
	
	offset = snprintf(send_buffer, buff_len, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "
		"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
		"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
		"xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" "
		"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
		"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">");
	
	offset += snprintf(send_buffer+offset, buff_len-offset, "<s:Header>"
		"<wsa:MessageID>uuid:%s</wsa:MessageID>"
		"<wsa:RelatesTo>%s</wsa:RelatesTo>"
		"<wsa:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To>"
		"<wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action>"
		"</s:Header>", onvif_uuid_create(), p_msg_id);

	offset += snprintf(send_buffer+offset, buff_len-offset, "<s:Body>"
		"<d:ProbeMatches><d:ProbeMatch>"
		"<wsa:EndpointReference>"
		"<wsa:Address>urn:uuid:%s</wsa:Address>"
		"</wsa:EndpointReference>"
		"<d:Types>dn:NetworkVideoTransmitter tds:Device</d:Types>",
        g_onvif_cfg.EndpointReference);

	offset += snprintf(send_buffer+offset, buff_len-offset, "<d:Scopes>");
    offset += onvif_bdscopes_text(send_buffer+offset, buff_len-offset);	
	offset += snprintf(send_buffer+offset, buff_len-offset, "</d:Scopes>");
	
	offset += snprintf(send_buffer+offset, buff_len-offset, "<d:XAddrs>%s</d:XAddrs>"
		"<d:MetadataVersion>1</d:MetadataVersion>"
		"</d:ProbeMatch></d:ProbeMatches></s:Body></s:Envelope>", 
		g_onvif_cfg.Capabilities.device.XAddr);

    
	// send to received addr
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = rip;
	addr.sin_port = rport;
	
	rlen = sendto(vfd, send_buffer, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

	// send to multicast addr
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ONVIF_GRP_ADDR);
	addr.sin_port = htons(ONVIF_GRP_PORT);
	
    rlen = sendto(vfd, send_buffer, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

	return rlen;
}

BOOL onvif_type_match(const char * types)
{
    int i = 0;
    const char * p_buf = types;
    char type[256] = {'\0'};

    // remove space
    while (*p_buf != '\0') 
    {
        if (*p_buf == ' ') p_buf++;
        else break;
    }
    
    while (*p_buf != '\0') 
    {
        if (*p_buf == ' ')
        {
            if (i > 0)
            {
                type[i] = '\0';

                if (soap_strcmp(type, "NetworkVideoTransmitter") == 0 || 
                    soap_strcmp(type, "Device") == 0)
				{
					return TRUE;
				}
            }
            
            i = 0;            
        }
        else if (i < 255)
        {
            type[i++] = *p_buf;
        }

        p_buf++;
    }

    if (i > 0)
    {
        type[i] = '\0';
        
        if (soap_strcmp(type, "NetworkVideoTransmitter") == 0 || 
            soap_strcmp(type, "Device") == 0)
		{
			return TRUE;
		}
    }
    
    return TRUE;
}

void onvif_probe_parse(SOCKET fd, char *rbuf, int rlen, uint32 sip, uint16 sport)
{
    char message_id[128] = {'\0'};
    XMLN * p_node;    
    
    p_node = _hxml_parse(rbuf, rlen);
	if (p_node == NULL)
	{
	}	
	else
	{
	    XMLN * p_Header;
        XMLN * p_Body;
        
		p_Header = _node_soap_get(p_node, "Header"); 
		if (p_Header)
		{
			XMLN * p_MessageID = _node_soap_get(p_Header, "MessageID"); 
			if (p_MessageID && p_MessageID->data)
			{
				strncpy(message_id, p_MessageID->data, sizeof(message_id)-1);
			}
		}

		p_Body = _node_soap_get(p_node, "Body");
		if (p_Body)
		{
			XMLN * p_Probe = _node_soap_get(p_Body, "Probe");
			if (p_Probe)
			{
			    XMLN * p_Scopes;
			    XMLN * p_Types;
			    char matchby[32] = {'\0'};
			    char scopes[1024] = {'\0'};

			    p_Scopes = _node_soap_get(p_Probe, "Scopes");
			    if (p_Scopes)
			    {
			        const char * p_MatchBy;

			        p_MatchBy = _attr_get(p_Scopes, "MatchBy");
			        if (p_MatchBy)
			        {
			            strncpy(matchby, p_MatchBy, sizeof(matchby)-1);
			        }

			        if (p_Scopes->data)
			        {
			            strncpy(scopes, p_Scopes->data, sizeof(scopes)-1);
			        }
			    }
			    
				p_Types = _node_soap_get(p_Probe, "Types");
				if (p_Types && p_Types->data)
				{
				    if (onvif_type_match(p_Types->data))
				    {
					
					    onvif_probe_rly(message_id, fd, sip, sport, matchby, scopes);
				    }
				}
				else
				{
					onvif_probe_rly(message_id, fd, sip, sport, matchby, scopes);
				}
			}
		}		
	}

	_node_del(p_node);
}

int onvif_probe_net_rx(SOCKET fd)
{
    int sret;
    int rlen;
    int addr_len;
	char rbuf[1024*10] = {'\0'};
	char message_id[128] = {'\0'};
	uint32 src_ip;
	uint16 src_port;
	fd_set fdr;
	struct timeval tv;
	struct sockaddr_in addr;
	
	memset(message_id,0,sizeof(message_id));
	
	FD_ZERO(&fdr);
	FD_SET(fd, &fdr);
	
	tv.tv_sec = 0;
	tv.tv_usec = 100 * 1000;
	
	sret = select((int)(fd+1), &fdr,NULL,NULL,&tv);
	if (sret == 0)
	{
		return 0;
	}
	else if (sret < 0)
	{
		return -1;
	}		
	
	addr_len = sizeof(struct sockaddr_in);
	rlen = recvfrom(fd, rbuf, sizeof(rbuf), 0, (struct sockaddr *)&addr, (socklen_t*)&addr_len);
	if (rlen < 0)
	{
		return -1;
	}
	
	src_ip = addr.sin_addr.s_addr;
	src_port = addr.sin_port;


    onvif_probe_parse(fd, rbuf, rlen, src_ip, src_port);

	return 0;
}

void onvif_hello()
{
    int rlen;
    int offset = 0;
    int mlen;
    char p_buf[1024*10];
    struct sockaddr_in addr;

    if (DiscoveryMode_NonDiscoverable == g_onvif_cfg.network.DiscoveryMode)
	{
		return;
	}
	
    mlen = sizeof(p_buf);
    
    offset += snprintf(p_buf+offset, mlen-offset,  
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "
	    "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
		"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
	    "xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
		"xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" "	
		"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
		"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">");

	offset += snprintf(p_buf+offset, mlen-offset, "<s:Header>"
	    "<wsa:MessageID>uuid:%s</wsa:MessageID>"
	    "<wsa:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
	    "<wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello</wsa:Action>"
		"</s:Header>", onvif_uuid_create());

	offset += snprintf(p_buf+offset, mlen-offset, "<s:Body><d:Hello>"
        "<wsa:EndpointReference>"
            "<wsa:Address>urn:uuid:%s</wsa:Address>"
        "</wsa:EndpointReference>"
        "<d:Types>dn:NetworkVideoTransmitter tds:Device</d:Types>",
        g_onvif_cfg.EndpointReference);

   	offset += snprintf(p_buf+offset, mlen-offset, "<d:Scopes>"); 
    offset += onvif_bdscopes_text(p_buf+offset, mlen-offset);   	
   	offset += snprintf(p_buf+offset, mlen-offset, "</d:Scopes>");

   	offset += snprintf(p_buf+offset, mlen-offset, "<d:XAddrs>%s</d:XAddrs>"
        "<d:MetadataVersion>1</d:MetadataVersion></d:Hello></s:Body></s:Envelope>",
        g_onvif_cfg.Capabilities.device.XAddr);

	
	// send to multicast addr
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ONVIF_GRP_ADDR);
	addr.sin_port = htons(ONVIF_GRP_PORT);
	
    rlen = sendto(g_onvif_cls.discovery_fd, p_buf, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
}

void onvif_bye()
{       
    int rlen;
    int offset = 0;
    int mlen;
    char p_buf[1024*10];
    struct sockaddr_in addr;

    if (DiscoveryMode_NonDiscoverable == g_onvif_cfg.network.DiscoveryMode)
	{
		return;
	}
	
    mlen = sizeof(p_buf);
    
    offset += snprintf(p_buf+offset, mlen-offset,  
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "
	    "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
		"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
	    "xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
		"xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" "	
		"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
		"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">");

	offset += snprintf(p_buf+offset, mlen-offset, "<s:Header>"
	    "<wsa:MessageID>uuid:%s</wsa:MessageID>"
	    "<wsa:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
	    "<wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Bye</wsa:Action>"
		"</s:Header>", onvif_uuid_create());

	offset += snprintf(p_buf+offset, mlen-offset, "<s:Body><d:Bye>"
        "<wsa:EndpointReference>"
            "<wsa:Address>urn:uuid:%s</wsa:Address>"
        "</wsa:EndpointReference>"
        "</d:Bye></s:Body></s:Envelope>",
        g_onvif_cfg.EndpointReference);

	
	// send to multicast addr
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ONVIF_GRP_ADDR);
	addr.sin_port = htons(ONVIF_GRP_PORT);

    rlen = sendto(g_onvif_cls.discovery_fd, p_buf, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	
}

void * onvif_discovery_thread(void * argv)
{
    onvif_hello();
	
	while (g_onvif_cls.discovery_flag)
	{
		onvif_probe_net_rx(g_onvif_cls.discovery_fd);

		usleep(20*1000);
	}

	g_onvif_cls.discovery_tid = 0;

	return NULL;
}

void onvif_start_discovery()
{
	g_onvif_cls.discovery_fd = onvif_probe_init();
	if (g_onvif_cls.discovery_fd <= 0)
	{
		printf("onvif_probe_init fd failed\r\n");
		return;
	}

	g_onvif_cls.discovery_flag = 1;
	g_onvif_cls.discovery_tid = sys_os_create_thread((void *)onvif_discovery_thread, NULL);
}

void onvif_stop_discovery()
{
	g_onvif_cls.discovery_flag = 0;
	while (g_onvif_cls.discovery_tid != 0)
	{
		usleep(10*1000);
	}

    if (g_onvif_cls.discovery_fd > 0)
    {
	    closesocket(g_onvif_cls.discovery_fd);
	    g_onvif_cls.discovery_fd = 0;
	}
}



