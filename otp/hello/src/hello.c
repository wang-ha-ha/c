#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h> 

#define FLASHOTPLOCK		_IOR('M', 1, int)

int main()
{
    int fd = open("/dev/flash_otp",O_RDWR);
    unsigned char buf[512];
    int i;

    printf("\n------------read test----------------\n");

    int ret = read(fd,buf,5);

    for(i = 0;i < 5 ;i++)
    {
        printf("%02x ",buf[i]);
        if(i != 0 && i % 15 == 0)
        {
            printf("\n");
        }
    }

    printf("\n---1---ret:%d---------------\n",ret);

    ret = read(fd,buf,256);

    for(i = 0;i < 256 ;i++)
    {
        printf("%02x ",buf[i]);
        if(i != 0 && i % 15 == 0)
        {
            printf("\n");
        }
    }

    printf("\n---2---ret:%d----------------\n",ret);

    ret = read(fd,buf,256);

    for(i = 0;i < 256 ;i++)
    {
        printf("%02x ",buf[i]);
        if(i != 0 && i % 15 == 0)
        {
            printf("\n");
        }
    }

    printf("\n---3---ret:%d----------------\n",ret);

    ret = read(fd,buf,256);

    for(i = 0;i < 256 ;i++)
    {
        printf("%02x ",buf[i]);
        if(i != 0 && i % 15 == 0)
        {
            printf("\n");
        }
    }

    printf("\n---4---ret:%d----------------\n",ret);
    printf("\n------------write test----------------\n");
    lseek(fd, 256, SEEK_SET);

    srand( (unsigned)time( NULL ) ); 
    for(i = 0;i < 300 ;i++)
    {   
        buf[i] = rand();
        printf("%02x ",buf[i]);
        if(i != 0 && i % 15 == 0)
        {
            printf("\n");
        }
    }

    ret = write(fd,buf,300);

    printf("\n---11ret:%d----------------\n",ret);
    lseek(fd, 256, SEEK_SET);

    ret = read(fd,buf,512);

    for(i = 0;i < 512 ;i++)
    {
        printf("%02x ",buf[i]);
        if(i != 0 && i % 15 == 0)
        {
            printf("\n");
        }
    }

    printf("\n---ret:%d----------------\n",ret);

    printf("\n------------lock test----------------\n");

    int sector = 0x100;

    ret = ioctl(fd,FLASHOTPLOCK,&sector);
    if (ret == -1) {
        printf("ioctl: %s\n", strerror(errno));
    }

    lseek(fd, 256, SEEK_SET);

    for(i = 0;i < 300 ;i++)
    {   
        buf[i] = rand();
        printf("%02x ",buf[i]);
        if(i != 0 && i % 15 == 0)
        {
            printf("\n");
        }
    }

    ret = write(fd,buf,300);

    printf("\n---11ret:%d----------------\n",ret);
    lseek(fd, 256, SEEK_SET);

    ret = read(fd,buf,512);

    for(i = 0;i < 512 ;i++)
    {
        printf("%02x ",buf[i]);
        if(i != 0 && i % 15 == 0)
        {
            printf("\n");
        }
    }

    printf("\n---ret:%d----------------\n",ret);

    close(fd);

    return 0;
}