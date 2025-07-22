
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

// #include <dirent.h>

static char page[MAX_PAGE_SZ];

static _CAMSETUP r_setup;
static _CAMSETUP_EXT r_setup_ext;

#define MAX_FILES 20
#define MAX_FILE_SIZE 1024

// cmd 
// ln -s /root/* /home/www/
// #define VIDEO_PATH "/home/www"
// ln -s /home/www/sda1/mp4/* /home/www
#define VIDEO_PATH "/home/www/sda1/mp4"

// #define VIDEO_PATH "/mnt/ipm"
// mount --bind /mnt/sda1/mp4/ /var/www/^

// ln -s /mnt/sda1/mp4/* /var/www
int main()
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



	// system("rm /home/www/*.mp4");
	// system("ln -s /home/www/sda1/mp4/* /home/www");

	DIR *dir;
	struct dirent *ent;
	// FILE *fp;
	// char file_path[256];
	char full_path[256];
	// char file_contents[MAX_FILES][MAX_FILE_SIZE];
	int i = 0;

	char str[1024], buf[1024 * 1024];

	load_form(JQUERY_HTML_VIDEO_PLAYER, page, lang);

	// if ((dir = opendir("/mnt/ipm/play"))) {
	//     while (i < MAX_FILES) {
	//         if (strstr(ent->d_name, ".mp4")) {
	//             snprintf(file_path, sizeof(file_path), "/mnt/ipm/play/%s", ent->d_name);
	//             fp = fopen(file_path, "r");
	//             fgets(file_contents[i], MAX_FILE_SIZE, fp);
	//             fclose(fp);
	//             i++;
	//         }
	//     }
	//     closedir(dir);
	// }

#if 0
    if ((dir = opendir("/mnt/ipm/play"))) {
#else
	if ((dir = opendir(VIDEO_PATH)))
	{
#endif

		// while ((ent=readdir(dir))) {
		for (i = 0; (ent = readdir(dir)); i<100)
		{
			if (strstr(ent->d_name, "_") && strstr(ent->d_name, ".mp4"))
			{
				sprintf(full_path, "/sda1/mp4/%s", ent->d_name);
				sprintf(str, "<tr><th class=\"playlist\" data-src=\"%s\" scope=\"row\">%03d/%s</th></tr>\n", full_path, i+1, ent->d_name);

				// sprintf(str, "<tr><th class=\"playlist\" data-src=\"%s\" scope=\"row\"></th></tr>\n", ent->d_name);
				// sprintf(str, "<tr><th class=\"playlist\" data-src=\"%s\" scope=\"row\"></th></tr>\n", ent->d_name);
				// sprintf(str, file_path);
				strcat(buf, str);
				i++;


				if (strlen(buf) > 1024 * 63)
					break;
			}
		}

		// strcat(buf, str);

		closedir(dir);
	}

	replace_form_once(page, "<!--##VIDEO_LIST##-->", buf);

retry:


	if (ajax_check_output(page, lang) < 0)
	{
		if (lang != 0)
		{
			lang = 0;
			goto retry;
		}
	}

	cgi_end();

	return 1;
}