
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

#include "./include/iconv/iconv.h"

static char page_utf8[MAX_PAGE_SZ];


static char* source_encode[MAX_LANGUAGE] = { "EUC-KR", "EUC-KR", "EUC-JP"};
static char* dest_encode[MAX_LANGUAGE] = { "", "EUC-KR", "EUC-JP"};


const char* ENC_UTF_8 = "UTF-8";

void ajax_conv(char* out_buf, char* page, int lang)
{
    if (lang != LANG_ENG)
	{
		int bConv = 0;

		int outBufSize = sizeof(page_utf8);
		memset(page_utf8, 0, outBufSize);
		iconv_t cd = iconv_open(ENC_UTF_8, source_encode[lang]);
		if (cd != (iconv_t)(-1))
		{
			char* in = page;
			char* out = page_utf8;
			size_t readBytes = strlen(page);
			size_t writeBytes = sizeof(page_utf8);

			if (iconv(cd, &in, &readBytes, &out, &writeBytes) != -1)
				bConv = 1;

			iconv_close(cd);
		}

		strcpy(out_buf, (bConv) ? page_utf8 : "");
	}
	else
		strcpy(out_buf, page);
}

void ajax_printout(char* page, int lang)
{
    if (lang != LANG_ENG)
	{
		int bConv = 0;

		int outBufSize = sizeof(page_utf8);
		memset(page_utf8, 0, outBufSize);
		iconv_t cd = iconv_open(ENC_UTF_8, source_encode[lang]);
		if (cd != (iconv_t)(-1))
		{
			char* in = page;
			char* out = page_utf8;
			size_t readBytes = strlen(page);
			size_t writeBytes = sizeof(page_utf8);

			if (iconv(cd, &in, &readBytes, &out, &writeBytes) != -1)
				bConv = 1;

			iconv_close(cd);
		}

		printf("%s", (bConv) ? page_utf8 : page);
	}
	else
		printf("%s", page);
}

int ajax_check_output(char* page, int lang)
{
	size_t readBytes = strlen(page);
	size_t writeBytes = sizeof(page_utf8);
	int bConv = 0;
	char* in = page;
	char* out = page_utf8;

    if (lang != LANG_ENG)
	{
		memset(page_utf8, 0, MAX_PAGE_SZ);
		iconv_t cd = iconv_open(ENC_UTF_8, source_encode[lang]);
		if (cd != (iconv_t)(-1))
		{
			if (iconv(cd, &in, &readBytes, &out, &writeBytes) != -1) {
				bConv = 1;
			}

			iconv_close(cd);
		}

		if (bConv) {
			printf("%s", page_utf8);
		}
		else {
			return -1;
		}
	}
	else
		printf("%s", page);

    return 1;
}

/*
int ajax_check_output_2(char* page, int lang)
{
#ifdef _USE_UTF8_
	return ajax_check_output(page, lang);
#else
	printf("%s", page);
	return 1;
#endif
}
*/


char* ajax_conv_utf8(char* page, int lang)
{
	int bConv = 0;
	int save_lang = lang;

	char* in = page;
	char* out = page_utf8;
	size_t readBytes = strlen(page);
	size_t writeBytes = sizeof(page_utf8);

	if (lang != LANG_ENG)
	{
		memset(page_utf8, 0, MAX_PAGE_SZ);
		iconv_t cd = iconv_open(dest_encode[lang], ENC_UTF_8);
		if (cd != (iconv_t)(-1))
		{
			if (iconv(cd, &in, &readBytes, &out, &writeBytes) != -1)
				bConv = 1;

			iconv_close(cd);
		}

		if (bConv) {
			return page_utf8;
		}
		else {
		}
	}

	for (lang = LANG_KOR; lang < MAX_LANGUAGE; lang++)
	{
		if (save_lang != lang) {
			memset(page_utf8, 0, MAX_PAGE_SZ);
			iconv_t cd = iconv_open(dest_encode[lang], ENC_UTF_8);
			if (cd != (iconv_t)(-1))
			{
				if (iconv(cd, &in, &readBytes, &out, &writeBytes) != -1)
					bConv = 1;

				iconv_close(cd);
			}

			if (bConv) {
				return page_utf8;
			}
			else {
			}
		}
	}

	return page;
}
