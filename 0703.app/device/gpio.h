#ifndef __GPIO_H__
#define __GPIO_H__



int gpio_factory_reset();
int gpio_tdn_mode(int mode);
int gpio_init();
void gpio_release();

#endif /*__GPIO_H__*/
