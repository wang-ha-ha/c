#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "shared_struct.h"

int ringbuf_writ_byte(ringbuf *rb,unsigned char u8Temp)
{
    //队列没有满
    if ((rb->ringbuf_tail + 1) % rb->ringbuf_len != rb->ringbuf_head)
    {
        rb->p_ringbuf[rb->ringbuf_tail] = u8Temp;
        rb->ringbuf_tail = (unsigned int)(rb->ringbuf_tail + 1) % rb->ringbuf_len;
    }
    else
    {
        return -1;
    }

    return 0;
}

void data_cal_check_sum(unsigned char *data,unsigned int data_size)
{
    unsigned int check_sum = 0;
    unsigned int validdata_len;
    unsigned char *str = data;
    unsigned char *str_check = (unsigned char *)&check_sum;

    validdata_len = data_size - 4;

    for(unsigned int i = 0; i < validdata_len ; i++)
    {
        check_sum += str[i];
    }
    check_sum += 0x55;

    for(unsigned int i = 0; i < 4 ; i++)
    {
        str[validdata_len + i] = str_check[i];
    }
}

/* Packet structure defines */
#define PACKET_RS_TRAILER_SIZE      ((unsigned int)2)               /* C0 C1 */
#define PACKET_RS_HEADER_SIZE       ((unsigned int)4)               /* ID + TYPE + CMD */
#define PACKET_RS_FOOTER_SIZE       ((unsigned int)2)               /* CRC16 */

#define PACKET_RS_ID_INDEX          ((unsigned int)1)
#define PACKET_RS_TYPE_INDEX        ((unsigned int)2)
#define PACKET_RS_CMD_INDEX         ((unsigned int)3)
#define PACKET_RS_DATA_INDEX        ((unsigned int)5)
#define PACKET_RS_CRC_C_INDEX       ((unsigned int)3)

/** @brief 协议包固定长度
 * c0 id type cmd crc c1
 * 1 + 1 + 1 + 2 + 2 + 1 = 8
 */
#define PACKET_FIX_SIZE             ((unsigned int)PACKET_RS_TRAILER_SIZE + PACKET_RS_HEADER_SIZE + PACKET_RS_FOOTER_SIZE)
#define PACKET_RS_SIZE              ((unsigned int)128)
#define PACKET_RS_1K_SIZE           ((unsigned int)1024)
/** @brief 串口发送超时时间ms */
#define TX_TIMEOUT                  ((unsigned int)200)

/** @brief c0c1返回状态 */
typedef enum
{
    RS_OK               = 0x00,
    RS_ERR_CRC          = 0x05,
    RS_ERR_LEN          = 0x06,
    RS_FORBIDDEN        = 0x10,
    RS_INVALID_VALUE    = 0x11,
    RS_INVALID_DATA     = 0x12
} RS_StatusTypeDef;

#define _SOF        ((unsigned char)0xC0)
#define _EOF        ((unsigned char)0xC1)
#define _TRANSFER   ((unsigned char)0x7D)
#define _XOR        ((unsigned char)0x20)

/**
 * @brief 接收数据包缓冲BUF
 * @note ATTENTION - please keep this variable 32bit alligned
 */
static unsigned char g_packet_data[PACKET_RS_1K_SIZE + PACKET_FIX_SIZE];
/** @brief 回调函数上报数据缓冲BUF*/
static unsigned char g_packet_send_data[PACKET_RS_SIZE];

/**
 * @brief 计算crc16值
 * @param p_data    需要计算数据BUF
 * @param size      需要计算数据BUF的长度
 * @retval crc值
 */
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

unsigned int c01c1_prepare_packet(unsigned char id, unsigned char type, unsigned short cmd,
                            unsigned char *p_source, unsigned int source_size,
                            unsigned char *p_packet)
{
    unsigned char buf[PACKET_RS_SIZE];
    unsigned short crc;
    unsigned int i, packet_size = 0, buf_size = 0;
    if(source_size > (PACKET_RS_SIZE - PACKET_FIX_SIZE))
    {
        return 0;
    }

    buf[buf_size++] = id;
    buf[buf_size++] = type;
    buf[buf_size++] = (unsigned char)(cmd >> 8);
    buf[buf_size++] = (unsigned char)(cmd);

    for (i = 0; i < source_size; i++)
    {
        buf[buf_size++] = p_source[i];
    }

    crc = cal_crc16(buf, buf_size);
    buf[buf_size++] = (unsigned char)(crc & 0x00FF);
    buf[buf_size++] = (unsigned char)(crc >> 8);

    p_packet[packet_size++] = _SOF;
    for (i = 0; i < buf_size; i++)
    {
        if (buf[i] == _SOF || buf[i] == _EOF || buf[i] == _TRANSFER)
        {
            p_packet[packet_size++] = _TRANSFER;
            p_packet[packet_size++] = (unsigned char)(buf[i] ^ _XOR);
        }
        else
        {
            p_packet[packet_size++] = buf[i];
        }
    }
    p_packet[packet_size++] = _EOF;

    return packet_size;
}

int main(int argc, char **argv)
{
    ringbuf *rb;

    int shm_id;
    key_t shm_key;

    if ((shm_key = ftok(SHARED_PATTH_NAME, SHARED_PROJ_ID)) == -1) {
        perror("ftok()");
        exit(1);
    }

    shm_id = shmget(shm_key, sizeof(ringbuf), IPC_CREAT |0600);
    if (shm_id < 0) {
        perror("shmget()");
        exit(1);
    }

    rb = (ringbuf *)shmat(shm_id, NULL, 0);
    if ((void *)rb == (void *)-1) {
        perror("shmat()");
        exit(1);
    }

    rb->ringbuf_head = 0;
    rb->ringbuf_len = RINGBUF_SIZE;

    unsigned char send_buf[1024];
    int id = 0;
    while (1)
    {
        // srand(time(NULL));

        // for(int i = 0; i < 1024;i++) 
        // {
        //     send_buf[i] = rand();
        // }

        // data_cal_check_sum(send_buf,sizeof(send_buf));

        // for(int i = 0; i < 1024;i++) 
        // {
        //     ringbuf_writ_byte(rb, send_buf[i]);
        // }
        // usleep(1000 * 30);   
        srand(time(NULL));
        for(int i = 0; i < 100;i++) 
        {
            g_packet_send_data[i] = rand();
        }
        
        int packet_length = c01c1_prepare_packet(id++, 2, 3, g_packet_send_data, 100, g_packet_data); 
        for(int i = 0; i < packet_length; i++)
        {
            int try = 10;
            while ( (--try) > 0 && (ringbuf_writ_byte(rb, g_packet_data[i]) != 0))
            {
                usleep(1);
            }

            if(try == 0)
            {
                printf("ID[%d]:The client is correspondingly slow\n",(id - 1)) % 256;
                break;
            }
        }

        usleep(1000 * 30);
    }

    return 0;
}
