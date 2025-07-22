#ifndef __DCIRIS_H__
#define __DCIRIS_H__

int dciris_init(int pwmid, int duty_open, int duty_close, int duty_stop);
int dciris_open(int step, int fast);
int dciris_close(int step, int fast);
int dciris_stop();

#endif /*__DCIRIS_H__*/
