

#ifndef	__H_WORD_ANALYSE_H__
#define	__H_WORD_ANALYSE_H__

/***************************************************************************************/
typedef enum word_type
{
	WORD_TYPE_NULL = 0,
	WORD_TYPE_STRING,
	WORD_TYPE_NUM,
	WORD_TYPE_SEPARATOR
} WORD_TYPE;


#ifdef __cplusplus
extern "C" {
#endif

HT_API BOOL is_char(char ch);
HT_API BOOL is_num(char ch);
HT_API BOOL is_separator(char ch);
HT_API BOOL is_ip_address(const char * address);
HT_API BOOL is_integer(char * p_str);

HT_API BOOL GetLineText(char * buf, int cur_line_offset, int max_len, int * len, int * next_line_offset);
HT_API BOOL GetSipLine(char * p_buf, int max_len, int * len, BOOL * bHaveNextLine);
HT_API BOOL GetLineWord(char * line, int cur_word_offset, int line_max_len, char * word_buf, int buf_len, int * next_word_offset, WORD_TYPE w_t);
HT_API BOOL GetNameValuePair(char * text_buf, int text_len, const char * name, char * value, int value_len);

#ifdef __cplusplus
}
#endif

#endif	//	__H_WORD_ANALYSE_H__



