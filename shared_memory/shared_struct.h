#ifndef __SHARED_STRUCT__H__

#define RINGBUF_SIZE (10240)
#define SHARED_PATTH_NAME "/etc/passwd"
#define SHARED_PROJ_ID (12345)

typedef struct _ringbuf_t_
{
    volatile unsigned int   ringbuf_head;
    volatile unsigned int   ringbuf_tail;
    volatile unsigned int   ringbuf_len;
    volatile unsigned char  p_ringbuf[RINGBUF_SIZE];
}ringbuf;

#endif // !__SHARED_STRUCT__H__