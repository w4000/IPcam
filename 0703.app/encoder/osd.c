#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "hi_common.h"
#include "hi_comm_video.h"
#include "hi_comm_sys.h"
#include "hi_comm_ive.h"
#include "hi_comm_vgs.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"

#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_vi.h"
#include "mpi_vo.h"

#include "mpi_ive.h"
#include "mpi_vgs.h"

#include "common.h"
#include "soc_common.h"
#include "osd.h"
#include "osd_status.h"
#include "osd_title.h"
#include "osd_datetime.h"
#if 0
#else
#define RGBCONV(r,g,b,T)		(((T) << 15) | (((r) & 0x00f8)<<7) | (((g) & 0x00f8)<<2) | (((b) & 0x00f8)>>3) )
#define ARGB(T,r,g,b)			((((T)& 0x1) << 15) | (((r) & 0x00f8)<<7) | (((g) & 0x00f8)<<2) | (((b) & 0x00f8)>>3) )
#endif	/* USE_BITMAP_32BIT */


#include "list.h"
/*********************************/
#include "lib/euc-kr.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#define FONT_FILE "/edvr/D2Coding.ttf"

int canvas_area_position[OSD_BUF_COUNT][OSD_AREA_INDEX] = {
	{    0,    0 },	
	{    0,  811 },//w4000 {    0,  811 },	x,y
	{  640,  811 },	//w4000  {  640,  811 },	{  850,  811 }
};

const int osd_position[OSD_LIST_MAX][OSD_AREA_INDEX] = {
	{   40,   40 },	
	{   40,  170 },
	{   40,  170 }, 
		//w4000 {	40,  100 },
		{	200,  50 },//x,y {	100,  1 }
		//w4000 {	40,  110 }, 
		{	200,  160 }, //{	150,  170 }
};

static int g_text_size = FONT_SIZE;

int getTextSize(void)
{
	return g_text_size;
}

void setTextSize(int size)
{
	if(size < 0 || size > 200) {
		return;
	}

	g_text_size = size;
}

void setDefaultTextSize(void)
{
	int size = FONT_SIZE;
	g_text_size = size;
}

static FT_Library library = NULL;
static char g_ft_file[128];

#define MAX_OSD_COUNT 128//w4000 32

static OSD_ITEM_S *g_osd[MAX_OSD_COUNT] = {NULL,};
static int g_language = 0;

int osd_font_init()
{
	{
		sprintf(g_ft_file, FONT_FILE);
	}

	int ret = FT_Init_FreeType(&library);
	if(ret != 0) {
		printf("Failed to run FT_Init_FreeType(%d)\n", ret);
	}

	return ret;
}

int osd_font_fini(void)
{
	int ret = FT_Done_FreeType(library);
	if(ret != 0) {
		printf("Failed to run FT_Done_FreeType(%d)\n", ret);
	}

	return ret;
}

static int is_new_item(int id)
{
	if(g_osd[id] == NULL) {
		return 1;
	}
	return 0;
}

static int add_item(int id)
{
	g_osd[id] = (OSD_ITEM_S *)malloc(sizeof(OSD_ITEM_S));
	memset(g_osd[id], 0, sizeof(OSD_ITEM_S));
	g_osd[id]->text.data = NULL;
	g_osd[id]->text.data_size = 0;
	g_osd[id]->text.font_size = 16;

	return 0;
}

static int is_shown_item(int id)
{
	if(is_new_item(id)) {
		return 0;
	}
	return g_osd[id]->is_show;
}

int loadVfbAreaForNovatek(int id, int osdPosIndex, VFB_AREA_S* vfb_area)
{
	if(osdPosIndex >= 0) {
		vfb_area->x = osd_position[osdPosIndex][0];

		vfb_area->y = osd_position[osdPosIndex][1];
		vfb_area->width = getVfbWidth(id);
		vfb_area->height = INIT_MAX_HEIGHT;
	}
	else {
		vfb_area->x = 0;
		vfb_area->y = 0;
		vfb_area->width = getVfbWidth(id);
		vfb_area->height = getVfbHeight(id);
	}
	//vfb_area->width=1500;//w4000

	return 0;
}


int loadVfbArea(int id, int osdPosIndex, VFB_AREA_S* vfb_area)
{
	if(vfb_area == NULL) {
		return -1;
	}
	loadVfbAreaForNovatek(id, osdPosIndex, vfb_area);

	return 0;
}

static int erase_prev_area_for_novatek(int id, OSD_AREA_S* area)
{
	VFB_AREA_S vfb_area;
	loadVfbArea(id, area->areaPosIndex, &vfb_area);

	vfb_clear_area(id, &vfb_area);
	update_osd_text(id, canvas_area_position[id]);

	return 0;
}


static int erase_prev_area(int id, OSD_AREA_S* area)
{
	if(area == NULL) {
		return -1;
	}
	erase_prev_area_for_novatek(id, area);
	return 0;
}

static int copy_osd_area(int index, OSD_AREA_S *dest, OSD_AREA_S *src)
{
	OSD_AREA_S temp_dest;
	int _max_w;
	int _max_h;
	_max_w = getVfbWidth(index);
	_max_h = getVfbHeight(index);

	if(src->x < 0 || src->y < 0) {
		if(src->x < 0) {
			temp_dest.x = 0;
		}
		if(src->y < 0) {
			temp_dest.y = 0;
		}
	}
	else if(src->x > _max_w || src->y > _max_h) {
		if(src->x > _max_w) {
			src->x = _max_w;
		}
		if(src->y > _max_h) {
			src->y = _max_h;
		}
	}
	else {
		temp_dest.x = (src->x >> 1) << 1;
		temp_dest.y = (src->y >> 1) << 1;
	}

	if(src->max_width < 0) {
		src->max_width = _max_w - src->x;
	}

	if(src->max_height < 0) {
		src->max_height = _max_h - src->y;
	}

	temp_dest.max_width = ((src->max_width >> 1) << 1);
	temp_dest.max_height = ((src->max_height >> 1) << 1);
	temp_dest.areaIndex = src->areaIndex;

	if(temp_dest.x != dest->x || temp_dest.y != dest->y ||
			temp_dest.max_width != dest->max_width || temp_dest.max_height != dest->max_height) {
		memcpy(dest, &temp_dest, sizeof(OSD_AREA_S));
		return 1;
	}

	return 0;
}

static int osd_binary_compare(const void *a, const void *b)
{
	return ( *(unsigned short*)a - *(unsigned short*)b );
}

static unsigned short osd_unicode_char(char *text, int lang, int *add)
{
	unsigned short *binaryPtr;
	unsigned short uniCode;
	int bFind = 1;
	int j;

	*add = 0;

	if((text[0] & 0x80) && (text[1] != '\0')) { // find cp936 code (2 byte)
		uniCode = (unsigned short )((text[0] << 8) & 0xFF00) | (text[1] & 0xFF);

		{
			if((binaryPtr = (unsigned short *)bsearch(&uniCode, euckrTbl, MAX_EUCKR_CHAR_NUM, 4, osd_binary_compare)) != NULL)
				j = ((char *)binaryPtr - (char *)euckrTbl) / 4;
			else
				j = MAX_EUCKR_CHAR_NUM;

			if(j < MAX_EUCKR_CHAR_NUM)
				uniCode = euckrTbl[j][1];//w4000 euc to kr
			else
				bFind = 0;
		}

		*add = 1;
	}
	else if(text[0] < 0x80) { 
		uniCode = (unsigned char)text[0];
	}
	else {
		bFind = 0;
	}

	if(bFind == 0) {
		return 0xFFFF;
	}

	return uniCode;
}

static int osd_font_get_size(char *text, int fh, int lang, int *w, int *h)
{
	int i;
	int add;
	int height = 0;
	int h2;
	int ret = -1;
	FT_Face face;
	int textLen = 0;
	int maxWidthLen = 0;

	if(text == NULL) {
		printf("text is NULL\n");
		return -1;
	}
	textLen = strlen(text);
	*w = 0;
	*h = 0;

	ret = FT_New_Face(library, g_ft_file, 0, &face);
	if(ret != 0) {
		printf("Failed to run FT_New_Face(err = %d)\n", ret);
		return 0;
	}

	ret = FT_Set_Char_Size(face, fh*64, 0, 96, 0);
	if(ret != 0) {
		FT_Done_Face(face);
		printf("Failed to run FT_Set_Char_Size(err = %d)\n", ret);
		return 0;
	}

	for(i = 0; i < textLen; i++) {
		unsigned short uniCode;

		uniCode = osd_unicode_char(&text[i], lang, &add);
		if(uniCode != 0xFFFF) {
			FT_Load_Char(face, uniCode, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_MONO);
			*w += (face->glyph->advance.x >> 6) + face->glyph->bitmap_left;
			h2 = face->glyph->bitmap.rows + (face->glyph->bitmap.rows - face->glyph->bitmap_top) + 4;
			if(*h < h2) {
				*h = h2;
			}
		}
		if(text[i] == '\n') {
			height += *h + 8;
			if(*w > maxWidthLen) {
				maxWidthLen = *w;
			}
			*w = 0;
		}
	}

	if(*w > maxWidthLen) {
		maxWidthLen = *w;
	}

	if(height != 0) {
		*h = height + *h;
	}

	*w = (((maxWidthLen >> 1) << 1) + 2);
	*h = ((*h >> 1) << 1) + 2;

	FT_Done_Face(face);

	return 0;
}

static int glyphBit(const FT_GlyphSlot glyph, const int x, const int y)
{
	int pitch = abs(glyph->bitmap.pitch);
	unsigned char *row = &glyph->bitmap.buffer[pitch * y];
	char cValue = row[x >> 3];
	return (cValue & (128 >> (x & 7))) != 0;
}


#if 1
static int osd_font_text2bitmap(VFB_BITMAP_S *bitmap, OSD_ITEM_S *item, const int v, int lang)
{
    int i;
    int add;
    int penx = 0;
    int fs = item->text.font_size;
    int peny = fs * 64;
    int char_h = 0;
    int peny2 = fs * 64;
    char* _text = item->text.data;
    unsigned int uIntColor = item->color;
    unsigned int backgroundColor =0x005555;//0xf00f;//blue 0x000000; // 배경색을 정의 (검정색 기본값)
    int _width = bitmap->area.width;
    int _height = bitmap->area.height;
    FT_Face face;
#if 0
	static unsigned int uIntColor1=0;
	if(uIntColor1)uIntColor1--;
	else uIntColor1=50;
	if(uIntColor1>40)uIntColor=0xf00f;//blue
	else if(uIntColor1>30)uIntColor=0xfff0;//yello
	else if(uIntColor1>20)uIntColor=0xf0f0;//green
	else if(uIntColor1>10)uIntColor=0xff00;//red
	else uIntColor=0xffff;//

	uIntColor = item->color=uIntColor;
	printf("w4000 uIntColor1=%x \r\n",uIntColor);
#endif

    if (FT_New_Face(library, g_ft_file, 0, &face) != 0) {
        return -1;
    }

    if (FT_Set_Char_Size(face, fs * 64, 0, 96, 0) != 0) {
        FT_Done_Face(face);
        return -1;
    }
	
#if 1//w4000
				// 드로잉 영역을 배경 색으로 초기화
				for (int y = 0; y < (_height*2/4); y++) {
					for (int x = 0; x < (_width*1.1); x++) {
						bitmap->data[y * _width + x] = backgroundColor; // 배경색 적용
					}
				}
			
#endif
    for (i = 0; i < strlen(_text); i++) {
        unsigned short uniCode;
        if (_text[i] == '\n') {
            penx = 0;
            peny2 += (char_h << 6);
            peny = peny2;
            char_h = 0;
            continue;
        }

        uniCode = osd_unicode_char(_text + i, lang, &add);
#define def_rows 8//20//8
#define def_pen 6//12//6
        if (uniCode != 0xFFFF) {
            FT_Load_Char(face, uniCode, FT_LOAD_RENDER | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO);
            {
                int x, y, v = 0;
                int bufx = 0;
                int bufy = 0;
                int glyphWidth = face->glyph->bitmap.width;
                int glyphHeight = face->glyph->bitmap.rows;
				static int test_woo=0;
#if 1
                y = 0;
                for (x = 0; x < glyphWidth; x++) {
                    if (face->glyph->bitmap.buffer[y * glyphWidth + x] != 0) {
                        bufy = (peny >> def_pen) - face->glyph->bitmap_top + y;
                        if (bufy < 0) {
                            v = bufy;
                            break;
                        }
                    }
                }
#endif
				{
	//int yy[100];
	//int xx[100];
	                for (y = 0; y < glyphHeight; y++) {
	                    for (x = 0; x < glyphWidth; x++) {
							
	                        if (glyphBit(face->glyph, x, y)) {
	                            bufx = (penx >> def_pen) + face->glyph->bitmap_left + x;
	                            bufy = (peny >> def_pen) - face->glyph->bitmap_top + y - v;
	                            if (bufx < 0 || bufx >= _width || bufy >= _height){
	                                continue;
	                            }
								
	                            bitmap->data[bufy * _width + bufx] = uIntColor;
	                        }
	                    }
	                }
				}
            }
	

            penx += face->glyph->advance.x;
            peny += face->glyph->advance.y;

            if (char_h < face->glyph->bitmap.rows + def_rows) {
                char_h = face->glyph->bitmap.rows + def_rows;
            }
        }
        i += add;
    }

    FT_Done_Face(face);
    return 0;
}

#else
static int osd_font_text2bitmap(VFB_BITMAP_S *bitmap, OSD_ITEM_S *item, const int v, int lang)
{
	int i;
	int add;
	int penx = 0;
	int fs = item->text.font_size;
	int peny = fs * 64;
	int char_h = 0;
	int peny2 = fs * 64;
	char* _text = item->text.data;
	unsigned int uIntColor = item->color;
	int _width = bitmap->area.width;
	int _height = bitmap->area.height;
	FT_Face face;
	#if 0
	static unsigned int uIntColor1=0;
	if(uIntColor1)uIntColor1--;
	else uIntColor1=50;
	if(uIntColor1>40)uIntColor=0xf00f;//blue
	else if(uIntColor1>30)uIntColor=0xfff0;//yello
	else if(uIntColor1>20)uIntColor=0xf0f0;//green
	else if(uIntColor1>10)uIntColor=0xff00;//red
	else uIntColor=0xffff;//

	uIntColor = item->color=uIntColor;
	printf("w4000 uIntColor1=%x \r\n",uIntColor);
	#endif
	if(FT_New_Face(library, g_ft_file, 0, &face) != 0) {
		return -1;
	}

	if(FT_Set_Char_Size(face, fs * 64, 0, 96, 0) != 0) {
		FT_Done_Face(face);
		return -1;
	}

	for(i = 0; i < strlen(_text); i++) {
		unsigned short uniCode;
		if(_text[i] == '\n') {
			penx = 0;
			peny2 += (char_h<<6);
			peny = peny2;
			char_h = 0;
			continue;
		}

		uniCode = osd_unicode_char(_text+i, lang, &add);
		if(uniCode != 0xFFFF) {
			FT_Load_Char(face, uniCode, FT_LOAD_RENDER | FT_LOAD_MONOCHROME | FT_LOAD_TARGET_MONO);
			{
				int x, y, v = 0;
				int bufx = 0;
				int bufy = 0;
				int glyphWidth = face->glyph->bitmap.width;
				int glyphHeight = face->glyph->bitmap.rows;

				y = 0;
				for(x = 0; x < glyphWidth; x++) {
					if(face->glyph->bitmap.buffer[y*glyphWidth+x] != 0) {
						bufy = (peny >> 6) - face->glyph->bitmap_top + y;
						if(bufy < 0) {
							v = bufy;
							break;
						}
					}
				}

				for(y = 0; y < glyphHeight; y++) {
					for(x = 0; x < glyphWidth; x++) {
						if(glyphBit(face->glyph, x, y)) {
							bufx = (penx >> 6) + face->glyph->bitmap_left + x;
							bufy = (peny >> 6) - face->glyph->bitmap_top + y - v;
							if(bufx < 0) {
								continue;
							}
							if(bufx >= _width || bufy >= _height) {
								continue;
							}
							bitmap->data[bufy * _width + bufx] = uIntColor;
						}
					}
				}
			}

			penx += face->glyph->advance.x;
			peny += face->glyph->advance.y;

			if(char_h < face->glyph->bitmap.rows + 8) { 
				char_h = face->glyph->bitmap.rows + 8; 
			}
		}
		i += add;
	}

	FT_Done_Face(face);

	return 0;
}
#endif
static int text2bitmap(VFB_BITMAP_S *bitmap, OSD_ITEM_S *item)
{
	int w = 0;
	int h = 0;
	int v = osd_font_get_size(item->text.data, item->text.font_size, 0, &w, &h);
	if(item->area.max_width > 0 || item->area.max_width <= w) {
		w = item->area.max_width;
	}

	if(item->area.max_height > 0 || item->area.max_height <= h) {
		h = item->area.max_height;
	}

	item->area.max_width = w;
	item->area.max_height = h;
	bitmap->area.x = item->area.x;
	bitmap->area.y = item->area.y;
	// FIXME
	bitmap->area.width = item->area.max_width;
	bitmap->area.height = item->area.max_height;
	//printf("! %s:%d > malloc size? %d \n", __func__, __LINE__, bitmap->area.width * bitmap->area.height * sizeof(PIXEL_DATA_FORMAT));
	bitmap->data = (PIXEL_DATA_FORMAT*)malloc(bitmap->area.width * bitmap->area.height * sizeof(PIXEL_DATA_FORMAT));
	memset(bitmap->data, 0x0, bitmap->area.width * bitmap->area.height * sizeof(PIXEL_DATA_FORMAT));
	osd_font_text2bitmap(bitmap, item, v, g_language);

	return 0;
}



int osd_set_language(int lang)
{
	int ret = 0;
	g_language = lang;
	ret = osd_font_init();
	if(ret < 0) {
		printf("Failed to run osd_font_init()\n");
	}

	return ret;
}

int osd_init(unsigned int screen_width, unsigned int screen_height, int lang)
{

	osd_set_language(lang);
	return 0;
}

int osd_reinit(unsigned int screen_width, unsigned int screen_height, int lang)
{
	vfb_fini();
	osd_set_language(lang);
	return 0;
}

void osd_fini(void)
{
	int i;
	for(i = 0; i < MAX_OSD_COUNT; i++) {
		if(g_osd[i] != NULL) {
			if(g_osd[i]->text.data != NULL) {
				free(g_osd[i]->text.data);
				g_osd[i]->text.data = NULL;
			}
			free(g_osd[i]);
			g_osd[i] = NULL;
		}
	}

	osd_font_fini();
	vfb_fini();

	return;
}

int osd_hide_text_for_novatek(int id, OSD_AREA_S* area)
{
	if(is_shown_item(area->areaIndex)) {
		g_osd[area->areaIndex]->is_show = 0;
		erase_prev_area(id, area);
	}

	return 0;
}

int osd_hide_text(int id, OSD_AREA_S* area)
{
	if((id >= MAX_OSD_COUNT || id < 0)  || (area == NULL)) {
		if(id >= MAX_OSD_COUNT || id < 0) {
		}

		if(area == NULL) {
		}
		return -1;
	}
	osd_hide_text_for_novatek(id, area);
}

int osd_draw_text_for_novatek(int id, OSD_AREA_S *area, char *text, OSD_COLOR_E color, int font_size)
{
#if defined(NARCH)
	int need_update = 0;
	VFB_BITMAP_S bitmap[1];

	memset(bitmap, 0, sizeof(bitmap));

	if(id >= OSD_BUF_COUNT) {
		return -1;
	}

	if(is_new_item(area->areaIndex)) {
		add_item(area->areaIndex);
	}

	if(color == S_TRANSPARENCY) {
		g_osd[area->areaIndex]->is_show = 0;
		erase_prev_area(id, area);
		return 0;
	}

	copy_osd_area(id, &g_osd[area->areaIndex]->area, area);
	if(g_osd[area->areaIndex]->color != color) {
		g_osd[area->areaIndex]->color = color;
		need_update = 1;
	}

	if(g_osd[area->areaIndex]->is_show != 1) {
		g_osd[area->areaIndex]->is_show = 1;
		need_update = 2;
	}

	if(g_osd[area->areaIndex]->text.font_size != font_size) {
		g_osd[area->areaIndex]->text.font_size = (font_size < 0)?16:font_size;
		need_update = 3;
	}

	if(g_osd[area->areaIndex]->text.data == NULL || g_osd[area->areaIndex]->text.data_size < strlen(text)+4) {
		g_osd[area->areaIndex]->text.data_size = (strlen(text) < 64)?64:strlen(text)+4;
		if(g_osd[area->areaIndex]->text.data != NULL) {
			free(g_osd[area->areaIndex]->text.data);
		}

		g_osd[area->areaIndex]->text.data = (char *)malloc(g_osd[area->areaIndex]->text.data_size);
	}


	if(strncmp(g_osd[area->areaIndex]->text.data, text, g_osd[area->areaIndex]->text.data_size) != 0) {
		strncpy(g_osd[area->areaIndex]->text.data, text, g_osd[area->areaIndex]->text.data_size);
		need_update = 4;
	}

	if(need_update > 0) {
		if(text2bitmap(&bitmap[0], g_osd[area->areaIndex]) < 0) {
			return -1;
		}

		vfb_add_bitmaps_with_outline(id, bitmap, 1, (PIXEL_DATA_FORMAT)color);

		free(bitmap[0].data);
		bitmap[0].data = NULL;

		update_osd_text(id, canvas_area_position[id]);
	}
#endif	/* NARCH */

	return 0;
}



int osd_draw_text(int id, OSD_AREA_S *area, char *text, OSD_COLOR_E color, int font_size)
{
	osd_draw_text_for_novatek(id, area, text, color, font_size);
}


/**************************************************/


int get_y_position (void)
{
	int res = 0;
	unsigned int w, h;


	if(g_setup.tmosd.position == 0) {
		res = 40;
	}
	else {
		res = h - 40 - g_text_size;
	}

	return res;
}


OSD_COLOR_E convertToOsdColor(int index)
{
	OSD_COLOR_E color;
	switch(index) {
		case 1: color = S_BLACK; break;
		case 2: color = S_RED; break;
		case 3: color = S_BLUE; break;
		case 4: color = S_GREEN; break;
		default: color = S_WHITE; break;
	}

	return color;
}

OSD_COLOR_E getDatetimeOsdColor(void)
{
	OSD_COLOR_E color = S_WHITE;
	
	return color;
}

OSD_COLOR_E getSystemOsdColor(void)
{
	OSD_COLOR_E color = S_WHITE;

	{
		switch(g_setup.cam [0].title_osd.color) {
			case 1: color = S_BLACK; break;
			case 2: color = S_RED; break;
			case 3: color = S_BLUE; break;
			case 4: color = S_GREEN; break;
			default: color = S_WHITE; break;
		}
	}


	return color;
}

void osd_datetime(void)
{
	osd_datetime_novatek();
}

// void osd_status(void)
// {
// 	osd_model_name_novatek();
// }

void osd_title(void)
{
	//osd_title_novatek();
	//osd_title_novatek1();
	osd_title_novatek2();
}

static time_t status_osd_time = 0;
static char status_osd_text[OSD_TEXT_LEN] = "\0";


time_t *getOsdStatusTime(void)
{
	return &status_osd_time;
}


static int s_bOsdStopSignal = 0;
static pthread_t s_hOsdThread = 0;

void osd_all_clear(void)
{
	int i;
	OSD_AREA_S area;
	for(i = 0;i < OSD_BUF_COUNT;i++) {
		area.areaPosIndex = -1;
		erase_prev_area(i, &area);
	}
}

static void init_osd_proc(void)
{
	PIC_SIZE_E enSize = def_max_resol(0);
	osd_init(def_resol_index_to_width(0, enSize),
				def_resol_index_to_height(0, enSize),
				SETUP_LANG(&g_setup));

}

int init_osd_buffer(int i, VFB_CANVAS_INFO* vfbInfo)
{
	init_vfb(i, vfbInfo);
	return 0;
}

int init_osd_language(void)
{
	osd_set_language(SETUP_LANG(&g_setup));
	return 0;
}

static void osd_environment(void)
{
}

static void *osd_proc(void *argc)
{
	osd_all_clear();
	setStartOsdThread();

	int cnt =0;
	int first = 1;
	while(0 == s_bOsdStopSignal) {
		#if 1//w4000_osd_title
		usleep(940*1000);
		osd_datetime();
		osd_title();
		
		#else
		usleep(300*1000);
		osd_datetime();
		if (cnt++ >= 3)
		{
			//w4000 
			osd_title();
			// if (first)
			// {
			// 	first = 0;
			// }
			cnt = 0;
		}
		#endif
	}

	osd_fini();

	return NULL;
}

void setStopOsdThread(void)
{
	s_bOsdStopSignal = 1;
}

void setStartOsdThread(void)
{
	s_bOsdStopSignal = 0;
}

int run_osd_thread(void)
{
	pthread_create(&s_hOsdThread, 0, osd_proc, NULL);
	return 0;
}

void deinit_osd_thread(void)
{
	pthread_join(s_hOsdThread, HI_NULL);
}
