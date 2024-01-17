#ifndef __RINGBUF_H__
#define __RINGBUF_H__

typedef struct _ringbuf_t_
{
    volatile unsigned int len;
    volatile unsigned int block_size;
    volatile unsigned int head;
    volatile unsigned int tail;
    volatile char p_buf[0];
}ringbuf;

typedef struct _ringbuf_msg_t_
{
    volatile unsigned int   len;
    volatile unsigned short type;
    volatile unsigned short crc;
    volatile char p_buf[0];
}ringbuf_msg;

typedef struct _ringbuf_stream_t_
{
    volatile unsigned int frame_len;
    volatile unsigned int len;
    volatile unsigned int type;
    volatile unsigned int seq;
    volatile unsigned long long timestamp;
    volatile char p_buf[0];
}ringbuf_stream;

int ringbuf_init(ringbuf *ringbuf,int len,int block_size,int flag);
int ringbuf_deinit(ringbuf *ringbuf);
int ringbuf_write_block(ringbuf *ringbuf,char *buf,int len);
int ringbuf_read_block(ringbuf *ringbuf,char *buf,int len);
int ringbuf_get_free_num(ringbuf *ringbuf);

#endif