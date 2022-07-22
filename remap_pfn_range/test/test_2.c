#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGE_SIZE (4*1024)
#define BUF_SIZE (PAGE_SIZE)
#define OFFSET (0)
#if 1 
#define DEV_NAME "/dev/global_event_dev"
#else
#define DEV_NAME "/dev/remap_pfn"
#endif

int main(int argc, const char *argv[])
{
	int fd;
	char *addr = NULL;
	int times = 0;

	fd = open(DEV_NAME, O_RDWR);
	if (fd < 0) {
		perror("open failed\n");
		exit(-1);
	}

	addr = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, OFFSET);
	if (!addr) {
		perror("mmap failed\n");
		exit(-1);
	}

	addr[0] = 0;

	while(1)
	{
		addr[0]++;
		times++;
		printf("[%d]%d %d\n", times, addr[0],addr[1]);
		sleep(1);
	}

	return 0;
}
