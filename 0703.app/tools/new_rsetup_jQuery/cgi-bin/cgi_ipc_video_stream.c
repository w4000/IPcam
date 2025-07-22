
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

static char page[MAX_PAGE_SZ];

static int _Hi_COMM_SYS_GetPicSize(unsigned enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstSize, char *name)
{
	pstSize->u32Width = def_resol_index_to_width(enNorm, enPicSize);
	pstSize->u32Height = def_resol_index_to_height(enNorm, enPicSize);
	sprintf(name, "%s", def_resol_index_to_string(enPicSize));

	return 0;
}

static char *qty_val_C[BITRATE_MAX] = {
	DFS_BITRATE_200, DFS_BITRATE_300, DFS_BITRATE_400, DFS_BITRATE_500,
	DFS_BITRATE_600, DFS_BITRATE_700, DFS_BITRATE_750, DFS_BITRATE_900, DFS_BITRATE_1100,
	DFS_BITRATE_1300, DFS_BITRATE_1500, DFS_BITRATE_1800,
	DFS_BITRATE_2000, DFS_BITRATE_2500, DFS_BITRATE_3000, DFS_BITRATE_3500, DFS_BITRATE_4000, DFS_BITRATE_5000,
	DFS_BITRATE_6000, DFS_BITRATE_7000, DFS_BITRATE_8000,
	DFS_BITRATE_9000,
	DFS_BITRATE_10000,
	DFS_BITRATE_12000,
	DFS_BITRATE_14000,
	DFS_BITRATE_16000};


static void replace_param(char *page, int lang)
{
#if (CAM_MAX > 2)
#endif

	char str[1024], buf[1024 * 64];
	char *selptr;
	int i, j, r;

	buf[0] = '\0';
	for (j = 0; j < 7; j++)
	{
		strcat(buf, "<tr>\n");

		switch (j)
		{
		case 0:
			strcat(buf, "<td>CODEC</td>\n");
			break;
		case 1:
			strcat(buf, "<td>MODE</td>\n");
			break;
		case 2:
			strcat(buf, "<td>RESOLUTION</td>\n");
			break;
		case 3:
			strcat(buf, "<td>FPS</td>\n");
			break;
		case 5:
			strcat(buf, "<td>MAX BITRATE</td>\n");
			break;
		case 6:
			strcat(buf, "<td>GOP</td>\n");
			break;
		}

		char *disabled = "";

		for (i = 0; i < CAM_MAX; i++)
		{
			switch (j)
			{
			case 0:

				sprintf(str, "<td><select %s id=\"v_codec_%d\" class=\"select_flat\">\n", disabled, i);
				strcat(buf, str);

				selptr = (r_setup.cam[i].codec == 0) ? "selected" : "";
				sprintf(str, "<option %s value=0>%s</option>\n", selptr, "H.264");
				strcat(buf, str);

#ifdef SUPPORT_H265
				selptr = (r_setup.cam[i].codec == 1) ? "selected" : "";
				sprintf(str, "<option %s value=1>%s</option>\n", selptr, "H.265");
				strcat(buf, str);
#endif

#ifdef SUPPORT_MJPEG

				if (i == 2)
				{
					{
						selptr = (r_setup.cam[i].codec == 2) ? "selected" : "";
						sprintf(str, "<option %s value=2>%s</option>\n", selptr, "MJPEG");
						strcat(buf, str);
					}
				}
#endif
				strcat(buf, "</select></td>\n");
				break;

			case 1:
				sprintf(str, "<td><select id=\"v_ratectl_%d\" class=\"select_flat\" onchange=\"onchange_ratectl(%d)\">\n", i, i);
				strcat(buf, str);

				selptr = (r_setup.cam [i].ratectrl == 0) ? "selected" : "";
				sprintf(str, "<option %s value=0>%s</option>\n", selptr, "VBR");
				strcat(buf, str);
				selptr = (r_setup.cam [i].ratectrl == 1) ? "selected" : "";
				sprintf(str, "<option %s value=1>%s</option>\n", selptr, "CBR");
				strcat(buf, str);
				strcat(buf, "</select></td>\n");
				break;

			case 2:
				sprintf(str, "<td><select id=\"v_resol_%d\" class=\"select_flat\" >\n", i);
				strcat(buf, str);

				{
					int j = 0;
					int resol;
					const PIC_SIZE_E *resols = def_support_resol_array(i);
					char *res_disabled = "";
					do
					{
						SIZE_S size;
						char name[32];
						resol = resols[j];

						if (resol >= PIC_BUTT)
						{
							break;
						}

						int ret = _Hi_COMM_SYS_GetPicSize(r_setup.videotype, resol, &size, name);
						if (ret >= 0)
						{
							selptr = (r_setup.cam[i].res == resol) ? "selected" : "";
							sprintf(str, "<option %s %s value=%d>%dx%d (%s)</option>\n", res_disabled,
									selptr, resol, size.u32Width, size.u32Height, name);
							strcat(buf, str);
						}
						j++;
					} while (j < PIC_BUTT);
				}

				strcat(buf, "</select></td>\n");
				break;

			case 3:
				sprintf(str, "<td><select id=\"v_fps_%d\" class=\"select_flat\"  onchange=\"onchange_fps(%d)\">\n", i, i);
				strcat(buf, str);

				int max_fps = 30;

				if (max_fps < r_setup.cam[i].fps)
					r_setup.cam[i].fps = max_fps;

				for (r = 0; r < max_fps; r++)
				{
					selptr = (r_setup.cam[i].fps == (r + 1)) ? "selected" : "";
					sprintf(str, "<option %s value=%d>%d</option>\n", selptr, r + 1, r + 1);
					strcat(buf, str);
				}
				strcat(buf, "</select></td>\n");
				break;


			case 5:
				sprintf(str, "<td><select id=\"v_max_bitrate_%d\" class=\"select_flat\" >\n", i);
				strcat(buf, str);

				if (def_max_bitrate(0) < r_setup.cam [0].ratectrl)
					r_setup.cam [0].ratectrl = def_max_bitrate(0);
				if (def_max_bitrate(1) < r_setup.cam [1].ratectrl)
					r_setup.cam [1].ratectrl = def_max_bitrate(1);
#if (CAM_MAX > 2)
				if (def_max_bitrate(2) < r_setup.cam [2].ratectrl)
					r_setup.cam [2].ratectrl = def_max_bitrate(2);
#endif

				for (r = 0; r < BITRATE_MAX; r++)
				{
					if (r > def_max_bitrate(i))
						break;

					selptr = (r_setup.cam [i].bitrate == r) ? "selected" : "";
					sprintf(str, "<option %s value=%d>%s</option>\n", selptr, r, qty_val_C[r]);
					strcat(buf, str);
				}
				strcat(buf, "</select></td>\n");
				break;

			case 6:
				sprintf(str, "<td><select id=\"v_gop_%d\" class=\"select_flat\" >\n", i);
				strcat(buf, str);

				for (r = 1; r <= 60; r++)
				{
					selptr = (r_setup.cam [i].gop == (r)) ? "selected" : "";
					sprintf(str, "<option %s value=%d>%d</option>\n", selptr, r, r);
					strcat(buf, str);
				}
				strcat(buf, "</select></td>\n");

				break;
			}
		}


		strcat(buf, "</tr>\n");
	}
	replace_form_once(page, "<!--##PARAM_VIDEO_LIST##-->", buf);





	sprintf(buf, "%d", r_setup.cam [0].title_osd.usage);
	replace_form_once(page, "##PARAM_USE_DISP_OSD##", buf);


	buf[0] = '\0';

	{
		i = 0;

		 char *title = r_setup.cam [i].title;	
		sprintf(buf, "%s", title);
		if (strlen(buf) > 0)
			replace_form_once(page, "##PARAM_CAM_TITEL##", buf);
		//w4000 replace_form_once(page, "##PARAM_CAM_TITEL##", buf);
		else 
			replace_form_once(page, "##PARAM_CAM_TITEL##", "");
		//replace_form_once(page, "##PARAM_CAM_TITEL##", "Video");

		buf[0] = '\0';

		int osd_pos = (r_setup.cam [0].title_osd.position == 1)? 1:0;
		sprintf(buf, "%d", osd_pos);
		replace_form_once(page, "##PARAM_OSD_POS##", buf);
		buf[0] = '\0';

		int osd_color = r_setup.cam [0].title_osd.color;
		if (osd_color == 1)
		{
			replace_form_once(page, "##PARAM_OSD_COLOR##", "1");
		}
		else if (osd_color == 2)
		{
			replace_form_once(page, "##PARAM_OSD_COLOR##", "2");
		}
		else if (osd_color == 3)
		{
			replace_form_once(page, "##PARAM_OSD_COLOR##", "3");
		}
		else if (osd_color == 4)
		{
			replace_form_once(page, "##PARAM_OSD_COLOR##", "4");
		}
		else 
		{
			replace_form_once(page, "##PARAM_OSD_COLOR##", "0");
		}
		

	}

	buf[0] = '\0';
}

static void replace_resource_string(char *page, int lang)
{
	char *szRES_STREAM_CH[] = RES_ID_ARRAY(RES_STREAM_CH);
	char *szRES_RATECTL[] = RES_ID_ARRAY(RES_MODE);
	char *szRES_FPS[] = RES_ID_ARRAY(RES_FPS);
	char *szRES_QUALITY[] = RES_ID_ARRAY(RES_QUALITY);
	char *szRES_MAX_BITRATE[] = RES_ID_ARRAY(RES_MAX_BITRATE);
	char *szRES_GOP[] = RES_ID_ARRAY(RES_GOP);
	char *szRES_VIDEO_CONFIG[] = RES_ID_ARRAY(RES_VIDEO_CONFIG);

	replace_form_once(page, "##RES_VIDEO_CONFIG##", szRES_VIDEO_CONFIG[lang]);
	replace_form_once(page, "##RES_RATECTL##", szRES_RATECTL[lang]);
	replace_form(page, "##RES_QUALITY##", szRES_QUALITY[lang]);
	replace_form(page, "##RES_MAX_BITRATE##", szRES_MAX_BITRATE[lang]);
	replace_form(page, "##RES_STREAM_CH##", szRES_STREAM_CH[lang]);
	replace_form(page, "##RES_FPS##", szRES_FPS[lang]);
	replace_form(page, "##RES_GOP##", szRES_GOP[lang]);



	char *szRES_TITLE[] = RES_ID_ARRAY(RES_TITLE);
	replace_form(page, "##RES_TITLE##", szRES_TITLE[lang]);

	char *szRES_SAVE[] = RES_ID_ARRAY(RES_SAVE);
	replace_form(page, "##RES_SAVE##", szRES_SAVE[lang]);

	char *szRES_UPDATE[] = RES_ID_ARRAY(RES_UPDATE);
	replace_form(page, "##RES_UPDATE##", szRES_UPDATE[lang]);

	char *szRES_SEC[] = RES_ID_ARRAY(RES_SEC);
	replace_form(page, "##RES_SEC##", szRES_SEC[lang]);

	char *szRES_UPDATE_MSG[] = RES_ID_ARRAY(RES_UPDATE_MSG);
	replace_form(page, "##RES_UPDATE_MSG##", szRES_UPDATE_MSG[lang]);

	char *szRES_MSG_STREAM_OVER_ERR[] = RES_ID_ARRAY(RES_MSG_STREAM_OVER_ERR);
	replace_form(page, "##RES_MSG_STREAM_OVER_ERR##", szRES_MSG_STREAM_OVER_ERR[lang]);
}

int main(void)
{
	cgi_init();
	cgi_session_start();

	cgi_process_form();
	cgi_init_headers();

	if (!check_session())
	{
		cgi_end();
		return 0;
	}

	load_setup(&r_setup, &r_setup_ext);
	int lang = r_setup.lang;

retry:
	load_form(JQUERY_HTML_SETUP_AJAX_VIDEO_STREAM, page, lang);

	replace_param(page, lang); //
	replace_resource_string(page, lang);

	if (ajax_check_output(page, lang) < 0)
	{
		if (lang != 0)
		{
			lang = 0;
			goto retry;
		}
	}

	cgi_end();

	return 0;
}
