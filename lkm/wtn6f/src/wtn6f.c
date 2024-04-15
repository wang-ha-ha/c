#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/delay.h>

static struct mutex g_mutex;
static int major;
static struct class *cls;
struct device *mydev;
static int gpio_clk = 41;
static int gpio_busy = 42;
static int gpio_data = 43;

static void wtn6f_send_cmd(unsigned char cmd)
{
    unsigned char tmp_cmd, i;
    unsigned char data;

    // printk("wtn6f_send_cmd:0x%x\n", cmd);

    gpio_direction_output(gpio_clk,1);
    gpio_direction_output(gpio_data,1);
    tmp_cmd= cmd;
    gpio_direction_output(gpio_clk,0);
    mdelay(5);
    data= tmp_cmd&0X01;
    for(i = 0;i < 8;i++) {
        gpio_direction_output(gpio_clk,0);
        if(data == 0) {
            gpio_direction_output(gpio_data,0);
        } else {
            gpio_direction_output(gpio_data,1);
        }
        udelay(300);
        gpio_direction_output(gpio_clk,1);
        udelay(300);
        tmp_cmd >>= 1;
        data= tmp_cmd&0X01;
    }
    gpio_direction_output(gpio_clk,1);
    gpio_direction_output(gpio_data,1);
}

static ssize_t wtn6f_cmd_store(struct device *dev,
		struct device_attribute *attr, const  char *buf, size_t count)
{
	unsigned long val;
	int ret = 0;

	ret = kstrtoul(buf, 16, &val);
	if (ret < 0) {
		return 0;
	}
	
	mutex_lock(&g_mutex);
    wtn6f_send_cmd(val);
	mutex_unlock(&g_mutex);

	return count;
}

static ssize_t wtn6f_status_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
    return sprintf(buf, "%d\n",gpio_get_value(gpio_busy));
}

// #define DBG_TEST
#ifdef DBG_TEST
static int aaa = 0;
static int ttt[50] = {0};
#endif

static int gpio_simulation_uart_read_byte()
{
    int val = 1,i,times = 100000;

    while(val) {
        val = gpio_get_value(gpio_busy);
        times--;
        if(times <=0 )
            return -1;
    }
    #ifdef DBG_TEST
    ttt[aaa++] = times;
    #endif
    udelay(100);

    for(i = 0; i < 8;i++) {
        udelay(50);
        val >>= 1;
        if(gpio_get_value(gpio_busy))
            val |= 0x80;
        udelay(50);
    }

    udelay(100);

    return val;
}

static ssize_t wtn6f_version_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
    char version[128] = {0};
    char tmp[128] = {0};
    int count = 100*1000;
    int i,val = 1,j = 0;
    mutex_lock(&g_mutex);
    wtn6f_send_cmd(0xF9);
    mutex_unlock(&g_mutex);
    #ifdef DBG_TEST
    aaa=0;
    #endif
    local_irq_disable();
    while(val && count) {
        udelay(1);
        val = gpio_get_value(gpio_busy);
        count--;
    }
    
    for(i = 0; i < 50;i++){
        tmp[i] = gpio_simulation_uart_read_byte();
    }
    #ifdef DBG_TEST
    for(i = 0; i < 50;i++){
        printk("%d:0x%02x,%d\n",i,tmp[i],ttt[i]);
    }
    #endif
    for(i = 18; i < 50;i++){
        // printk("%d:0x%02x\n",i,tmp[i]);
        if(tmp[i] == -1){
            // printk("aaa\n");
            version[j] = 0;
            break;
        }
        version[j++] = tmp[i];
    }
	local_irq_enable();
    
    return sprintf(buf, "%s\n",version);
}

static DEVICE_ATTR(cmd, S_IWUSR, NULL, wtn6f_cmd_store);
static DEVICE_ATTR(status, S_IRUGO, wtn6f_status_show, NULL);
static DEVICE_ATTR(version, S_IRUGO, wtn6f_version_show, NULL);

static struct attribute *wtn6f_mid_att[] = {
	&dev_attr_cmd.attr,
    &dev_attr_status.attr,
    &dev_attr_version.attr,
	NULL
};

static struct attribute_group wtn6f_gr = {
	.name = "property",
	.attrs = wtn6f_mid_att
};

struct file_operations wtn6f_ops = {
    .owner = THIS_MODULE,
};

static int __init wtn6f_module_init(void)
{
    mutex_init(&g_mutex);

    gpio_request(gpio_clk,"wtn6f_clk");
    gpio_request(gpio_data,"wtn6f_data");
    gpio_request(gpio_busy,"wtn6f_busy");
    gpio_direction_output(gpio_clk,1);
    gpio_direction_output(gpio_data,1);
    gpio_direction_input(gpio_busy);
    major = register_chrdev(0, "wtn6f", &wtn6f_ops);
    cls = class_create(THIS_MODULE, "wtn6f_class");
    mydev = device_create(cls, 0, MKDEV(major, 0), NULL, "wtn6f_device");
 
    if (sysfs_create_group(&(mydev->kobj), &wtn6f_gr)) {
        return -1;
    }

	return 0;
}

static void __exit wtn6f_module_exit(void)
{
	printk("wtn6f_module_exit\n");    
    gpio_free(gpio_clk);
    gpio_free(gpio_data);
    gpio_free(gpio_busy);
    sysfs_remove_group(&(mydev->kobj), &wtn6f_gr);
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, "wtn6f");
}

module_init(wtn6f_module_init);
module_exit(wtn6f_module_exit);

MODULE_AUTHOR("wangchuanqi <wangchuanqi@70mai.com");
MODULE_DESCRIPTION("wtn6f Driver");
MODULE_LICENSE("GPL v2");