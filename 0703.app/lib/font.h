#ifndef __FONT_H__
#define __FONT_H__

int font_init(char *font_file);
void font_fini();
int font_text2rgb1555(const char *text, const int size, const int pitch,
		const unsigned short color, unsigned short buf[], const int left);

#endif /*__FONT_H__*/
