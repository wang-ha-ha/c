/**
 * Copyright (C) 2020 Ingenic Semiconductor Co., Ltd.
 * This program is free software; you can redistribute it and/or modify it
 * published by the Free Software Foundation.
 *
 * @file efuse_tool.c
 * @brief Scboot burn eFuse entry
 *
 * @author qli, qi.li@ingenic.com
 * @version 1
 * @date 2020-04-19
 *
 * @attention This resource is only used for T40
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include <efuse.h>

#define SHA256_LEN_BYTE (32)

// 慎重打开，一旦写了，soc不可恢复
// #define SCB_ENABLE_WRITE 1

/*
 *static bool is_scb_write_already()
 *{
 *#define writed true
 *#define no_writed false
 *    uint8_t scb_buf[SCB_DATA_SIZE] = {0};
 *    read_efuse(SCB_DATA, 0, scb_buf, SCB_DATA_SIZE);
 *    int i = 0;
 *    for(; i < SCB_DATA_SIZE; i++) {
 *        if(scb_buf[i])
 *            return writed;
 *    }
 *    uint8_t protect0 = 0;
 *    read_efuse(TRIM_DATA, 0, &protect0, TRIM_DATA_SIZE);
 *    if(protect0)
 *        return writed;
 *    return no_writed;
 *}
 *
 */

struct ReadCfg
{
	enum segment_id id;
	enum segment_size size;
	const char *msg;
};

struct ReadCfg readcfgs[] = {
	[0] = {
		TRIM_DATA,
		TRIM_DATA_SIZE,
		"trim data seg into"
	},
	[1] = {
		TRIM_DATA_BAK,
		TRIM_DATA_SIZE,
		"trim data bak seg into"
	},
	[2] = {
		SCB_DATA,
		SCB_DATA_SIZE,
		"scb data seg info"
	},
	[3] = {
		SCB_DATA_BAK,
		SCB_DATA_SIZE,
		"scb data bak seg info"
	},
	[4] = {
		USER_DATA,
		USER_DATA_SIZE,
		"user seg info"
	},
	[5] = {
		USER_DATA_BAK,
		USER_DATA_SIZE,
		"user bak seg info"
	}
};

static void show_hex(unsigned char *buf, int len, const char *msg)
{
	printf("==> msg: %s, seg size: %d\n", msg, len);
	int i;
	for(i = 0; i < len; i++) {
		if( i != 0 && i % 4 == 0 )
			printf("\n");
		printf("%02x", buf[i]);
	}
	printf("\n=======================================================\n");
}


#define SCB_READ
#ifdef SCB_READ
static void scb_info_show()
{
	printf("=======================================================\n");
#define MAX_BUF_SIZE (128)
	int i = 0;
	for(; i < sizeof(readcfgs)/sizeof(struct ReadCfg); i++) {
		unsigned char buf[MAX_BUF_SIZE];
		read_efuse(readcfgs[i].id, 0, buf, readcfgs[i].size);
		show_hex(buf, readcfgs[i].size, readcfgs[i].msg);
	}
	printf("=======================================================\n");
}
#endif

extern int special_trim_segment_efuse_write(unsigned int value);
// scb 使能bit 位
// enable scb && rsa mod  e, some bits:
// 0:sc_en, 1:prot_scboot, 2:SC_RSA_E3, 3:SC_USB, 4:SC_SD
static int scb_write_enable(int rsa_e)
{
	int val = 0;
	if(rsa_e == 3) {
		//0b: 11100
		val = (1 | 1 << 1 | 1 << 2);
	} else if(rsa_e == 0x10001) {
		//0b: 11000
		val = (1 | 1 << 1);
	} else {
		LOG_ERR("rsa_e error: %d, val: %d\n", rsa_e, val);
		return -1;
	}

#if SCB_ENABLE_WRITE

	printf("trim val write: %x\n", val);
	if(special_trim_segment_efuse_write(val) < 0) {
		LOG_ERR("write scb enable error\n");
		return -1;
	}

#endif
	return 0;
}


// 写mod_n_sha256, 32Bytes 数据
static int write_scb(const char *mod_n_sha256)
{

	int ret, fd, fileLen = 0;
#define SCB_CHK

#ifdef SCB_CHK
	uint8_t scb_buf[SCB_DATA_SIZE] = {0};
#endif
#define MALLOC_BUF (1024)
	fd = open(mod_n_sha256, O_RDONLY);
	if(fd < 0) {
		LOG_ERR("open file error: %s\n", strerror(errno));
		return -1;
	}

	uint8_t *buf = (uint8_t *)malloc(MALLOC_BUF);
	if(!buf) {
		LOG_ERR("malloc error: %s\n", strerror(errno));
		goto SCB_ERROR;
	}
	memset(buf, 0, MALLOC_BUF);

	fileLen = read(fd, buf, MALLOC_BUF);
	if(fileLen != SHA256_LEN_BYTE) {
		printf("read file error, file error\n");
		goto SCB_ERROR;
	}

	show_hex(buf, fileLen, "mod n sha");

#define SCB_WRITE 1

#if  SCB_WRITE
	ret = write_efuse(SCB_DATA, 0, buf, SHA256_LEN_BYTE, 1);
	if(ret < 0) {
		printf("rsa mod n is error\n");
		goto SCB_ERROR;
	}
#endif

	//read from efuse, scb data, check
#ifdef SCB_CHK
	read_efuse(SCB_DATA, 0, scb_buf, SCB_DATA_SIZE);
	int i = 0;
	for(i = 0; i < SHA256_LEN_BYTE;i++) {
		if(buf[i] != scb_buf[i]) {
			LOG_ERR("==================== efuse write error:\n\n");
			LOG_ERR("==================== efuse write error:\n\n");
			scb_info_show();
			LOG_ERR("==================== efuse write error:\n\n");
			LOG_ERR("==================== efuse write error:\n\n");
			goto SCB_ERROR;
		}
	}

#endif

	close(fd);
	free(buf);

	return 0;
SCB_ERROR:

	close(fd);
	free(buf);

	return -1;
}

static void Useage(const char *exec)
{
	printf("Usage:\n");
	printf("%s ", exec);
	printf(" [e] [gpio_num] [./xx.bin]\n");
	printf("   like: %s 65537 52 rsa_mod_n_sha256.bin\n", exec);
}

int main(int argc, char *argv[])
{
	if(argc != 4) {
		Useage(argv[0]);
		return -1;
	}

	int rsa_e = atoi(argv[1]);
	if(rsa_e != 3 && rsa_e != 0x10001) {
		LOG_ERR("rsa e must equal 3 or 65537\n");
		return -1;
	}

	int gpio_num = atoi(argv[2]);
	const char *mod_n_sha256 = argv[3];
	if(access(mod_n_sha256, F_OK) != 0) {
		LOG_ERR("no file error: %s\n", mod_n_sha256);
		return -1;
	}

	int ret = init_efuse(gpio_num);
	if(ret < 0) {
		LOG_ERR("init efuse error\n");
		return -1;
	}

#ifdef SCB_READ
	scb_info_show();
#endif
	ret = write_scb(mod_n_sha256);
	if(ret < 0) {
		LOG_ERR("scb write error, please check\n");
		return -1;
	}

	ret = scb_write_enable(rsa_e);
	if(ret < 0) {
		LOG_ERR("scb enable error\n");
	}
#ifdef SCB_READ
	scb_info_show();
#endif

	deinit_efuse();
	return 0;
}

