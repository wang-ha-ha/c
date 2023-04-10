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

#include "layout.h"
#include "crc32.h"

// #define DEBUG

typedef struct
{
    char * name;
    uint32_t offset;
    uint32_t size;
    uint32_t flag;
    uint32_t crc;
    uint32_t env_count;
    uint32_t env_len;
    int dirty_flag;
    unsigned char * data;
} section_t;

typedef struct env_dev_s{
    section_t *sections;
    int section_num;
    struct semaphore sem;
    uint32_t refcnt;
} env_dev_t;

static env_dev_t *g_env_dev;

static int factory_mode = 0;
module_param(factory_mode, int, S_IRUGO);

static int read_from_flash(int from,unsigned char *buf, int len);
static int write_to_flash(int to, char *buf, int len);

static section_t * _get_sections(env_dev_t *dev, const char *section_name)
{
    int i;

    for(i = 0; i < dev->section_num; i++) {
        if(section_name && strcmp(section_name,dev->sections[i].name) == 0) {
            return &dev->sections[i];
        }
    }

    return NULL;
}

static char *getenv(env_dev_t *dev, const char *section_name, const char *key, int no_sema)
{
    char *envret = NULL;
    char *tmp = NULL;
    int envlen = 0;
    section_t * section;

    if ((!no_sema) && down_interruptible(&dev->sem)) {
        return NULL;
    }

    section = _get_sections(dev,section_name);

    if (section == NULL) {
        goto done;
    }

    if (section->data == NULL || section->env_count == 0) {
        goto done;
    }

    envlen = section->env_len;
    tmp = section->data + 4;

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

static int setenv(env_dev_t *dev, const char *section_name, const char *key, const char *value,
          int overwrite)
{
    int ret = 0;
    char *tmp,*env_buf;
    int envlen = 0;
    int old_l, new_l, tmpl;
    section_t * section;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    section = _get_sections(dev,section_name);
    
    if(section == NULL) {
        ret = -ENOENT;
        goto done;
    }

    if (section->flag == ENV_RO ) {
        ret = -EPERM;
        goto done;
    }

    env_buf = tmp = section->data + 4;
    envlen = section->env_len;

    if (envlen + strlen(key) + strlen(value) + 6 > section->size) {
        ret = -ENOMEM;
        goto done;
    }

    section->dirty_flag = 1;

    if (overwrite == 1) {
        tmp = getenv(dev, section_name, key, 1);
        if ((tmp != NULL) && (*(tmp - 1) == '=')) {
            char *p;
            old_l = strlen(tmp); // length of old value
            new_l = strlen(value);
            p = tmp + strlen(tmp); // to find next param
            if (*p == '\0') {
                memmove(tmp + new_l + 1, tmp + old_l + 1,envlen - (tmp + old_l + 1 - env_buf));
                memcpy(tmp, value, strlen(value));
                *(tmp + new_l) = '\0';
                envlen = envlen - old_l + new_l;
                env_buf[envlen] = '\0';
            }
        } else {
            goto setenv_add; // if cann't find key should set as a new env.
        }
    } else {
        tmp = getenv(dev, section_name, key, 1);
        if (tmp != NULL) {
            ret = -1;
            goto done;
        } else {
        setenv_add:
            tmp = env_buf + envlen;
            sprintf(tmp, "%s=%s", key, value);
            tmpl = strlen(tmp) + 1;
            tmp += strlen(tmp);
            *tmp = '\0';
            envlen += tmpl;
            section->env_count++;
        }
    }

    section->env_len = envlen;

done:
    up(&dev->sem);
    return ret;
}

static int unsetenv(env_dev_t *dev, const char *section_name, const char *key)
{
    int ret = 0;
    char *tmp,*env_buf;
    section_t * section;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    #ifdef DEBUG
    printk(KERN_ERR"[env]:unsetenv section %s: key %s!\n",section_name, key);
    #endif
    section = _get_sections(dev,section_name);
    if(section == NULL) {
        ret = -ENOENT;
        goto done;
    }

    if (section->flag == ENV_RO ) {
        ret = -EPERM;
        goto done;
    }

    if (section->data == NULL || section->env_count == 0) {
        goto done;
    }

    env_buf = section->data + 4;

    tmp = getenv(dev, section_name, key, 1);
    if (tmp != NULL) {
        if (*(tmp - 1) == '=') {
            tmp -= (strlen(key) + 1); // point to header
            section->env_len -= (strlen(tmp) + 1);
            memmove(tmp, tmp + strlen(tmp) + 1,
                section->env_len - (tmp - env_buf));
            env_buf[section->env_len] = '\0';
            section->env_count--;
        }
        section->dirty_flag = 1;
    }

done:
    up(&dev->sem);
    return ret;
}

static int clearenv(env_dev_t *dev, const char *section_name)
{
    section_t * section;
    int ret = 0;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    #ifdef DEBUG
    printk(KERN_ERR"[env]:clearenv section %s!\n",section_name);
    #endif

    section = _get_sections(dev,section_name);
    if(section == NULL) {
        ret = -ENOENT;
        goto done;
    }

    if (section->flag == ENV_RO ) {
        ret = -EPERM;
        goto done;
    }

    memset(section->data, 0, section->size);
    section->env_len = 0;
    section->env_count = 0;

    section->dirty_flag = 1;

done:
    up(&dev->sem);
    return ret;
}

static int printenv(env_dev_t *dev, const char *section_name, char *buf, int maxlen)
{
    int cnt;
    int i = 0;
    char *env_buf;
    int envlen;
    int offset;
    int ret = 0;
    section_t * section;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    section = _get_sections(dev,section_name);

    if (section == NULL) {
        ret = -ENOENT;
        goto done;
    }

    if (section->data == NULL || section->env_count == 0) {
        goto done;
    }

    env_buf = section->data + 4;
    envlen = section->env_len;

    offset = snprintf(buf, maxlen, "%s\n", &env_buf[i]);

    cnt = section->env_count - 1;
    for (i = 0; i < envlen; i++) {
        if (env_buf[i] == '\0') {
            if (cnt == 0) {
                break;
            }
            cnt--;
            offset += snprintf(buf + offset, maxlen - offset,
                       "%s\n", &env_buf[i + 1]);
        }
    }

done:
    up(&dev->sem);
    return 0;
}

static int write_to_flash(int to, char *buf, int len)
{
    int ret;
    int total_len;
    int mtd_offset;
    struct erase_info ei;

    struct mtd_info *mtd = get_mtd_device_nm("Env");
    if (IS_ERR(mtd)) {
        printk(KERN_ERR "[env]ERROR %ld: No Env partition!\n", PTR_ERR(mtd));
        return -ENODEV;
    }

    if(len > mtd->size) {
        return -ENOMEM;
    }

    mtd_offset = to;
    total_len = 0;

    while( len > 0) {
        if ((len & (mtd->erasesize-1)) || (len < mtd->erasesize)) {            
            char *bak;
            int write_len;
            int piece_size;
            unsigned int piece, bakaddr;
            bak = kzalloc(mtd->erasesize, GFP_KERNEL);
            
            if(bak == NULL) {
                return -ENOMEM;
            }
            
            bakaddr = mtd_offset & ~(mtd->erasesize - 1);
            
            read_from_flash(bakaddr, bak, mtd->erasesize);

            memset(&ei, 0, sizeof(ei));
            ei.addr = bakaddr;
            ei.mtd = mtd;
            ei.len = mtd->erasesize;
            ret = mtd_erase(mtd, &ei);
            if (ret) {
                printk(KERN_ERR "[env]ERROR:erase error at 0x%llx\n", ei.addr);
                kfree(bak);
                return -EIO;
            }

            piece = mtd_offset & (mtd->erasesize - 1);
            piece_size = min((uint32_t)len, mtd->erasesize - piece);
            memcpy(bak + piece, buf + mtd_offset - to, piece_size);
            write_len = 0;
            
            #ifdef DEBUG
            printk(KERN_ERR "[env]mtd_offset:0x%llx,piece:0x%x piece_size:%d\n", ei.addr,piece,piece_size);
            #endif
            
            while (write_len < mtd->erasesize) {
                int _len;
                #ifdef DEBUG
                printk(KERN_ERR "[env]write_len:0x%x\n", write_len);
                #endif
                ret = mtd_write(mtd, bakaddr + write_len, mtd->erasesize - write_len, &_len,
                        bak + write_len);
                if (ret) {
                     kfree(bak);
                    return -EIO;
                }

                if (_len == 0)
                    break;
                else if (_len < 0) {
                    printk(KERN_ERR"[env]ERROR: write failure\n");
                    kfree(bak);
                    return -EIO;
                }

                total_len += _len;
                write_len += _len;
                len -= _len;
            }

            kfree(bak);
        }
        else {
            int write_len;

            memset(&ei, 0, sizeof(ei));
            ei.addr = mtd_offset & ~(mtd->erasesize - 1);
            ei.mtd = mtd;
            ei.len = mtd->erasesize;
            ret = mtd_erase(mtd, &ei);
            //    if (ret || ei.state == MTD_ERASE_FAILED) {
            if (ret) {
                printk(KERN_ERR "[env]ERROR:erase error at 0x%llx\n", ei.addr);
                return -EIO;
            }

            write_len = 0;

            while (write_len < mtd->erasesize) {
                int _len;
                ret = mtd_write(mtd, mtd_offset, mtd->erasesize - write_len, &_len,
                        buf + write_len);
                if (ret) {
                    return -EIO;
                }

                if (_len == 0)
                    break;
                else if (_len < 0) {
                    printk(KERN_ERR"[env]ERROR: write failure\n");
                    return -EIO;
                }

                total_len += _len;
                write_len += _len;
                mtd_offset += _len;
                len -= _len;
            }
        }
    }

    return total_len;
}

static int read_from_flash(int from,unsigned char *buf, int len)
{
    int read_len;
    int total_len;
    int mtd_offset;
    unsigned char * tmp;

    struct mtd_info *mtd = get_mtd_device_nm("Env");
    if (IS_ERR(mtd)) {
        printk(KERN_ERR "[env]ERROR %ld: No Env partition!\n", PTR_ERR(mtd));
        return -ENODEV;
    }

    read_len = len;
    total_len = 0;
    mtd_offset = from;
    tmp = buf;

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

    return read_len;
}

static int loadenv(env_dev_t *dev)
{
    int ret = 0;
    int i;

    dev->section_num = sizeof(g_env_layout)/sizeof(env_layout_t);
    dev->sections = kzalloc(sizeof(section_t) * dev->section_num, GFP_KERNEL);
    
    if (dev->sections == NULL) {
        return -ENOMEM;
    }

    for(i = 0; i < dev->section_num; i++) {
        uint32_t secrc = 0;
        unsigned char *p = NULL;
        dev->sections[i].name = g_env_layout[i].name;
        dev->sections[i].offset = g_env_layout[i].offset;
        dev->sections[i].size = g_env_layout[i].size;
        #ifdef DEBUG
        printk(KERN_ERR"[env] factory_mode:%d\n",factory_mode);
        #endif
        if(factory_mode == 0) {
            dev->sections[i].flag = g_env_layout[i].flag;
        }
        else {
            dev->sections[i].flag = ENV_WO;
        }
        dev->sections[i].crc = 0;
        dev->sections[i].dirty_flag = 0;
        dev->sections[i].env_count = 0;
        dev->sections[i].env_len = 0;
        dev->sections[i].data = kzalloc(dev->sections[i].size, GFP_KERNEL);
        if (dev->sections[i].data == NULL) {
            return -ENOMEM;
        }

        ret = read_from_flash(dev->sections[i].offset,dev->sections[i].data, dev->sections[i].size);
        if (ret < 0) {
            return ret;
        }

        dev->sections[i].crc = crc32(0, (uint8_t *)(dev->sections[i].data + 4), dev->sections[i].size - 4);
        secrc = *(uint32_t *)dev->sections[i].data;
        
        if (secrc == 0 || secrc != dev->sections[i].crc) {
            dev->sections[i].data[4] = 0;
            printk(KERN_ERR"[env]ERROR: crc error in section %s: expect 0x%x, got 0x%x!\n", dev->sections[i].name, dev->sections[i].crc, secrc);
            continue;
        }

        #ifdef DEBUG
        printk(KERN_ERR"[env]: section %s: size %d!\n",dev->sections[i].name, dev->sections[i].size);
        #endif
        p = dev->sections[i].data + 4;
        while(p < dev->sections[i].data + dev->sections[i].size) {
            int len = 0;

            if(*p < 32 || *p > 128) {
                break;
            }

            len = strlen(p);
            p += len + 1;
            dev->sections[i].env_count++;
            dev->sections[i].env_len += len + 1;
        }
        #ifdef DEBUG
        printk(KERN_ERR"[env]: section %s: env_count %d, env_len %d!\n", dev->sections[i].name, dev->sections[i].env_count, dev->sections[i].env_len);
        #endif
    }

    return ret;
}

static int saveenv(env_dev_t *dev, int no_sema)
{
    int i;
    int ret = 0;
    int env_crc32;

    if ((!no_sema) && down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    for(i = 0; i < dev->section_num; i++) {
        if ( dev->sections[i].flag == ENV_RO ||
            dev->sections[i].dirty_flag == 0 ) {
           continue; 
        }

        #ifdef DEBUG
        printk(KERN_ERR"[env]: saveenv section %s!\n", dev->sections[i].name);
        #endif

        env_crc32 = crc32(0, dev->sections[i].data + 4, dev->sections[i].size - 4);
        memcpy(dev->sections[i].data,(unsigned *)&env_crc32,4);
    
        ret = write_to_flash(dev->sections[i].offset, dev->sections[i].data, dev->sections[i].size);

        dev->sections[i].dirty_flag = 0;
    }

    if (!no_sema) {
        up(&dev->sem);
    }
    return ret;
}

static int env_open(struct inode *inode, struct file *filp)
{
    int ret = 0;

    filp->private_data = (void *)g_env_dev;

    if (down_interruptible(&g_env_dev->sem)) {
        return -ERESTARTSYS;
    }

    if (g_env_dev->refcnt > 0) {
        g_env_dev->refcnt++;
        goto done;
    }
    
    ret = loadenv(g_env_dev);
    if (ret < 0) {
        goto done;
    }

    g_env_dev->refcnt = 1;
    ret = 0;

done:
    up(&g_env_dev->sem);
    return ret;
}

static int env_release(struct inode *inode, struct file *filp)
{
    int i;
    env_dev_t *dev = (env_dev_t *)filp->private_data;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }
    
    saveenv(dev, 1);

    dev->refcnt--;

    if (dev->refcnt == 0) {
        for(i = 0; i < dev->section_num; i++) {
            if (dev->sections[i].data != NULL) {
                kfree(dev->sections[i].data);
            }
        }

        if (dev->sections != NULL) {
            kfree(dev->sections);
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
    char section_name[ENV_NAME_MAXLEN];
    char key[ENV_NAME_MAXLEN];
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

        env_str = getenv(dev, karg.section_name, karg.key, 0);
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
        retval = setenv(dev, karg.section_name, karg.key, value, karg.overwrite);
        kfree(value);
        break;
    }
    case ENV_IOCUNSET: {
        retval = unsetenv(dev, karg.section_name, karg.key);
        break;
    }
    case ENV_IOCCLR: {
        retval = clearenv(dev, karg.section_name);
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
        retval = printenv(dev, karg.section_name, buf, karg.maxlen);
        if (retval == 0 && copy_to_user(karg.buf, buf, karg.maxlen)) {
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

    if (g_env_dev) {
        kfree(g_env_dev);
    }
}

static __init int env_init(void)
{
    int ret = misc_register(&env_miscdev);
    if (ret) {
        ret = -ENODEV;
        goto fail;
    }

    g_env_dev = kzalloc(sizeof(env_dev_t), GFP_KERNEL);
    if (!g_env_dev) {
        ret = -ENOMEM;
        goto fail;
    }

    sema_init(&g_env_dev->sem, 1);

    return 0;

fail:
    env_exit();
    return ret;
}

module_init(env_init);
module_exit(env_exit);

MODULE_LICENSE("GPL");