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
	int a[100];
	int *p_a;
	
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

	printf("addr: %p a:%p\n", addr,a);

	while(1)
	{
		printf("[mmap content]%d %d\n", addr[0],addr[1]);
		sleep(2);
	}

	return 0;
}
