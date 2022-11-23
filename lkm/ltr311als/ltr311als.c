#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/interrupt.h>

#define DEBUG

#define LTR311ALS_MAIN_CTRL_REG				0x80
#define LTR311ALS_TIME_SCALE_REG			0x85
#define LTR311ALS_INTERRUPT_SETTING_REG		0xA0
#define LTR311ALS_INTERRUPT_PERSIST_REG		0xA1
#define LTR311ALS_UPPER_THRESHOLD_L_REG		0xAA
#define LTR311ALS_UPPER_THRESHOLD_H_REG		0xAB
#define LTR311ALS_LOWER_THRESHOLD_L_REG		0xAC
#define LTR311ALS_LOWER_THRESHOLD_H_REG		0xAD

//readonly
#define LTR311ALS_MANUFAC_ID_REG			0xAF
#define LTR311ALS_DATA_0_REG				0x8B
#define LTR311ALS_DATA_1_REG				0x8C
#define LTR311ALS_STATE_REG					0x88

struct ltr311als_private {
	struct mutex mutex;
	unsigned short upper_threshold;
	unsigned short lower_threshold;
	int int_gpio_num;
	int int_gpio_irq_num;
};

static struct ltr311als_private *ltr311als_data;
static unsigned short g_upper_threshold = 5000;
static unsigned short g_lower_threshold = 1000;

module_param(g_upper_threshold, ushort, S_IRUGO | S_IWUSR);
module_param(g_lower_threshold, ushort, S_IRUGO | S_IWUSR);

static int ltr311als_init(struct i2c_client *i2c_client)
{
	int ret = 0;
	unsigned char data0,data1;
	struct ltr311als_private *data = i2c_get_clientdata(i2c_client);

	data->upper_threshold = g_upper_threshold;
	data->lower_threshold = g_lower_threshold;

	i2c_smbus_write_byte_data(i2c_client, LTR311ALS_TIME_SCALE_REG, 0xE2);
    i2c_smbus_write_byte_data(i2c_client, LTR311ALS_INTERRUPT_SETTING_REG, 0x0C); //ALS measurement can trigger interrupt
    i2c_smbus_write_byte_data(i2c_client, LTR311ALS_INTERRUPT_PERSIST_REG, 0x05);

    data1 = ((data->upper_threshold & 0xff00) >> 8);
    data0 = (data->upper_threshold & 0x00ff);
    i2c_smbus_write_byte_data(i2c_client, LTR311ALS_UPPER_THRESHOLD_L_REG, data0);
    i2c_smbus_write_byte_data(i2c_client, LTR311ALS_UPPER_THRESHOLD_H_REG, data1);

	#ifdef DEBUG
	dev_info(&i2c_client->dev,"LTR311ALS_UPPER_THRESHOLD_H_REG0:0x%x LTR311ALS_UPPER_THRESHOLD_H_REG1:0x%x LTR311ALS_UPPER_THRESHOLD_H_REG:0x%x\n",data0,data1,data0 | (data1 << 8));
	#endif

    data1 = ((data->lower_threshold & 0xff00) >> 8);
    data0 = (data->lower_threshold & 0x00ff);
    i2c_smbus_write_byte_data(i2c_client, LTR311ALS_LOWER_THRESHOLD_L_REG, data0);
    i2c_smbus_write_byte_data(i2c_client, LTR311ALS_LOWER_THRESHOLD_H_REG, data1);

	#ifdef DEBUG
	dev_info(&i2c_client->dev,"LTR311ALS_LOWER_THRESHOLD_L_REG0:0x%x LTR311ALS_LOWER_THRESHOLD_L_REG1:0x%x LTR311ALS_LOWER_THRESHOLD_L_REG:0x%x\n",data0,data1,data0 | (data1 << 8));
	#endif

	return ret;
}

static irqreturn_t ltr311als_gpio_irq_thread(int irq, void *dev_id)
{
	struct i2c_client *i2c_client= (struct i2c_client *)dev_id;

	dev_info(&i2c_client->dev, "Interrupt is triggered\n");

	return IRQ_HANDLED;
}

static int ltr311als_register_interrupt(struct i2c_client *i2c_client)
{
	int ret = 0;
	struct ltr311als_private *data = i2c_get_clientdata(i2c_client);

	if (devm_gpio_request(&i2c_client->dev, data->int_gpio_num, "ALS INT") == 0) 
	{
		data->int_gpio_irq_num  = gpio_to_irq(data->int_gpio_num);
		if (data->int_gpio_irq_num &&
		    devm_request_threaded_irq(&i2c_client->dev,data->int_gpio_irq_num, NULL,
								 ltr311als_gpio_irq_thread,
								 IRQF_TRIGGER_RISING | IRQF_ONESHOT,
								 dev_name(&i2c_client->dev), i2c_client) == 0) 
		{
			dev_info(&i2c_client->dev, "request irq succeed\n");
		}
		else 
		{
			dev_err(&i2c_client->dev, "request irq failed\n");
			data->int_gpio_irq_num = 0;
			ret = -1;
		}
	} 
	else 
	{
		dev_err(&i2c_client->dev, "request gpio failed\n");
		ret = -2;
	}

	return ret;
}

static ssize_t ltr311als_enable_store(struct device *dev,
		struct device_attribute *attr, const  char *buf, size_t count)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct ltr311als_private *data = i2c_get_clientdata(i2c_client);
	int ret_val;
	unsigned long val;

	ret_val = kstrtoul(buf, 10, &val);
	if (ret_val)
	{
		return ret_val;
	}

	mutex_lock(&data->mutex);

	ret_val = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_MAIN_CTRL_REG);
	if (ret_val < 0)
	{
		goto fail;
	}

	ret_val = ret_val & 0xFE;

	if (val != 0)
	{
		ret_val = (ret_val | 0x01);	
	}

	ret_val = i2c_smbus_write_byte_data(i2c_client, LTR311ALS_MAIN_CTRL_REG, ret_val);

	if (ret_val >= 0) {
		mutex_unlock(&data->mutex);
		return count;
	}

fail:
	mutex_unlock(&data->mutex);

	return ret_val;
}

static ssize_t ltr311als_status_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct ltr311als_private *data = i2c_get_clientdata(i2c_client);
	int  val;

	mutex_lock(&data->mutex);
	val = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_MAIN_CTRL_REG);
	mutex_unlock(&data->mutex);
	#ifdef DEBUG
	dev_info(dev,"LTR311ALS_MAIN_CTRL_REG:0x%x\n",val);
	#endif
	if (val < 0)
	{
		return val;
	}
	if (val & 0x1)
	{
		return sprintf(buf, "Active\n");
	}	
	else
	{
		return sprintf(buf, "Stand-by\n");
	}	
}

static ssize_t ltr311als_value_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct ltr311als_private *data = i2c_get_clientdata(i2c_client);
	int  val,val1, val2;

	mutex_lock(&data->mutex);
	val = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_MAIN_CTRL_REG);
	if(val < 0 || (val & 0x01) == 0)
	{
		val = 0;
		mutex_unlock(&data->mutex);
		goto ret;
	}
	i2c_smbus_write_byte_data(i2c_client, LTR311ALS_MAIN_CTRL_REG, val);
	val1 = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_DATA_0_REG);
	val2 = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_DATA_1_REG);
	val = val1 | (val2 << 8);
	mutex_unlock(&data->mutex);
	#ifdef DEBUG
	dev_info(dev,"LTR311ALS_DATA_REG:0x%x.%d\n",val,val);
	dev_info(dev,"LTR311ALS_DATA_0_REG:0x%x LTR311ALS_DATA_1_REG:0x%x LTR311ALS_DATA_REG:0x%x\n",val1,val2,val1 | (val2 << 8));
	#endif
	if (val < 0)
	{
		return val;
	}

ret:
	return sprintf(buf, "%d\n", val);
}

static ssize_t ltr311als_upper_threshold_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct ltr311als_private *data = i2c_get_clientdata(i2c_client);
	unsigned int  val,val1, val2;

	mutex_lock(&data->mutex);
	val1 = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_UPPER_THRESHOLD_L_REG);
	val2 = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_UPPER_THRESHOLD_H_REG);
	val = val1 | (val2 << 8);
	mutex_unlock(&data->mutex);
	#ifdef DEBUG
	dev_info(dev,"LTR311ALS_UPPER_THRESHOLD_H_REG:0x%x.%d\n",val,val);
	dev_info(dev,"LTR311ALS_UPPER_THRESHOLD_H_REG0:0x%x LTR311ALS_UPPER_THRESHOLD_H_REG1:0x%x LTR311ALS_UPPER_THRESHOLD_H_REG:0x%x\n",val1,val2,val1 | (val2 << 8));
	#endif

	return sprintf(buf, "%d\n", val);
}

static ssize_t ltr311als_upper_threshold_store(struct device *dev,
		struct device_attribute *attr, const  char *buf, size_t count)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct ltr311als_private *data = i2c_get_clientdata(i2c_client);
	int ret_val;
	unsigned char data0,data1;
	unsigned long val;

	ret_val = kstrtoul(buf, 10, &val);
	if (ret_val && val > data->lower_threshold && val != data->upper_threshold)
	{
		return ret_val;
	}

	data->upper_threshold = val;
	mutex_lock(&data->mutex);

	data1 = ((val & 0xff00) >> 8);
    data0 = (val & 0x00ff);

	ret_val = i2c_smbus_write_byte_data(i2c_client, LTR311ALS_UPPER_THRESHOLD_L_REG, data0);
    ret_val = i2c_smbus_write_byte_data(i2c_client, LTR311ALS_UPPER_THRESHOLD_H_REG, data1);
	if (ret_val >= 0) {
		mutex_unlock(&data->mutex);
		return count;
	}

	mutex_unlock(&data->mutex);

	return ret_val;
}

static ssize_t ltr311als_lower_threshold_show(struct device *dev,
			struct device_attribute *attr,  char *buf)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct ltr311als_private *data = i2c_get_clientdata(i2c_client);
	unsigned int  val,val1, val2;

	mutex_lock(&data->mutex);
	val1 = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_LOWER_THRESHOLD_L_REG);
	val2 = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_LOWER_THRESHOLD_H_REG);
	val = val1 | (val2 << 8);
	mutex_unlock(&data->mutex);
	#ifdef DEBUG
	dev_info(dev,"LTR311ALS_LOWER_THRESHOLD_L_REG:0x%x.%d\n",val,val);
	dev_info(dev,"LTR311ALS_LOWER_THRESHOLD_L_REG0:0x%x LTR311ALS_LOWER_THRESHOLD_L_REG1:0x%x LTR311ALS_LOWER_THRESHOLD_L_REG:0x%x\n",val1,val2,val1 | (val2 << 8));
	#endif
	if (val < 0)
	{
		return val;
	}

	return sprintf(buf, "%d\n", val);
}

static ssize_t ltr311als_lower_threshold_store(struct device *dev,
		struct device_attribute *attr, const  char *buf, size_t count)
{
	struct i2c_client *i2c_client = to_i2c_client(dev);
	struct ltr311als_private *data = i2c_get_clientdata(i2c_client);
	int ret_val;
	unsigned char data0,data1;
	unsigned long val;

	ret_val = kstrtoul(buf, 10, &val);
	if (ret_val && val < data->upper_threshold && val != data->lower_threshold)
	{
		return ret_val;
	}

	data->lower_threshold = val;
	mutex_lock(&data->mutex);

	data1 = ((val & 0xff00) >> 8);
    data0 = (val & 0x00ff);

	ret_val = i2c_smbus_write_byte_data(i2c_client, LTR311ALS_LOWER_THRESHOLD_L_REG, data0);
    ret_val = i2c_smbus_write_byte_data(i2c_client, LTR311ALS_LOWER_THRESHOLD_H_REG, data1);
	if (ret_val >= 0) {
		mutex_unlock(&data->mutex);
		return count;
	}

	mutex_unlock(&data->mutex);

	return ret_val;
}

static DEVICE_ATTR(enable, S_IWUSR, NULL, ltr311als_enable_store);
static DEVICE_ATTR(status, S_IRUGO, ltr311als_status_show, NULL);
static DEVICE_ATTR(value, S_IRUGO, ltr311als_value_show, NULL);
static DEVICE_ATTR(upper_threshold, S_IRUGO | S_IWUSR, ltr311als_upper_threshold_show, ltr311als_upper_threshold_store);
static DEVICE_ATTR(lower_threshold, S_IRUGO | S_IWUSR, ltr311als_lower_threshold_show, ltr311als_lower_threshold_store);

static struct attribute *ltr311als_mid_att[] = {
	&dev_attr_enable.attr,
	&dev_attr_status.attr,
	&dev_attr_value.attr,
	&dev_attr_upper_threshold.attr,
	&dev_attr_lower_threshold.attr,
	NULL
};

static struct attribute_group ltr311als_gr = {
	.name = "ltr311als",
	.attrs = ltr311als_mid_att
};

static int ltr311als_i2c_probe(struct i2c_client *i2c_client,
					const struct i2c_device_id *id)
{
	int ret,try = 3;

	dev_info(&i2c_client->dev, "ltr311als_i2c_probe\n");
	
	do
	{
		ret = i2c_smbus_read_byte_data(i2c_client, LTR311ALS_MANUFAC_ID_REG);
		if (ret < 0)
		{
			dev_err(&i2c_client->dev,"read ltr311als Manufac ID failed.ret:%d\n",ret);		
			try--;
		}
	} while (ret < 0 && try > 0);

	if (ret < 0)
	{
		return ret;
	}
	
	dev_info(&i2c_client->dev,"ltr311als Manufac ID:0x%x\n",ret);

	ltr311als_data = devm_kzalloc(&i2c_client->dev,sizeof(struct ltr311als_private), GFP_KERNEL);
	if(ltr311als_data == NULL)
	{
		dev_err(&i2c_client->dev, "devm_kzalloc failed\n");
		return -ENOMEM;
	}

	i2c_set_clientdata(i2c_client, ltr311als_data);
	mutex_init(&ltr311als_data->mutex);

	ret = ltr311als_init(i2c_client);
	if(ret)
	{
		dev_err(&i2c_client->dev, "ltr311als_init failed:%d\n", ret);
		goto error;
	}

	ltr311als_data->int_gpio_num = of_get_named_gpio_flags(i2c_client->dev.of_node, "int-gpios", 0, NULL);
	if(gpio_is_valid(ltr311als_data->int_gpio_num)) 
	{
		ltr311als_register_interrupt(i2c_client);
	}
	else
	{
		dev_err(&i2c_client->dev, "The interrupt GPIO is invalid\n");
	}

	ret = sysfs_create_group(&i2c_client->dev.kobj, &ltr311als_gr);
	if (ret) 
	{
		dev_err(&i2c_client->dev, "device create file failed:%d\n", ret);
	}

error:
	return ret;
}

static  int ltr311als_i2c_remove(struct i2c_client *i2c_client)
{
	dev_info(&i2c_client->dev, "ltr311als_i2c_remove\n");

	return 0;
}

static void ltr311als_i2c_shutdown(struct i2c_client *i2c_client)
{
	dev_info(&i2c_client->dev, "ltr311als_i2c_shutdown\n");

	return;
}

static struct i2c_device_id ltr311als_id[] = {
	{ "ltr311als", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ltr311als_id);

static struct i2c_driver ltr311als_driver = {
	.driver = {
		.name = "ltr311als",
	},
	.probe = ltr311als_i2c_probe,
	.remove = ltr311als_i2c_remove,
	.shutdown = ltr311als_i2c_shutdown,
	.id_table = ltr311als_id,
};

static int __init ltr311als_module_init(void)
{
	int ret;

	printk("ltr311als_module_init\n");

	ret = i2c_add_driver(&ltr311als_driver);
	if (ret < 0) 
	{
		printk("Unable to register ltr311als driver, ret= %d", ret);
		return ret;
	}

	return 0;
}

static void __exit ltr311als_module_exit(void)
{
	printk("ltr311als_module_exit\n");
	i2c_del_driver(&ltr311als_driver);
}

module_init(ltr311als_module_init);
module_exit(ltr311als_module_exit);

MODULE_AUTHOR("wangchuanqi <wangchuanqi@70mai.com");
MODULE_DESCRIPTION("ltr311als ALS Driver");
MODULE_LICENSE("GPL v2");