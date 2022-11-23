#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>

/* 布局
0-1:   magic
2-3:   len (只是env的长度)
4-n:   env (字符串形式的kv键值对，每个键值对以0结尾)
n-n+2: crc16 (计算 magic len env)
*/

#define ENV_MAXLEN 0x1000

typedef struct env_dev_s{
    char *buf;
    char *env_buf;
    uint32_t env_len;
    uint32_t max_len;
    uint32_t cnt;

    struct semaphore sem;
    uint32_t refcnt;
    int dirty_flag;
} env_dev_t;

static env_dev_t *env_dev;

static char *getenv(env_dev_t *dev, const char *key, int no_sema)
{
    char *envret = NULL;
    char *tmp;
    int envlen;

    if ((!no_sema) && down_interruptible(&dev->sem)) {
        return NULL;
    }

    if (dev->env_buf == NULL) {
        goto done;
    }

    tmp = dev->env_buf;
    envlen = dev->env_len;
    while (envlen > 0) {
        int ret = strncmp(tmp, key, strlen(key));
        if (ret == 0 && (*(tmp + strlen(key)) == '=')) {
            tmp += (strlen(key) + 1); // point to value
            envret = tmp;
            goto done;
        }
        envlen -= (strlen(tmp) + 1); // point to next key
        tmp += (strlen(tmp) + 1);
    }

done:
    if (!no_sema) {
        up(&dev->sem);
    }
    return envret;
}

static int setenv(env_dev_t *dev, const char *name, const char *value,
          int overwrite)
{
    int ret = 0;
    char *tmp;
    int old_l, new_l, tmpl;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    if (dev->env_buf == NULL) {
        ret = -1;
        goto done;
    }

    if (dev->env_len + strlen(name) + strlen(value) + 2 > dev->max_len) {
        ret = -ENOMEM;
        goto done;
    }

    dev->dirty_flag = 1;

    if (overwrite == 1) {
        tmp = getenv(dev, name, 1);
        if ((tmp != NULL) && (*(tmp - 1) == '=')) {
            char *p;
            old_l = strlen(tmp); // length of old value
            new_l = strlen(value);
            p = tmp + strlen(tmp); // to find next param
            if (*p == '\0') {
                memmove(tmp + new_l + 1, tmp + old_l + 1,
                    dev->env_len -
                        (tmp + old_l + 1 - dev->env_buf));
                memcpy(tmp, value, strlen(value));
                *(tmp + new_l) = '\0';
                dev->env_len = dev->env_len - old_l + new_l;
                dev->env_buf[dev->env_len] = '\0';
            }
        } else {
            goto setenv_add; // if cann't find name should set as a new env.
        }
    } else {
        tmp = getenv(dev, name, 1);
        if (tmp != NULL) {
            ret = -1;
            goto done;
        } else {
        setenv_add:
            tmp = dev->env_buf + dev->env_len;
            sprintf(tmp, "%s=%s", name, value);
            tmpl = strlen(tmp) + 1;
            tmp += strlen(tmp);
            *tmp = '\0';
            dev->env_len += tmpl;
            dev->cnt++;
            goto done;
        }
    }

done:
    up(&dev->sem);
    return ret;
}

static int unsetenv(env_dev_t *dev, const char *name)
{
    int ret = 0;
    char *tmp;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    if (dev->env_buf == NULL) {
        ret = -1;
        goto done;
    }

    tmp = getenv(dev, name, 1);
    if (tmp != NULL) {
        if (*(tmp - 1) == '=') {
            tmp -= (strlen(name) + 1); // point to header
            dev->env_len -= (strlen(tmp) + 1);
            memmove(tmp, tmp + strlen(tmp) + 1,
                dev->env_len - (tmp - dev->env_buf));
            dev->env_buf[dev->env_len] = '\0';
            dev->cnt--;
        }
    }

    dev->dirty_flag = 1;

done:
    up(&dev->sem);
    return ret;
}

static int clearenv(env_dev_t *dev)
{
    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    if (dev->env_buf != NULL) {
        memset(dev->env_buf, 0, dev->env_len);
    }
    dev->env_len = 0;
    dev->cnt = 0;

    dev->dirty_flag = 1;

    up(&dev->sem);
    return 0;
}

static int printenv(env_dev_t *dev, char *buf, int maxlen)
{
    int cnt;
    int i = 0;
    int offset;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    if (dev->env_buf == NULL || dev->cnt == 0) {
        goto done;
    }

    offset = snprintf(buf, maxlen, "%s\n", &dev->env_buf[i]);

    cnt = dev->cnt - 1;
    for (i = 0; i < dev->env_len; i++) {
        if (dev->env_buf[i] == '\0') {
            if (cnt == 0) {
                break;
            }
            cnt--;
            offset += snprintf(buf + offset, maxlen - offset,
                       "%s\n", &dev->env_buf[i + 1]);
        }
    }

done:
    up(&dev->sem);
    return 0;
}

static const unsigned short crc16tab[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108,
    0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
    0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b,
    0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee,
    0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
    0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d,
    0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5,
    0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
    0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4,
    0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
    0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13,
    0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
    0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e,
    0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1,
    0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
    0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0,
    0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657,
    0x7676, 0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
    0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882,
    0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e,
    0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
    0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d,
    0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
    0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

static unsigned short crc16_ccitt(const void *buf, int len)
{
    register int counter;
    register unsigned short crc = 0;
    for (counter = 0; counter < len; counter++)
        crc = (crc << 8) ^
              crc16tab[((crc >> 8) ^ *(char *)buf++) & 0x00FF];
    return crc;
}

static int write_to_flash(char *buf, int env_len)
{
    int ret;
    int buf_len;
    int total_len;
    int mtd_offset;
    int env_crc16;
    struct erase_info ei;

    struct mtd_info *mtd = get_mtd_device_nm("Env");
    if (IS_ERR(mtd)) {
        printk(KERN_ERR "[env]ERROR %ld: No Env partition!\n", PTR_ERR(mtd));
        return -ENODEV;
    }

    buf[0] = 0xA5;
    buf[1] = 0x5A;
    buf[2] = env_len >> 8;
    buf[3] = env_len & 0xff;

    env_crc16 = crc16_ccitt(buf,env_len + 4);

    buf[env_len + 4 ] = env_crc16 >> 8;
    buf[env_len + 4 + 1] = env_crc16 & 0xff;

    memset(&ei, 0, sizeof(ei));
    ei.addr = 0;
    ei.mtd = mtd;
    ei.len = ((mtd->erasesize + env_len + 6 ) / mtd->erasesize) * mtd->erasesize;
    printk(KERN_ERR "[env]11ei.len:%lld env_len:%d env_crc16:0x%x\n", ei.len,env_len,env_crc16);
    ret = mtd_erase(mtd, &ei);
    //    if (ret || ei.state == MTD_ERASE_FAILED) {
    if (ret) {
        printk(KERN_ERR "[env]ERROR:erase error at 0x%llx\n", ei.addr);
        return -EIO;
    }

    total_len = 0;
    buf_len = env_len + 4 + 2;
    mtd_offset = 0;

    while (total_len < buf_len) {
        int len;
        ret = mtd_write(mtd, mtd_offset, buf_len - total_len, &len,
                buf + total_len);
        if (ret) {
            return -EIO;
        }

        if (len == 0)
            break;
        else if (len < 0) {
            printk(KERN_ERR
                   "[env]ERROR: write failure\n");
            return -EIO;
        }
        total_len += len;
        mtd_offset += len;
    }

    return total_len;
}

static int read_from_flash(unsigned char *buf, int maxlen)
{
    int ret;
    int retlen;
    int env_len;

    int read_len;
    int total_len;
    int mtd_offset;
    int env_crc16;

    unsigned char * tmp;

    struct mtd_info *mtd = get_mtd_device_nm("Env");
    if (IS_ERR(mtd)) {
        printk(KERN_ERR "[env]ERROR %ld: No Env partition!\n", PTR_ERR(mtd));
        return -ENODEV;
    }

    // 读前4个字节获取magic和env长度
    ret = mtd_read(mtd, 0, 4, &retlen, buf);
    if (ret || retlen != 4) {
        printk(KERN_ERR
               "[env]ERROR: Cannot read para header ret = %d,retlen = %d, %d\n",
               ret, retlen, maxlen);
        return -EIO;
    }

    if(buf[0] != 0xA5 || buf[1] != 0x5A) {
        printk(KERN_ERR
               "[env]ERROR: para header magic number. (Expect 0xA55A, read 0x%02X%02X)\n",
               (int)buf[0],(int)buf[1]);
        goto format;
    }

    env_len = ((int)buf[2] << 8) | buf[3];

    if (env_len > mtd->size) {
        printk(KERN_ERR "[env]ERROR: Para header length is not valid\n");
        goto format;
    }

    read_len = env_len + 2;
    total_len = 0;
    mtd_offset = 4;
    tmp = buf + 4;

    while (total_len < read_len) {
        int len;
        int ret = mtd_read(mtd, mtd_offset, read_len - total_len, &len,
                   tmp + total_len);
        if (ret) {
            return -EIO;
        }

        if (len == 0)
            break;
        else if (len < 0) {
            printk(KERN_ERR "[env]ERROR: read failure\n");
            return -EIO;
        }
        total_len += len;
        mtd_offset += len;
    }

    total_len += 4;
    env_crc16 = ((int)buf[total_len - 2] << 8) | buf[total_len - 1];

    if(env_crc16 != crc16_ccitt(buf,env_len + 4))
    {
        printk(KERN_ERR "[env]ERROR:env crc error\n");
        goto format;
    }

    return env_len;
format:
    write_to_flash(buf,0);
    return -150;
}

static int loadenv(env_dev_t *dev, char *buf, int maxlen)
{
    int ret = 0;
    int i;

    if (maxlen <= 0) {
        return -1;
    }

    dev->cnt = 0;

    ret = read_from_flash(buf, maxlen);
    if (ret < 0) {
        return ret;
    }

    dev->max_len = maxlen - 4 - 2;

    dev->buf = buf;
    dev->env_buf = buf + 4;
    dev->env_len = ret;

    for (i = 0; i < dev->env_len; i++) {
        if (dev->env_buf[i] == '\0') {
            dev->cnt++;
        }
    }

    dev->dirty_flag = 0;

    return ret;
}

static int saveenv(env_dev_t *dev, int no_sema)
{
    int ret = 0;

    if ((!no_sema) && down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    if (dev->env_buf == NULL) {
        ret = -1;
        goto done;
    }

    ret = write_to_flash(dev->buf, dev->env_len);

    dev->dirty_flag = 0;

done:
    if (!no_sema) {
        up(&dev->sem);
    }
    return ret;
}

static int env_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    char *buf;

    filp->private_data = (void *)env_dev;

    if (down_interruptible(&env_dev->sem)) {
        return -ERESTARTSYS;
    }

    if (env_dev->refcnt > 0) {
        env_dev->refcnt++;
        goto done;
    }

    buf = kzalloc(ENV_MAXLEN, GFP_KERNEL);
    if (buf == NULL) {
        ret = -ENOMEM;
        goto done;
    }

    ret = loadenv(env_dev, buf, ENV_MAXLEN);
    if (ret < 0) {
        goto done;
    }

    env_dev->refcnt = 1;
    ret = 0;

done:
    up(&env_dev->sem);
    return ret;
}

static int env_release(struct inode *inode, struct file *filp)
{
    env_dev_t *dev = (env_dev_t *)filp->private_data;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    if (dev->dirty_flag) {
        saveenv(dev, 1);
    }

    dev->refcnt--;

    if (dev->refcnt == 0) {
        if (dev->buf) {
            kfree(dev->buf);
        }
    }

    up(&dev->sem);
    return 0;
}

#define ENV_IOC_MAGIC 'e'
#define ENV_IOC_MAXNR 5
#define ENV_IOCGET _IOR(ENV_IOC_MAGIC, 0, unsigned long)
#define ENV_IOCSET _IOW(ENV_IOC_MAGIC, 1, unsigned long)
#define ENV_IOCUNSET _IOW(ENV_IOC_MAGIC, 2, unsigned long)
#define ENV_IOCCLR _IOW(ENV_IOC_MAGIC, 3, unsigned long)
#define ENV_IOCPRT _IOR(ENV_IOC_MAGIC, 4, unsigned long)
#define ENV_IOCSAVE _IOR(ENV_IOC_MAGIC, 5, unsigned long)

#define ENV_NAME_MAXLEN 64
typedef struct env_ioctl_args {
    char name[ENV_NAME_MAXLEN];
    char *buf;
    int maxlen;
    int overwrite;
} env_ioctl_args_t;

static long env_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int retval = 0;
    env_dev_t *dev = (env_dev_t *)filp->private_data;
    env_ioctl_args_t __user *uarg = (env_ioctl_args_t __user *)arg;
    env_ioctl_args_t karg;

    if (_IOC_TYPE(cmd) != ENV_IOC_MAGIC)
        return -ENOTTY;
    if (_IOC_NR(cmd) > ENV_IOC_MAXNR)
        return -ENOTTY;

    memset(&karg, 0, sizeof(karg));

    if (uarg) {
        if (copy_from_user(&karg, uarg, sizeof(karg))) {
            return -EFAULT;
        }
    }

    switch (cmd) {
    case ENV_IOCGET: {
        char *env_str;

        if (karg.buf == NULL || karg.maxlen <= 0) {
            return -EINVAL;
        }

        env_str = getenv(dev, karg.name, 0);
        if (env_str == NULL) {
            return -ENODATA;
        }

        if (strlen(env_str) + 1 > karg.maxlen) {
            return -ENOBUFS;
        }

        if (copy_to_user(karg.buf, env_str, karg.maxlen)) {
            return -EFAULT;
        }

        break;
    }
    case ENV_IOCSET: {
        char *value;
        if (karg.buf == NULL || karg.maxlen <= 0) {
            return -EINVAL;
        }
        value = kzalloc(karg.maxlen, GFP_KERNEL);
        if (!value) {
            return -ENOMEM;
        }
        if (copy_from_user(value, karg.buf, karg.maxlen)) {
            kfree(value);
            return -EFAULT;
        }
        retval = setenv(dev, karg.name, value, karg.overwrite);
        kfree(value);
        break;
    }
    case ENV_IOCUNSET: {
        retval = unsetenv(dev, karg.name);
        break;
    }
    case ENV_IOCCLR: {
        retval = clearenv(dev);
        break;
    }
    case ENV_IOCPRT: {
        char *buf;
        if (karg.buf == NULL || karg.maxlen <= 0) {
            return -EINVAL;
        }
        buf = kzalloc(karg.maxlen, GFP_KERNEL);
        if (!buf) {
            return -ENOMEM;
        }
        retval = printenv(dev, buf, karg.maxlen);
        if (copy_to_user(karg.buf, buf, karg.maxlen)) {
            kfree(buf);
            return -EFAULT;
        }
        kfree(buf);
        break;
    }
    case ENV_IOCSAVE: {
        retval = saveenv(dev, 0);
        break;
    }
    default:
        return -ENOTTY;
    }

    return retval;
}

char *kernel_env_get(const char *key)
{
    return getenv(env_dev, key, 1);
}
EXPORT_SYMBOL(kernel_env_get);

static struct file_operations env_fops = {
    .open = env_open,
    .unlocked_ioctl = env_ioctl,
    .release = env_release,
};

static struct miscdevice env_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "env",
    .fops = &env_fops,
};

static void env_exit(void)
{
    misc_deregister(&env_miscdev);

    if (env_dev) {
        kfree(env_dev);
    }
}

static __init int env_init(void)
{
    int ret = misc_register(&env_miscdev);
    if (ret) {
        ret = -ENODEV;
        goto fail;
    }

    env_dev = kzalloc(sizeof(env_dev_t), GFP_KERNEL);
    if (!env_dev) {
        ret = -ENOMEM;
        goto fail;
    }

    sema_init(&env_dev->sem, 1);

    return 0;

fail:
    env_exit();
    return ret;
}

module_init(env_init);
module_exit(env_exit);

MODULE_LICENSE("GPL");