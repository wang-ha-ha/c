#ifndef __IPC_MSG_QUEUE_H__
#define __IPC_MSG_QUEUE_H__

typedef struct _ipc_msg_queue_t_
{
    int msq_id;
    int max_size;
    int seq;
    char *recv_buf;
    char *send_buf;
}ipc_msg_queue;

typedef struct _msq_msg_t_
{
    long int type;
    int seq;
    int recv_type;
    char p_buf[0];
}msq_msg;

#define _IPC_MSG_QUEUE_FIX_LEN 8
#define _IPC_MSG_QUEUE_RECV_HANDLE_ERROR_MSG "handle error"

int ipc_msg_queue_init(ipc_msg_queue *ipc_msg_queue,int msg_max_size,const char * path,int id);
int ipc_msg_queue_deinit(ipc_msg_queue *ipc_msg_queue);
char * ipc_msg_queue_send(ipc_msg_queue *ipc_msg_queue,int recv_type,char *buf,int len,int * response_len);
int ipc_msg_queue_recv_loop(ipc_msg_queue *ipc_msg_queue,char * (*fn)(char *,int,int *));

#endif