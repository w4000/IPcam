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
#include "vfb.h"
#include "hs_common.h"

#if defined(NARCH)
#include "nv_video_capture.h"
#include "nv_define.h"
#else
#endif	/* NARCH */

enum {
	SHADOW_WHITE,
	SHADOW_BLACK,
	SHADOW_BLUE,
	SHADOW_GREEN,
	SHADOW_RED,
	COLOR_MAX,
};

static unsigned int g_max_width = 1920;
static unsigned int g_max_height = 1080;

static VFB_CANVAS_INFO gVfbInfo[OSD_BUF_COUNT];
unsigned short* imgBuf[OSD_BUF_COUNT];

static void area2aligned(VFB_AREA_S *area)
{
	area->x = (area->x >> 1) << 1;
	area->y = (area->y >> 1) << 1;
	area->width = (area->width >> 1) << 1;
	area->height = (area->height >> 1) << 1;
}

int vfb_add_bitmaps_with_outline(int id, VFB_BITMAP_S bitmap[], int array_size, PIXEL_DATA_FORMAT color)
{
	PIXEL_DATA_FORMAT *canvas;
	PIXEL_DATA_FORMAT shadowColor = S_BLACK;

	const char* show_osd_file = "/mnt/ipm/show_osd_area";
	int i;
	int x;
	int y;
	int width;
	int height;
	int bx = 0;
	int by = 0;
	int osd_area_mode = 0;

	if(access(show_osd_file, F_OK) == 0) {
		osd_area_mode = 1;
	}

#if defined(NARCH)
	if(color == S_BLACK || color == S_BLUE || color == S_RED) {
#else
	if(color == S_BLACK || color == S_BLUE) {
#endif	/* NARCH */
		shadowColor = S_WHITE;
	}

	// canvas information
#if defined(NARCH)
	canvas = imgBuf[id];
#else
#endif	/* NARCH */

#if defined(NARCH)
	for(i = 0; i < array_size; i++) {
		gVfbInfo[id].area.x = bitmap[i].area.x;
		gVfbInfo[id].area.y = bitmap[i].area.y;

		width = bitmap[i].area.width;
		height = bitmap[i].area.height;
		bx = bitmap[i].area.x;
		by = bitmap[i].area.y;

		area2aligned(&bitmap[i].area);
		for(y = 0; y < height; y++) {
			for(x = 0; x < width; x++) {
				if(bx + x > gVfbInfo[id].area.width || by + y > gVfbInfo[id].area.height) {
					continue;
				}
				int canvas_pos = (gVfbInfo[id].area.width * (y + by + 2)) + (bx + x);
				if(bitmap[i].data[y * width + x] == 0 && (y > 2 || y < height - by - 2)) {
					if((bitmap[i].data[(y - 1) * width + x] == color) || (bitmap[i].data[(y - 2) * width + x] == color)) {
						canvas[canvas_pos] = shadowColor;
						continue;
					}
				}
				canvas[canvas_pos] = bitmap[i].data[y * width + x];
				if (osd_area_mode == 1 && bitmap[i].data[y * bitmap[i].area.width + x] != color) {
					canvas[canvas_pos] = S_GRAY;
				}
			}
		}
	}

	return 0;
#else
#endif	/* NARCH */
}

int vfb_clear_area_for_novatek(int index, VFB_AREA_S *area)
{
#if defined(NARCH)
	int x, y;
	int w;
	int h;
	int ax;
	int ay;
	int aw;
	int ah;
	PIXEL_DATA_FORMAT *canvas;
	canvas = (PIXEL_DATA_FORMAT *)imgBuf[index];

	w = gVfbInfo[index].area.width;
	h = gVfbInfo[index].area.height;
	ax = area->x;
	ay = area->y;
	aw = area->width;
	ah = area->height;

	int canvas_pos = 0;
	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) {
			if((y >= ay && y <= ay + ah) && (x >= ax && x <= ax + aw)) {
				canvas_pos = w * y + x;
				canvas[canvas_pos] = 0x0000;
			}
		}
	}
#endif	/* NARCH */
	return HI_SUCCESS;
}


int vfb_clear_area(int index, VFB_AREA_S *area)
{
#if defined(NARCH)
	vfb_clear_area_for_novatek(index, area);
#endif	/* NARCH */
}

void vfb_fini()
{
#if !defined(NARCH)
	RGN_DestroyRegion(&g_stSrcChn);
#endif	/* NARCH */
	return;
}

int vfb_size(unsigned int *width, unsigned int *height)
{
	*width = g_max_width;
	*height = g_max_height;

	return 0;
}

#if !defined(NARCH)
#if defined(SUPPORT_FISHEYE)
static int RGN_CreateOverlayEx(MPP_CHN_S *pstChn, int width, int height)
{
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CHN_ATTR_S stChnAttr = {0};

	if (pstChn == HI_NULL)
    {
        printf("Input pstChn cannot be NULL! \n");
        return HI_FAILURE;
    }

    /*attach the OSD to the vpss*/
    stChn.enModId  = pstChn->enModId;
    stChn.s32DevId = pstChn->s32DevId;`
    stChn.s32ChnId = pstChn->s32ChnId;

	stRgnAttrSet.enType = OVERLAY_RGN;
	stRgnAttrSet.unAttr.stOverlay.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
	stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = width;
	stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = height;
	stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0x0;
	stRgnAttrSet.unAttr.stOverlay.u32CanvasNum     = 2;

	s32Ret = HI_MPI_RGN_Create(OSD_HANDLE, &stRgnAttrSet);
	if (s32Ret != HI_SUCCESS)
	{
		return s32Ret;
	}

	stChnAttr.bShow  = HI_FALSE;
	stChnAttr.enType = OVERLAY_RGN;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
	stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 0;
	stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 128;
	stChnAttr.unChnAttr.stOverlayChn.u32Layer     = OSD_HANDLE;
	stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = HI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp = 30;
	stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bQpDisable = HI_TRUE;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = HI_FALSE;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = 0;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = 16;
	stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = 16;

	s32Ret = HI_MPI_RGN_AttachToChn(OSD_HANDLE, &stChn, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		return s32Ret;
	}

    return HI_SUCCESS;

}
#else	/* SUPPORT_FISHEYE */
static int RGN_CreateOverlayEx(MPP_CHN_S *pstChn, int width, int height)
{
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CHN_ATTR_S stChnAttr = {0};

	if (pstChn == HI_NULL)
    {
        return HI_FAILURE;
    }

    /*attach the OSD to the vpss*/
    stChn.enModId  = pstChn->enModId;
    stChn.s32DevId = pstChn->s32DevId;
    stChn.s32ChnId = pstChn->s32ChnId;

	stRgnAttrSet.enType = OVERLAYEX_RGN;
	stRgnAttrSet.unAttr.stOverlayEx.enPixelFmt       = PIXEL_FORMAT_RGB_1555;

	stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Width  = width;
	stRgnAttrSet.unAttr.stOverlayEx.stSize.u32Height = height;
	stRgnAttrSet.unAttr.stOverlayEx.u32BgColor       = 0x0;


	s32Ret = HI_MPI_RGN_Create(OSD_HANDLE, &stRgnAttrSet);
	if (s32Ret != HI_SUCCESS)
	{
		return s32Ret;
	}

	stChnAttr.bShow  = HI_TRUE;
	stChnAttr.enType = OVERLAYEX_RGN;
	stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = 0;
	stChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = 0;
	stChnAttr.unChnAttr.stOverlayExChn.u32BgAlpha   = 0;
	stChnAttr.unChnAttr.stOverlayExChn.u32FgAlpha   = 255;
	stChnAttr.unChnAttr.stOverlayExChn.u32Layer     = OSD_HANDLE;

	s32Ret = HI_MPI_RGN_AttachToChn(OSD_HANDLE, &stChn, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		return s32Ret;
	}

    return HI_SUCCESS;

}
#endif	/* SUPPORT_FISHEYE */

static int RGN_DestroyRegion(MPP_CHN_S *pstChn)
{
    HI_S32 s32Ret;

	s32Ret = HI_MPI_RGN_DetachFromChn(OSD_HANDLE, pstChn);
	if (HI_SUCCESS != s32Ret)
	{
		return s32Ret;
	}

	s32Ret = HI_MPI_RGN_Destroy(OSD_HANDLE);
	if (HI_SUCCESS != s32Ret)
	{
		return s32Ret;
	}

    return HI_SUCCESS;

}

static int RGN_OverlayExShow(MPP_CHN_S *pstChn, int isshow)
{
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_CHN_ATTR_S stChnAttr = {0};

	if (pstChn == HI_NULL)
    {
        return HI_FAILURE;
    }

    /*attach the OSD to the vpss*/
    stChn.enModId  = pstChn->enModId;
    stChn.s32DevId = pstChn->s32DevId;
    stChn.s32ChnId = pstChn->s32ChnId;

	s32Ret = HI_MPI_RGN_GetDisplayAttr(OSD_HANDLE, &stChn, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		return s32Ret;
	}

	stChnAttr.bShow  = (isshow)?HI_TRUE:HI_FALSE;

	s32Ret = HI_MPI_RGN_SetDisplayAttr(OSD_HANDLE, &stChn, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		return s32Ret;
	}

    return HI_SUCCESS;
}

int vfb_init(__u16 width, __u16 height)
{
	g_max_width = width;
	g_max_height = height;

	g_stSrcChn.enModId = OSD_MOD;
	g_stSrcChn.s32DevId = 0;
	g_stSrcChn.s32ChnId = 0;
	if(RGN_CreateOverlayEx(&g_stSrcChn, width, height) != HI_SUCCESS) {
		return -1;
	}

	if(RGN_OverlayExShow(&g_stSrcChn, 1) != HI_SUCCESS) {
		return -1;
	}

	return 0;
}

int vfb_add_bitmaps(VFB_BITMAP_S bitmap[], int array_size)
{
    HI_S32 s32Ret;
	RGN_CANVAS_INFO_S stCanvasInfo;
	PIXEL_DATA_FORMAT *canvas;
	int i, x, y;

	s32Ret = HI_MPI_RGN_GetCanvasInfo(OSD_HANDLE, &stCanvasInfo);
	if (HI_SUCCESS != s32Ret)
	{
		return s32Ret;
	}

	canvas = (PIXEL_DATA_FORMAT *)stCanvasInfo.u32VirtAddr;
	int strideUnitSize = sizeof(PIXEL_DATA_FORMAT);

	for(i = 0; i < array_size; i++) {
		area2aligned(&bitmap[i].area);
		for(y = 0; y < bitmap[i].area.height; y++) {
			for(x = 0; x < bitmap[i].area.width; x++) {
				if(bitmap[i].area.x + x > stCanvasInfo.stSize.u32Width ||
						bitmap[i].area.y + y > stCanvasInfo.stSize.u32Height) {
					continue;
				}
				int canvas_pos = (stCanvasInfo.u32Stride/strideUnitSize)*(y+bitmap[i].area.y) + (bitmap[i].area.x+x);
				canvas[canvas_pos] = bitmap[i].data[y*bitmap[i].area.width + x];
			}
		}
	}

	s32Ret = HI_MPI_RGN_UpdateCanvas(OSD_HANDLE);
	if (HI_SUCCESS != s32Ret)
	{
		return s32Ret;
	}

	return HI_SUCCESS;
}
#else	/* NARCH */
int getVfbWidth(int index)
{
	return gVfbInfo[index].area.width;
}

int getVfbHeight(int index)
{
	return gVfbInfo[index].area.height;
}

int loadImagefromFile(int index, const char* file_path_name, VFB_AREA_S* area)
{
	if(access(file_path_name, F_OK) != 0) {
		return -1;
	}

	if(imgBuf == NULL) {
		return -2;
	}

	int fd;
	int ret;
	int x = area->x;
	int y = area->y;

	fd = open(file_path_name, O_RDONLY);
	if(fd == -1){
		return -1;
	}

	int line = 0;
	unsigned int readSize = 0;
	unsigned short *logoTmp;
	for(line = y;line < area->height + y;line++) {
		logoTmp = *(imgBuf) + line * 1920 + (x * sizeof(PIXEL_DATA_FORMAT));
		ret = read(fd, logoTmp, (area->width * sizeof(unsigned short)));
		readSize += ret;
		int i;
		int count = 0;
		for(i = 0;i < area->width;i++) {
			if(*(logoTmp + i) != 0) {
				count++;
				if(count > 3) {
					break;
				}
			}
		}
	}
	close(fd);

	if(readSize != (area->width * area->height * sizeof(unsigned short))) {
		return -1;
	}
	return 0;
}

int init_vfb(int index, VFB_CANVAS_INFO* vfbInfo)
{
	if(vfbInfo == NULL) {
		return -1;
	}

	gVfbInfo[index].area.width = vfbInfo->area.width;
	gVfbInfo[index].area.height = vfbInfo->area.height;
	gVfbInfo[index].stamp.stamp_pa = vfbInfo->stamp.stamp_pa;
	gVfbInfo[index].stamp.stamp_size = vfbInfo->stamp.stamp_size;
	gVfbInfo[index].stamp.vp_stamp_path = vfbInfo->stamp.vp_stamp_path;

	return 0;
}

int deinit_vfb(int index)
{
	memset(&gVfbInfo[index].area, 0, sizeof(VFB_AREA_S));
	memset(&gVfbInfo[index].stamp, 0, sizeof(STAMP_DATA));
}

/*
 * @position : osd buffer position (1 ~ 4)
 */
int update_osd_text(int index, const int* position)
{
	int ret;
	HD_DIM dim;
	HD_IPOINT point;

	dim.w = gVfbInfo[index].area.width;
	dim.h = gVfbInfo[index].area.height;

	ret = set_proc_stamp_param(gVfbInfo[index].stamp.vp_stamp_path, gVfbInfo[index].stamp.stamp_pa, gVfbInfo[index].stamp.stamp_size, imgBuf[index], &dim, position);

	if(ret != HD_OK) {
		printf("Failed to call set_cap_stamp_param()!!! (ret=%d;index = %d)\n", ret, index);
		return ret;
	}
	return 0;
}

void setImageBuffer(int index, unsigned short* buf)
{
	if(index < 0 || index >= OSD_BUF_COUNT) {
		return;
	}
	if(buf == NULL) {
		return;
	}
	imgBuf[index] = buf;
}

#endif	/* NARCH */
