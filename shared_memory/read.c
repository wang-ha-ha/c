#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shared_struct.h"

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

int ringbuf_read(ringbuf *rb,unsigned char *msg, unsigned int len)
{
    int count = 0 ;

    while((rb->ringbuf_tail != rb->ringbuf_head) && (count < len))
    {
        count++;
        *msg++ =  rb->p_ringbuf[rb->ringbuf_head];
        rb->ringbuf_head = (unsigned int)(rb->ringbuf_head + 1) % rb->ringbuf_len;
    }

    return count;
}

int dump(unsigned char *msg, unsigned int len)
{
    int i;
    printf("msg[%d]:\n", len);

    for(i = 0; i < len; i++) 
    {
        printf("%02x ", msg[i]);
        if( (i+1) % 16 == 0) 
        {
            printf("\n");
        }
    }

    if(i%16 != 0) 
    {
        printf("\n");
    }
}

int data_check_sum(unsigned char *data,unsigned int data_size)
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
        if(str_check[i] != str[validdata_len + i])
        {
            return 1;
        }
    }

    return 0;
}

static int receive_packet(ringbuf *rb,unsigned char *p_data, unsigned int *p_length)
{
    int status;
    static unsigned int packet_size = 0;
    unsigned char char1, flag_transfer = 0;

    *p_length = 0;

    while (1)
    {
        status = ringbuf_read(rb,&char1, 1);
        //没有接收到数据
        if (status == 0)
        {
            return 2;
        }

        if (char1 == _SOF)
        {
            *p_length = 0;
            *(p_data + (packet_size)++) = char1;
        }
        else
        {
            /* C0 xx ... */
            if (packet_size)
            {
                if (flag_transfer)
                {
                    flag_transfer = 0x00;
                    *(p_data + (packet_size)++) = (char1 ^ _XOR);
                }
                else
                {
                    if (char1 == _TRANSFER)
                    {
                        flag_transfer = 0x80;
                        continue;
                    }
                    *(p_data + (packet_size)++) = char1;
                    /* ... xx C1 */
                    if (char1 == _EOF)
                    {
                        break;
                    }
                }
            }
        }
        if (packet_size == (PACKET_RS_SIZE + PACKET_RS_1K_SIZE - 1))
        {
            packet_size = 0;
            return -1;
        }
    }/* while END*/
    status = 0;
    /* Simple packet sanity check */
    if (packet_size < (PACKET_RS_TRAILER_SIZE + PACKET_RS_HEADER_SIZE + PACKET_RS_FOOTER_SIZE))
    {
        packet_size = 0;
        status = -1;
    }

    *p_length = packet_size;
    
    packet_size = 0;

    return status;
}

static const unsigned short crc16_tab[256] = {
0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static unsigned short crc_16(unsigned char*buffer, unsigned short buff_len)
{
    unsigned char x_or = 0;
    unsigned short crc = 0xFFFF;
    unsigned short len = buff_len;
    unsigned char *buff = (unsigned char*)buffer;

    while (len--)
    {
        x_or = (*buff++)^crc;
        crc >>= 8;
        crc ^= crc16_tab[x_or];
    }

    return crc;
}

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

int wirelesscharge_c0c1_process(ringbuf *rb)
{
    unsigned char cmd = 0,type = 0,id = 0;
    unsigned short crc, data_length;
    unsigned int packet_length,send_packet_length;
    RS_StatusTypeDef status = RS_OK;
    // wirelesscharge_c0c1_handler_list *temp = g_c0c1_type_FS_handler;
    int i = 0,j = 0;
    int ret = 0;
    while ((ret = receive_packet(rb,g_packet_data, &packet_length)) == 0)
    {   
        i++;
        // printf("i:%d\n",i);
        id = g_packet_data[PACKET_RS_ID_INDEX];
        type = g_packet_data[PACKET_RS_TYPE_INDEX];
        cmd = (unsigned short)(g_packet_data[PACKET_RS_CMD_INDEX] << 8 | g_packet_data[PACKET_RS_CMD_INDEX + 1]);

        /* Check packet CRC */
        crc = (unsigned short)(g_packet_data[packet_length - PACKET_RS_CRC_C_INDEX + 1] << 8 | g_packet_data[packet_length - PACKET_RS_CRC_C_INDEX]);
        if (cal_crc16(&(g_packet_data[PACKET_RS_ID_INDEX]), (packet_length - PACKET_RS_TRAILER_SIZE - PACKET_RS_FOOTER_SIZE)) != crc)
        {
            status = RS_ERR_CRC;
            break;
        }

        // status = RS_INVALID_VALUE;

        // for(j = 0; temp[j].cmd != 0 ; j++)
        // {
        //     if (cmd == temp[j].cmd)
        //     {
        //         status = RS_OK;
        //         break;
        //     }
        // }

        // if (status != RS_OK)
        // {
        //     status = RS_INVALID_VALUE;
        //     break;
        // }

        /* DATA: content */
        data_length = (unsigned short)(packet_length - (PACKET_RS_TRAILER_SIZE + PACKET_RS_HEADER_SIZE + PACKET_RS_FOOTER_SIZE));

        if (data_length > PACKET_RS_1K_SIZE)
        {
            status = RS_INVALID_DATA;
            break;
        }

        // if(temp[j].function != NULL)
        // {
        //     temp[j].function(g_packet_data,packet_length);
        //     // /* Response */
        //     // packet_length = wirelesscharge_c01c1_prepare_packet(cal_id(id), type, cmd, g_packet_send_data, send_packet_length, g_packet_data);
        //     // wirelesscharge_c0c1_transmit_packet(g_packet_data, packet_length);
        // }
        // printf("ID[%d]:The right package was read\n",id);
        // return 0;
    }

    if(ret == 2)
    {
        // printf("on data\n");
        return 2;
    }
    else if(ret != 0)
    {
        printf("An error package was read\n");
        return 1;
    }

    return 0;
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

    shm_id = shmget(shm_key, sizeof(ringbuf), 0);
    if (shm_id < 0) {
        perror("shmget()");
        exit(1);
    }

    rb = (ringbuf *)shmat(shm_id, NULL, 0);
    if ((void *)rb == (void *)-1) {
        perror("shmat()");
        exit(1);
    }

    rb->ringbuf_tail = 0;
    rb->ringbuf_len = RINGBUF_SIZE;

    unsigned char recv_buf[1024] = { 0 };
    while(1)
    {
        // int count = ringbuf_read(rb, recv_buf,sizeof(recv_buf));
        // if(count == 0)
        // {
        //     printf("on data\n");
        // }
        // else
        // {
        //     if(data_check_sum(recv_buf,count) == 1)
        //     {
        //         printf("An error package was read\n");
        //     }
        //     else
        //     {
        //         printf("The right package was read\n");
        //     }
        // }
        // usleep(1000 * 40);

        wirelesscharge_c0c1_process(rb);

        usleep(1000 * 2000); 
    }

    return 0;
}
