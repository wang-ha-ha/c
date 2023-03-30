#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <ctype.h>
#include <errno.h>

#define BUFFER_SIZE 1024

int recv_timeout(int fd,void *buf, int len, int timeout_ms)
{
    int ret;
    size_t  rsum = 0;
    ret = 0;
    fd_set rset;
    struct timeval t;

    while (rsum < len)
    {
        t.tv_sec = timeout_ms/1000;
        t.tv_usec = (timeout_ms - t.tv_sec*1000)*1000;
        FD_ZERO(&rset);
        FD_SET(fd, &rset);
        ret = select(fd+1, &rset, NULL, NULL, &t);
        if (ret <= 0) {
            if (ret == 0) 
            {   
                if(rsum != 0)
                {
                    return rsum;
                }
                else
                {
                    //timeout
                    return -1;
                }
            }
            if (errno == EINTR) 
            {
                // 信号中断
                continue;
            }
            return -errno;
        } 
        else
        {
            ret = read(fd, (char *)buf + rsum, len - rsum);
            if (ret < 0)
            {
                printf("read error: %d\n", ret);
                return ret;
            }
            else
            {
                rsum += ret;
            }
        }
    }

    return rsum;
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("Usage: %s <host> <port>\n",argv[0]);
        return -1;
    }

    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
 
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2])); 
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
 
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("Connecting to %s %s\n",argv[1],argv[2]);
 
    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        printf("send:%s\n", sendbuf);
        send(sock_cli, sendbuf, strlen(sendbuf),0);
        if(strcmp(sendbuf,"exit\n")==0)
            break;
        int ret = recv_timeout(sock_cli, recvbuf, sizeof(recvbuf),200);
        printf("recv timeout: %d\n",ret);
        if(ret <= 0)
        {
            break;
        }
        
        for(int i=0; i < ret;i++)
        {
            if( isalnum(recvbuf[i]) == 0 && recvbuf[i] != '\n'  && recvbuf[i] != '\r' && recvbuf[i] != '<'  && recvbuf[i] != '>'&& recvbuf[i] != '_'  && recvbuf[i] != ':')
            {
                printf("unusual character:%d-%c\n", recvbuf[i],recvbuf[i]);
            } 
        }

        printf("recv[%d]:%s\n", ret, recvbuf);

        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
 
    close(sock_cli);
    return 0;
}