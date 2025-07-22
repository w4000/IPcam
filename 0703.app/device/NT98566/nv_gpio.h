#ifndef __NV_GPIO_H__
#define __NV_GPIO_H__

typedef enum {
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT
} NV_GPIO_DIR_E;


int nv_gpio_init(void);
void nv_gpio_release(void);

int nv_gpio_pinmux(int gpio, unsigned int pin, int gpio_mode);
int nv_gpio_dir(int gpio, unsigned int pin, NV_GPIO_DIR_E dir);
int nv_gpio_read(int gpio, unsigned int pin);
int nv_gpio_write(int gpio, unsigned int pin, unsigned int val);

#endif /*__NV_GPIO_H__*/
