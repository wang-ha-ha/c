#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>

#define DEBUG

extern int bbu_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag, int lcd);
extern int bbu_mb_spic_trans(const u8 code, const u32 addr, u8 *buf, const size_t n_tx, const size_t n_rx, int flag, int lcd);

#define SPIC_READ_BYTES (1<<0)
#define SPIC_WRITE_BYTES (1<<1)
#define SPIC_DEBUG (1 << 7)
#define FLASH_USE 0

/* Flash opcodes. */
#define OPCODE_WREN        6    /* Write enable */
#define OPCODE_WRDI        4    /* Write disable */
#define OPCODE_RDSR        5    /* Read status register */
#define OPCODE_WRSR        1    /* Write status register */
#define OPCODE_READ        3    /* Read data bytes */
#define OPCODE_PP        2    /* Page program */
#define OPCODE_SE        0xD8    /* Sector erase */
#define OPCODE_RES        0xAB    /* Read Electronic Signature */
#define OPCODE_RDID        0x9F    /* Read JEDEC ID */
#define OPCODE_DOR            0x3B    /* Dual Output Read */
#define OPCODE_QOR            0x6B    /* Quad Output Read */
#define OPCODE_DIOR                     0xBB    /* Dual IO High Performance Read */
#define OPCODE_QIOR                     0xEB    /* Quad IO High Performance Read */


#define SECURITY_REG_START_ADDR        0x1000 /*first security register addr*/
#define SECURITY_REG_ADDR_OFFSET    0x1000 /*diff between consecutive reg*/
#define SECURITY_REG_NUM        3 /* number of security registers */
#define SECURITY_REG_SIZE        256 /* bytes per security register */
#define SECURITY_REG_TOTAL_SIZE        (SECURITY_REG_NUM * SECURITY_REG_SIZE)
#define SPI_NOR_UNIQUE_ID_LEN        8 /*number of bytes of unique ID */

/* SPI FLASH opcodes */
#define SPINOR_OP_RDSR          0x05 /* Read status register 1 */
#define SPINOR_OP_RD_SR2        0x35 /* Read status register 2 */
#define SPINOR_OP_PR_SECURITY_REG    0x42 /* Program security register */
#define SPINOR_OP_ER_SECURITY_REG    0x44 /* Erase security register */
#define SPINOR_OP_RD_SECURITY_REG    0x48 /* Read security register */
#define SPINOR_OP_RD_UNIQUE_ID        0x4B /* Read unique id */

/* Status register 2 */
#define SR2_LB1_BIT            3 /* security register lock bit 1 */

/* Get start addr of the security reg*/
#define SEC_REG_START_ADDR(addr) (addr & 0x3000)

static DEFINE_MUTEX(flash_otp_mutex);

/*
 * Converts address range
 *    0 - 0xFF    -> 0x1000 - 0x10FF
 *    0x100 - 0x1FF    -> 0x2000 - 0x20FF
 *    0x200 - 0x2FF    -> 0x3000 - 0x30FF
 *
 * This func assumes that sanity checks on addr are done and is in valid range
 */
static int translate_addr(int addr)
{
    int i;
    int new_addr = SECURITY_REG_START_ADDR;

    for (i = 0; i < SECURITY_REG_NUM; i++) {
        if (addr < ((i+1)*SECURITY_REG_SIZE)) {
            new_addr |= addr & (SECURITY_REG_SIZE-1);
            break;
        }
        new_addr += SECURITY_REG_ADDR_OFFSET;
    }

    return new_addr;
}

static int raspi_read_rg(u8 code, u8 *val)
{
    return bbu_spic_trans(code, 0, val, 1, 1, SPIC_READ_BYTES, FLASH_USE);
}

static int raspi_write_rg(u8 code, u8 *val)
{
    u32 address = (*val) << 24;
    return bbu_spic_trans(code, address, val, 2, 0, SPIC_WRITE_BYTES, FLASH_USE);
}

static int write_enable()
{
    u8 code = OPCODE_WREN;
    return bbu_spic_trans(code, 0, NULL, 1, 0, 0, FLASH_USE);
}

static int write_disable()
{
    u8 code = OPCODE_WRDI;
    return bbu_spic_trans(code, 0, NULL, 1, 0, 0, FLASH_USE);
}

static int raspi_read_devid(u8 *rxbuf, int n_rx)
{
    u8 code = OPCODE_RDID;
    int retval;

    retval = bbu_spic_trans(code, 0, rxbuf, 1, 3, SPIC_READ_BYTES, FLASH_USE);
    if (!retval)
        retval = n_rx;

    if (retval != n_rx) {
        printk("%s: ret: %x\n", __func__, retval);
        return retval;
    }
    return retval;
}

static void raspi_wait_ready(int timeout)
{
    char sr1;

    do
    {
        raspi_read_rg(0x05,&sr1);
        // printk("sr1:%x\n",sr1);
        msleep(1);
        if(timeout-- <= 0){
            break;
        }
    }while(sr1 & 0x01);
}

static int raspi_otp_read(int from,char * buf,int len)
{
    int rdlen = 0;

    raspi_wait_ready(3);

    do {
        int rc, more;
        more = 32;

        if (len - rdlen <= more) {
            rc = bbu_mb_spic_trans(SPINOR_OP_RD_SECURITY_REG, from, (buf+rdlen), 1, (len-rdlen), SPIC_READ_BYTES, FLASH_USE);

            if (rc != 0) {
                printk("%s: failed\n", __func__);
                break;
            }
            rdlen = len;
        }
        else {
            rc = bbu_mb_spic_trans(SPINOR_OP_RD_SECURITY_REG, from, (buf+rdlen), 1, more, SPIC_READ_BYTES, FLASH_USE);

            if (rc != 0) {
                printk("%s: failed\n", __func__);
                break;
            }
            rdlen += more;
            from += more;
        }
    } while (rdlen < len);

    return rdlen;
}

static int raspi_otp_erase(int sector)
{
    raspi_wait_ready(5);
    write_enable();

    if(sector < 0x100){
        sector = 0x1000;
    }
    else if(sector < 0x200){
        sector = 0x2000;
    }
    else if(sector < 0x300){
        sector = 0x3000;
    }

    if(sector != 0x1000 && sector != 0x2000 && sector != 0x3000){
        printk("sector error:%x\n",sector);
        return -1;
    }

    return bbu_spic_trans(SPINOR_OP_ER_SECURITY_REG, sector, NULL, 4, 0, 0, FLASH_USE);
}

static int raspi_otp_lock(int sector)
{
    int sr2;
    raspi_wait_ready(5);

    if(sector < 0x100){
        sector = 0;
    }
    else if(sector < 0x200){
        sector = 1;
    }
    else if(sector < 0x300){
        sector = 2;
    }
    else{
        return -1;
    }

    raspi_read_rg(0x35,&sr2);
    printk("sr2:%x\n",sr2);

    write_enable();
    sr2 |= (0x08 << sector);
    raspi_write_rg(0x31,&sr2);
    
    raspi_read_rg(0x35,&sr2);
    printk("sr2:%x\n",sr2);
    return 0;
}

static int raspi_otp_write(int wrto,char * buf,int len)
{
    int wrlen = len;
    char *wrbuf = buf;

    raspi_wait_ready(5);
    write_enable();

    do {
        int rc, more;
        more = 32;

        if (wrlen <= more) {
            rc = bbu_mb_spic_trans(SPINOR_OP_PR_SECURITY_REG, wrto, wrbuf, wrlen, 0, SPIC_WRITE_BYTES, FLASH_USE);

            if (rc != 0) {
                printk("%s: failed\n", __func__);
                break;
            }
            wrlen = 0;
        }
        else {
            rc = bbu_mb_spic_trans(SPINOR_OP_PR_SECURITY_REG, wrto, wrbuf, more, 0, SPIC_WRITE_BYTES, FLASH_USE);

            if (rc != 0) {
                printk("%s: failed\n", __func__);
                break;
            }
            wrto += more;
            wrlen -= more;
            wrbuf += more;
        }
        if (wrlen > 0) {
            raspi_wait_ready(10);
            write_enable();
        }
    } while (wrlen > 0);

    return wrlen;
}

static int raspi_otp_erase_write(int reg_addr,int wrto,unsigned char * buf,int len)
{
    int i;
    u8 *kbuf = kmalloc(SECURITY_REG_SIZE, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;
    
#ifdef DEBUG
    printk("reg_addr:%x wrto:%x,len:%d\n",reg_addr,wrto,len);
#endif

    raspi_otp_read(reg_addr,kbuf,SECURITY_REG_SIZE);

    raspi_otp_erase(reg_addr);

    memcpy(kbuf + wrto - reg_addr, buf, len);

    raspi_otp_write(reg_addr,kbuf,SECURITY_REG_SIZE);

    kfree(kbuf);
    return 0;
}

static int flash_otp_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int flash_otp_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t flash_otp_read_test(struct file *file, char __user *buff, size_t count, int *offp)
{
    u8 code = 0x4b;
    int retval,i;
    int temp_addr;
    char sr1;
    int len;

    u8 *unid = kmalloc(SECURITY_REG_SIZE, GFP_KERNEL);
    if (!unid)
        return -ENOMEM;

    retval = bbu_mb_spic_trans(code, 0, unid, 1, 12, SPIC_READ_BYTES, FLASH_USE);

    printk(KERN_ERR"WCQ====retval:%d unique_id:%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",retval,unid[0],unid[1],unid[2],unid[3],unid[4],unid[5],unid[6],unid[7],unid[8],unid[9],unid[10],unid[11]);

     temp_addr = translate_addr(0);

    raspi_otp_read(temp_addr,unid,128);
    for(i = 0;i < 128 ;i++){
        printk("%02x ",unid[i]);
        if(i != 0 && i % 15 == 0){
            printk("\n");
        }
    }

    printk("\n-------------:%x--------------\n",temp_addr);

    temp_addr = translate_addr(0x100);

    raspi_otp_read(temp_addr,unid,128);
    for(i = 0;i < 128 ;i++){
        printk("%02x ",unid[i]);
        if(i != 0 && i % 15 == 0){
            printk("\n");
        }
    }

    printk("\n-------------:%x--------------\n",temp_addr);

    temp_addr = translate_addr(0x200);

    raspi_otp_read(temp_addr,unid,128);
    for(i = 0;i < 128 ;i++){
        printk("%02x ",unid[i]);
        if(i != 0 && i % 15 == 0){
            printk("\n");
        }
    }

    printk("\n-------------:%x--------------\n",temp_addr);

    struct mtd_info *mtd;

    mtd = get_mtd_device_nm("Factory");
    if(mtd != NULL){
        mtd->_read(mtd,0x00000,128,&retval,unid);
        for(i = 0;i < 128 ;i++){
            printk("%02x ",unid[i]);
            if(i != 0 && i % 15 == 0){
                printk("\n");
            }
        }

        printk("\n-------------:%x--------------\n",temp_addr);
    }

    kfree(unid);
    return 0;
}

static ssize_t flash_otp_write_test(struct file *file,const  char __user *buff, size_t count, int *offp)
{
    u8 buf[5];
    u8 code = 0x4b;
    int retval,i;
    int temp_addr;
    char sr1,sr2,sr3;

    u8 *unid = kmalloc(SECURITY_REG_SIZE, GFP_KERNEL);
    if (!unid)
        return -ENOMEM;

    retval = bbu_mb_spic_trans(code, 0, unid, 1, 8, SPIC_READ_BYTES, FLASH_USE);

    printk(KERN_ERR"WCQ====retval:%d unique_id:%02x %02x %02x %02x %02x %02x %02x %02x\n",retval,unid[0],unid[1],unid[2],unid[3],unid[4],unid[5],unid[6],unid[7]);

    retval = raspi_read_devid(buf, 3);

    printk(KERN_ERR"flash_otp_init\n");
    printk("retval:%d deice id : %x %x %x \n",retval, buf[0], buf[1], buf[2]);

    retval = bbu_mb_spic_trans(OPCODE_RDID, 0, buf, 0, 3, SPIC_READ_BYTES, FLASH_USE);
    printk("retval:%d deice id : %x %x %x \n",retval, buf[0], buf[1], buf[2]);

    code = 0x05;
    raspi_read_rg(code,&sr1);
    code = 0x35;
    raspi_read_rg(code,&sr2);
    code = 0x15;
    raspi_read_rg(code,&sr3);

    printk("sr1:%x sr2:%x sr3:%x \n",sr1,sr2,sr3);

    write_enable();
    sr2 &= (~0x08);
    code = 0x31;
    raspi_write_rg(code,&sr2);

    code = 0x05;
    raspi_read_rg(code,&sr1);
    code = 0x35;
    raspi_read_rg(code,&sr2);
    code = 0x15;
    raspi_read_rg(code,&sr3);

    printk("sr1:%x sr2:%x sr3:%x \n",sr1,sr2,sr3);

    raspi_otp_erase(0);

    for(i = 0;i < 128;i++)
    {
        unid[i] = i + 6;
    }

    temp_addr = translate_addr(0);
    raspi_otp_write(temp_addr, unid,128);

    raspi_otp_erase(0x100);

    for(i = 0;i < 128;i++)
    {
        unid[i] = i + 7;
    }

    temp_addr = translate_addr(0x100);
    raspi_otp_write(temp_addr, unid,128);

    raspi_otp_erase(0x200);

    for(i = 0;i < 128;i++)
    {
        unid[i] = i + 8;
    }

    temp_addr = translate_addr(0x200);
    raspi_otp_write(temp_addr, unid,128);

    kfree(unid);

    return count;
}

static ssize_t flash_otp_read(struct file *file, char __user *buff, size_t count, loff_t *offp)
{
    int ret = 0;
    u32 i, read_len = 0, end_addr = 0, sreg_offset = 0,from = 0;
    int temp_addr = 0,total_retlen = 0;
    unsigned char *kbuf = 0;

#ifdef DEBUG
    printk("*offp:%x,count:%d\n",(int)*offp,count);
#endif

    if(*offp >= SECURITY_REG_TOTAL_SIZE)
        return -EINVAL;

    if (*offp + count > SECURITY_REG_TOTAL_SIZE)
        count = SECURITY_REG_TOTAL_SIZE - *offp;

    if(count == 0)
        return 0;

    mutex_lock(&flash_otp_mutex);
    kbuf = kmalloc(SECURITY_REG_SIZE, GFP_KERNEL);
    if (!kbuf){
        mutex_unlock(&flash_otp_mutex);
        return -ENOMEM;
    }

    from = *offp;
    end_addr = *offp + count;

    for (i = from; i < end_addr; i += read_len) {
        sreg_offset = i & (SECURITY_REG_SIZE-1);
        /* if offset not on boundary, read first few bytes */
        if (sreg_offset) {
            /* check if everything has to be read from 1 reg */
            if ((sreg_offset + count) <= SECURITY_REG_SIZE)
                read_len = count;
            else
                read_len = SECURITY_REG_SIZE - sreg_offset;
        }
        /* if it is last chunk, read the remaining bytes */
        else if ((end_addr - i) < SECURITY_REG_SIZE)
            read_len = end_addr - i;
        else
            read_len = SECURITY_REG_SIZE;

        temp_addr = translate_addr(i);
        ret = raspi_otp_read(temp_addr,kbuf,read_len);
        if (copy_to_user(buff, kbuf, read_len)) {
            kfree(kbuf);
            mutex_unlock(&flash_otp_mutex);
            return -EFAULT;
        }        
 
    #ifdef DEBUG       
        printk("read:*offp:%x,end_addr,%x,temp_addr:%x,%d,i:%d\n",(int)*offp,end_addr,temp_addr,read_len,i);
    #endif

        total_retlen += read_len;
        buff += read_len;
    }

    *offp = end_addr;

#ifdef DEBUG
    printk("read end:*offp:%x,end_addr,%x,temp_addr:%x,%d\n",(int)*offp,end_addr,temp_addr,total_retlen);
#endif

    kfree(kbuf);
    mutex_unlock(&flash_otp_mutex);
    return total_retlen;
}

static ssize_t flash_otp_write(struct file *file,const  char __user *buff, size_t count, loff_t *offp)
{
    int ret = 0;
    u32 i, write_len = 0, end_addr = 0, sreg_offset = 0,wrto = 0;
    int temp_addr = 0,total_retlen = 0;
    unsigned char *kbuf = 0;

#ifdef DEBUG
    printk("*offp:%x,count:%d\n",(int)*offp,count);
#endif

    if(*offp >= SECURITY_REG_TOTAL_SIZE)
        return -EINVAL;

    if (*offp + count > SECURITY_REG_TOTAL_SIZE)
        count = SECURITY_REG_TOTAL_SIZE - *offp;

    if(count == 0)
        return 0;

    mutex_lock(&flash_otp_mutex);
    kbuf = kmalloc(SECURITY_REG_SIZE, GFP_KERNEL);
    if (!kbuf){
        mutex_unlock(&flash_otp_mutex);
        return -ENOMEM;
    }

    wrto = *offp;
    end_addr = *offp + count;    

    for (i = wrto; i < end_addr; i += write_len) {
        sreg_offset = i & (SECURITY_REG_SIZE-1);
        /* if offset not on boundary, read first few bytes */
        if (sreg_offset) {
            /* check if everything has to be read from 1 reg */
            if ((sreg_offset + count) <= SECURITY_REG_SIZE)
                write_len = count;
            else
                write_len = SECURITY_REG_SIZE - sreg_offset;
        }
        /* if it is last chunk, read the remaining bytes */
        else if ((end_addr - i) < SECURITY_REG_SIZE)
            write_len = end_addr - i;
        else
            write_len = SECURITY_REG_SIZE;

        temp_addr = translate_addr(i);
        
        if (copy_from_user(kbuf, buff, write_len)) {
            kfree(kbuf);
            mutex_unlock(&flash_otp_mutex);
            return -EFAULT;
        }     
        raspi_otp_erase_write(SEC_REG_START_ADDR(temp_addr),temp_addr,kbuf,write_len);   

    #ifdef DEBUG
        printk("write:*offp:%x,end_addr,%x,temp_addr:%x,%d,i:%d\n",(int)*offp,end_addr,temp_addr,write_len,i);
    #endif

        total_retlen += write_len;
        buff += write_len;
    }

    *offp = end_addr;

#ifdef DEBUG
    printk("write end:*offp:%x,end_addr,%x,temp_addr:%x,%d\n",(int)*offp,end_addr,temp_addr,total_retlen);
#endif

    kfree(kbuf);
    mutex_unlock(&flash_otp_mutex);

    return total_retlen;
}

static loff_t flash_otp_llseek(struct file *file,loff_t offset,int orig)
{
#ifdef DEBUG
    printk("offset:%d orig:%d\n",(int)offset,orig);
#endif

    switch (orig) {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            offset += file->f_pos;
            break;
        case SEEK_END:
            offset += SECURITY_REG_TOTAL_SIZE;
            break;
        default:
            return -EINVAL;
    }

    if (offset >= 0 && offset <= SECURITY_REG_TOTAL_SIZE)
        return file->f_pos = offset;

    return -EINVAL;
}

#define FLASHOTPLOCK		_IOR('M', 1, int)

static long flash_otp_unlocked_ioctl(struct file *file, u_int cmd, u_long arg)
{
    long ret =0;
    void __user *argp = (void __user *)arg;

    mutex_lock(&flash_otp_mutex);
    
    switch (cmd) {
        case FLASHOTPLOCK:{
            int sector;
            if (copy_from_user(&sector, argp, sizeof(int)))
                return -EFAULT;
            
            if(raspi_otp_lock(sector) != 0){
                return -EINVAL;
            }

            break;
        }
        default:
            ret = -ENOTTY;
    }

    mutex_unlock(&flash_otp_mutex);
    return ret;
}

static struct file_operations flash_otp_fops = {
    .owner = THIS_MODULE,
    .open  = flash_otp_open,
    .release = flash_otp_release,
    .read  = flash_otp_read,
    .write = flash_otp_write,
    .llseek = flash_otp_llseek,
    .unlocked_ioctl	= flash_otp_unlocked_ioctl,
};

static struct miscdevice flash_otp_miscdev = {
    .minor        = MISC_DYNAMIC_MINOR,
    .name        = "flash_otp",
    .fops        = &flash_otp_fops,
};
static void flash_otp_exit(void)
{
    printk(KERN_ERR"flash_otp_exit\n");
    misc_deregister(&flash_otp_miscdev);
}

static __init int flash_otp_init(void)
{
    u8 buf[5];

    int ret = misc_register(&flash_otp_miscdev);
    if (ret) {
        ret = -ENODEV;
        goto fail;
    }


    raspi_read_devid(buf, 3);

    printk(KERN_ERR"flash_otp_init\n");
    printk("deice id : %x %x %x \n", buf[0], buf[1], buf[2]);

    return 0;

fail:
    flash_otp_exit();
    return ret;
}

module_init(flash_otp_init);
module_exit(flash_otp_exit);

MODULE_AUTHOR("wangchuanqi <wangchuanqi@70mai.com");
MODULE_DESCRIPTION("Winbond otp operation");
MODULE_LICENSE("GPL");
