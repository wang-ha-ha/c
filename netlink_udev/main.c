#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <asm/types.h>
//该头文件需要放在netlink.h前面防止编译出现__kernel_sa_family未定义
#include <sys/socket.h>  
#include <linux/netlink.h>

static int parse_date(char *buf, int len)
{
    if (strstr(buf, "ACTION=add") != 0
        && strstr(buf, "DEVNAME=mmcblk0p1") != 0) {
        return 2;//代表插入sd卡
    }
    else if (strstr(buf, "ACTION=remove") != 0
        && strstr(buf, "DEVNAME=mmcblk0p1") != 0)
    {
        return 1;//代表拔掉sd卡
    }

	return 0;
}

void MonitorNetlinkUevent()
{
    int sockfd;
    struct sockaddr_nl sa;
    int len;
    char buf[4096];
    struct iovec iov;
    struct msghdr msg;
    int i;
    int ret = 0;

    memset(&sa,0,sizeof(sa));
    sa.nl_family=AF_NETLINK;
    sa.nl_groups=NETLINK_KOBJECT_UEVENT;
    sa.nl_pid = 0;//getpid(); both is ok
    memset(&msg,0,sizeof(msg));
    iov.iov_base=(void *)buf;
    iov.iov_len=sizeof(buf);
    msg.msg_name=(void *)&sa;
    msg.msg_namelen=sizeof(sa);
    msg.msg_iov=&iov;
    msg.msg_iovlen=1;

    sockfd=socket(AF_NETLINK,SOCK_RAW,NETLINK_KOBJECT_UEVENT);
    if(sockfd==-1)
        printf("socket creating failed:%s\n",strerror(errno));
    if(bind(sockfd,(struct sockaddr *)&sa,sizeof(sa))==-1)
        printf("bind error:%s\n",strerror(errno));
    
    while(1)
    {
        len=recvmsg(sockfd,&msg,0);
        if(len<0)
            printf("receive error\n");
        else if(len<32||len>sizeof(buf))
            printf("invalid message");
        for(i=0;i<len;i++)
            if(*(buf+i)=='\0')
                buf[i]='\n';
        printf("received %d bytes\n%s\n",len,buf);
        
        ret = parse_date(buf,len);
        printf("ret:%d\n",ret);
        if( 2 == ret )
        {
            printf("-------------------------\n");
            printf("SD Card insert\n");
            printf("-------------------------\n");
        }
        else if(1 == ret)
        {
            printf("-------------------------\n");
            printf("SD Card remove\n");
            printf("-------------------------\n");
        }
    }
}


int main(int argc,char **argv)
{
    MonitorNetlinkUevent();
    return 0;
}