#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/msg.h>

#include "ipc_msg_queue.h"

#define _IPC_MSG_QUEUE_TYPE 1

int ipc_msg_queue_init(ipc_msg_queue *ipc_msg_queue,int max_size,const char * path,int id)
{
	key_t msg_key;

    if(ipc_msg_queue == NULL) {
        return -1;
    }

    if ((msg_key = ftok(path, id)) == -1) {
        perror("ftok()");
        return -1;
    }

    ipc_msg_queue->msq_id = msgget(msg_key, IPC_CREAT|0666);
    if (ipc_msg_queue->msq_id < 0) {
        perror("msgget()");
        return -1;
    }

    ipc_msg_queue->seq = 0;
    ipc_msg_queue->max_size = max_size;
    ipc_msg_queue->send_buf = (char *)malloc(max_size + sizeof(msq_msg));
    ipc_msg_queue->recv_buf = (char *)malloc(max_size + sizeof(msq_msg));
    if(ipc_msg_queue->send_buf == NULL || ipc_msg_queue->recv_buf == NULL) {
        printf("ipc_msg_queue->p_buf malloc eeror\n");
        return -1;
    }

    return 0;
}

int ipc_msg_queue_deinit(ipc_msg_queue *ipc_msg_queue)
{
    if(ipc_msg_queue->send_buf != NULL) {
        free(ipc_msg_queue->send_buf);
    }
    if(ipc_msg_queue->recv_buf != NULL) {
        free(ipc_msg_queue->recv_buf);
    }
    return 0;
}

char * ipc_msg_queue_send(ipc_msg_queue *ipc_msg_queue,int recv_type,char *buf,int len,int * response_len)
{
    int ret = 0;
    int count = 0;
    if(len > ipc_msg_queue->max_size) {
        printf("ipc_msg_queue_send msg is big\n");
        return NULL;
    }
    msq_msg *msg = (msq_msg *)ipc_msg_queue->send_buf;
    msg->type = _IPC_MSG_QUEUE_TYPE;
    msg->seq = ipc_msg_queue->seq;
    msg->recv_type = recv_type;
    memcpy(msg->p_buf,buf,len);
    printf("send:seq:%d,%s\n", msg->seq, msg->p_buf);
    if(msgsnd(ipc_msg_queue->msq_id,msg,len + _IPC_MSG_QUEUE_FIX_LEN,IPC_NOWAIT) < 0) {
        perror("msgsnd");
        *response_len = 0;
        return NULL;
    }

reread:
    msg->type = recv_type;
    do{
        usleep(100 * 1000);
        ret = msgrcv(ipc_msg_queue->msq_id, msg, ipc_msg_queue->max_size + _IPC_MSG_QUEUE_FIX_LEN, msg->type, IPC_NOWAIT);
        count++;
    }while(ret < 0 && count < 10);

    if(ret > 0) {
        if(ipc_msg_queue->seq > msg->seq) {
            printf("recv seq is small count:%d,seq:%d,%s\n",count, msg->seq, msg->p_buf);
            count = 0;
            goto reread;
        } else if(ipc_msg_queue->seq == msg->seq) {
            printf("recv:count:%d,seq:%d,%s\n",count, msg->seq, msg->p_buf);
            memcpy(buf,msg->p_buf,ret);
        } else {
            printf("recv seq is big count:%d,seq:%d,%s\n", count,msg->seq, msg->p_buf);
            ipc_msg_queue->seq++;
            *response_len = 0;
            return NULL;
        }
    } else {
        printf("recv timeout\n");
        *response_len = 0;
        return NULL;
    }

    ipc_msg_queue->seq++;
    *response_len = ret;
    return msg->p_buf;
}

int ipc_msg_queue_recv_loop(ipc_msg_queue *ipc_msg_queue,char * (*fn)(char *,int,int *))
{
    msq_msg *msg = (msq_msg *)ipc_msg_queue->recv_buf;

    while(1) {
        msg->type = _IPC_MSG_QUEUE_TYPE;
        int ret = msgrcv(ipc_msg_queue->msq_id, msg, ipc_msg_queue->max_size + _IPC_MSG_QUEUE_FIX_LEN, msg->type, 0);
        if(ret < 0){
            perror("ipc_msg_queue_recv_loop msgrcv()");
            continue;
        }
        
        printf("recv:msg->recv_type:%d,seq:%d,type:%ld,ret:%d,%s\n",msg->recv_type, msg->seq,msg->type,ret, msg->p_buf);

        msg->type = msg->recv_type;
        int response_len = 0;
        char * response = fn(msg->p_buf,ret - _IPC_MSG_QUEUE_FIX_LEN,&response_len);
        if(response_len > 0){
            memcpy(msg->p_buf,response,response_len);
        } else {
            response_len = strlen(_IPC_MSG_QUEUE_RECV_HANDLE_ERROR_MSG) + 1;
            memcpy(msg->p_buf,_IPC_MSG_QUEUE_RECV_HANDLE_ERROR_MSG,strlen(_IPC_MSG_QUEUE_RECV_HANDLE_ERROR_MSG) + 1);
        }

        if(msgsnd(ipc_msg_queue->msq_id,msg,response_len + _IPC_MSG_QUEUE_FIX_LEN,IPC_NOWAIT) < 0) {
            perror("ipc_msg_queue_recv_loop msgsnd");
            continue;
        }
    }

    return 0;
}