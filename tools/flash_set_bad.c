#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

static int is_bad_block(int fd ,loff_t offset)
{
    int ret = 0;

    if (fd < 0) {
        return -1;
    }

    if (ioctl(fd, MEMGETBADBLOCK, &offset) > 0) {
        printf("Block at offset 0x%x is bad\n", offset);
        ret = 1;
    }

    return ret;
}

int main (int argc,char *argv[])
{
	if(argc != 4 && argc != 3 && argc != 2) {
		return 1;
	}

	int fd = open(argv[1],O_RDWR);
	mtd_info_t mtd_info;
	ioctl(fd, MEMGETINFO, &mtd_info);

	printf("info.size=%d\n", mtd_info.size);
    printf("info.erasesize=%d\n", mtd_info.erasesize);
    printf("info.writesize=%d\n", mtd_info.writesize);
    printf("info.oobsize=%d\n", mtd_info.oobsize);
    
	if(argc == 4) {
		erase_info_t erase;

        erase.start = atoi(argv[2]) * mtd_info.erasesize;
        erase.length = mtd_info.erasesize;
        if (ioctl(fd, MEMERASE, &erase)) {
            printf("Error: MEMERASE failed\n");
            return 0;
        } else {
            return 1;
        }

	} else if(argc == 3){
		loff_t blk_addr = atoi(argv[2]) * mtd_info.erasesize;

		if (ioctl(fd, MEMSETBADBLOCK, &blk_addr)) {
			printf("Error: MEMSETBADBLOCK failed\n");
			return 0;
		} else {
			return 1;
		}
	} else {
		int count = mtd_info.size /  mtd_info.erasesize;
		for(int i = 0; i < count; i++){
			if(is_bad_block(fd,i * mtd_info.erasesize) != 0){
				printf("%d is bad block\n", i);
			}
		}
	}

	return 0;
}

