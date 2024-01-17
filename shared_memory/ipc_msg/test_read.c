#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include "ipc_msg_queue.h"

#if 0
int main(int argc, char **argv)
{
    int msq_id;
	key_t msg_key;

    if ((msg_key = ftok("/etc/passwd", 12345)) == -1) {
        perror("ftok()");
        exit(1);
    }

    msq_id = msgget(msg_key, IPC_CREAT|0666);
    if (msq_id < 0) {
        perror("msgget()");
        exit(1);
    }

    srand(time(NULL));
    int seq = 0;
    int len = 0;
    int count = 0;
    int ret = 0;
    while(1){
        msq_msg msg;
        msg.type = 1;
        ret = msgrcv(msq_id, &msg, sizeof(msg.p_buf) + _IPC_MSG_QUEUE_FIX_LEN, msg.type, 0);
        if(ret < 0){
            perror("msgrcv()");
            continue;
        }
        seq = msg.seq;
        printf("recv:seq:%d,type:%ld,ret:%d,%s\n", seq,msg.type,ret, msg.p_buf);


        msg.type = 2;
        msg.seq = seq;
        len = rand() % 20 + 1;
        for(int i = 0;i < len;i++)
            msg.p_buf[i] = rand() % 26 + 'a';
        msg.p_buf[len] = '\0';

        printf("recv ack:seq:%d,%s\n", seq, msg.p_buf);
        if(msgsnd(msq_id,&msg,len + _IPC_MSG_QUEUE_FIX_LEN + 1,IPC_NOWAIT) < 0) {
            perror("msgsnd");
        }
    }

    return 0;
}
#else 
char g_buf[1024];
char * fn(char *buf,int len,int *response_len)
{
    printf("recv[%d]:%s\n",len,buf);

    *response_len = rand() % 20 + 1;
    for(int i = 0;i < *response_len;i++)
        g_buf[i] = rand() % 26 + 'a';
    g_buf[*response_len] = '\0';
    *response_len += 1;
    return g_buf;
}

int main()
{
    ipc_msg_queue g_ipc_msg_queue;
    ipc_msg_queue_init(&g_ipc_msg_queue,1024,"/etc/passwd",12345);

    ipc_msg_queue_recv_loop(&g_ipc_msg_queue,fn);

    return 0;
}
#endif