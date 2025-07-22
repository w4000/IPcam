

#include "sys_inc.h"
#include "hxml.h"
#include "xml_node.h"

/***************************************************************************************
 *
 * XML operational functions
 *
***************************************************************************************/
HT_API XMLN * _node_add(XMLN * parent, char * name)
{
	XMLN * p_node = (XMLN *)malloc(sizeof(XMLN));
	if (p_node == NULL)
	{
		return NULL;
	}

	memset(p_node, 0, sizeof(XMLN));

	p_node->type = NTYPE_TAG;
	p_node->name = name;

	if (parent != NULL)
	{
		p_node->parent = parent;
		
		if (parent->f_child == NULL)
		{
			parent->f_child = p_node;
			parent->l_child = p_node;
		}
		else
		{
			parent->l_child->next = p_node;
			p_node->prev = parent->l_child;
			parent->l_child = p_node;
		}
	}

	return p_node;
}

HT_API void _node_del(XMLN * p_node)
{
    XMLN * p_attr;
    XMLN * p_child;
    
	if (p_node == NULL)
	{
		return;
	}
	
	p_attr = p_node->f_attrib;
	while (p_attr)
	{
		XMLN * p_next = p_attr->next;

		free(p_attr);

		p_attr = p_next;
	}

	p_child = p_node->f_child;
	while (p_child)
	{
		XMLN * p_next = p_child->next;
		
		_node_del(p_child);
		
		p_child = p_next;
	}

	if (p_node->prev)
	{
		p_node->prev->next = p_node->next;
	}
	
	if (p_node->next)
	{
		p_node->next->prev = p_node->prev;
	}
	
	if (p_node->parent)
	{
		if (p_node->parent->f_child == p_node)
		{
			p_node->parent->f_child = p_node->next;
		}
		
		if (p_node->parent->l_child == p_node)
		{
			p_node->parent->l_child = p_node->prev;
		}	
	}

	free(p_node);
}

HT_API XMLN * _node_get(XMLN * parent, const char * name)
{
    XMLN * p_node;
    
	if (parent == NULL || name == NULL)
	{
		return NULL;
	}
	
	p_node = parent->f_child;
	while (p_node != NULL)
	{
		if (strcasecmp(p_node->name, name) == 0)
		{
			return p_node;
		}
		
		p_node = p_node->next;
	}

	return NULL;
}

HT_API int soap_strcmp(const char * str1, const char * str2)
{
    const char * ptr1;
    const char * ptr2;
    
	if (strcasecmp(str1, str2) == 0)
	{
		return 0;
	}
	
	ptr1 = strchr(str1, ':');
	ptr2 = strchr(str2, ':');
	
	if (ptr1 && ptr2)
	{
		return strcasecmp(ptr1+1, ptr2+1);
	}	
	else if (ptr1)
	{
		return strcasecmp(ptr1+1, str2);
	}	
	else if (ptr2)
	{
		return strcasecmp(str1, ptr2+1);
	}	
	else
	{
		return strcasecmp(str1, str2);
	}	
}

HT_API void soap_strncpy(char * dest, const char * src, int size)
{
    const char * ptr;
    
	ptr = strchr(src, ':');
	
	if (ptr)
	{
		strncpy(dest, ptr+1, size);
	}	
	else
	{
		strncpy(dest, src, size);
	}	
}

HT_API XMLN * _node_soap_get(XMLN * parent, const char * name)
{
    XMLN * p_node;
    
	if (parent == NULL || name == NULL)
	{
		return NULL;
	}
	
	p_node = parent->f_child;
	while (p_node != NULL)
	{
		if (soap_strcmp(p_node->name, name) == 0)
		{
			return p_node;
		}
		
		p_node = p_node->next;
	}

	return NULL;
}

/***************************************************************************************/
HT_API XMLN * _attr_add(XMLN * p_node, const char * name, const char * value)
{
    XMLN * p_attr;
    
	if (p_node == NULL || name == NULL || value == NULL)
	{
		return NULL;
	}
	
	p_attr = (XMLN *)malloc(sizeof(XMLN));
	if (p_attr == NULL)
	{
		return NULL;
	}

	memset(p_attr, 0, sizeof(XMLN));

	p_attr->type = NTYPE_ATTRIB;
	p_attr->name = name;	
	p_attr->data = value;	
	p_attr->dlen = (int)strlen(value);

	if (p_node->f_attrib == NULL)
	{
		p_node->f_attrib = p_attr;
		p_node->l_attrib = p_attr;
	}
	else
	{
		p_attr->prev = p_node->l_attrib;
		p_node->l_attrib->next = p_attr;
		p_node->l_attrib = p_attr;
	}

	return p_attr;
}

HT_API void _attr_del(XMLN * p_node, const char * name)
{
    XMLN * p_attr;
    
	if (p_node == NULL || name == NULL)
	{
		return;
	}
	
	p_attr = p_node->f_attrib;
	while (p_attr != NULL)
	{
		if (strcasecmp(p_attr->name, name) == 0)
		{
			_node_del(p_attr);
			return;
        }

		p_attr = p_attr->next;
	}
}

HT_API const char * _attr_get(XMLN * p_node, const char * name)
{
    XMLN * p_attr;
    
	if (p_node == NULL || name == NULL)
	{
		return NULL;
	}
	
	p_attr = p_node->f_attrib;
	while (p_attr != NULL)
	{
		if ((NTYPE_ATTRIB == p_attr->type) && (0 == soap_strcmp(p_attr->name, name)))
		{
			return p_attr->data;
		}
		
		p_attr = p_attr->next;
	}

	return NULL;
}

HT_API XMLN * _attr_node_get(XMLN * p_node, const char * name)
{
    XMLN * p_attr;
    
	if (p_node == NULL || name == NULL)
	{
		return NULL;
	}
	
	p_attr = p_node->f_attrib;
	while (p_attr != NULL)
	{
		if ((NTYPE_ATTRIB == p_attr->type) && (0 == soap_strcmp(p_attr->name, name)))
		{
			return p_attr;
		}
		
		p_attr = p_attr->next;
	}

	return NULL;
}

/***************************************************************************************/
HT_API void _cdata_set(XMLN * p_node, const char * value, int len)
{
	if (p_node == NULL || value == NULL || len <= 0)
	{
		return;
	}
	
	p_node->data = value;
	p_node->dlen = len;
}

/***************************************************************************************/
HT_API int _calc_buf_len(XMLN * p_node)
{
	int xml_len = 0;
	XMLN * p_attr;
	
	xml_len += 1 + (int)strlen(p_node->name);

	p_attr = p_node->f_attrib;
	while (p_attr)
	{
		if (p_attr->type == NTYPE_ATTRIB)
		{
			xml_len += (int)strlen(p_attr->name) + 4 + (int)strlen(p_attr->data);
		}	
		else if (p_attr->type == NTYPE_CDATA)
		{
		    if (0x0a == (*p_attr->data))
			{
				p_attr = p_attr->next;
				continue;
			}
			
			xml_len += 1 + (int)strlen(p_attr->data) + 2 + (int)strlen(p_node->name) + 1;
			return xml_len;
		}

		p_attr = p_attr->next;
	}

	if (p_node->f_child)
	{
	    XMLN * p_child;
	    
		xml_len += 1;
		
		p_child = p_node->f_child;
		while (p_child)
		{
			xml_len += _calc_buf_len(p_child);
			
			p_child = p_child->next;
		}

		xml_len += 2 + (int)strlen(p_node->name) + 1;
	}
	else if (p_node->data)
	{
		xml_len += 4 + (int)strlen(p_node->data) + (int)strlen(p_node->name);
	}
	else
	{
		xml_len += 2;
	}

	return xml_len+1;
}

/***************************************************************************************/
HT_API int _write_buf(XMLN * p_node, char * xml_buf)
{
	int xml_len = 0;
    XMLN * p_attr;
    
	xml_len += sprintf(xml_buf+xml_len, "<%s", p_node->name);

	p_attr = p_node->f_attrib;
	while (p_attr)
	{
		if (p_attr->type == NTYPE_ATTRIB)
		{
			xml_len += sprintf(xml_buf+xml_len, " %s=\"%s\"", p_attr->name, p_attr->data);
		}	
		else if (p_attr->type == NTYPE_CDATA)
		{
		    if (0x0a == (*p_attr->data))
			{
				p_attr = p_attr->next;
				continue;
			}
			
			xml_len += sprintf(xml_buf+xml_len, ">%s</%s>", p_attr->data, p_node->name);
			return xml_len;
		}

		p_attr = p_attr->next;
	}

	if (p_node->f_child)
	{
	    XMLN * p_child;
	    
		xml_len += sprintf(xml_buf+xml_len, ">");
		
		p_child = p_node->f_child;
		while (p_child)
		{
			xml_len += _write_buf(p_child, xml_buf+xml_len);
			p_child = p_child->next;
		}

		xml_len += sprintf(xml_buf+xml_len, "</%s>", p_node->name);
	}
	else if (p_node->data)
	{
		xml_len += sprintf(xml_buf+xml_len, ">%s</%s>", p_node->data, p_node->name);
	}
	else
	{
		xml_len += sprintf(xml_buf+xml_len, "/>");
	}

	return xml_len;
}

/***************************************************************************************/
HT_API int _nwrite_buf(XMLN * p_node, char * xml_buf, int buf_len)
{
    int ret = 0;
	int xml_len = 0;
    XMLN * p_attr;
    
	if ((NULL == p_node) || (NULL == p_node->name))
	{
		return -1;
	}
	
	if (strlen(p_node->name) >= (size_t)buf_len)
	{
		return -1;
	}
	
	xml_len += snprintf(xml_buf+xml_len, buf_len-xml_len, "<%s", p_node->name);

	p_attr = p_node->f_attrib;
	while (p_attr)
	{
		if (p_attr->type == NTYPE_ATTRIB)
		{
			if ((strlen(p_attr->name) + strlen(p_attr->data) + xml_len) > (size_t)buf_len)
			{
				return -1;
			}
			
			xml_len += snprintf(xml_buf+xml_len, buf_len-xml_len, " %s=\"%s\"", p_attr->name, p_attr->data);
		}
		else if (p_attr->type == NTYPE_CDATA)
		{
			if (0x0a == (*p_attr->data))
			{
				p_attr = p_attr->next;
				continue;
			}
			
			if ((strlen(p_attr->data) + strlen(p_node->name) + xml_len) >= (size_t)buf_len)
			{
				return -1;
			}
			
			xml_len += snprintf(xml_buf+xml_len, buf_len-xml_len, ">%s</%s>", p_attr->data, p_node->name);
			
			return xml_len;
		}

		p_attr = p_attr->next;
	}

	if (p_node->f_child)
	{
	    XMLN * p_child;
	    
		xml_len += snprintf(xml_buf+xml_len, buf_len-xml_len, ">");
		
		p_child = p_node->f_child;
		while (p_child)
		{
			ret = _nwrite_buf(p_child, xml_buf+xml_len, buf_len-xml_len);
			if (ret < 0)
			{
				return ret;
			}
			
			xml_len += ret;
			p_child = p_child->next;
		}

		xml_len += snprintf(xml_buf+xml_len, buf_len-xml_len, "</%s>", p_node->name);
	}
    else if (p_node->data)
	{
		xml_len += snprintf(xml_buf+xml_len, buf_len-xml_len, ">%s</%s>", p_node->data, p_node->name);
	}
	else
	{
		xml_len += snprintf(xml_buf+xml_len, buf_len-xml_len, "/>");
	}

	return xml_len;
}

void stream_startElement(void * userdata, const char * name, const char ** atts)
{
    XMLN * parent;
    XMLN * p_node;
	XMLN ** pp_node = (XMLN **)userdata;

	if (pp_node == NULL)
	{
		return;
	}

	parent = *pp_node;
	p_node = _node_add(parent, (char *)name);

	if (atts)
	{
		int i=0;
		
		while (atts[i] != NULL)
		{
			if (atts[i+1] == NULL)
			{
				break;
			}
			
			_attr_add(p_node, atts[i], atts[i+1]);

			i += 2;
		}
	}

	*pp_node = p_node;
}

void stream_endElement(void * userdata, const char * name)
{
    XMLN * p_node;
	XMLN ** pp_node = (XMLN **)userdata;

	if (pp_node == NULL)
	{
		return;
	}

	p_node = *pp_node;
	if (p_node == NULL)
	{
		return;
	}

	p_node->finish = 1;

	if (p_node->type == NTYPE_TAG && p_node->parent == NULL)
	{
		// parse finish
	}
	else
	{
		*pp_node = p_node->parent;	// back up a level
	}
}

void stream_charData(void* userdata, const char* s, int len)
{
    XMLN * p_node;
	XMLN ** pp_node = (XMLN **)userdata;

	if (pp_node == NULL)
	{
		return;
	}
	
	p_node = *pp_node;
	if (p_node == NULL)
	{
		return;
	}
	
	p_node->data = s;
	p_node->dlen = len;
}

HT_API XMLN * _hxml_parse(char * p_xml, int len)
{
    int status;
	XMLN * p_root = NULL;
	XMLPRS parse;
	
	memset(&parse, 0, sizeof(parse));

	parse.userdata = &p_root;
	parse.startElement = stream_startElement;
	parse.endElement = stream_endElement;
	parse.charData = stream_charData;

	parse.xmlstart = p_xml;
	parse.xmlend = p_xml + len;
	parse.ptr = parse.xmlstart;

	status = hxml_parse(&parse);
	if (status < 0)
	{
		_node_del(p_root);
		
		p_root = NULL;
	}

	return p_root;
}


