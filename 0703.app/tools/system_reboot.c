
#ifndef SYSTEM_REBOOT_C_
#define SYSTEM_REBOOT_C_

#define DEVICE_SENSOR			"/dev/dio"
#define GPIO_HW_WATCHDOG_RESET	39
#define GPIO_HW_RESET			43

#ifndef BUILD_RELEASE
#define BUILD_RELEASE 1
#endif

int get_reboot_flag()
{
	if(access("/mnt/ipm/normal_reboot", F_OK) == 0) {
		unlink("/mnt/ipm/normal_reboot");
		return 1;
	}

	return 0;
}

void set_reboot_flag()
{
	int fd;
	char str[4];

	if((fd = open("/mnt/ipm/normal_reboot", O_RDWR |O_CREAT, 0644)) == -1 )
	{
		perror ("open");
	}

	sprintf(str, "1");
	write(fd, str, sizeof(str));

	close (fd);
}

static void hw_reset()
{
	set_reboot_flag();
	printf("System reboot...\n");
	printf("reboot -f &\n");

	// { int i; for (i = 3; i >= 0; i--)	{ sleep(1); printf("%d\n", i); } }
	system("reboot -f &");
}

#endif /* SYSTEM_REBOOT_C_ */
