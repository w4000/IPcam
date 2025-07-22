

#ifndef	H_XML_H
#define	H_XML_H

#define XML_MAX_STACK_DEPTH	    1024
#define XML_MAX_ATTR_NUM		200

typedef struct xmlparser 
{
	char *	xmlstart;
	char *	xmlend;
	char *	ptr;		// pointer to current character
	int		xmlsize;
	char *	e_stack[XML_MAX_STACK_DEPTH];
	int		e_stack_index;
	char *	attr[XML_MAX_ATTR_NUM];
	void *	userdata;
	
	void (*startElement)(void * userdata, const char * name, const char ** attr);
	void (*endElement)(void * userdata, const char * name);
	void (*charData)(void * userdata, const char * str, int len);
} XMLPRS;

#ifdef __cplusplus
extern "C" {
#endif

HT_API int  hxml_parse_header(XMLPRS * parse);
HT_API int  hxml_parse_attr(XMLPRS * parse);
HT_API int  hxml_parse_element_end(XMLPRS * parse);
HT_API int  hxml_parse_element_start(XMLPRS * parse);
HT_API int  hxml_parse_element(XMLPRS * parse);
HT_API int  hxml_parse(XMLPRS * parse);

#ifdef __cplusplus
}
#endif

#endif	// H_XML_H



