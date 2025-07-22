#ifndef __UPGR_H
#define __UPGR_H


#define	XDISK_UPGR_MOUNT_PATH	"/tmp/upgr"



#define	XDISK_DOWNLOAD_FIRMWARE_FILE 		"/mnt/ipm/DOWNLOAD_FIRMWARE.BIN"
#define	XDISK_ROOTFS_FILE 		"/mnt/ipm/FUPGRADEFS.BIN"
#define	XDISK_UPGR_OK			"/mnt/ipm/UPGRADE.OK"


#define	NAND_PAGE_SIZE			(512)
#define	NAND_BLOCK_SIZE			(NAND_PAGE_SIZE *32)

enum UPGR_IMG_TYPE {
	UPGR_IMG_ROOTFS,
	UPGR_IMG_MAX
};

#define 	HUPDT_KEY	0x72172850
#define 	TUPDT_KEY	0x75479791
#define 	TUPDT_KEY2	0x32992128

#if 1
typedef struct upgr_hdr {
    union {
        struct {
            unsigned int u_key;          // 4 bytes
            unsigned int dummy;          // 4 bytes
            __u8 ver[4];                 // 4 bytes (S/W version)
            unsigned int dummy2;         // 4 bytes
            __u32 size[UPGR_IMG_MAX];    // 4 bytes (1 element, 4 bytes since UPGR_IMG_MAX is 1)
            unsigned int dummy3;         // 4 bytes
            __u32 crc32[UPGR_IMG_MAX];   // 4 bytes (1 element, 4 bytes since UPGR_IMG_MAX is 1)
            // __u8 buf[0]; // This would be omitted since it's not included in the 30 bytes
        };
        unsigned char padding[30];      
    };
} ST_UPGRHDR;


typedef struct upgr_tail {
    union {
        struct {
            unsigned int dummy;        // 4 bytes
            unsigned char ver[3];      // 3 bytes
            unsigned int dummy2;       // 4 bytes
            unsigned char channel;     // 1 byte
            unsigned char gui;         // 1 byte
            unsigned char lang;        // 1 byte
            unsigned short model;      // 2 bytes
            unsigned char codec;       // 1 byte
            unsigned int dummy3;       // 4 bytes
            unsigned int u_key;        // 4 bytes
        };
        unsigned char padding[30];   
    };
} ST_UPGRTAIL;

#else

typedef struct upgr_hdr
{
	__u8 	ver [4];						// S/W version.
	__u32	crc32 [UPGR_IMG_MAX];		// 32-bit CRC value for raw image in buf[].

	__u8 	buf [0];						// Raw image data.
} ST_UPGRHDR;

typedef struct upgr_tail
{
	unsigned char		channel;
	unsigned char		codec;
} ST_UPGRTAIL;


#endif



typedef struct __nand
{
	unsigned 		addr;
	unsigned 		len;
	char	    		data[NAND_BLOCK_SIZE];
} ST_NAND;

extern unsigned int get_file_crc32(const char *filename);

extern dword crc32_upgr (char *, int, __u32);
extern __u32 crc32_file (int);
extern void * upgr_th (void *);

extern int is_upgr_mode();
extern int upgr_function(int* type);

#endif
