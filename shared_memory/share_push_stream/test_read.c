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

    shm_id = shmget(shm_key, sizeof(ringbuf) + 512 * 4096, 0);    //medisource模块会创建该共享内存
    if (shm_id < 0) {
        perror("shmget()");
        exit(1);
    }

    rb = (ringbuf *)shmat(shm_id, NULL, 0);
    if ((void *)rb == (void *)-1) {
        perror("shmat()");
        exit(1);
    }

    // ringbuf_init(rb,512,4096,1);
#ifndef STREAM
    while (1) {
        if(ringbuf_read_block(rb,g_msg_buf,sizeof(g_msg_buf)) < 0) {
            usleep(1);
            continue;
        }
        ringbuf_msg *msg = (ringbuf_msg *)g_msg_buf;
        unsigned short crc = cal_crc16(msg->p_buf,msg->len);
        if(crc != msg->crc)
            printf("error:crc:%d,type:%d\n",msg->crc,msg->type);
    }
#else
    while(1) {
        if(ringbuf_read_block(rb,g_msg_buf,sizeof(g_msg_buf)) < 0) {
            usleep(1);
            continue;
        }
        ringbuf_stream *msg;
    restart:
        msg = (ringbuf_stream *)g_msg_buf;
        printf("frame_len:%d,type:%d,seq:%d,len:%d\n",msg->frame_len,msg->type,msg->seq,msg->len);
        if(msg->type != 0){
            char *stream_buf = (char *) malloc(msg->frame_len);
            if(stream_buf == NULL) {
                printf("Error malloc !\n");
                exit(1);
            }
            int real_read = 0;
            int frame_len = msg->frame_len;
            memcpy(stream_buf + real_read,msg->p_buf,msg->len);
            real_read += msg->len;
            while(real_read < frame_len) {
                if(ringbuf_read_block(rb,g_msg_buf,sizeof(g_msg_buf)) < 0) {
                    usleep(1);
                    continue;
                }
                if(msg->type == 0){
                    memcpy(stream_buf + real_read,msg->p_buf,msg->len);
                    real_read += msg->len;
                } else {
                    printf("error: frame read,Dropped frame\n");
                    free(stream_buf);
                    goto restart;
                }
            }
            if(real_read != frame_len) {
                printf("error: read len\n");
            }
            free(stream_buf);
        } else {
            printf("error: first type is zero,Dropped frame\n");
        }
    }
#endif
    return 0;
}