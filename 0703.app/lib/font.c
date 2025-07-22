#include <stdio.h>

#include "font.h"
#include "euc-kr.h"
#include "ft2build.h"
#include FT_FREETYPE_H

#define FONT_FILE "/edvr/D2Coding.ttf"
//#define FONT_FILE "unifont-10.0.06.ttf"

static int is_init = 0;
static FT_Library library;
static FT_Face face;

static int binary_compare(const void *a, const void *b)
{
	return ( *(unsigned short*)a - *(unsigned short*)b );
}

int font_text2rgb1555(const char *text, const int size, const int pitch,
		const unsigned short color, unsigned short buf[], const int left)
{
	int i, j;
	int penx = 0;
	int peny = size*64;

	if(is_init == 0) {
		return -1;
	}

	if(FT_Set_Char_Size(face, size*64, 0, 96/*96*/, 0) != 0) {
		return -1;
	}
	for(i = 0; i < strlen(text); i++)	{
		unsigned short *binaryPtr;
		unsigned short uniCode;
		int bFind = 1;

		if((text[i] & 0x80) && ((i+1) < strlen(text))) { // find cp949 code (2 byte)
			uniCode = (unsigned short )((text[i] << 8) & 0xFF00) | (text[i+1] & 0xFF);

			/* binary searching */
			if((binaryPtr = (unsigned short *)bsearch(&uniCode, euckrTbl, MAX_EUCKR_CHAR_NUM, 4, binary_compare)) != NULL)
				j = ((char *)binaryPtr - (char *)euckrTbl) / 4;
			else
				j = MAX_EUCKR_CHAR_NUM;

			if(j < MAX_EUCKR_CHAR_NUM)//found
				uniCode = euckrTbl[j][1];
			else
				bFind = 0;

			i++;
		}
		else if(text[i] < 0x80) { //find ascii code
			uniCode = (unsigned char)text[i];
		}
		else {
			bFind = 0;
		}

		if(bFind)
		{
			FT_Load_Char(face, uniCode, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);

			{
				int x, y;
				int bufx = 0;
				int bufy = 0;
				int width = face->glyph->bitmap.width;
				int height = face->glyph->bitmap.rows;

				for(y = 0; y < height; y++) {
					for(x = 0; x < width; x++) {
						if(face->glyph->bitmap.buffer[y*width+x] != 0) {
							bufx = (penx >> 6) + face->glyph->bitmap_left + x;
							bufy = (peny >> 6) - face->glyph->bitmap_top + y;
							if(bufy < 0 || bufx < 0) {
								continue;
							}
							buf[bufy*pitch + bufx + left] = 0x8000 | color;
						}
					}
				}
			}

			penx += face->glyph->advance.x;
			peny += face->glyph->advance.y;
		}
	}
	return 0;
}
