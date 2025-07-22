#ifndef __VFB_H__
#define __VFB_H__

typedef unsigned short	__u16;
typedef unsigned int	__u32;

#if defined(USE_BITMAP_32BIT)
typedef __u32 PIXEL_DATA_FORMAT;
#else
typedef __u16 PIXEL_DATA_FORMAT;
#endif

#if defined(NARCH)
// ARGB444
typedef enum {
	S_TRANSPARENCY = 0x0,
	S_WHITE = 0xFFFF, 
	S_BLACK = 0xF000,
	S_BLUE = 0xF00F,
	S_GREEN = 0xF0F0,
	S_RED = 0xFF00,
	S_GRAY = 0xF777,
} OSD_COLOR_E;

#else	/* !NARCH */
typedef enum {
#if defined(USE_BITMAP_32BIT)
	S_TRANSPARENCY = 0x0,
	S_WHITE = 0xFFFFFFFF,
	S_BLACK = 0xFF000000,
	S_BLUE = 0xFF0000FF,
	S_GREEN = 0xFF00FF00,
	S_RED = 0xFFFF0000,
	S_GRAY = 0x808F8080,
#else
	S_TRANSPARENCY = 0x0,
	S_WHITE = 0xFFFF,
	S_BLACK = 0x8000,
	S_BLUE = 0x801F,
	S_GREEN = 0x83E0,
	S_RED = 0xFC00,
	S_GRAY = 0xC610,
#endif
} OSD_COLOR_E;
#endif	/* NARCH */

/* ONLY NOVATEK */
enum {
	OSD_1ST_AREA,
	OSD_2ND_AREA,
	OSD_3RD_AREA,
	// OSD_4TH_AREA,
	OSD_BUF_COUNT,	// osd buffer count (flexiable)
};

enum {
	OSD_AREA_X,
	OSD_AREA_Y,
	OSD_AREA_INDEX,		// fixed value (x, y)
};

enum {
	OSD_DATETIME_ID,
	OSD_STATUS_ID,
	OSD_TITLE_ID,
};

enum {
	OSD_DATETIME_LEFT_UP,
	OSD_DATETIME_LFET_DOWN,
	OSD_STATUS_MIDDLE_DOWN,
	OSD_TITLE_LEFT_UP,
	OSD_TITLE_LEFT_DOWN,
	OSD_LIST_MAX,
};
/* ONLY NOVATEK */

typedef struct _VFB_AREA_S {
	int x;
	int y;
	unsigned int width;
	unsigned int height;
} VFB_AREA_S;

typedef struct _VFB_BITMAP_S {
	VFB_AREA_S area;
	PIXEL_DATA_FORMAT *data;
} VFB_BITMAP_S;

// NOVATEK
typedef struct _STAMP_DATA {
	__u32 vp_stamp_path;
	__u32 stamp_pa;
	__u32 stamp_size;
} STAMP_DATA;

typedef struct _VFB_CANVAS_INFO {
	STAMP_DATA stamp;
	VFB_AREA_S area;
} VFB_CANVAS_INFO;

// COMMON
int vfb_add_bitmaps(VFB_BITMAP_S bitmap[], int array_size);

int vfb_init(__u16 width, __u16 height);
void vfb_fini();

int vfb_add_bitmaps_with_outline(int id, VFB_BITMAP_S bitmap[], int array_size, PIXEL_DATA_FORMAT);
int vfb_clear_area(int, VFB_AREA_S *area);

int vfb_size(unsigned int *width, unsigned int *height);

// NOVATEK
int init_vfb(int, VFB_CANVAS_INFO*);
void setImageBuffer(int, unsigned short*);

int getVfbWidth(int);
int getVfbHeight(int);

int update_osd_text(int, const int*);
#endif /*__VFB_H__*/
