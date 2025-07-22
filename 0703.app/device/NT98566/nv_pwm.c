#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "nv_pwm.h"
#include "nv_reg_io.h"

#define PROC_PWM_CMD "/proc/pwm/cmd"
#define OPEN_PWM "echo w openpwm %d %d %d %d %d %d %d %d > %s"
#define CLOSE_PWM "echo w closepwm %d > %s"

// echo "w openpwm <channel> <base period> <rising> <falling> <clk div> <inv> <cycle> <reload>" > /proc/pwm/cmd
// echo "w closepwm <channel>" > /proc/pwm/cmd

int pwm_write(unsigned int PwmNum, unsigned int Duty, unsigned int Period, unsigned int enable) 
{
	int reload;
	char cmd[128];
	unsigned int pwm_en = (unsigned int)nv_reg_read(0xF0210100);
	reload = (pwm_en & (1<<PwmNum))?1:0;

	if(enable) {
		sprintf(cmd, OPEN_PWM, 
				PwmNum, 	// channel		: 0~11
				Period,		// base period	: 2~65535(PWM0~7), 2~255(PWM8~11)
				0, 			// rising		: 0~65535(PWM0~7), 0~255(PWM8~11)
				Duty, 		// falling		: 0~65535(PWM0~7), 0~255(PWM8~11), base_period >= falling >= rising
				0x3fff, 	// clk div		: 2~0x3fff, PWM input clock will be 120MHz/(clk_div+1)
				0, 			// inv			: 0, 1
				0, 			// cycle		: 0~65535(0:pwm will keep output)
				reload, 	// reload		: 0, 1(0:not reload, for first time config)
				PROC_PWM_CMD);
	}
	else {
		sprintf(cmd, CLOSE_PWM, PwmNum, PROC_PWM_CMD);
	}
	
	//printf("\n\n\n pwm_cmd = %s\n\n\n", cmd);

	system(cmd);

	return 0;

Fail:
	printf("failed to control pwm(%d)!\n", PwmNum);
	return -1;
}

#if 0
int pwm_write(unsigned int PwmNum, unsigned int Duty, unsigned int Period, unsigned int enable) 
{
	static int reload = 0;

	FILE *fp;
	fp = fopen(PROC_PWM_CMD, "w");
	if(fp == NULL) {
		goto Fail;
	}

	if(enable) {
		fprintf(fp, OPEN_PWM, 
				PwmNum, 	// channel		: 0~11
				Period,		// base period	: 2~65535(PWM0~7), 2~255(PWM8~11)
				0, 			// rising		: 0~65535(PWM0~7), 0~255(PWM8~11)
				Duty, 		// falling		: 0~65535(PWM0~7), 0~255(PWM8~11), base_period >= falling >= rising
				0x3fff, 	// clk div		: 2~0x3fff, PWM input clock will be 120MHz/(clk_div+1)
				0, 			// inv			: 0, 1
				0, 			// cycle		: 0~65535(0:pwm will keep output)
				reload); 	// reload		: 0, 1(0:not reload, for first time config)
		reload = 1;
	}
	else {
		fprintf(fp, CLOSE_PWM, PwmNum);
	}

	fclose(fp);

	return 0;

Fail:
	printf("failed to control pwm(%d)!\n", PwmNum);
	return -1;
}
#endif
