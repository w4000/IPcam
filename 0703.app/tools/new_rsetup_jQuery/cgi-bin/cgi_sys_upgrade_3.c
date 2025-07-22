
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"

int main(void)
{
	const char* PROGRESS_FILE = "/tmp/progress";


	cgi_init();
	cgi_session_start();

	cgi_init_headers();

	if (!check_session()) {
		goto out;
	}

	char str[4];
	int progress = 0;
	char cmd[128];


	int fd = open(PROGRESS_FILE, O_RDONLY);
	if (read(fd, str, 3) <= 0) {
		printf ("waiting");
		// printf ("err");
		goto out;
		
	}

	progress = atoi(str);
	close(fd);


	sprintf (cmd, "echo \"func:%s, line: %d > progress? %d \" | cat >> /root/uplog.txt", __func__, __LINE__, progress);
	system(cmd);


	if (progress == 25 || progress == 26 || progress == 27)
	{
		printf ("err");
	}
	else
	{

		printf ("%d", progress);
	} 

out:
	cgi_end();

	return 0;
}

