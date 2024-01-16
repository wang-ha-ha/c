#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#include "ringbuf.h"

char g_msg_buf[4096];

static unsigned short cal_crc16(const unsigned char* p_data, unsigned int size)
{
    unsigned short crc = 0xFFFF;
    unsigned short i, j;

    for(i = 0; i < size; i ++)
    {
        crc ^= (p_data[i] & 0xFF);
        for (j = 0; j < 8; j++)
        {
            if ((crc & 0x0001) > 0)
                crc = ((crc >> 1) ^ 0x8408);
            else
                crc = (crc >> 1);
        }
    }

  return crc;
}

int main(int argc, char **argv)
{
    ringbuf *rb;
    int shm_id;
	key_t shm_key;

    if ((shm_key = ftok("/etc/passwd", 12345)) == -1) {
        perror("ftok()");
        exit(1);
    }

    shm_id = shmget(shm_key, sizeof(ringbuf) + 512 * 4096, IPC_CREAT |0600);    //medisource模块会创建该共享内存
    if (shm_id < 0) {
        perror("shmget()");
        exit(1);
    }

    rb = (ringbuf *)shmat(shm_id, NULL, 0);
    if ((void *)rb == (void *)-1) {
        perror("shmat()");
        exit(1);
    }

    ringbuf_init(rb,512,4096,0);    
    srand(time(NULL));

#ifndef STREAM
    int type = 0;
    while (1) {
        ringbuf_msg *msg = (ringbuf_msg *)g_msg_buf;
        msg->len = rand() % (4096 - sizeof(ringbuf_msg));
        msg->type = type++;
        for(int i = 0; i < msg->len; i++)
            msg->p_buf[i] = rand();
        unsigned short crc = cal_crc16(msg->p_buf,msg->len);
        msg->crc = crc;

        ringbuf_write_block(rb,g_msg_buf,msg->len + sizeof(ringbuf_msg));
    }
#else
    int seq = 0;
    int send_len = 0;
    int type = 0;
    while (1) {
        if(seq % 45 == 0) {
            send_len = 120 * 1024 + rand() % ( 20 * 1024); // I 帧
            type = 1;
        } else {
            send_len = 5 * 1024 + rand() % ( 15 * 1024);   // P 帧
            type = 2;
        }
        char *stream_buf = (char *) malloc(send_len);
        if(stream_buf == NULL) {
            printf("Error malloc !\n");
            continue;
        }
        for(int i = 0; i < send_len; i++)
            stream_buf[i] = rand();
        
        
        int rb_free_num = ringbuf_get_free_num(rb);
        // printf("send_len:%d,rb_free_num:%d,%d\n",send_len,rb_free_num,rb_free_num * (4096 - sizeof(ringbuf_stream));
        if(rb_free_num * (4096 - sizeof(ringbuf_stream)) < send_len) {
            printf("no free,Dropped frame!\n");
            seq++;
            free(stream_buf);
            usleep(66 * 1000);
            continue;
        }

        int real_send = 0;
        int remain_send = send_len;
        ringbuf_stream *msg = (ringbuf_stream *)g_msg_buf;
        msg->timestamp = time(NULL);
        msg->frame_len = send_len;
        printf("frame_len:%d,type:%d,seq:%d--rb_free_num:%d\n",msg->frame_len,type,seq,rb_free_num);
        while(real_send < send_len) {
            msg->len = remain_send > (4096 - sizeof(ringbuf_stream)) ? (4096 - sizeof(ringbuf_stream)) : remain_send;
            msg->seq = seq;
            if(real_send == 0)
                msg->type = type;
            else
                msg->type = 0;
            memcpy(msg->p_buf, &stream_buf[real_send], msg->len);
            real_send += msg->len;
            remain_send -= msg->len;
            ringbuf_write_block(rb,g_msg_buf,msg->len + sizeof(ringbuf_msg));
        }
        seq++;
        free(stream_buf);
        usleep(66 * 1000);
    }
#endif
    return 0;
}