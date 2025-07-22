#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "hs_common.h"
#include "common.h"



static const int __max_bitrate[CAM_MAX] = { BITRATE_16000, BITRATE_2000, BITRATE_1100 };
const int def_max_bitrate(int ch)
{
	return __max_bitrate[ch];
}

const int def_bitrate_index_to_value(const int index)
{
	int data;
	switch (index)
	{
		case BITRATE_200:		data = 200;		break;
		case BITRATE_300:		data = 300;		break;
		case BITRATE_400:		data = 400;		break;
		case BITRATE_500:		data = 500;		break;
		case BITRATE_600:		data = 600;		break;
		case BITRATE_700:		data = 700;		break;
		case BITRATE_750:		data = 750;		break;
		case BITRATE_900:		data = 900;		break;
		case BITRATE_1100:		data = 1100;	break;
		case BITRATE_1300:		data = 1300;	break;
		case BITRATE_1500:		data = 1500;	break;
		case BITRATE_1800:		data = 1800;	break;
		case BITRATE_2000:		data = 2000;	break;
		case BITRATE_2500:		data = 2500;	break;
		case BITRATE_3000:		data = 3000;	break;
		case BITRATE_3500:		data = 3500;	break;
		case BITRATE_4000:		data = 4000;	break;
		case BITRATE_5000:		data = 5000;	break;
		case BITRATE_6000:		data = 6000;	break;
		case BITRATE_7000:		data = 7000;	break;
		case BITRATE_8000:		data = 8000;	break;
		case BITRATE_9000:		data = 9000;	break;
		case BITRATE_10000:		data = 10000;	break;
		case BITRATE_12000:		data = 12000;	break;
		case BITRATE_14000:		data = 14000;	break;
		case BITRATE_16000:		data = 16000;	break;
		default:				data = 5000;	break;
	}

	return data;
}

const int def_bitrate_value_to_index(const int value)
{
	if(value < 300) {
		return BITRATE_200;
	}
	else if(value < 400) {
		return BITRATE_300;
	}
	else if(value < 500) {
		return BITRATE_400;
	}
	else if(value < 600) {
		return BITRATE_500;
	}
	else if(value < 700) {
		return BITRATE_600;
	}
	else if(value < 750) {
		return BITRATE_700;
	}
	else if(value < 900) {
		return BITRATE_750;
	}
	else if(value < 1100) {
		return BITRATE_900;
	}
	else if(value < 1300) {
		return BITRATE_1100;
	}
	else if(value < 1500) {
		return BITRATE_1300;
	}
	else if(value < 1800) {
		return BITRATE_1500;
	}
	else if(value < 2000) {
		return BITRATE_1800;
	}
	else if(value < 2500) {
		return BITRATE_2000;
	}
	else if(value < 3000) {
		return BITRATE_2500;
	}
	else if(value < 3500) {
		return BITRATE_3000;
	}
	else if(value < 4000) {
		return BITRATE_3500;
	}
	else if(value < 5000) {
		return BITRATE_4000;
	}
	else if(value < 6000) {
		return BITRATE_5000;
	}
	else if(value < 7000) {
		return BITRATE_6000;
	}
	else if(value < 8000) {
		return BITRATE_7000;
	}
	else if(value < 9000) {
		return BITRATE_8000;
	}
	else if(value < 10000) {
		return BITRATE_9000;
	}
	else if(value < 12000) {
		return BITRATE_10000;
	}
	else if(value < 14000) {
		return BITRATE_12000;
	}
	else if(value < 16000) {
		return BITRATE_14000;
	}

	return BITRATE_16000;
}


static const PIC_SIZE_E __max_resol[CAM_MAX] = { PIC_HD1080, PIC_HD720, PIC_QVGA };

const PIC_SIZE_E def_max_resol(int ch)
{
	return __max_resol[ch];
}


const char *def_resol_index_to_string(const PIC_SIZE_E index)
{
	static char name[32];

	switch (index)
	{
		case PIC_QCIF:
			strcpy(name, "QCIF");
			break;
		case PIC_CIF:
			strcpy(name, "CIF");
			break;
		case PIC_HD1:
			strcpy(name, "HD1");
			break;
		case PIC_D1:
			strcpy(name, "D1");
			break;
		case PIC_960H:
			strcpy(name, "960H");
			break;
		case PIC_2CIF:
			strcpy(name, "2CIF");
			break;
		case PIC_QVGA:   
			strcpy(name, "QVGA");
			break;
		case PIC_VGA:    
			strcpy(name, "VGA");
			break;
		case PIC_640x360:
			strcpy(name, "nHD");
			break;
		case PIC_800x600:
			strcpy(name, "SVGA");
			break;
		case PIC_XGA:    
			strcpy(name, "XGA");
			break;
		case PIC_SXGA:   
			strcpy(name, "SXGA");
			break;
		case PIC_UXGA:   
			strcpy(name, "UXGA");
			break;
		case PIC_QXGA:   
			strcpy(name, "QXGA");
			break;
		case PIC_WVGA:   
			strcpy(name, "WVGA");
			break;
		case PIC_WSXGA:  
			strcpy(name, "WSXGA");
			break;
		case PIC_WUXGA:  
			strcpy(name, "WUXGA");
			break;
		case PIC_WQXGA:  
			strcpy(name, "WQXGA");
			break;
		case PIC_HD720:  
			strcpy(name, "HD720");
			break;
		case PIC_HD1080: 
			strcpy(name, "HD1080");
			break;
		default:
			strcpy(name, "HD1080");
			break;
	}

    return name;
}

static void __resol_index_to_size(int ispal, const PIC_SIZE_E index, int *width, int *height)
{
	switch (index)
	{
		case PIC_QCIF:
			*width = 176;
			*height = 120;
			break;
		case PIC_CIF:
			*width = 352;
			*height = 240;
			break;
		case PIC_HD1:
			*width = 720;
			*height = 240;
			break;
		case PIC_D1:
			*width = 720;
			*height = 480;
			break;
		case PIC_960H:
			*width = 960;
			*height = 540;
			break;
		case PIC_2CIF:
			*width = 360;
			*height = 480;
			break;
		case PIC_QVGA: 
			*width = 320;
			*height = 240;
			break;
		case PIC_VGA:    
			*width = 640;
			*height = 480;
			break;
		case PIC_640x360:   
			*width = 640;
			*height = 360;
			break;
		case PIC_800x600:   
			*width = 800;
			*height = 600;
			break;
		case PIC_XGA:    
			*width = 1024;
			*height = 768;
			break;
		case PIC_SXGA:    
			*width = 1400;
			*height = 1050;
			break;
		case PIC_UXGA:  
			*width = 1600;
			*height = 1200;
			break;
		case PIC_QXGA:   
			*width = 2048;
			*height = 1536;
			break;
		case PIC_WVGA:   
			*width = 854;
			*height = 480;
			break;
		case PIC_WSXGA:  
			*width = 1680;
			*height = 1050;
			break;
		case PIC_WUXGA:  
			*width = 1920;
			*height = 1200;
			break;
		case PIC_WQXGA:  
			*width = 2560;
			*height = 1600;
			break;
		case PIC_HD720:   
			*width = 1280;
			*height = 720;
			break;
		case PIC_HD1080:  
			*width = 1920;
			*height = 1080;
			break;

		default:
			*width = 1920;
			*height = 1080;
			break;
	}

    return;
}

const int def_resol_index_to_width(int ispal, const PIC_SIZE_E index)
{
	int width, height;

	__resol_index_to_size(ispal, index, &width, &height);

	return width;
}

const int def_resol_index_to_height(int ispal, const PIC_SIZE_E index)
{
	int width, height;

	__resol_index_to_size(ispal, index, &width, &height);

	return height;
}

const int def_get_picsize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S* pstSize)
{
    switch (enPicSize)
    {
        case PIC_QCIF:
            pstSize->u32Width  = 176;
            pstSize->u32Height = 120;
            break;
        case PIC_CIF:
            pstSize->u32Width  = 352;
            pstSize->u32Height = 240;
            break;
        case PIC_D1:
            pstSize->u32Width  = 720;
            pstSize->u32Height = 480;
            break;
        case PIC_960H:
            pstSize->u32Width  = 960;
            pstSize->u32Height = 540;
            break;
        case PIC_2CIF:
            pstSize->u32Width  = 360;
            pstSize->u32Height = 480;
            break;
        case PIC_QVGA:    /* 320 * 240 */
            pstSize->u32Width  = 320;
            pstSize->u32Height = 240;
            break;
        case PIC_VGA:     /* 640 * 480 */
            pstSize->u32Width  = 640;
            pstSize->u32Height = 480;
            break;
		case PIC_640x360:     /* 640 * 360 */
            pstSize->u32Width  = 640;
            pstSize->u32Height = 360;
            break;
		case PIC_800x600:     /* 800 * 600 */
            pstSize->u32Width  = 800;
            pstSize->u32Height = 600;
            break;
        case PIC_XGA:     /* 1024 * 768 */
            pstSize->u32Width  = 1024;
            pstSize->u32Height = 768;
            break;
        case PIC_SXGA:    /* 1400 * 1050 */
            pstSize->u32Width  = 1400;
            pstSize->u32Height = 1050;
            break;
        case PIC_UXGA:    /* 1600 * 1200 */
            pstSize->u32Width  = 1600;
            pstSize->u32Height = 1200;
            break;
        case PIC_QXGA:    /* 2048 * 1536 */
            pstSize->u32Width  = 2048;
            pstSize->u32Height = 1536;
            break;
        case PIC_WVGA:    /* 854 * 480 */
            pstSize->u32Width  = 854;
            pstSize->u32Height = 480;
            break;
        case PIC_WSXGA:   /* 1680 * 1050 */
            pstSize->u32Width = 1680;
            pstSize->u32Height = 1050;
            break;
        case PIC_WUXGA:   /* 1920 * 1200 */
            pstSize->u32Width  = 1920;
            pstSize->u32Height = 1200;
            break;
        case PIC_WQXGA:   /* 2560 * 1600 */
            pstSize->u32Width  = 2560;
            pstSize->u32Height = 1600;
            break;
        case PIC_HD720:   /* 1280 * 720 */
            pstSize->u32Width  = 1280;
            pstSize->u32Height = 720;
            break;
        case PIC_HD1080:  /* 1920 * 1080 */
            pstSize->u32Width  = 1920;
            pstSize->u32Height = 1080;
            break;
        default:
            return 1;
    }
    return 0;
}


const int def_resol_size_to_index(int ispal, const char *size)
{
	PIC_SIZE_E i;
	for(i = 0; i < PIC_BUTT; i++) {
		SIZE_S stSize;
		if(def_get_picsize((VIDEO_NORM_E)ispal, (PIC_SIZE_E)i, &stSize) != 0) {
			continue;
		}
		char sz[32];
		sprintf(sz, "%dx%d", stSize.u32Width, stSize.u32Height);

		if(strcasecmp(size, sz) == 0) {
			return (int)i;
		}
	}

	return -1;
}

static const PIC_SIZE_E __support_resol[][PIC_BUTT] = {
	{
		PIC_640x360,
		PIC_VGA, 	 /* 640 * 480 */
		PIC_WVGA, 	 /* 854 * 480 */
		PIC_800x600, /* 800 * 600 */
		PIC_960H,    /* 960 * 540 */
		PIC_HD720,   /* 1280 * 720 */
		PIC_HD1080,  /* 1920 * 1080 */
		PIC_BUTT,
	},
	{
		PIC_QVGA,    /* 320 * 240 */
		PIC_640x360,
		PIC_VGA,     /* 640 * 480 */
		PIC_WVGA, 	 /* 854 * 480 */
		PIC_800x600, /* 800 * 600 */
		PIC_960H,    /* 960 * 540 */
		PIC_HD720,   /* 1280 * 720 */
		PIC_BUTT,
	},
	{
		PIC_QVGA,    /* 320 * 240 */
		PIC_BUTT,
	}
};


const PIC_SIZE_E *def_support_resol_array(const int ch)
{
	return __support_resol[ch];
}

static PIC_SIZE_E __all_resol[PIC_BUTT*2] = { PIC_BUTT, };
PIC_SIZE_E *def_all_resol_array()
{
	int i = 0, j, k;
	int resol_value;
	int idx = 0;
	int isFound = 0;
	for(j = CAM_MAX - 1;j >= 0;j--) {
		const PIC_SIZE_E *resols = __support_resol[j];
		for(i = 0;i < PIC_BUTT;i++) {
			resol_value = resols[i];
			if(resol_value >= PIC_BUTT) {
				break;
			}
			isFound = 0;
			for(k = 0;k < idx;k++) {
				if(resol_value == __all_resol[k]) {
					isFound = 1;
					break;
				}
			}
			if(isFound == 1) {
				continue;
			}
			__all_resol[idx] = resol_value;
			idx++;
		}
	}
	__all_resol[idx] = PIC_BUTT;
	return __all_resol;
}


// return: ex) HD1080
static const char *__resol_string = "HD1080";
const char *def_resol_string()
{
	return __resol_string;
}

