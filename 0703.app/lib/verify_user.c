/*	
 * verify_user.c
 *
 ****************************************************************************/
#include "common.h"


// Confirm if user name is valid.
// Return
//	0, if okay.
//	1, No alphabet is included.
//	2, No number is included.
//	3, User name is too short.
//	4, if user name is zero length.
//	5, if user name has special character
//	6, if user name has space character

int verify_usrname (const char *usrname)
{
	char	__usrname [USRNAME_MAX+1];
	int		len = 0;
	int		i, j;
	int		nr_alpha = 0;
	int		nr_num = 0;

	char spc_char[25] = {33, 64, 35, 36, 37, 94, 38, 40, 41, 95, 43,	123, 125, 58, 34, 63, 45, 61, 91, 93, 59, 39, 44, 46, 47};
	char space_char = 32;	// space
		
	assert (usrname);

	snprintf (__usrname, sizeof (__usrname), usrname);
	len = strlen (__usrname);

	if (len <= 0)	// User name is zero length.
		return USRNAME_CHK_ZERO_LENGTH;

	if (len < 5)	// Too short user name.
		return USRNAME_CHK_TOO_SHORT;

	for (i = 0; i < len; i ++) {
		// upper eng, lower eng check
		if((usrname[i] >=65 && usrname[i]<=90)|| (usrname[i]>=91 && usrname[i]<=122))
			nr_alpha++;

		// num check
		if(usrname[i]>=48 && usrname[i] <=57)
			nr_num++;

		// special character check
		for (j=0; j<25; j++) {
			if (usrname[i] == spc_char[j])
				return USRNAME_CHK_SPECIAL_INCLUDED;
		}

		// space character check
		if (usrname[i] == space_char)
			return USRNAME_CHK_SPACE_INCLUDED;
	}

	if (nr_alpha < 1)
		return USRNAME_CHK_NO_ALPHABET;

	if (nr_num < 1)
		return USRNAME_CHK_NO_NUMBER;

	return USRNAME_CHK_OK;
}

int verify_usrpwd (const char * pwd)
{
	int i, j, pwd_len;
	int num_cnt, eng_cnt, spc_cnt, space_cnt = 0;
	int rpt_cnt, seq_cnt;
	char tmp_pwd[USRPWD_MAX+1];
	char fchar = 0;
	int chk_limit;
	
	const char spc_char[25] = 
	{
#if 0
		33, 64, 35, 36, 37, 94, 38, 42, 40, 41, 95, 43,			// "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+"
		123, 125,	58, 34, 60, 62, 63, 							// "{", "}", ":", """, "<", ">", "?"
		45, 61, 91, 93, 59, 39, 44, 46, 47						// "-", , "=", "[", "]", ";", "'"",", ".", "/"
#else
		33, 64, 35, 36, 37, 94, 38, 40, 41, 95, 43,				// "!", "@", "#", "$", "%", "^", "&", "(", ")", "_", "+"
		123, 125,	58, 34, 63, 								// "{", "}", ":", """, "?"
		45, 61, 91, 93, 59, 39, 44, 46, 47						// "-", , "=", "[", "]", ";", "'"",", ".", "/"
#endif
	};

	const char space_char[1] = 
	{
		32					// space
	};

	num_cnt = 0;
	eng_cnt = 0;
	spc_cnt = 0;
	rpt_cnt = 0;
	seq_cnt = 0;
	
	snprintf (tmp_pwd, sizeof (tmp_pwd), pwd);
	pwd_len = strlen(tmp_pwd);

	if (pwd_len <= 0)
		return USRPWD_CHK_ZERO_LENGTH;

	if (pwd_len < 8)
		return USRPWD_CHK_TOO_SHORT;

#if 1
	chk_limit = 4;
#else
	if (pwd_len <= 10)
		chk_limit = 3;
	else
		chk_limit = 4;
#endif	
	
	// repeat check
	fchar = 0;
	for (i=0; i<pwd_len; i++) {
		debug("repeat check %d rpt=%d (0x%02x = 0x%02x)\n", i, rpt_cnt, fchar, tmp_pwd[i]);
		if (rpt_cnt >= chk_limit)
			return USRPWD_CHK_REPEAT_CHAR;

		if (fchar == tmp_pwd[i])
			rpt_cnt++;
		else
			rpt_cnt = 1;
		fchar = tmp_pwd[i];
	}

	// num sequencial	
	fchar = 0;
	seq_cnt = 0;
	for (i=0; i<pwd_len; i++) {
		debug("num seq1 check %d seq=%d (0x%02x = 0x%02x)\n", i, seq_cnt, fchar, tmp_pwd[i]);
		if (seq_cnt >= chk_limit)
			return USRPWD_CHK_SEQUENCIAL_NUM;

		if(tmp_pwd[i]>=48 && tmp_pwd[i] <=57) {
			if (fchar +1 == tmp_pwd[i])
				seq_cnt++;
			else
				seq_cnt = 1;
		} else {
			seq_cnt = 0;
		}
		fchar = tmp_pwd[i];
	}

	// num reverse sequencial 	
	fchar = 0;
	seq_cnt = 0;
	for (i=0; i<pwd_len; i++) {
		debug("num r seq2 check %d seq=%d (0x%02x = 0x%02x)\n", i, seq_cnt, fchar, tmp_pwd[i]);
		if (seq_cnt >= chk_limit)
			return USRPWD_CHK_SEQUENCIAL_NUM;

		if(tmp_pwd[i]>=48 && tmp_pwd[i] <=57) {
			if (fchar -1 == tmp_pwd[i])
				seq_cnt++;
			else
				seq_cnt = 1;
		} else {
			seq_cnt = 0;
		}
		fchar = tmp_pwd[i];
	}
	
	// char sequencial 1	
	fchar = 0;
	seq_cnt = 0;
	for (i=0; i<pwd_len; i++) {
		debug("char seq3 check %d seq=%d (0x%02x = 0x%02x)\n", i, seq_cnt, fchar, tmp_pwd[i]);
		if (seq_cnt >= chk_limit)
			return USRPWD_CHK_SEQUENCIAL_CHAR;

		if (tmp_pwd[i] >=65 && tmp_pwd[i]<=90) {
			if (fchar +1 == tmp_pwd[i])
				seq_cnt++;
			else
				seq_cnt = 1;
		} else {
			seq_cnt = 0;
		}
		fchar = tmp_pwd[i];
	}

	// char sequencial 2	
	fchar = 0;
	seq_cnt = 0;
	for (i=0; i<pwd_len; i++) {
		debug("char seq4 check %d seq=%d (0x%02x = 0x%02x)\n", i, seq_cnt, fchar, tmp_pwd[i]);
		if (seq_cnt >= chk_limit)
			return USRPWD_CHK_SEQUENCIAL_CHAR;

		if (tmp_pwd[i]>=97 && tmp_pwd[i]<=122) {
			if (fchar +1 == tmp_pwd[i])
				seq_cnt++;
			else
				seq_cnt = 1;
		} else {
			seq_cnt = 0;
		}
		fchar = tmp_pwd[i];
	}

	// char reverse sequencial 1
	fchar = 0;
	seq_cnt = 0;
	for (i=0; i<pwd_len; i++) {
		debug("char r seq5 check %d seq=%d (0x%02x = 0x%02x)\n", i, seq_cnt, fchar, tmp_pwd[i]);
		if (seq_cnt >= chk_limit)
			return USRPWD_CHK_SEQUENCIAL_CHAR;

		if (tmp_pwd[i] >=65 && tmp_pwd[i]<=90) {
			if (fchar -1 == tmp_pwd[i])
				seq_cnt++;
			else
				seq_cnt = 1;
		} else {
			seq_cnt = 0;
		}
		fchar = tmp_pwd[i];
	}

	// char reverse sequencial 2
	fchar = 0;
	seq_cnt = 0;
	for (i=0; i<pwd_len; i++) {
		debug("char r seq6 check %d seq=%d (0x%02x = 0x%02x)\n", i, seq_cnt, fchar, tmp_pwd[i]);
		if (seq_cnt >= chk_limit)
			return USRPWD_CHK_SEQUENCIAL_CHAR;

		if (tmp_pwd[i]>=97 && tmp_pwd[i]<=122) {
			if (fchar -1 == tmp_pwd[i])
				seq_cnt++;
			else
				seq_cnt = 1;
		} else {
			seq_cnt = 0;
		}
		fchar = tmp_pwd[i];
	}

	for ( i = 0; i < pwd_len; i++ ) {
		// upper eng, lower eng check
		if((tmp_pwd[i] >=65 && tmp_pwd[i]<=90)|| (tmp_pwd[i]>=91 && tmp_pwd[i]<=122))
			eng_cnt++;

		// num check
		if(tmp_pwd[i]>=48 && tmp_pwd[i] <=57)
			num_cnt++;

		// spcial charcter check
		for ( j=0; j< sizeof (spc_char); j++ ) {
			if ( tmp_pwd[i] ==  spc_char[j] )
				spc_cnt++;
		}

		// spcial charcter check
		for ( j=0; j<1; j++ ) {
			if ( tmp_pwd[i] ==  space_char[j] )
				space_cnt++;
		}
	}

	if (pwd_len <= 10) {
		if ( eng_cnt < 1 )
			return USRPWD_CHK_NO_ALPHABET;

		if ( num_cnt < 1 )
			return USRPWD_CHK_NO_NUMBER;

		if ( spc_cnt < 1 )
			return USRPWD_CHK_NO_SPECIAL;
	} else {		// when pwd_len is 10 over
		int	okay = 0;
		if (eng_cnt > 0)
			okay ++;
		if (num_cnt > 0)
			okay ++;
		if (spc_cnt > 0)
			okay ++;

		if (okay < 2)
			return USRPWD_CHK_NEED_2_COMBINATION;
	}

	if ( space_cnt > 0 )
		return USRPWD_CHK_SPACE_INCLUDED;

	return USRPWD_CHK_OK;
}

