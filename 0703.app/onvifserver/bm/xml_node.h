

#ifndef	XML_NODE_H
#define	XML_NODE_H

/***************************************************************************************
 * 
 * XML node define
 *
***************************************************************************************/
#define NTYPE_TAG		0
#define NTYPE_ATTRIB	1
#define NTYPE_CDATA		2

#define NTYPE_LAST		2
#define NTYPE_UNDEF		-1

typedef struct XMLN
{
	const char *	name;
	uint32	        type;
	const char *	data;
	int				dlen;
	int				finish;
	struct XMLN *	parent;
	struct XMLN *	f_child;
	struct XMLN *	l_child;
	struct XMLN *	prev;
	struct XMLN *	next;
	struct XMLN *	f_attrib;
	struct XMLN *	l_attrib;
} XMLN;


#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************/
HT_API XMLN *       _node_add(XMLN * parent, char * name);
HT_API void         _node_del(XMLN * p_node);
HT_API XMLN *       _node_get(XMLN * parent, const char * name);

HT_API int          soap_strcmp(const char * str1, const char * str2);
HT_API void         soap_strncpy(char * dest, const char * src, int size);
HT_API XMLN *       _node_soap_get(XMLN * parent, const char * name);

/***************************************************************************************/
HT_API XMLN *       _attr_add(XMLN * p_node, const char * name, const char * value);
HT_API void         _attr_del(XMLN * p_node, const char * name);
HT_API const char * _attr_get(XMLN * p_node, const char * name);
HT_API XMLN *       _attr_node_get(XMLN * p_node, const char * name);

/***************************************************************************************/
HT_API void         _cdata_set(XMLN * p_node, const char * value, int len);

/***************************************************************************************/
HT_API int          _calc_buf_len(XMLN * p_node);
HT_API int          _write_buf(XMLN * p_node, char * xml_buf);
HT_API int          _nwrite_buf(XMLN * p_node, char * xml_buf, int buf_len);

/***************************************************************************************/
HT_API XMLN *       _hxml_parse(char * p_xml, int len);

#ifdef __cplusplus
}
#endif

#endif	// XML_NODE_H



