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
        ipc_msg_queue msg;
        msg.type = 1;
        msg.seq = seq;
        len = rand() % 20 + 1;
        for(int i = 0;i < len;i++)
            msg.p_buf[i] = rand() % 26 + 'a';
        msg.p_buf[len] = '\0';

        printf("send:seq:%d,%s\n", seq, msg.p_buf);
        if(msgsnd(msq_id,&msg,len+5,IPC_NOWAIT) < 0) {
            perror("msgsnd");
        }

    reread:
        msg.type = 2;
        // ret = msgrcv(msq_id, &msg, sizeof(msg.p_buf) + 4, msg.type, 0);
        do{
            usleep(100 * 1000);
            ret = msgrcv(msq_id, &msg, sizeof(msg.p_buf) + 4, msg.type, IPC_NOWAIT);
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