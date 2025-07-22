#include <stdio.h>
#include "nv_gpio.h"
#include "nv_pwm.h"
#include "device/gpio.h"
#include "nv_reg_io.h"
#include "oem_tool.h"
#include "oem_xml_define.h"

#define w4000_dbg()printf("%s, %d \r\n",__FUNCTION__,__LINE__)

#define GPIO_FACTORY	7

#if 0// new board // 2025-1-21 w4000 GPIO

#define GPIO_CDS	4
#define Day_gpio 3
#define Night_gpio 2

#else
#define GPIO_CDS	8
#define Day_gpio 1
#define Night_gpio 0
#endif

int gpio_factory_reset()
{
	int val = nv_gpio_read(0X1C, GPIO_FACTORY);
	//w4000_dbg();
	return val;	
}



int gpio_is_tdn_day(void)
{
	//w4000_dbg();
#if 1//w4000 
	if(nv_gpio_read(0X1C, GPIO_CDS))
		return 0;
	else
		return 1;
#else	//org
	
	return	nv_gpio_read(0X1C, GPIO_CDS);
#endif
}



int gpio_tdn_enable()//
{
	w4000_dbg();
	//return 0;//w4000
	nv_gpio_write(0X1C, Night_gpio, 0);
	nv_gpio_write(0X1C, Day_gpio, 0);
}

int gpio_tdn_mode(int mode)
{

	switch(mode) {
		case 1:	//night
			nv_gpio_write(0X1C, Night_gpio,1);
			//nv_gpio_write(0X1C, Day_gpio, 0);
			wait_delay_ms(500); 
			nv_gpio_write(0X1C, Night_gpio,0);//w4000
			break;
		case 0: //day 
			//nv_gpio_write(0X1C, Night_gpio, 0);
			nv_gpio_write(0X1C, Day_gpio, 1);
			wait_delay_ms(500); 
			nv_gpio_write(0X1C, Day_gpio, 0);//w4000
			break;
	}

	wait_delay_ms(200); 
	
	return 0;
}

int gpio_init()
{
	w4000_dbg();

	// PIN Mux
	nv_gpio_pinmux(0X1C, Night_gpio, 1);
	nv_gpio_pinmux(0X1C, Day_gpio, 1);
	nv_gpio_pinmux(0X1C, GPIO_CDS, 1);
	nv_gpio_pinmux(0X1C, GPIO_FACTORY, 1);

	nv_gpio_dir(0X1C, Night_gpio, GPIO_DIR_OUT);
	nv_gpio_dir(0X1C, Day_gpio, GPIO_DIR_OUT);
	nv_gpio_dir(0X1C, GPIO_CDS, GPIO_DIR_IN);
	nv_gpio_dir(0X1C, GPIO_FACTORY, GPIO_DIR_IN);

#if 1//w4000
	nv_gpio_write(0X1C, Night_gpio, 0);
	nv_gpio_write(0X1C, Day_gpio, 0);
	wait_delay_ms(200); //w4000
	nv_gpio_write(0X1C, Night_gpio, 0);
	nv_gpio_write(0X1C, Day_gpio, 1);
	wait_delay_ms(500); //w4000
	nv_gpio_write(0X1C, Night_gpio, 0);
	nv_gpio_write(0X1C, Day_gpio, 0);
#endif
	printf("w4000 gpio_init >>>>>>>>>>>>>>>> \r\n");
	//printf("w4000 gpio_init >>>>>>>>>>>>>>>> \r\n");
	//printf("w4000 gpio_init >>>>>>>>>>>>>>>> \r\n");

	return 0;
}

void gpio_release()
{
	w4000_dbg();
	nv_gpio_release();
}

int check_hw_system(void)
{
	w4000_dbg();
	return 0;
}
