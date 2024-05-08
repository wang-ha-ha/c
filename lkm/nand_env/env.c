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
#include <linux/delay.h>
#include "layout.h"
#include "crc32.h"

// #define DEBUG
#define MTD_NAME "Env"
#define ENV_BLOCK_SIZE 0x20000
#define FIRST_BLOCK 0
#define FIRST_BAK_BLOCK 4
#define FIRST_BLOCK_OFFSET (FIRST_BLOCK * ENV_BLOCK_SIZE)
#define FIRST_BAK_BLOCK_OFFSET (FIRST_BAK_BLOCK * ENV_BLOCK_SIZE)

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
    int locked;
    char name[16];
    struct mtd_info *mtd;
    uint32_t sections_crc_flag;
    uint32_t base_offset;
    uint32_t start_offset;
} env_dev_t;

static env_dev_t *g_env_dev;
static env_dev_t *g_env_bak_dev;

static int g_badblocks[8] = { 0 };

static int read_from_flash(int from,unsigned char *buf, int len);
static int write_to_flash(int to, char *buf, int len);
static int markbad_flash(int from);
// static void erase_callback (struct erase_info *self);

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
    int ret = 0,i;
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
                if(old_l > new_l) {
                    for(i = envlen;(i < envlen + old_l)&&(i < section->size);i++) {
                        env_buf[i] = '\0';
                    }
                }
                section->dirty_flag = 1;
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
            section->dirty_flag = 1;
        }
    }

    section->env_len = envlen;

done:
    up(&dev->sem);
    return ret;
}

static int unsetenv(env_dev_t *dev, const char *section_name, const char *key)
{
    int ret = 0,i,size = 0;
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
            size = (strlen(tmp) + 1);
            section->env_len -= size;
            memmove(tmp, tmp + size,
                section->env_len - (tmp - env_buf));
            env_buf[section->env_len] = '\0';
            #ifdef DEBUG
            printk("env:%d,%d\n", section->env_len,size);
            #endif
            for(i = section->env_len;(i < section->env_len + size)&&(i < section->size);i++) {
                env_buf[i] = '\0';
            }
            section->env_count--;
            section->dirty_flag = 1;
        }
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

    struct mtd_info *mtd = get_mtd_device_nm(MTD_NAME);
    if (IS_ERR(mtd)) {
        printk(KERN_ERR "[env]ERROR %ld: No %s partition!\n", PTR_ERR(mtd),MTD_NAME);
        return -ENODEV;
    }

    if(len > mtd->size) {
        return -ENOMEM;
    }

    mtd->flags |= MTD_WRITEABLE;

    mtd_offset = to;
    total_len = 0;

    while( len > 0) {
        bool bFree = false;
        uint8_t *pBuf = NULL;
        int offset = mtd_offset & (mtd->erasesize - 1);
        int write_len = len > mtd->erasesize - offset ? mtd->erasesize - offset : len;
        uint32_t addr = mtd_offset & ~(mtd->erasesize - 1);
        uint32_t _len = 0;

        //read
        if (offset || write_len != mtd->erasesize) {
            pBuf = kzalloc(mtd->erasesize, GFP_KERNEL);
            if (pBuf == NULL) {
                ret = -ENOMEM;
                goto error;
            }
            read_from_flash(addr, pBuf, mtd->erasesize);
            memcpy(pBuf + offset, buf + total_len, write_len);
            bFree = true;
        } else {
            pBuf = buf + total_len;
            bFree = false;
        }

        //erase
        memset(&ei, 0, sizeof(ei));
        ei.addr = addr;
        ei.mtd = mtd;
        ei.len = mtd->erasesize;
        ret = mtd_erase(mtd, &ei);
        if (!ret) {
            set_current_state(TASK_UNINTERRUPTIBLE);
            add_wait_queue(&waitq, &wait);
            if (ei.state != MTD_ERASE_DONE &&
                ei.state != MTD_ERASE_FAILED)
                schedule();
            remove_wait_queue(&waitq, &wait);
            set_current_state(TASK_RUNNING);

            ret = (ei.state == MTD_ERASE_FAILED)?-EIO:0;
        }
        if (ret) {
            printk(KERN_ERR "[env]ERROR:erase error at 0x%llx.ret:%d\n", ei.addr,ret);
            if (bFree) {
                kfree(pBuf);
            }
            ret = -EIO;
            goto error;
        }

        int current_total_write_len = 0;
        //write
        while (current_total_write_len < mtd->erasesize) {
            ret = mtd_write(mtd, addr + current_total_write_len, mtd->erasesize - current_total_write_len, &_len, pBuf + current_total_write_len);
            if (ret) {
                if (bFree) {
                    kfree(pBuf);
                }
                ret = -EIO;
                goto error;
            }
            if (_len == 0) {
                break;
            }
            else if (_len < 0) {
                printk(KERN_ERR"[env]ERROR: write failure\n");
                if (bFree) {
                    kfree(pBuf);
                }
                ret = -EIO;
                goto error;
            }
            current_total_write_len += _len;
        }

        if (bFree) {
            kfree(pBuf);
        }
        if (current_total_write_len != mtd->erasesize) {
            ret = -EIO;
            goto error;
        }
        //update
        mtd_offset += write_len;
        len -= write_len;
        total_len += write_len;
    }

    mtd->flags &= ~MTD_WRITEABLE;
    return total_len;

error:
    mtd->flags &= ~MTD_WRITEABLE;
    return ret;
}

static int read_from_flash(int from,unsigned char *buf, int len)
{
    int read_len;
    int total_len;
    int mtd_offset;
    unsigned char * tmp;

    struct mtd_info *mtd = get_mtd_device_nm(MTD_NAME);
    if (IS_ERR(mtd)) {
        printk(KERN_ERR "[env]ERROR %ld: No %s partition!\n", PTR_ERR(mtd),MTD_NAME);
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

static int markbad_flash(int from)
{
    struct mtd_info *mtd = get_mtd_device_nm(MTD_NAME);
    if (IS_ERR(mtd)) {
        printk(KERN_ERR "[env]ERROR %ld: No %s partition!\n", PTR_ERR(mtd),MTD_NAME);
        return -ENODEV;
    }
    mtd->flags |= MTD_WRITEABLE;
    int ret = mtd_block_markbad(mtd,from);
    mtd->flags &= ~MTD_WRITEABLE;
    g_badblocks[from/ENV_BLOCK_SIZE] = 1;
    #ifdef DEBUG
    printk(KERN_ERR "[env]ERROR:mtd_block_markbad,0x%x,%d,%d\n",from,ret,from/ENV_BLOCK_SIZE);
    #endif // DEBUG
    return ret;
}

// static void erase_callback (struct erase_info *self)
// {
//     printk("erase_callback\n");
// }

static int find_good_block(env_dev_t *dev)
{
    int i;
    for(i = 0;i < 4;i++) {
        if (!mtd_block_isbad(dev->mtd, dev->start_offset + i * ENV_BLOCK_SIZE) && g_badblocks[i + dev->start_offset/ENV_BLOCK_SIZE] == 0 ) {
            dev->base_offset = dev->start_offset + i * ENV_BLOCK_SIZE;
            #ifdef DEBUG
            printk("[%s]:found block %d,0x%x\n", dev->name, i,dev->base_offset);
            #endif
            return 0;
        }
    }
    
    printk("%s, It's all bad blocks\n", dev->name);

    return -1;
}

static int loadenv(env_dev_t *dev)
{
    int ret = 0;
    int i;
    
    #ifdef DEBUG
    printk("TEST:load\n");
    #endif
    if(find_good_block(dev) != 0) {
        return -ENOMEM;
    }

    dev->section_num = sizeof(g_env_layout)/sizeof(env_layout_t);
    dev->sections = kzalloc(sizeof(section_t) * dev->section_num, GFP_KERNEL);
    #ifdef DEBUG
    printk(KERN_ERR"[env] section_num:%d\n",dev->section_num);
    #endif
    if (dev->sections == NULL) {
        return -ENOMEM;
    }

    for(i = 0; i < dev->section_num; i++) {
        uint32_t secrc = 0;
        unsigned char *p = NULL;
        dev->sections[i].name = g_env_layout[i].name;
        dev->sections[i].offset = g_env_layout[i].offset;
        dev->sections[i].size = g_env_layout[i].size;
        dev->sections[i].flag = ENV_WO;
        dev->sections[i].crc = 0;
        dev->sections[i].dirty_flag = 0;
        dev->sections[i].env_count = 0;
        dev->sections[i].env_len = 0;
        dev->sections[i].data = kzalloc(dev->sections[i].size, GFP_KERNEL);
        if (dev->sections[i].data == NULL) {
            return -ENOMEM;
        }
        #ifdef DEBUG
        printk(KERN_ERR"[%s]:%s offset:0x%x\n",dev->name,dev->sections[i].name,dev->sections[i].offset);
        #endif
        ret = read_from_flash(dev->sections[i].offset + dev->base_offset ,dev->sections[i].data, dev->sections[i].size);
        if (ret < 0) {            
            markbad_flash(dev->base_offset);
            return ret;
        }

        dev->sections[i].crc = crc32(0, (uint8_t *)(dev->sections[i].data + 4), dev->sections[i].size - 4);
        secrc = *(uint32_t *)dev->sections[i].data;
        
        if (secrc == 0 || secrc != dev->sections[i].crc) {
            dev->sections[i].data[4] = 0;
            dev->sections_crc_flag = 0;
            printk(KERN_ERR"[%s]ERROR: crc error in section %s: expect 0x%x, got 0x%x!\n",dev->name, dev->sections[i].name, dev->sections[i].crc, secrc);
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
    #ifdef DEBUG
    printk("TEST:save\n");
    #endif

    if(g_env_dev->locked == 1) {
        if (!no_sema) {
            up(&dev->sem);
        }
        return -EPERM;
    }

retry:

    if(find_good_block(dev) != 0) {
        if (!no_sema) {
            up(&dev->sem);
        }
        return -ENOMEM;
    }

    for(i = 0; i < dev->section_num; i++) {
        if ( dev->sections[i].flag == ENV_RO ||
            dev->sections[i].dirty_flag == 0 ) {
           continue; 
        }

        #ifdef DEBUG
        printk(KERN_ERR"[%s]: saveenv section %s,flag:%d,dirty:%d!\n",dev->name ,dev->sections[i].name,dev->sections[i].flag,dev->sections[i].dirty_flag);
        printk(KERN_ERR"[%s]:saveenv %s offset:0x%x\n",dev->name,dev->sections[i].name,dev->sections[i].offset);
        #endif


        env_crc32 = crc32(0, dev->sections[i].data + 4, dev->sections[i].size - 4);
        memcpy(dev->sections[i].data,(unsigned *)&env_crc32,4);
    
        ret = write_to_flash(dev->sections[i].offset + dev->base_offset, dev->sections[i].data, dev->sections[i].size);
        if(ret < 0) {
            markbad_flash(dev->base_offset);
            goto retry;
        }
        dev->sections[i].dirty_flag = 0;
    }

    if (!no_sema) {
        up(&dev->sem);
    }
    return ret;
}

static int env_open(struct inode *inode, struct file *filp)
{
    filp->private_data = (void *)g_env_dev;

    if (down_interruptible(&g_env_dev->sem)) {
        return -ERESTARTSYS;
    }
    #ifdef DEBUG
    printk("TEST:env_open\n");
    #endif
    if (g_env_dev->refcnt > 0) {
        g_env_dev->refcnt++;
        goto done;
    }

    g_env_dev->refcnt = 1;

done:
    up(&g_env_dev->sem);
    return 0;
}

static int env_release(struct inode *inode, struct file *filp)
{
    env_dev_t *dev = (env_dev_t *)filp->private_data;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }
    #ifdef DEBUG
    printk("TEST:RELEASE\n");
    #endif
    saveenv(dev, 1);
    saveenv(g_env_bak_dev, 1);

    dev->refcnt--;

    up(&dev->sem);
    return 0;
}

static int env_check_and_update(env_dev_t *env_dev,  env_dev_t *env_bak_dev)
{
    int i;
    //printk("env_dev->sections_crc_flag = %d, env_bak_dev->sections_crc_flag= %d \n", env_dev->sections_crc_flag, env_bak_dev->sections_crc_flag);
    if ((env_dev->sections_crc_flag == 1 && env_bak_dev->sections_crc_flag == 0)) {
        for (i = 0; i < env_dev->section_num; i++) {
            env_bak_dev->sections[i].size = env_dev->sections[i].size;
            env_bak_dev->sections[i].env_count = env_dev->sections[i].env_count;
            env_bak_dev->sections[i].env_len = env_dev->sections[i].env_len;
            env_bak_dev->sections[i].dirty_flag = 1;
            memcpy(env_bak_dev->sections[i].data, env_dev->sections[i].data, env_dev->sections[i].size);
        }
        env_bak_dev->sections_crc_flag = 1;
        #ifdef DEBUG
        printk("TEST:restore bak\n");
        #endif
        saveenv(env_bak_dev, 1);
        printk("%s:restore backup",env_bak_dev->name);
    }else if (env_dev->sections_crc_flag == 0 && env_bak_dev->sections_crc_flag == 1) {
        for (i = 0; i < env_bak_dev->section_num; i++) {
            env_dev->sections[i].size = env_bak_dev->sections[i].size;
            env_dev->sections[i].env_count = env_bak_dev->sections[i].env_count;
            env_dev->sections[i].env_len = env_bak_dev->sections[i].env_len;
            env_dev->sections[i].dirty_flag = 1;
            memcpy(env_dev->sections[i].data, env_bak_dev->sections[i].data, env_bak_dev->sections[i].size);
        }
        env_dev->sections_crc_flag = 1;
        #ifdef DEBUG
        printk("TEST:resetoru env\n");
        #endif
        saveenv(env_dev, 1);
        printk("%s:restore backup",env_dev->name);
    } else if(env_dev->sections_crc_flag == 0 && env_bak_dev->sections_crc_flag == 0) {
        printk("ERROR: No valid environment variables found.\n");
        return -1;
    }

    return 0;
}

#define ENV_IOC_MAGIC 'e'
#define ENV_IOC_MAXNR 7
#define ENV_IOCGET _IOR(ENV_IOC_MAGIC, 0, unsigned long)
#define ENV_IOCSET _IOW(ENV_IOC_MAGIC, 1, unsigned long)
#define ENV_IOCUNSET _IOW(ENV_IOC_MAGIC, 2, unsigned long)
#define ENV_IOCCLR _IOW(ENV_IOC_MAGIC, 3, unsigned long)
#define ENV_IOCPRT _IOR(ENV_IOC_MAGIC, 4, unsigned long)
#define ENV_IOCSAVE _IOR(ENV_IOC_MAGIC, 5, unsigned long)
#define ENV_IOCLOCK _IOR(ENV_IOC_MAGIC, 6, unsigned long)
#define ENV_IOCUNLOCK _IOR(ENV_IOC_MAGIC, 7, unsigned long)

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
        #ifdef DEBUG
        printk(KERN_ERR"[env]: ENV_IOCGET section %s.karg.key:%s!\n", karg.section_name,karg.key);
        #endif
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

        #ifdef DEBUG
        printk(KERN_ERR"[env]: ENV_IOCGET section %s.karg.key:%s,env_str:%s,karg.maxlen:%d!\n", karg.section_name,karg.key,env_str,karg.maxlen);
        #endif

        if (copy_to_user(karg.buf, env_str, strlen(env_str) + 1)) {
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
        setenv(g_env_bak_dev, karg.section_name, karg.key, value, karg.overwrite);
        #ifdef DEBUG
        printk(KERN_ERR"[env]: ENV_IOCSET section %s.karg.key:%s,env_str:%s!\n", karg.section_name,karg.key,value);
        #endif
        kfree(value);
        break;
    }
    case ENV_IOCUNSET: {
        retval = unsetenv(dev, karg.section_name, karg.key);
        unsetenv(g_env_bak_dev, karg.section_name, karg.key);
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
        #ifdef DEBUG
        printk("TEST:ENV_IOCSAVE\n");
        #endif
        retval = saveenv(dev, 0);
        saveenv(g_env_bak_dev, 0);
        break;
    }
    case ENV_IOCLOCK: {
        if ( down_interruptible(&dev->sem)) {
            return -ERESTARTSYS;
        }
        #ifdef DEBUG
        printk("TEST:ENV_IOCLOCK\n");
        #endif
        saveenv(dev, 1);
        saveenv(g_env_bak_dev, 1);
        dev->locked = 1;
        up(&dev->sem);
        break;
    }
    case ENV_IOCUNLOCK: {
        if ( down_interruptible(&dev->sem)) {
            return -ERESTARTSYS;
        }
        dev->locked = 0;
        up(&dev->sem);
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
    int i;
    misc_deregister(&env_miscdev);

    if (g_env_dev) {
    for(i = 0; i < g_env_dev->section_num; i++) {
        if (g_env_dev->sections[i].data != NULL) {
            kfree(g_env_dev->sections[i].data);
        }
    }

    if (g_env_dev->sections != NULL) {
        kfree(g_env_dev->sections);
    }
    kfree(g_env_dev);
    }

    if (g_env_bak_dev) {
        for(i = 0; i < g_env_bak_dev->section_num; i++) {
            if (g_env_bak_dev->sections[i].data != NULL) {
                kfree(g_env_bak_dev->sections[i].data);
            }
        }

        if (g_env_bak_dev->sections != NULL) {
            kfree(g_env_bak_dev->sections);
        }
        kfree(g_env_bak_dev);
    }
}

static __init int env_init(void)
{
    int ret = 0;
    g_env_dev = kzalloc(sizeof(env_dev_t), GFP_KERNEL);
    if (!g_env_dev) {
        ret = -ENOMEM;
        goto fail;
    }
    g_env_dev->locked = 0;
    sema_init(&g_env_dev->sem, 1);
    strcpy((char*)g_env_dev->name, MTD_NAME);
    g_env_dev->mtd = get_mtd_device_nm(MTD_NAME);
    g_env_dev->sections_crc_flag = 1;
    g_env_dev->base_offset = 0;
    g_env_dev->start_offset = FIRST_BLOCK_OFFSET;
    ret = loadenv(g_env_dev);
    // if (ret < 0) {
    //     goto fail;
    // }

    //init g_env_bak_dev init
    g_env_bak_dev = kzalloc(sizeof(env_dev_t), GFP_KERNEL);
    if (!g_env_bak_dev) {
        ret = -ENOMEM;
        goto fail;
    }
    g_env_bak_dev->locked = 0;
    sema_init(&g_env_bak_dev->sem, 1);
    g_env_bak_dev->sections_crc_flag = 1;
    strcpy((char*)g_env_bak_dev->name, "Env_bak");
    g_env_bak_dev->mtd = get_mtd_device_nm(MTD_NAME);
    g_env_bak_dev->base_offset = 0;
    g_env_bak_dev->start_offset = FIRST_BAK_BLOCK_OFFSET;
    ret = loadenv(g_env_bak_dev);
    // if (ret < 0) {
    //     goto fail;
    // }

    //g_env_dev and g_env_bak_dev compare, to decide which one replace the other.
    if (env_check_and_update(g_env_dev, g_env_bak_dev)) {
        printk("ERROR: env_check_and_update!!!!\n");
        goto fail;
    }
    
    ret = misc_register(&env_miscdev);
    if (ret) {
        ret = -ENODEV;
        goto fail;
    }

    return 0;

fail:
    // env_exit();
    return ret;
}

module_init(env_init);
module_exit(env_exit);

MODULE_LICENSE("GPL");