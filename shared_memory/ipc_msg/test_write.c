#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <pthread.h>

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
    int ret = 0;
    while(1){
        int count = 0;
        msq_msg msg;
        msg.type = 1;
        msg.seq = seq;
        len = rand() % 20 + 1;
        for(int i = 0;i < len;i++)
            msg.p_buf[i] = rand() % 26 + 'a';
        msg.p_buf[len] = '\0';

        printf("send:seq:%d,%s\n", seq, msg.p_buf);
        if(msgsnd(msq_id,&msg,len + _IPC_MSG_QUEUE_FIX_LEN + 1,IPC_NOWAIT) < 0) {
            perror("msgsnd");
        }

    reread:
        msg.type = 2;
        // ret = msgrcv(msq_id, &msg, sizeof(msg.p_buf) + 4, msg.type, 0);
        do{
            usleep(100 * 1000);
            ret = msgrcv(msq_id, &msg, sizeof(msg.p_buf) + _IPC_MSG_QUEUE_FIX_LEN, msg.type, IPC_NOWAIT);
            count++;
        }while(ret < 0 && count < 10);

        if(ret > 0) {
            if(seq > msg.seq) {
                printf("recv seq is small count:%d,seq:%d,%s\n",count, msg.seq, msg.p_buf);
                goto reread;
            } else if(seq == msg.seq) {
                printf("recv:count:%d,seq:%d,%s\n",count, msg.seq, msg.p_buf);
            } else {
                printf("recv seq is big count:%d,seq:%d,%s\n", count,msg.seq, msg.p_buf);
            }
        } else {
            printf("recv timeout\n");
        }

        seq++;
        sleep(1);
    }

    return 0;
}
#else 
void* send_msg_fn(void* tid)
{
    printf("11\n");
    int *i = (int*)tid;
    printf("22\n");
    int recv_type = 2 + *i;
    ipc_msg_queue g_ipc_msg_queue;
    printf("33\n");
    ipc_msg_queue_init(&g_ipc_msg_queue,1024,"/etc/passwd",12345);
    printf("44\n");
    srand(time(NULL));
    char buf[1024];
    while(1){
        int len = rand() % 20 + 1;
        for(int i = 0;i < len;i++)
            buf[i] = rand() % 26 + 'a';
        buf[len] = '\0';

        int recv_len = 0;
        char *recv_buf = ipc_msg_queue_send(&g_ipc_msg_queue,recv_type,buf,len + 1,&recv_len);
        printf("recv_len: %d,%s\n",recv_len,recv_buf);
        usleep(200*1000);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t threads[10];
    int status,i;
    for(i=0;i<10;i++){
        printf("Main here. Creating thread %d\n",i);
       
        status=pthread_create(&threads[i],NULL,send_msg_fn,(void*)&i);
        if(status!=0) {
            printf("pthread_create returned error code %d\n",status);
            exit(-1);
        }
    }
    while(1)
    {
        sleep(1);
    }
    return 0;
}
#endif