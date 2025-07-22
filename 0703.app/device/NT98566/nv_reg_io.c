#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "nv_reg_io.h"

static int init_mem_dev(void)
{
	int fd = 0;
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	return fd;
}

static void uninit_mem_dev(int fd)
{
	close(fd);
	return;
}

void* mem_mmap(int fd, unsigned int mapped_size, off_t phy_addr)
{
	void *map_base = NULL;
	unsigned page_size = 0;

	page_size = getpagesize();
	map_base = mmap(NULL,
			mapped_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			phy_addr & ~(off_t)(page_size - 1));

	if(map_base == MAP_FAILED)
		return NULL;

	return map_base;
}

int mem_munmap(void* map_base, unsigned int mapped_size)
{
	if (munmap(map_base, mapped_size) == -1)
		return -1;

	return 0;
}

static unsigned int cmd_mem_r(int fd, unsigned int addr)
{
	unsigned int val = 0;
	unsigned int virtual_addr;
	void*  map_addr;
	unsigned int page_align_addr, map_size, map_offset;

	addr &= 0xFFFFFFFC;
	page_align_addr = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
	map_offset = addr - page_align_addr;
	map_size = map_offset + 4;

	map_addr = mem_mmap(fd, map_size, page_align_addr);
	if(map_addr == NULL) {
		printf("r:mmap error for addr 0x%x\r\n", addr);
		return -1;
	}
	virtual_addr = (unsigned int)(map_addr + map_offset);
	val = *((unsigned int *)virtual_addr);
	mem_munmap(map_addr, map_size);

	return val;
}

static void cmd_mem_w(int fd, unsigned int addr, unsigned int data)
{
	void* map_addr;
	unsigned int virtual_addr, length = 0x100;
	unsigned int page_align_addr, map_size, map_offset;

	addr &= 0xFFFFFFFC;
	page_align_addr = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);
	map_offset = addr - page_align_addr;
	map_size = map_offset + length;

	map_addr = mem_mmap(fd, map_size, page_align_addr);
	if(map_addr == NULL) {
		printf("w:mmap error for addr 0x%x\r\n", addr);
		return;
	}

	virtual_addr = (unsigned int)(map_addr + map_offset);
	*((unsigned int*)virtual_addr) = data;
	mem_munmap(map_addr, map_size);
}

unsigned int nv_reg_read(unsigned int address)
{
	int fd;
    unsigned int val = 0;

	fd = init_mem_dev();
	if(fd < 0) {
		printf("Open /dev/mem device failed\n");
		return -1;
	}

	val = cmd_mem_r(fd, address);

	uninit_mem_dev(fd);

    return val;
}

int nv_reg_write(unsigned int address, unsigned int value)
{
	int fd;

	fd = init_mem_dev();
	if(fd < 0) {
		printf("Open /dev/mem device failed\n");
		return -1;
	}

	cmd_mem_w(fd, address, value);

	uninit_mem_dev(fd);

	return 0;
}


