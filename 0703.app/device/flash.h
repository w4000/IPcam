/*
 *	flash.h
 *
 *	Purpose:
 *		Provide flash memory IO.
 *
 *	Last update:
 *		 2005.4.9.
 ********************************************************************************/
#ifndef __FLASH_H
#define __FLASH_H


/* USB Flash related */
#define FLASH_SETUPPATH			"/mnt/ipm/setup"

/* FlashROM related */
#define FLASH_DEVICE0           "/dev/mtd0"
#define FLASH_DEVICE1           "/dev/mtd1"
#define FLASH_DEVICE2           "/dev/mtd2"
#define FLASH_DEVICE3           "/dev/mtd3"



typedef struct {
	unsigned 	page_size;
	unsigned 	chip_size;
	unsigned 	erase_size;

	unsigned	option;
} ST_NAND_INFO;

typedef struct __st_nand_partinfo {
	char			name[12];
	unsigned		size;

} __attribute__ ((packed)) ST_NAND_PARTINFO;


typedef struct flash_dev {
	int	fd;

	int	(*read_fs)  (struct flash_dev *, void *, void *);
	int (*write_fs) (struct flash_dev *, void *, void *);

	struct mtd_info_user meminfo;	// flash memory info will be stored here.

	ST_NAND_PARTINFO	partinfo;
	ST_NAND_INFO		nandinfo;

	int	(*read) (struct flash_dev *, unsigned long, char *, int);
	int 	(*write) (struct flash_dev *, unsigned long, char *, int);
	void	(*release) (struct flash_dev *);

} ST_FLASH;

extern int flash_scan (char *);
extern int flash_init (ST_FLASH *, char *);

#endif

