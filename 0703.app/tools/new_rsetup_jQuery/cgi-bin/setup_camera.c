
#include "common2.h"
#include "jQuery_common.h"

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;


int main(int argc, char* argv[])
{
	int i;
	int data;

	int max_width = 1920;
	int max_height = 1080;


	cgi_init();
	cgi_process_form();
	cgi_init_headers();

	char *pdata;

	load_setup (&r_setup, &r_setup_ext);


	pdata = (char*)cgi_param("category");
	if (pdata) {
		else if (strcmp(pdata, "video_setup") == 0) {
		}
		else if (strcmp(pdata, "mdin_setup_quality") == 0) {
			pdata = (char*)cgi_param("brightness");
			{
				data = (pdata) ? atoi(pdata) : 127;
				r_setup_ext.mdin_vo_setup.brightness = data;
			}
			pdata = (char*)cgi_param("contrast");
			{
				data = (pdata) ? atoi(pdata) : 127;
				r_setup_ext.mdin_vo_setup.contrast = data;
			}
			pdata = (char*)cgi_param("saturation");
			{
				data = (pdata) ? atoi(pdata) : 127;
				r_setup_ext.mdin_vo_setup.saturation = data;
			}
		}
		else if (strcmp(pdata, "whitebalance") == 0) {
			pdata = (char*)cgi_param("wb_mode");
			data = (pdata) ? atoi(pdata) : 0;
			r_setup_ext.isp.wb.mode = data;

			pdata = (char*)cgi_param("r_offset");
			data = (pdata) ? atoi(pdata) : 127;
			r_setup_ext.isp.wb.RGStrength = data;

			pdata = (char*)cgi_param("b_offset");
			data = (pdata) ? atoi(pdata) : 127;
			r_setup_ext.isp.wb.BGStrength = data;

		}
		else if (strcmp(pdata, "backlight") == 0) {
			pdata = (char*)cgi_param("bl_mode");
			data = (pdata) ? atoi(pdata) : 0;
			r_setup_ext.isp.bl.mode = data;

			pdata = (char*)cgi_param("wdr_level");
			data = (pdata) ? atoi(pdata) : 0;
			r_setup_ext.isp.bl.wdr_level = data;

		}
		else if (strcmp(pdata, "exposure") == 0) {

			pdata = (char*)cgi_param("shutter");
			data = (pdata) ? atoi(pdata) : 0;
			r_setup_ext.isp.exp.shutter = data;

			if(r_setup_ext.isp.exp.lens_mode == IRIS_AUTO) {
				r_setup_ext.isp.exp.shutter_speed[0] = SS_30;
				r_setup_ext.isp.exp.shutter_speed[1] = SS_30;
			}

			pdata = (char*)cgi_param("brightness");
			data = (pdata) ? atoi(pdata) : 127;
			r_setup_ext.isp.exp.brightness = (data > 0) ? data : 127;	

		}
		else if (strcmp(pdata, "daynight") == 0) {
			pdata = (char*)cgi_param("mode");
			data = (pdata) ? atoi(pdata) : 0;
			r_setup_ext.isp.tdn.mode = data;


		}
	}

	cgi_end();


	return 0;
}
