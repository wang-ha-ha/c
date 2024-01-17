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
    int count = 0;
    int ret = 0;
    while(1){
        ipc_msg_queue msg;
        msg.type = 1;
        ret = msgrcv(msq_id, &msg, sizeof(msg.p_buf) + 4, msg.type, 0);
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
        if(msgsnd(msq_id,&msg,len+5,IPC_NOWAIT) < 0) {
            perror("msgsnd");
        }
    }

    return 0;
}