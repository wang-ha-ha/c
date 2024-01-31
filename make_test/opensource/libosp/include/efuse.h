/**
 * Copyright (C) 2020 Ingenic Semiconductor Co., Ltd.
 * This program is free software; you can redistribute it and/or modify it
 * published by the Free Software Foundation.
 *
 * @file efuse.h
 * @brief Define eFuse address register information
 *
 * @author qli, qi.li@ingenic.com
 * @version 1
 * @date 2020-05-23
 *
 * @attention This resource is only used for T31
 */

#ifndef _EFUSE_H__
#define _EFUSE_H__
#include <stdint.h>

#define EFUCTRL		(0x0)	/* T31, 0x2640000 */
#define EFUCFG		(0x4)	/* T31, 0x00000000 */
#define EFUSTATE	(0x8)	/* T31, 0x0000??01 */
#define EFUDATA		(0xc)	/* T31, 0x00?????? */
#define EFUSPEEN	(0x10)	/* T31, 0x00000000 */
#define EFUSPESEG	(0x14)	/* T31, 0x00000000 */

#define CPM_BASE	(0x10000000)
#define EFUSE_BASE	(0x13540000)		/* T31 */

#define CPM_SIZE		(0xE8)
#define EFUSE_SIZE		(0x100)

#define CPM_CPCCR (0x0)
#define CPM_CPMPCR (0x14)
#define CPM_CLKGR0 (0x20)

#define readl(addr, off) *(volatile unsigned int *)(addr + off)
#define writel(addr, off, val) (*(volatile unsigned int *)(addr + off) = val)

#define PLL_OUT(FREF, PLLM, PLLN, PLLOD, PLLOD1) \
	FREF * ((PLLM +1) * 2 / ( (PLLN + 1) * (PLLOD) * (PLLOD1 + 1)))

#define EXCLK (24 * (1000000))

#define CHIP_ID_ADDR (0x00)
#define CHIP_ID_BAK_ADDR (0x0C)
#define USER_ID_ADDR (0x18)
#define USER_ID_BAK_ADDR (0x1C)
#define SARADC_CAL (0x20)
#define SARADC_BAK_CAL (0x22)
#define TRIM_ADDR (0x24)
#define TRIM_BAK_ADDR (0x25)
#define PROGRAM_PROTECT_ADDR (0x26)
#define PROGRAM_PROTECT_BAK_ADDR (0x27)
#define CPU_ID_ADDR (0x28)
#define CPU_ID_BAK_ADDR (0x2A)
#define SPECIAL_ADDR (0x2C)
#define SPECIAL_BAK_ADDR (0x2E)
#define CUSTOMER_RESV_ADDR (0x30)
#define CUSTOMER_RESV_BAK_ADDR (0x58)
#define SCB_DATA_ADDR (0x80)
#define SCB_DATA_BAK_ADDR (0xC0)

/* for T40 */
enum segment_id {
	CHIP_ID,
	CHIP_ID_BAK,
	USER_ID,
	USER_ID_BAK,
	ADC_CALIB,
	ADC_CALIB_BAK,
	TRIM_DATA,
	TRIM_DATA_BAK,
	PROTECT_ID,
	PROTECT_ID_BAK,
	CPU_ID,
	CPU_ID_BAK,
	SPECIAL_USE,
	SPECIAL_USE_BAK,
	USER_DATA,
	USER_DATA_BAK,
	SCB_DATA,
	SCB_DATA_BAK,
};

enum segment_size {
	CHIP_ID_SIZE = 12,	/* 12 Bytes,  96 bits */
	CHIP_ID_SIZE_BAK = 12,	/* 12 Bytes,  96 bits */
	USER_ID_SIZE = 4,	/*  4 Bytes,  32 bits */
	USER_ID_SIZE_BAK = 4,	/*  4 Bytes,  32 bits */
	ADC_CALIB_SIZE = 2,	/*  2 Bytes,  16 bits */
	ADC_CALIB_SIZE_BAK = 2,	/*  2 Bytes,  16 bits */
	TRIM_DATA_SIZE = 1,	/*  1 Bytes,  8 bits */
	TRIM_DATA_SIZE_BAK = 1,	/*  1 Bytes,  8 bits */
	PROTECT_ID_SIZE = 1,	/*  1 Bytes,   8 bits */
	PROTECT_ID_SIZE_BAK = 1,	/*  1 Bytes,   8 bits */
	CPU_ID_SIZE = 2,		/*  2 Bytes,  16 bits */
	CPU_ID_SIZE_BAK = 2,		/*  2 Bytes,  16 bits */
	SPECIAL_USE_SIZE = 2,    /*  2 Bytes,  16 bits */
	SPECIAL_USE_SIZE_BAK = 2,    /*  2 Bytes,  16 bits */
	USER_DATA_SIZE = 40,	/* 40 Bytes, 320 bits */
	USER_DATA_SIZE_BAK = 40,	/* 40 Bytes, 320 bits */
	SCB_DATA_SIZE = 64,      /* 64 Bytes, 512 bits */
	SCB_DATA_SIZE_BAK = 64      /* 64 Bytes, 512 bits */
};

// did 存放在scb 长度为9
#define DID_OFFSET  33
#define DID_LEN     9
// mac 存放在scb 长度为12
#define MAC_OFFSET  43
#define MAC_LEN     12
// psk 存放在user 长度16
#define PSK_OFFSET  0
#define PSK_LEN     16

#define GPIO_PC(n) (2 * 32 + (n))
#define EfuseWriteEnableGpio GPIO_PC(16)

int initEfuse(int GpioNum);
int deinitEfuse(void);
int writeRsaMode(int RsaE,const char *ModNSha256);

#endif
