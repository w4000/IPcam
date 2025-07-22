
#ifndef __SYS_INFO_H
#define __SYS_INFO_H

#define CPU_MODEL					"NT98566"
#define DEVICE_TYPE					2				


enum __fw_codec {
	C_CODEC_UNKNOWN,
	C_CODEC_MPEG4,
	C_CODEC_H264,

	C_CODEC_MAX
};


enum pfilesystem {
	FS_FAT16 = 0,
	FS_FAT32,
	FS_NTFS,
	FS_EXT2,
	FS_EXT3,
	FS_NONE
};

#if 1

extern int handle_sysinfo_request (struct st_stream * svr, ST_PHDR phdr);

#endif
#endif


