/**
 * Copyright (C) 2020 Ingenic Semiconductor Co., Ltd.
 * This program is free software; you can redistribute it and/or modify it
 * published by the Free Software Foundation.
 *
 * @file efuse.c
 * @brief EFuse burning entry, eFuse read-write timing configuration. Users
 * are advised to modify only GPIO, otherwise unexpected errors may occur
 *
 * @author qli, qi.li@ingenic.com
 * @version 1
 * @date 2021-04-16
 *
 * @attention This resource is only used for T40
 */

#include <stdio.h>
#include <efuse.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/time.h>
#include <time.h>

#define UDELAY_US (20)

static inline void udelay(uint64_t us)
{
#define TIME_CLOCK(a)  \
	clockid_t a##clk_id = CLOCK_REALTIME;	\
	struct timespec a##time; \
	clock_gettime(a##clk_id, &a##time);  \
	uint64_t a = ((uint64_t)(a##time).tv_sec * 1000000 )  + ((uint64_t)(a##time).tv_nsec / 1000);
	TIME_CLOCK(start);
	while(1) {
		TIME_CLOCK(end);
		if(end - start >= us)
			return;
	}
}

typedef struct
{
	void *map_addr;
	uint32_t offset;
	int length;
} map_info;

enum M_TYPE {
	TYPE_CPM = 0,
	TYPE_EFUSE_CFG,
	TYPE_NUMS
};

struct efuse_info
{
	int init_already;
	int map_num;
	int gpio_num;
	map_info minfo[TYPE_NUMS+1];
	uint32_t *id2addr;
};

uint32_t seg_addr[] = {
	CHIP_ID_ADDR,
	CHIP_ID_BAK_ADDR,
	USER_ID_ADDR,
	USER_ID_BAK_ADDR,
	SARADC_CAL,
	SARADC_BAK_CAL,
	TRIM_ADDR,
	TRIM_BAK_ADDR,
	PROGRAM_PROTECT_ADDR,
	PROGRAM_PROTECT_BAK_ADDR,
	CPU_ID_ADDR,
	CPU_ID_BAK_ADDR,
	SPECIAL_ADDR,
	SPECIAL_BAK_ADDR,
	CUSTOMER_RESV_ADDR,
	CUSTOMER_RESV_BAK_ADDR,
	SCB_DATA_ADDR,
	SCB_DATA_BAK_ADDR,
};

static struct efuse_info g_efuse = {
	.init_already = 0,
	.map_num = TYPE_NUMS,
	.minfo = {
		[TYPE_CPM].map_addr = NULL,
		[TYPE_CPM].offset = CPM_BASE,
		[TYPE_CPM].length = CPM_SIZE,

		[TYPE_EFUSE_CFG].map_addr = NULL,
		[TYPE_EFUSE_CFG].offset = EFUSE_BASE,
		[TYPE_EFUSE_CFG].length = EFUSE_SIZE,
	},
	.id2addr = seg_addr,
};

static int gpio_init(int gpio)
{
	int fd;
	char value[256] = {0};
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(fd < 0) {
		return -1;
	}

	sprintf(value, "%d", gpio);
	write(fd, value, strlen(value));
	close(fd);
	memset(value, 0x0, sizeof(value));
	sprintf(value, "/sys/class/gpio/gpio%d/direction", gpio);
	fd = open(value, O_RDWR);
	if(fd < 0) {
		return -1;
	}
	write(fd, "out", 3);
	close(fd);
	memset(value, 0x0, sizeof(value));
	sprintf(value, "/sys/class/gpio/gpio%d/active_low", gpio);
	fd = open(value, O_RDWR);
	if(fd < 0) {
		return -1;
	}
	write(fd, "0", 1);
	close(fd);

	return 0;
}

#if 0

static int gpio_set_direction(int gpio, int direction_out)
{
	int fd;
	char value[256] = {0};
	sprintf(value, "/sys/class/gpio/gpio%d/direction", gpio);
	fd = open(value, O_RDWR);
	if(fd < 0) {
		LOG_ERR("%s, error, %s\n", __FUNCTION__, strerror(errno));
		return -1;
	}
	write(fd, "out", 3);
	close(fd);
	memset(value, 0x0, sizeof(value));
	sprintf(value, "/sys/class/gpio/gpio%d/active_low", gpio);
	fd = open(value, O_RDWR);
	if(fd < 0) {
		return -1;
	}
	write(fd, "0", 1);
	close(fd);

	return 0;
}

#endif

static int gpio_open(int gpio)
{
	int fd;
	char value[256] = {0};
	sprintf(value, "/sys/class/gpio/gpio%d/value", gpio);
	if(access(value, F_OK) != 0) {
		LOG_ERR("no init gpio: %d\n", gpio);
		return -1;
	}
	fd = open(value, O_RDWR);
	if(fd < 0) {
		return -1;
	}
	write(fd, "1", 1);
	close(fd);
	return 0;
}

static int gpio_close(int gpio)
{
	int fd;
	char value[256] = {0};
	sprintf(value, "/sys/class/gpio/gpio%d/value", gpio);
	if(access(value, F_OK) != 0) {
		LOG_ERR("no init gpio: %d\n", gpio);
		return -1;
	}
	fd = open(value, O_RDWR);
	if(fd < 0) {
		return -1;
	}
	write(fd, "0", 1);
	close(fd);
	return 0;
}

/**
 * @brief efuse_vddq_set: 当进行efuse写操作时，需要进行gpio拉低，
 * 控制EFUSE_AVP 使能进行写操作
 *
 * @param[] is_on
 */
static void efuse_vddq_set(unsigned long is_on)
{
	static int flag = 0;
	if(flag == 0) {
		gpio_init(g_efuse.gpio_num);
		flag++;
	}

	if(is_on) {
		gpio_close(g_efuse.gpio_num);	//拉低
	} 
	// else {
	// 	gpio_open(g_efuse.gpio_num);		//拉高
	// }
}

static unsigned long get_h2clk_rate(void *cpm_base)
{
	//AHB2 parent MPLL ? SCLK_A ?
	int cpccr_val = readl(cpm_base, CPM_CPCCR);
	int h2clk_div = (cpccr_val >> 12) & 0xf;
	unsigned int pll_rate = 0;

	switch((cpccr_val >> 24) & 0x3) {
	case 0x0:	/* h2clk disable */
		//TODO
		LOG_ERR("h2clk error, h2clk parent is mpll");
		return 0;
		break;
	case 0x1:	/* sclk_a, EXCLK ? APLL , 0b01*/
		//TODO
		LOG_ERR("sclk error, h2clk parent is mpll");
		return 0;
		break;
	case 0x2:	/*mpll, 0b10*/
		{
			//caculate MPLL rate
			int cpmpcr_val = readl(cpm_base, CPM_CPMPCR);
			int pllm = (cpmpcr_val >> 20) & 0xfff;
			int plln = (cpmpcr_val >> 14) & 0x3f;
			int pllod = (cpmpcr_val >> 11) & 0x7;
			int pllod1 = (cpmpcr_val >> 7) & 0xf;
			pll_rate = PLL_OUT(EXCLK, pllm, plln, pllod, pllod1);
		}
		break;
	}
	return pll_rate == 0 ? EXCLK : pll_rate / (h2clk_div + 1);
}

int init_efuse(int gpio_num)
{
	if(g_efuse.init_already) {
		return 0;
	}

	//	printf("pllout: %u\n", PLL_OUT(EXCLK, 108, 1, 1));
	int dev_fd = open("/dev/mem", O_RDWR | O_NDELAY);
	if(dev_fd < 0) {
		LOG_ERR("open dev mem error\n");
		return -1;
	}

	uint32_t rd_adj, wr_adj;
	uint32_t val, ps;
	int rd_strobe, wr_strobe;

	int i;
	for(i = 0; i < g_efuse.map_num; i ++) {
		g_efuse.minfo[i].map_addr = mmap(NULL, g_efuse.minfo[i].length,
										 PROT_READ | PROT_WRITE, MAP_SHARED,
										 dev_fd, g_efuse.minfo[i].offset);
		if(!g_efuse.minfo[i].map_addr) {
			LOG_ERR("mmap error: num %d\n", i);
			goto MAP_ERROR;
		}
	}

	//gate efuse clk
	writel(g_efuse.minfo[TYPE_CPM].map_addr, CPM_CLKGR0,
		   readl(g_efuse.minfo[TYPE_CPM].map_addr, CPM_CLKGR0) & ~(1 << 1));

	unsigned long rate = get_h2clk_rate(g_efuse.minfo[TYPE_CPM].map_addr);
	if(rate <= 0) {
		LOG_ERR("get_h2clk_rate error\n");
		goto MAP_ERROR;
	}

	ps = 1000000000 / (rate /1000);
	LOG_DEBUG("rate = %lu, ps = %d\n", rate, ps);
	for(i = 0; i < 0xf; i++)
		if((( i + 1) * ps) > 25000)
			break;

	if(i == 0xf) {
		LOG_ERR("get efuse cfg rd_adj fail!\n");
		goto MAP_ERROR;
	}
	rd_adj = i;

	for(i = 0; i < 0xf; i++)
		if(((i + 1) * ps) > 20000)
			break;
	if(i == 0xf) {
		LOG_ERR("get efuse cfg wr_adj fail!\n");
		goto MAP_ERROR;
	}
	wr_adj = i;

	for(i = 0; i < 0xf; i++)
		if(((rd_adj + i + 1) * ps) > 20000)
			break;
	if(i == 0xf) {
		LOG_ERR("get efuse cfg rd_strobe fail!\n");
		goto MAP_ERROR;
	}
	rd_strobe = i;

	for(i = 1; i < 0xfff; i++) {
		val = (wr_adj + i + 2000) * ps;
		if( val > (11000 * 1000) && val < (13000 * 1000))
			/*if(val > 10000)*/
			break;
	}
	if(i >= 0xfff) {
		LOG_ERR("get efuse cfg wd_strobe fail!\n");
		goto MAP_ERROR;
	}
	wr_strobe = i;

	LOG_DEBUG("rd_adj = %d | rd_strobe = %d | "
			  "wr_adj = %d | wr_strobe = %d\n", rd_adj, rd_strobe,
			  wr_adj, wr_strobe);
	/*set configer register*/
	/*val = 1 << 31 | rd_adj << 20 | rd_strobe << 16 | wr_adj << 12 | wr_strobe;*/
	val = rd_adj << 23 | rd_strobe << 18 | wr_adj << 14 | wr_strobe;
	writel(g_efuse.minfo[TYPE_EFUSE_CFG].map_addr, EFUCFG, val);
	g_efuse.init_already = 1;
	g_efuse.gpio_num = gpio_num;

	close(dev_fd);
	return 0;

MAP_ERROR:
	close(dev_fd);
	for(i = 0; i < g_efuse.map_num; i++) {
		if(g_efuse.minfo[i].map_addr)
			munmap(g_efuse.minfo[i].map_addr, g_efuse.minfo[i].length);
	}
	return -1;
}

int deinit_efuse(void)
{
	int i = 0;
	for(i = 0; i < g_efuse.map_num; i++) {
		if(g_efuse.minfo[i].map_addr) {
			munmap(g_efuse.minfo[i].map_addr, g_efuse.minfo[i].length);
			g_efuse.minfo[i].map_addr = NULL;
		}
	}
	g_efuse.init_already = 0;
	return 0;
}

int read_efuse(int seg_id, int offset, unsigned char *buf, int len)
{
	if(!g_efuse.init_already) {
		LOG_ERR("need init first\n");
		return -1;
	}
	void *cfg_addr = g_efuse.minfo[TYPE_EFUSE_CFG].map_addr;
	if(!cfg_addr) {
		LOG_ERR("mmap error\n");
		return -1;
	}

	unsigned int i;
	unsigned char *save_buf = (unsigned char *)buf;
	uint32_t val, addr = 0, remainder, data = 0, count;
	count = len;
	addr = (g_efuse.id2addr[seg_id] + offset) / 4;

	remainder = (g_efuse.id2addr[seg_id] + offset) % 4;

	writel(cfg_addr, EFUSTATE, 0);
	val = addr << 21;
	writel(cfg_addr, EFUCTRL, val);
	val |= 1;
	writel(cfg_addr, EFUCTRL, val);
	while(!(readl(cfg_addr, EFUSTATE) & 1));
	data =  readl(cfg_addr, EFUDATA);

	if ((count + remainder) <= 4) {
		data = data >> (8 * remainder);
		while(count){
			*(save_buf) = data & 0xff;
			data = data >> 8;
			count--;
			save_buf++;
		}
		goto end;
	}else {
		data = data >> (8 * remainder);
		for (i = 0; i < (4 - remainder); i++) {
			*(save_buf) = data & 0xff;
			data = data >> 8;
			count--;
			save_buf++;
		}
	}

	/* Middle word reading */
again:
	if (count > 4) {
		addr++;
		writel(cfg_addr, EFUSTATE, 0);

		val = addr << 21;
		writel(cfg_addr, EFUCTRL, val);
		val |= 1;
		writel(cfg_addr, EFUCTRL, val);

		while(!(readl(cfg_addr, EFUSTATE) & 1));
		data = readl(cfg_addr, EFUDATA);

		for (i = 0; i < 4; i++) {
			*(save_buf) = data & 0xff;
			data = data >> 8;
			count--;
			save_buf++;
		}

		goto again;
	}

	/* Final word reading */
	addr++;
	writel(cfg_addr, EFUSTATE, 0);

	val = addr << 21;
	writel(cfg_addr, EFUCTRL, val);
	val |= 1;
	writel(cfg_addr, EFUCTRL, val);

	while(!(readl(cfg_addr, EFUSTATE) & 1));
	data = readl(cfg_addr, EFUDATA);

	while(count) {
		*(save_buf) = data & 0xff;
		data = data >> 8;
		count--;
		save_buf++;
	}

	writel(cfg_addr, EFUSTATE, 0);
	return 0;
end:
	return 0;
}

int write_efuse(int seg_id, int offset, unsigned char *buf, int len, int force)
{

	if(!g_efuse.init_already) {
		LOG_ERR("need init first\n");
		return -1;
	}
	void *cfg_addr = g_efuse.minfo[TYPE_EFUSE_CFG].map_addr;
	if(!cfg_addr) {
		LOG_ERR("mmap error\n");
		return -1;
	}

	unsigned int val, addr = 0, remainder;
	unsigned int count = len;
	unsigned char *save_buf = (unsigned char *)buf;
	unsigned char data[4] = {0};
	unsigned int i;

	efuse_vddq_set(1);
	addr = (g_efuse.id2addr[seg_id] + offset) / 4;

	remainder = (g_efuse.id2addr[seg_id] + offset) % 4;

	if ((count + remainder) <= 4) {
		for (i = 0; i < remainder; i++)
			data[i] = 0;
		while(count) {
			data[i] = *save_buf;
			save_buf++;
			i++;
			count--;
		}
		while(i < 4) {
			data[i] = 0;
			i++;
		}

		//40 change
		writel(cfg_addr, EFUCTRL, 0x20);

		val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

		writel(cfg_addr, EFUDATA, val);
		val = addr << 21 | 1 << 15 | 1 << 5;
		writel(cfg_addr, EFUCTRL, val);

		//TODO efuse gpio set 1
		udelay(UDELAY_US);

		val |= 2;
		writel(cfg_addr, EFUCTRL, val);

		while(!(readl(cfg_addr, EFUSTATE) & 2));

		writel(cfg_addr, EFUCTRL, 0);
		writel(cfg_addr, EFUSTATE, 0);

		goto end;
	}else {
		for (i = 0; i < remainder; i++)
			data[i] = 0;
		for (i = remainder; i < 4; i++) {
			data[i] = *save_buf;
			save_buf++;
			count--;
		}

		//40 change
		writel(cfg_addr, EFUCTRL, 0x20);
		val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

		writel(cfg_addr, EFUDATA, val);
		val = addr << 21 | 1 << 15 | 1 << 5;
		writel(cfg_addr, EFUCTRL, val);

		udelay(UDELAY_US);
		val |= 2;
		writel(cfg_addr, EFUCTRL, val);

		while(!(readl(cfg_addr, EFUSTATE) & 2));

		writel(cfg_addr, EFUCTRL, 0);
		writel(cfg_addr, EFUSTATE, 0);
	}
again:
	if (count > 4) {
		addr++;
		for (i = 0; i < 4; i++) {
			data[i] = *save_buf;
			save_buf++;
			count--;
		}

		//40 change
		writel(cfg_addr, EFUCTRL, 0x20);
		val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
		writel(cfg_addr, EFUDATA, val);
		val = addr << 21 | 1 << 15 | 1 << 5;
		writel(cfg_addr, EFUCTRL, val);

		udelay(UDELAY_US);
		val |= 2;
		writel(cfg_addr, EFUCTRL, val);

		while(!(readl(cfg_addr, EFUSTATE) & 2));

		writel(cfg_addr, EFUCTRL, 0);
		writel(cfg_addr, EFUSTATE, 0);

		goto again;
	}

	/* Final word writing */
	addr++;
	for (i = 0; i < 4; i++) {
		if (count) {
			data[i] = *save_buf;
			save_buf++;
			count--;
		}else {
			data[i] = 0;
		}
	}

	//40 change
	writel(cfg_addr, EFUCTRL, 0x20);
	val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	writel(cfg_addr, EFUDATA, val);
	val = addr << 21 | 1 << 15 | 1 << 5;
	writel(cfg_addr, EFUCTRL, val);

	udelay(UDELAY_US);
	val |= 2;
	writel(cfg_addr, EFUCTRL, val);

	while(!(readl(cfg_addr, EFUSTATE) & 2));

	writel(cfg_addr, EFUCTRL, 0);
	writel(cfg_addr, EFUSTATE, 0);

	efuse_vddq_set(0);
	return 0;

end:
	LOG_DEBUG("write end\n");
	efuse_vddq_set(0);
	return 0;
}

#if 1
int special_trim_segment_efuse_write(unsigned int value)
{
	void *cfg_addr = g_efuse.minfo[TYPE_EFUSE_CFG].map_addr;
	if(!cfg_addr) {
		LOG_ERR("mmap error\n");
		return -1;
	}

	unsigned int val;
	unsigned int addr = 0;

	efuse_vddq_set(1);
	writel(cfg_addr, EFUSTATE, 0);

	val = 0xa55aa55a;
	writel(cfg_addr, EFUSPEEN, val);
	writel(cfg_addr, EFUCTRL, 0x20);
	
	val = value;
	writel(cfg_addr, EFUDATA, val);

	/* t40 address needs to be changed,aligned to word */
	addr = TRIM_ADDR / 4;
	val = addr << 21 | 1 << 15 | 1 << 5;
	writel(cfg_addr, EFUCTRL, val);

	udelay(UDELAY_US);

	val |= 1 << 1;
	writel(cfg_addr, EFUCTRL, val);

	while(!(readl(cfg_addr, EFUSTATE) & 0x2));

	writel(cfg_addr, EFUCTRL, 0);
	writel(cfg_addr, EFUSTATE, 0);
	writel(cfg_addr, EFUSPEEN, 0);

	efuse_vddq_set(0);

}
#endif
