/*
 * verify_user.h
 *
 *******************************************************************************/
#ifndef __VERIFY_USER_H
#define __VERIFY_USER_H

enum
{
	USRNAME_CHK_OK,					//	0, if okay.

	USRNAME_CHK_NO_ALPHABET,		//	1, No alphabet is included.
	USRNAME_CHK_NO_NUMBER,			//	2, No number is included.
	USRNAME_CHK_TOO_SHORT,			//	3, User name is too short.
	USRNAME_CHK_ZERO_LENGTH,		//	4, if user name is zero length.
	USRNAME_CHK_SPECIAL_INCLUDED,	//	5, if user name has special character
	USRNAME_CHK_SPACE_INCLUDED,		//	6, if user name has space character

	USRNAME_CHK_MAX
};

enum
{
	USRPWD_CHK_OK,					//	0, if okay.

	USRPWD_CHK_NO_ALPHABET,			//	1, No alphabet is included.
	USRPWD_CHK_NO_NUMBER,			//	2, No number is included.
	USRPWD_CHK_NO_SPECIAL,			//	3, No special character is included.
	USRPWD_CHK_SPACE_INCLUDED,		//	4, Space is included.
	USRPWD_CHK_NEED_2_COMBINATION,	//	5, Need at least two combination among alpha, num, special  when len is over 10.
	USRPWD_CHK_TOO_SHORT,			//	6, Password is too short.
	USRPWD_CHK_ZERO_LENGTH,			//	7, Password is zero length.
	USRPWD_CHK_REPEAT_CHAR,			//	8, Repeat same character
	USRPWD_CHK_SEQUENCIAL_NUM,		//	9, Sequencial number
	USRPWD_CHK_SEQUENCIAL_CHAR,		// 10, Sequencial character

	USRPWD_CHK_MAX,
};

int verify_usrname (const char *usrname);
int verify_usrpwd (const char * pwd);

#if 0
void verify_idpwd_and_geterr(const char* usrid, const char* usrpwd, char* errmsg, int errmsgsz);
#endif

#endif

