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

    ringbuf_init(rb,512,4096);
    
    srand(time(NULL));

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

    return 0;
}