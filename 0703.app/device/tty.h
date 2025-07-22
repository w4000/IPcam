

#ifndef __TTY_H
#define __TTY_H


typedef struct st_ttydev {
	int	fd;						// device descriptor.

	/*------- Serial communication parameters -------*/
	int baud;
	int databit;
	int stopbit;
	int parity;
	int handshake;

} ST_TTYDEV;



#endif
