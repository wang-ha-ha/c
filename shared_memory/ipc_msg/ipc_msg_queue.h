#ifndef __IPC_MSG_QUEUE_H__
#define __IPC_MSG_QUEUE_H__

typedef struct _ipc_msg_queue_t_
{
    long int type;
    int seq ;
    char p_buf[1024];
}ipc_msg_queue;

int ipc_msg_queue_init(ipc_msg_queue *ipc_msg_queue,int len,int block_size,int flag);
int ipc_msg_queue_deinit(ipc_msg_queue *ipc_msg_queue);
int ipc_msg_queue_write_block(ipc_msg_queue *ipc_msg_queue,char *buf,int len);
int ipc_msg_queue_read_block(ipc_msg_queue *ipc_msg_queue,char *buf,int len);
int ipc_msg_queue_get_free_num(ipc_msg_queue *ipc_msg_queue);

#endif