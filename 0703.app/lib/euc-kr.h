/*
#
#    Name:     cp949 to Unicode table
#    Unicode version: 2.0
#    Table version: 2.01
#    Table format:  Format A
#    Date:          1/7/2000
#
#    Contact:       Shawn.Steele@microsoft.com
#
#    General notes: none
#
#    Format: Three tab-separated columns
#        Column #1 is the cp949 code (in hex)
#        Column #2 is the Unicode (in hex as 0xXXXX)
#        Column #3 is the Unicode name (follows a comment sign, '#')
#
#    The entries are in cp949 order
#
*/
#ifndef __EUCKR_H__
#define __EUCKR_H__
#define MAX_EUCKR_CHAR_NUM	17048

extern const unsigned short euckrTbl[MAX_EUCKR_CHAR_NUM][2];

#endif /*__EUCKR_H__*/
