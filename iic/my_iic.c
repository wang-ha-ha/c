/*
 * my_iic.c
 *
 *  Created on: 2019年10月24日
 *      Author: wangchuanqi
 */

#include "cmd_thread.h"
#include "my_iic.h"

extern void print_to_console(const char *pFmt, ...);
#define xp_log(format, args...)  print_to_console(format, ##args)


static void delay_5us();

static unsigned int my_iic_start(my_iic *iic);
static unsigned int my_iic_stop(my_iic *iic);
static unsigned char my_iic_ack(my_iic *iic);
static void my_iic_shend_ack(my_iic *iic,unsigned char ack);
static unsigned int my_iic_write(my_iic *iic,unsigned char dat);
static unsigned char my_iic_read(my_iic *iic);

static unsigned int my_iic_pin_set_output(ioport_port_pin_t pin);
static unsigned int my_iic_pin_set_input(ioport_port_pin_t pin);
static unsigned int my_iic_pin_set_high(ioport_port_pin_t pin);
static unsigned int my_iic_pin_set_low(ioport_port_pin_t pin);
static unsigned int my_iic_pin_read(ioport_port_pin_t pin,ioport_level_t * p_pin_value);


unsigned int my_iic_init(my_iic *iic,ioport_port_pin_t scl,ioport_port_pin_t sda,unsigned char slave_addr)
{
    unsigned int err = 0;

    if(iic == NULL)
    {
        return 1;
    }

    iic->scl = scl;
    iic->sda = sda;
    iic->slave_addr = slave_addr;
    iic->init_fig = 99;

    err = my_iic_pin_set_output(iic->scl);
    err = my_iic_pin_set_output(iic->sda);

    return err;
}

unsigned int my_iic_change_slave_addr(my_iic *iic,unsigned char slave_addr)
{
    unsigned int err =0;

    if(iic == NULL)
    {
        return 1;
    }

    iic->slave_addr = slave_addr;

    return err;
}

unsigned int my_iic_write_double_byte_addr(my_iic *iic,unsigned short addr,unsigned char dat)
{
    unsigned int err = 0;

    if(iic == NULL)
    {
        return 1;
    }
    __disable_irq();
    my_iic_start(iic);
    my_iic_write(iic,iic->slave_addr);
    my_iic_ack(iic);

    my_iic_write(iic, (unsigned char) (addr>>8 & 0xff));
    my_iic_ack(iic);

    my_iic_write(iic,(unsigned char)(addr & 0xff));
    my_iic_ack(iic);

    my_iic_write(iic,dat);
    my_iic_ack(iic);

    my_iic_stop(iic);
    __enable_irq();
    return err;
}

unsigned int my_iic_read_double_byte_addr(my_iic *iic,unsigned short addr,unsigned char *dat)
{
    unsigned int err = 0;

    if(iic == NULL)
    {
        return 1;
    }
    __disable_irq();
    my_iic_start(iic);
    my_iic_write(iic,iic->slave_addr);
    my_iic_ack(iic);

    my_iic_write(iic, (unsigned char) (addr>>8 & 0xff));
    my_iic_ack(iic);

    my_iic_write(iic,(unsigned char)(addr & 0xff));
    my_iic_ack(iic);

    my_iic_start(iic);

    my_iic_write(iic,iic->slave_addr | 0x01);
    my_iic_ack(iic);

    *dat = my_iic_read(iic);

    my_iic_stop(iic);
    __enable_irq();
    return err;
}

unsigned int my_iic_writes_double_byte_addr(my_iic *iic,unsigned short addr,unsigned char *dat,unsigned int len)
{
    unsigned int err = 0;
    unsigned int i;

    if(iic == NULL)
    {
        return 1;
    }
    __disable_irq();
    my_iic_start(iic);
    my_iic_write(iic,iic->slave_addr);
    my_iic_ack(iic);

    my_iic_write(iic,(unsigned char) (addr>>8 & 0xff));
    my_iic_ack(iic);

    my_iic_write(iic,(unsigned char)(addr & 0xff));
    my_iic_ack(iic);

    for(i = 0; i < len; i++)
    {
        my_iic_write(iic,dat[i]);
        my_iic_ack(iic);
    }

    my_iic_stop(iic);
    __enable_irq();
    return err;
}

unsigned int my_iic_reads_double_byte_addr(my_iic *iic,unsigned short addr,unsigned char *dat,unsigned int len)
{
    unsigned int err = 0;
    unsigned int i;

    if(iic == NULL)
    {
        return 1;
    }
    __disable_irq();
    my_iic_start(iic);
    my_iic_write(iic,iic->slave_addr);
    my_iic_ack(iic);

    my_iic_write(iic,(unsigned char) (addr>>8 & 0xff));
    my_iic_ack(iic);

    my_iic_write(iic,(unsigned char)(addr & 0xff));
    my_iic_ack(iic);
    my_iic_stop(iic);

    my_iic_start(iic);

    my_iic_write(iic,iic->slave_addr | 0x01);
    my_iic_ack(iic);

    for(i = 0; i < len - 1; i++)
    {
        dat[i] = my_iic_read(iic);
        my_iic_shend_ack(iic,1);
    }

    dat[i] = my_iic_read(iic);
    my_iic_shend_ack(iic,0);

    my_iic_stop(iic);
    __enable_irq();
    return err;
}

unsigned int my_iic_write_single_byte_addr(my_iic *iic,unsigned char addr,unsigned char dat)
{
    unsigned int err = 0;

    if(iic == NULL)
    {
        return 1;
    }
    __disable_irq();
    my_iic_start(iic);
    my_iic_write(iic,iic->slave_addr);
    my_iic_ack(iic);

    my_iic_write(iic,addr);
    my_iic_ack(iic);

    my_iic_write(iic,dat);
    my_iic_ack(iic);

    my_iic_stop(iic);
    __enable_irq();
    return err;
}

unsigned int my_iic_writes_single_byte_addr(my_iic *iic,unsigned char addr,unsigned char *dat,unsigned int len)
{
    unsigned int err = 0;
    unsigned int i;

    if(iic == NULL)
    {
        return 1;
    }
    __disable_irq();
    my_iic_start(iic);
    my_iic_write(iic,iic->slave_addr);
    my_iic_ack(iic);

    my_iic_write(iic,addr);
    my_iic_ack(iic);

    for(i = 0; i < len; i++)
    {
        my_iic_write(iic,dat[i]);
        my_iic_ack(iic);
    }

    my_iic_stop(iic);
    __enable_irq();
    return err;
}

unsigned int my_iic_read_single_byte_addr(my_iic *iic,unsigned char addr,unsigned char *dat)
{
    unsigned int err =0;

    if(iic == NULL)
    {
        return 1;
    }
    __disable_irq();
    my_iic_start(iic);
    my_iic_write(iic,iic->slave_addr);
    my_iic_ack(iic);

    my_iic_write(iic,addr);
    my_iic_ack(iic);

    my_iic_start(iic);

    my_iic_write(iic,iic->slave_addr | 0x01);
    my_iic_ack(iic);

    *dat = my_iic_read(iic);

    my_iic_stop(iic);
    __enable_irq();
    return err;
}

unsigned int my_iic_reads_single_byte_addr(my_iic *iic,unsigned char addr,unsigned char *dat,unsigned int len)
{
    unsigned int err = 0;
    unsigned int i;

    if(iic == NULL)
    {
        return 1;
    }
    __disable_irq();
    my_iic_start(iic);
    my_iic_write(iic,iic->slave_addr);
    my_iic_ack(iic);

    my_iic_write(iic,addr);
    my_iic_ack(iic);
    my_iic_stop(iic);

    my_iic_start(iic);

    my_iic_write(iic,iic->slave_addr | 0x01);
    my_iic_ack(iic);

    for(i = 0; i < len - 1; i++)
    {
        dat[i] = my_iic_read(iic);
        my_iic_shend_ack(iic,1);
    }

    dat[i] = my_iic_read(iic);
    my_iic_shend_ack(iic,0);

    my_iic_stop(iic);
    __enable_irq();
    return err;
}

static unsigned int my_iic_start(my_iic *iic)
{
    unsigned int err = 0;

    err = my_iic_pin_set_high(iic->sda);
    err = my_iic_pin_set_high(iic->scl);
    delay_5us();
    err = my_iic_pin_set_low(iic->sda);
    delay_5us();
    err = my_iic_pin_set_low(iic->scl);

    return err;
}

static unsigned int my_iic_stop(my_iic *iic)
{
    unsigned int err = 0;

    err = my_iic_pin_set_low(iic->sda);
    err = my_iic_pin_set_high(iic->scl);
    delay_5us();
    err = my_iic_pin_set_high(iic->sda);

    return err;
}

static unsigned char my_iic_ack(my_iic *iic)
{
    ioport_level_t p_pin_value;

    delay_5us();
    my_iic_pin_set_high(iic->sda);
    delay_5us();
    my_iic_pin_set_high(iic->scl);
    delay_5us();

    my_iic_pin_set_input(iic->sda);
    my_iic_pin_read(iic->sda,&p_pin_value);
    my_iic_pin_set_output(iic->sda);


    my_iic_pin_set_low(iic->scl);

    if(p_pin_value == IOPORT_LEVEL_HIGH)
    {
        my_iic_stop(iic);
        xp_log("ack failed\r\n");
    }


    return p_pin_value;
}

static void my_iic_shend_ack(my_iic *iic,unsigned char ack)
{
       delay_5us();
       if(ack == 1)
       {
           my_iic_pin_set_low(iic->sda);
       }
       else
       {
           my_iic_pin_set_high(iic->sda);
       }
       delay_5us();
       my_iic_pin_set_high(iic->scl);
       delay_5us();
       my_iic_pin_set_low(iic->scl);
}


static unsigned int my_iic_write(my_iic *iic,unsigned char dat)
{
    unsigned int err = 0;

    unsigned char index;
    for(index = 0x80;index != 0;index >>= 1)
    {
        if((dat&index) != 0)
        {
            err = my_iic_pin_set_high(iic->sda);
        }
        else
        {
            err = my_iic_pin_set_low(iic->sda);
        }
        delay_5us();
        err = my_iic_pin_set_high(iic->scl);
        delay_5us();
        err = my_iic_pin_set_low(iic->scl);
    }

    return err;
}

static unsigned char my_iic_read(my_iic *iic)
{
    unsigned char index,dat=0;
    ioport_level_t p_pin_value;

    my_iic_pin_set_input(iic->sda);

    for(index = 0x80;index != 0;index >>= 1)
    {
        delay_5us();
        my_iic_pin_set_high(iic->scl);
        delay_5us();

        my_iic_pin_read(iic->sda,&p_pin_value);
        if(p_pin_value == IOPORT_LEVEL_HIGH)
        {
            dat |= index;
        }

        my_iic_pin_set_low(iic->scl);
    }

    my_iic_pin_set_output(iic->sda);

    return dat;
}


static unsigned int my_iic_pin_set_output(ioport_port_pin_t pin)
{
    unsigned int err = 0;

    err = g_ioport.p_api->pinCfg(pin,IOPORT_CFG_PORT_DIRECTION_OUTPUT | IOPORT_CFG_PORT_OUTPUT_HIGH);

    if(SSP_SUCCESS != err)
    {
        xp_log("pin set output failed:[%d]",err);
    }

    return err;
}

static unsigned int my_iic_pin_set_input(ioport_port_pin_t pin)
{
    unsigned int err = 0;

    err = g_ioport.p_api->pinCfg(pin,IOPORT_CFG_PORT_DIRECTION_INPUT);
    if(SSP_SUCCESS != err)
    {
        xp_log("pin set input failed:[%d]",err);
    }
    return err;
}

static unsigned int my_iic_pin_set_high(ioport_port_pin_t pin)
{
    unsigned int err = 0;

    err = g_ioport.p_api->pinWrite(pin,IOPORT_LEVEL_HIGH);

    if(SSP_SUCCESS != err)
    {
        xp_log("pin set high failed:[%d]",err);
    }

    return err;
}

static unsigned int my_iic_pin_set_low(ioport_port_pin_t pin)
{
    unsigned int err = 0;

    err = g_ioport.p_api->pinWrite(pin,IOPORT_LEVEL_LOW);

    if(SSP_SUCCESS != err)
    {
        xp_log("pin set low failed:[%d]",err);
    }

    return err;
}

static unsigned int my_iic_pin_read(ioport_port_pin_t pin,ioport_level_t * p_pin_value)
{
    unsigned int err = 0;

    err = g_ioport.p_api->pinRead(pin,p_pin_value);
    if(SSP_SUCCESS != err)
    {
        xp_log("pin set read failed:[%d]",err);
    }

    return err;
}

static void delay_5us()
{
    R_BSP_SoftwareDelay(4,BSP_DELAY_UNITS_MICROSECONDS);
}
