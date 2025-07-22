#include <stdio.h>//w4000
#include "common.h"
#include "vfb.h"
#include "osd.h"
#include "osd_title.h"


int getTitleOsdArea(int id, OSD_AREA_S *tm_area)
{
	if(id != OSD_2ND_AREA && id != OSD_1ST_AREA) {
		return -1;
	}

	int posIndex = 0;
	
	{
		posIndex = OSD_TITLE_LEFT_UP;
		if(id == OSD_2ND_AREA) {
			posIndex = OSD_TITLE_LEFT_DOWN;
		}
	}
	
    tm_area->x = osd_position[posIndex][0];
	tm_area->y = osd_position[posIndex][1];
	tm_area->max_width = INIT_MAX_WIDTH_FOR_TITLE;
	tm_area->max_height = INIT_MAX_HEIGHT;
	tm_area->areaPosIndex = posIndex;

    return posIndex;
}

int getTitleOsdId(void)
{
	{
#if defined(NARCH)
		if(0 == g_setup.cam [0].title_osd.position) {
			return OSD_1ST_AREA;
		}
		return OSD_2ND_AREA;
#else
		return 0;
#endif	/* NARCH */
	}
	return 0;
}

void osd_title_novatek(void)
{
	static int t_usage = 0;
	return;

#if defined(NARCH)
	if(g_setup.cam [0].title_osd.usage == 0 && t_usage == 0) {
		return;
	}
#endif	/* NARCH */

	// id by position
	static int t_pos = -1;
	static int osdPosIndex = -1;
	int t_id;
	t_id = t_pos;
	if(t_pos < 0) {
		t_id = getTitleOsdId();
	}

	OSD_AREA_S t_area;
	t_area.areaPosIndex = osdPosIndex;
	t_area.areaIndex = OSD_TITLE_ID;

#if defined(NARCH)
	if(g_setup.cam [0].title_osd.usage == 0 && t_usage == 1) {
		t_usage = 0;
		if(t_pos >= 0) {
			getTitleOsdArea(t_pos, &t_area);
			osdPosIndex = t_area.areaPosIndex;
			osd_hide_text(t_pos, &t_area);
		}
		return;
	}
#endif	/* NARCH */
	t_usage = 1;
	if(t_pos != getTitleOsdId()) {
		if(t_pos >= 0) {
			t_area.areaPosIndex = osdPosIndex;
			t_area.areaIndex = OSD_TITLE_ID;
			osd_hide_text(t_id, &t_area);
		}
		t_pos = getTitleOsdId();
		getTitleOsdArea(t_pos, &t_area);
		osdPosIndex = t_area.areaPosIndex;
	}

	OSD_COLOR_E color;
	color = getSystemOsdColor();
	getTitleOsdArea(t_pos, &t_area);
	setTextSize(getTextSize());
	// 2025-1-21 w4000 
	
	//t_area.x=t_area.x+700;
	//t_area.max_width=t_area.max_width+700;
	osd_draw_text(t_pos, &t_area, g_setup.cam [0].title, color, getTextSize());
	// 2025-2-3 w4000 printf("w4000 title =%s\r\n",g_setup.cam [0].title);
	
}

void osd_title_novatek1(void)
{
	static int t_usage = 0;

#if defined(NARCH)
	if(g_setup.cam [0].title_osd.usage == 0 && t_usage == 0) {
		return;
	}
#endif	/* NARCH */

	// id by position
	static int t_pos = -1;
	static int osdPosIndex = -1;
	int t_id;
	t_id = t_pos;
	if(t_pos < 0) {
		t_id = getTitleOsdId();
	}

	OSD_AREA_S t_area;
	t_area.areaPosIndex = osdPosIndex;
	t_area.areaIndex = OSD_TITLE_ID;

#if defined(NARCH)
	if(g_setup.cam [0].title_osd.usage == 0 && t_usage == 1) {
		t_usage = 0;
		if(t_pos >= 0) {
			getTitleOsdArea(t_pos, &t_area);
			osdPosIndex = t_area.areaPosIndex;
			osd_hide_text(t_pos, &t_area);
		}
		return;
	}
#endif	/* NARCH */
	t_usage = 1;
	if(t_pos != getTitleOsdId()) {
		if(t_pos >= 0) {
			t_area.areaPosIndex = osdPosIndex;
			t_area.areaIndex = OSD_TITLE_ID;
			osd_hide_text(t_id, &t_area);
		}
		t_pos = getTitleOsdId();//OSD_2ND_AREA;//
		getTitleOsdArea(t_pos, &t_area);
		osdPosIndex = t_area.areaPosIndex;
	}

	OSD_COLOR_E color;
	color = getSystemOsdColor();
	getTitleOsdArea(t_pos, &t_area);
	setTextSize(getTextSize());
	// 2025-1-21 w4000 osd_draw_text(t_pos, &t_area, g_setup.cam [0].title, color, getTextSize());
	// 2025-2-3 w4000 printf("w4000 title =%s\r\n",g_setup.cam [0].title);
	
	enum { OSD_BUF_SIZE = 128, };
	char osd[OSD_BUF_SIZE];
	getDateTimeToText(osd);
	// 2025-2-3 w4000 
	/*
	printf("w4000 osd =%s\r\n",osd);
	printf("w4000 osd x =%d, y=%d \r\n",t_area.x,t_area.y);
	printf("w4000 osd usage=%d \r\n",g_setup.cam [0].title_osd.usage); //*/
	//t_area.x=1;t_area.y=1;
	t_area.x=t_area.x+0;
	osd_draw_text(t_pos, &t_area, osd, color, getTextSize());
	//t_area.y=t_area.y+10;
	//osd_draw_text(t_pos, &t_area, g_setup.cam [0].title, color, getTextSize());
}



void osd_title_novatek2(void)
{
	static int t_usage = 0;
	size_t length11;//w4000

#if defined(NARCH)
	if(g_setup.cam [0].title_osd.usage == 0 && t_usage == 0) {
		return;
	}
#endif	/* NARCH */

	// id by position
	static int t_pos = -1;
	static int osdPosIndex = -1;
	int t_id;
	t_id = t_pos;
	if(t_pos < 0) {
		t_id = getTitleOsdId();
	}

	OSD_AREA_S t_area;
	t_area.areaPosIndex = osdPosIndex;
	t_area.areaIndex = OSD_TITLE_ID;

#if defined(NARCH)
	if(g_setup.cam [0].title_osd.usage == 0 && t_usage == 1) {
		t_usage = 0;
		if(t_pos >= 0) {
			getTitleOsdArea(t_pos, &t_area);
			osdPosIndex = t_area.areaPosIndex;
			osd_hide_text(t_pos, &t_area);			
			t_area.y=t_area.y+40*1;
			osd_hide_text(t_pos, &t_area);
			osd_all_clear();//w4000

		}
		return;
	}
#endif	/* NARCH */
	t_usage = 1;
	if(t_pos != getTitleOsdId()) {
		if(t_pos >= 0) {
			t_area.areaPosIndex = osdPosIndex;
			t_area.areaIndex = OSD_TITLE_ID;
			osd_hide_text(t_id, &t_area);
			t_area.y=t_area.y+40*1;
			osd_hide_text(t_id, &t_area);
			osd_all_clear();//w4000

		}
		t_pos = getTitleOsdId();//OSD_2ND_AREA;//
		getTitleOsdArea(t_pos, &t_area);
		osdPosIndex = t_area.areaPosIndex;
	}

	OSD_COLOR_E color;
	color = getSystemOsdColor();
	getTitleOsdArea(t_pos, &t_area);
	setTextSize(getTextSize());
	// 2025-1-21 w4000 osd_draw_text(t_pos, &t_area, g_setup.cam [0].title, color, getTextSize());
	// 2025-2-3 w4000 printf("w4000 title =%s\r\n",g_setup.cam [0].title);
	
	enum { OSD_BUF_SIZE = 128, };
	char osd[OSD_BUF_SIZE];
	getDateTimeToText(osd);
	// 2025-2-3 w4000 
	/*
	printf("w4000 osd =%s\r\n",osd);
	printf("w4000 osd x =%d, y=%d \r\n",t_area.x,t_area.y);
	t_area.max_width=800;
	printf("w4000 osd w =%d, h=%d \r\n",t_area.max_width,t_area.max_height);
	printf("w4000 osd usage=%d \r\n",g_setup.cam [0].title_osd.usage);*/
	get_osd_canvas_area_position();
	//t_area.x=1;t_area.y=1;
	t_area.x=t_area.x+0;

	//memcpy(osd+20,"   ",3); 
	//memcpy(osd+21,g_setup.cam [0].title,20); 
	
	osd_draw_text(t_pos, &t_area, osd, color, getTextSize());
	{
		static int clear_back=0;
		length11=strlen(g_setup.cam [0].title); 
		//printf("w4000 osd =%s\r\n",g_setup.cam [0].title);
		if(length11>0)
			{
			//printf("w4000 osd length11 =%d\r\n",length11);
			clear_back=0;
			t_area.y=t_area.y+40*1;
			osd_draw_text(t_pos, &t_area, g_setup.cam [0].title, color, getTextSize());
			}
		else
			{
				t_area.y=t_area.y+40*1;
				//osd_hide_text(t_pos, &t_area);
				if(clear_back==0)
				{
					//printf("w4000 osd_all_clear =%d\r\n",length11);
					clear_back=1;
					osd_all_clear();//w4000
				}
			}
	}
		
}

