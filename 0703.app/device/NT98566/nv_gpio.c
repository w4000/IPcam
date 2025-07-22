#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "nv_gpio.h"
#include "nv_reg_io.h"

#define GPIO_BASE 0xF0070000
#define GPIO_DATA 		(GPIO_BASE | 0x00)
#define GPIO_DIR 		(GPIO_BASE | 0x20)
#define GPIO_DATA_SET 	(GPIO_BASE | 0x40)
#define GPIO_DATA_CLR	(GPIO_BASE | 0x60)

#define GPIO_MUX_BASE 		0xF0010000
#define GPIO_MUX_DIS_GPIO 	(GPIO_MUX_BASE | 0xE8)

static unsigned int pinmux_addr(int gpio)
{
	switch(gpio) {
		case 0X1C: return GPIO_MUX_DIS_GPIO;
		default: return 0;
	}
	return 0;
}


int nv_gpio_pinmux(int gpio, unsigned int pin, int gpio_mode)
{
	unsigned int addr = pinmux_addr(gpio);
	unsigned int value;

	if(addr == 0) {
		return -1;
	}

	value = (unsigned int)nv_reg_read(addr);
	if(gpio_mode == 1) { // GPIO
		value |= pin;
	}
	else { // FUNCTION
		value &= ~pin;
	}
	nv_reg_write(addr, value);
	return 0;
}

int nv_gpio_dir(int gpio, unsigned int pin, NV_GPIO_DIR_E dir)
{
	unsigned int addr = (GPIO_DIR | gpio);
	unsigned int value;

	value = (unsigned int)nv_reg_read(addr);
	if(dir == GPIO_DIR_OUT) {
		value |= (1<<pin);
	}
	else {
		value &= ~(1<<pin);
	}
	nv_reg_write(addr, value);

	return 0;
}

int nv_gpio_read(int gpio, unsigned int pin)
{
	unsigned int addr = (GPIO_DATA | gpio);
	unsigned int value;

	value = (unsigned int)nv_reg_read(addr);

	return (value&(1<<pin))?1:0;
}

int nv_gpio_write(int gpio, unsigned int pin, unsigned int val)
{
	unsigned int addr = (GPIO_DATA | gpio);
	unsigned int value;

	value = (unsigned int)nv_reg_read(addr);
	if(val) {
		value |= (1<<pin);
	}
	else {
		value &= ~(1<<pin);
	}
	nv_reg_write(addr, value);

	return 0;
}

int nv_gpio_init(void)
{
	return 0;
}

void nv_gpio_release(void)
{
	return;
}
