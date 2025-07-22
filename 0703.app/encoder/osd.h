#ifndef __OSD_H__
#define __OSD_H__

#include "vfb.h"

enum {
    INIT_MAX_WIDTH = -1,
	INIT_MAX_WIDTH_FOR_TITLE = -1,
	INIT_MAX_WIDTH_FOR_STATUS = -1,
#if defined(NARCH)
    INIT_MAX_HEIGHT = 60,
	FONT_SIZE = 24,//w4000_font 32,
#endif	/* NARCH */

};

typedef struct _OSD_AREA_S {
	int areaIndex;
	int areaPosIndex;
	int x;
	int y;
	int max_width;
	int max_height;
} OSD_AREA_S;

typedef struct _OSD_ITEM_S {
	OSD_AREA_S area;	// x, y, width, height
	OSD_COLOR_E color;
	int is_show;
	struct OSD_TEXT {
		int font_size;
		int data_size;	// text length
		char *data;		// text content
	} text;
} OSD_ITEM_S;

enum {
	OSD_TEXT_SIZE,
};

int osd_init(unsigned int screen_width, unsigned int screen_height, int lang);
int osd_reinit(unsigned int screen_width, unsigned int screen_height, int lang);
void osd_fini(void);
int osd_draw_text(int id, OSD_AREA_S *area, char *text, OSD_COLOR_E color, int font_size);

// NOVATEK
#if defined(NARCH)
int init_osd_buffer(int, VFB_CANVAS_INFO *);
#endif	/* NARCH */
/* ONLY NOVATEK */
extern int canvas_area_position[OSD_BUF_COUNT][OSD_AREA_INDEX];
extern const int osd_position[OSD_LIST_MAX][OSD_AREA_INDEX];
/* ONLY NOVATEK */

// char* get_osd_ecam_version(void);
time_t *getOsdStatusTime(void);
int osd_hide_text(int, OSD_AREA_S*);
void setStartOsdThread(void);
void setStopOsdThread(void);
int run_osd_thread(void);

OSD_COLOR_E getSystemOsdColor(void);
OSD_COLOR_E getDatetimeOsdColor(void);

#endif /*__OSD_H__*/
