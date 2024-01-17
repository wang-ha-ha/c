
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#include "ringbuf.h"

int ringbuf_init(ringbuf *ringbuf,int len,int block_size,int flag)
{
    ringbuf->block_size = block_size;
    ringbuf->len = len;
    if(flag == 0) {
        ringbuf->tail = 0;
        ringbuf->head = 0;
    }
    return 0;
}

int ringbuf_deinit(ringbuf *ringbuf)
{
    return 0;
}

int ringbuf_write_block(ringbuf *ringbuf,char *buf,int len)
{
    if(len > ringbuf->block_size)
        return -1;

    if ((ringbuf->tail + 1) % ringbuf->len != ringbuf->head) {
        char *tmp = (char *)&ringbuf->p_buf + ringbuf->tail * ringbuf->block_size;
        memcpy(tmp , buf, len);
        ringbuf->tail= (ringbuf->tail + 1) % ringbuf->len;
        return len;
    }

    return -1;
}

int ringbuf_read_block(ringbuf *ringbuf,char *buf,int len)
{
    if(len > ringbuf->block_size)
        return -1;

    if(ringbuf->tail == ringbuf->head)
        return -1;

    char *tmp = (char *)&ringbuf->p_buf + ringbuf->head * ringbuf->block_size;
    memcpy(buf, tmp, len);

    ringbuf->head = (ringbuf->head + 1) % ringbuf->len;
    
    return len;
}

int ringbuf_get_free_num(ringbuf *ringbuf)
{
    // printf("%d=%d\n", ringbuf->head, ringbuf->tail);
    if(ringbuf->head <= ringbuf->tail)
        return ringbuf->head - ringbuf->tail - 1 + ringbuf->len;
    return ringbuf->len - ringbuf->head - ringbuf->tail - 1;
}