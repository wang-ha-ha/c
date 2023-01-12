/*
 * my_iic.h
 *
 *  Created on: 2019年10月24日
 *      Author: wangchuanqi
 */

#ifndef COMPONENT_IIC_MY_IIC_H_
#define COMPONENT_IIC_MY_IIC_H_

typedef struct _my_iic_t_
{
    ioport_port_pin_t sda;
    ioport_port_pin_t scl;

    unsigned char slave_addr;

    unsigned int init_fig;
}my_iic;


extern unsigned int my_iic_init(my_iic *iic,ioport_port_pin_t scl,ioport_port_pin_t sda,unsigned char slave_addr);
extern unsigned int my_iic_change_slave_addr(my_iic *iic,unsigned char slave_addr);

extern unsigned int my_iic_write_single_byte_addr(my_iic *iic,unsigned char addr,unsigned char dat);
extern unsigned int my_iic_read_single_byte_addr(my_iic *iic,unsigned char addr,unsigned char *dat);

extern unsigned int my_iic_writes_single_byte_addr(my_iic *iic,unsigned char addr,unsigned char *dat,unsigned int len);
extern unsigned int my_iic_reads_single_byte_addr(my_iic *iic,unsigned char addr,unsigned char *dat,unsigned int len);

extern unsigned int my_iic_write_double_byte_addr(my_iic *iic,unsigned short addr,unsigned char dat);
extern unsigned int my_iic_read_double_byte_addr(my_iic *iic,unsigned short addr,unsigned char *dat);

extern unsigned int my_iic_writes_double_byte_addr(my_iic *iic,unsigned short addr,unsigned char *dat,unsigned int len);
extern unsigned int my_iic_reads_double_byte_addr(my_iic *iic,unsigned short addr,unsigned char *dat,unsigned int len);

#endif /* COMPONENT_IIC_MY_IIC_H_ */
