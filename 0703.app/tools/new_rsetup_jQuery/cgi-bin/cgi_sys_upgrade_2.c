
#include "common2.h"
#include "jQuery_common.h"
#include "ajax_common.h"


#include "lib/upgr.h"



// #define BUF_SIZE	(1024)
// #define BUF_SIZE	(2048)
#define BUF_SIZE	(3072)
#define PROGRESS_FILE	"/tmp/progress"



int dbg(char *msg) {
    // 파일 열기
    FILE *file = fopen("/root/upload.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // 파일에 문자 쓰기
    // fputc('A', file);  // 파일에 'A' 쓰기
    // fputc('\n', file); // 파일에 줄바꿈 문자 쓰기

    // 파일에 문자열 쓰기
    fputs(msg, file);

    // 형식을 지정하여 문자열 쓰기

    // 파일 닫기
    fclose(file);

    return 0;
}



static int find_value_from_header(const char* header, const char* name, char* value)
{
	int i, j, l = strlen(header);
	for (i = 0; i < l; i++)
	{
		if (memcmp(header + i, name, 5) == 0)
		{
			strcpy(value, header + i + strlen(name) + 1);

			for (j = 0; j < l - i; j++)
			{
				if (value[j] == '"')
				{
					value[j] = 0;
					break;
				}
			}

			return 1;
		}
	}

	return 0;
}


static void __write_upgr_progress(int value)
{
	int fd;
	char str[4];


	unlink(PROGRESS_FILE);
	fd = open(PROGRESS_FILE, O_RDWR |O_CREAT, 0644);
	if(fd == -1)
	{
		// perror ("open");
	}

	sprintf(str, "%03d", value);
	write(fd, str, sizeof(str));

	// printf("Progress writing for upgr = %d : %s \n", value, str);
	close (fd);
}


static int my_process_form_data(FILE* fp, int _contents_length, int* file_length)
{
	char buf[BUF_SIZE * 2];
	char token[256], header[BUF_SIZE];
	char name[256];
	char contents[10240];
	int contents_length = _contents_length;
	int i, ret, read = 0, read2 = 0, token_size = 0, read_size;
	int header_started = 0;
	int header_pos = 0;
	int contents_pos = 0;
	FILE* multi_fp = NULL;
	char cmd[128];

	*file_length = 0;
	memset(token, 0, sizeof(token));
	memset(buf + BUF_SIZE, 0, BUF_SIZE);

	read_size = min(contents_length, BUF_SIZE);
	read = fread(buf + BUF_SIZE, 1, read_size, fp);


#if 1
	int step = 0;
	int remain_step = -1;
	static ST_UPGRHDR hdr;

	static int remain_size = 0;

	static int fd = -1;
	char setBuf[32];

	static int prev_prog_ratio = 0;
	int prog_ratio;
	
	static int g_tot_size = 0;
	static int g_wr_size = 0;



#endif

	if(access(XDISK_DOWNLOAD_FIRMWARE_FILE, 0) == 0) {
		remove(XDISK_DOWNLOAD_FIRMWARE_FILE);
	}

	if(access(XDISK_ROOTFS_FILE, 0) == 0) {
		remove(XDISK_ROOTFS_FILE);
	}

	if(access(PROGRESS_FILE, 0) == 0) {
		remove(PROGRESS_FILE);
	}

	__write_upgr_progress(0);




	// while (read && contents_length > 0)
	while (read && step < 4)
	{
		memcpy(buf, buf + BUF_SIZE, BUF_SIZE);
		memset(buf + BUF_SIZE, 0, BUF_SIZE);


		// if (read == read_size)

		if (contents_length)
		{
			contents_length -= read;
			read_size = min(contents_length, BUF_SIZE);

			if (read_size > 0)
			{
				read2 = fread(buf + BUF_SIZE, 1, read_size, fp);

				if (read2 < 1024)
				{

				}
				//printf(buf + BUF_SIZE);
			}
 
		}
 

		for (i = 0; i < read; i++)
		{
			if (token[0] == 0)
			{
				if (buf[i] == 0x0d && buf[i+1] == 0x0a)
				{
					token_size = i + 2;
					token[0] = 0x0d;
					token[1] = 0x0a;
					memcpy(token + 2, buf, i);
					i++;	// skip for 0x0a

					header_started = 1;
					header_pos = 0;
					memset(header, 0, sizeof(header));
				}
			}
			else
			{
				if (header_started)
				{
					if (buf[i] == 0x0d && buf[i+1] == 0x0a && buf[i+2] == 0x0d && buf[i+3] == 0x0a)
					{
						header[header_pos] = 0;
						header_started = 0;
						i += 3;

						ret = find_value_from_header(header, "name=", name);
						if (ret)
						{
						}

						if (!ret)
							return 0;
					}
					else
					{
						header[header_pos++] = buf[i];
					}
				}
				else
				{
 
					{
						if (strcmp(name, "cgi_from") == 0)
						{

						}
						else if (strcmp(name, "newver") == 0)
						{

						}
						else if (strcmp(name, "upgr_file") == 0)
						{
							//End
							if (contents_length < read_size) {

								if (buf[i + 0] == '-' && buf[i + 1] == '-' && buf[i + 2] == '-' && buf[i + 3] == '-' &&
									buf[i + 4] == '-' && buf[i + 5] == '-' && buf[i + 6] == '-' && buf[i + 7] == '-')
								{
									break;
								}
							}

							contents[contents_pos++] = buf[i];
							// if (contents_pos >= 1024 || remain_step)
							if (contents_pos >= read)
							{
								remain_step = -1;

								int data_pos = 0;


								if(step == 0) {
									char _buf[BUF_SIZE];
									memset(_buf, 0, BUF_SIZE);

									memcpy(&hdr, contents, sizeof(ST_UPGRHDR));
									data_pos = sizeof(ST_UPGRHDR);

									if (hdr.u_key != HUPDT_KEY)
									{
										sprintf (cmd, "echo \"Firmaware update fail. not match firmware head info \" | cat >> /root/uplog.txt");
										system(cmd);
										goto Fail;
									}
									step = 1;
								}
							// root-fs
							if(step == 1) {

								static int write_sz = 0;
								int len = contents_pos;
								
								
								if(hdr.size[0] > 0) {

									if (write_sz <= 0)
									{
										remain_size = hdr.size[0];
										g_tot_size = hdr.size[0];

									}

									if(fd < 0) {
										fd = open(XDISK_DOWNLOAD_FIRMWARE_FILE, O_RDWR | O_CREAT, 0644);
										if(fd < 0) {
											goto Fail;
										}

										sprintf(setBuf, "%d", 0);
									}


									char *p = contents + data_pos;
									int sz = len - data_pos;

									if (remain_size - len <= 0)
									{
										data_pos = remain_size;
										sz = remain_size;

										step = 2;
									}

									remain_size -= sz;
									write_sz += sz;
									ret = write(fd, p, sz);

#if 1
									g_wr_size += ret;
									prog_ratio = g_wr_size / (g_tot_size / 100);

									
									if (prog_ratio != prev_prog_ratio && prog_ratio >= 25)
									{
										__write_upgr_progress (prog_ratio);
										// printf ("---- g_wr_size = %d; g_tot_size = %d)!!! ---\n", g_wr_size, g_tot_size);
										// printf ("---- prog_ratio = %d(<= %d)!!! ---\n", prog_ratio, prev_prog_ratio);
										prev_prog_ratio = prog_ratio;
									}
#endif 

									// if(ret < 0) {
									// 	close(fd);
									// 	fd = -1;
									// 	goto Fail;
									// }
								}
								else {
									step = 2;
								}

								if(step == 2) {
									if(fd >= 0) {
										close(fd);
										fd = -1;
										sprintf(setBuf, "%d", write_sz);
										ipc_status_set("upgr", "rootfs", setBuf);
										sprintf (cmd, "echo \"func:%s, line: %d > rootfs write sz? %d \" | cat >> /root/uplog.txt", __func__, __LINE__, setBuf);
										system (cmd);
										sleep(1);
									}
								}
							}

								if(step == 2) 
								{
									ST_UPGRTAIL tail;
									char *p = contents + data_pos;

									memcpy(&tail, p, sizeof(ST_UPGRTAIL));
									
									sprintf (cmd, "echo \"func:%s, line: %d > tail.dummy? %08x/%08x / tail.u_key? %08x/%08x \" | cat >> /root/uplog.txt", __func__, __LINE__, tail.dummy, TUPDT_KEY2, tail.u_key, TUPDT_KEY);
									system(cmd);


									if (tail.dummy  != TUPDT_KEY2 && tail.u_key != TUPDT_KEY)	// this err
									{
										
										char cmd[128];
										sprintf (cmd, "echo \"Firmaware update fail. not match firmware tail info \" | cat >> /root/uplog.txt");
										system(cmd);
										goto Fail;
									}


									if((access(XDISK_DOWNLOAD_FIRMWARE_FILE, 0) == 0) && remain_size == 0) 
									{
										char cmd[128];
										sprintf(cmd, "mv %s %s", XDISK_DOWNLOAD_FIRMWARE_FILE, XDISK_UPGR_OK);
										system(cmd);

										sprintf (cmd, "echo \"Firmaware update > cmd? %s\" | cat >> /root/uplog.txt", cmd);
										system(cmd);

									}
									step = 3;
									goto out;

							}


	#if 0
								fwrite(contents, 1, contents_pos, multi_fp);
	#endif
								*file_length += contents_pos;
								contents_pos = 0;

								printf(".");
								fflush(stdout);
							}
						}
					}
				}
			}
		}




		read = read2;
	}


out:



	if (multi_fp)
	{
		if (contents_pos > 0)
		{

			fwrite(contents, 1, contents_pos, multi_fp);
			*file_length += contents_pos;
			contents_pos = 0;
		}

		fclose(multi_fp);
	}

	return 0;

Fail:
	return -1;

}

void ___write_actioninfo(int type)
{
	int fd;
	char str[4];

	unlink(ACTION_INFO);
	fd = open(ACTION_INFO, O_RDWR |O_CREAT, 0644);
	if(fd > 0)
	{
		sprintf(str, "%d", type);
		write(fd, str, sizeof(str));


		close (fd);
	}
}


int main(void)
{
	int ret;
	cgi_init();
	cgi_session_start();

	cgi_init_headers();

	if (!check_session()) {
		goto out;
	}

	unlink(PROGRESS_FILE);

	char *tmp_data;
	int content_length, file_length;

	tmp_data = getenv("CONTENT_LENGTH");
	if (tmp_data == NULL)
	{
		printf("err");
		goto out;
	}

	content_length = atoi(tmp_data);

	if(content_length <= 0) {
		printf("err");
		goto out;
	}

	int64_t msec = get_time_msec();
	ret = my_process_form_data(stdin, content_length, &file_length);

	if(msec - get_time_msec() < 3000) {
		int i;
		for(i = 0; i < 10; i++) {
			printf(".");
			fflush(stdout);
			usleep(500000);
		}
	}

	printf("#%d", file_length);
	fflush(stdout);


	char cmd[128];
	sprintf (cmd, "echo \"Firmaware update > ret? %d\" | cat >> /root/uplog.txt", ret);
	system(cmd);
	
	if (ret == 0)
	{
		___write_actioninfo(ACTIONINFO_UPGRADE_START);		// ready update while > break .
	}
	else
	{
		___write_actioninfo(ACTIONINFO_UPGRADE_FAIL);
	}

	if((access(XDISK_DOWNLOAD_FIRMWARE_FILE, 0) == 0))
	{
		char cmd[128];
		sprintf(cmd, "rm %s", XDISK_DOWNLOAD_FIRMWARE_FILE);
		system(cmd);
	}


out:
	cgi_end();

	return 0;
}

