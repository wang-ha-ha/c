#ifndef __SINATURE_RSA_TOOL_H__
#define __SINATURE_RSA_TOOL_H__

#include <stdint.h>

int get_module_and_e(const char *priName, uint8_t *module, int module_size, int *e);

int pri_encrypt(const char *priname, char *buf, int size, char *out, int o_size);
int pub_decrypt(const char *priname, char *buf, int size, char *out, int o_size);

int get_sha256(const char *buf, int size, uint8_t *out, int out_maxlen);

int get_file_buf(const char *fileName, uint8_t *buf, long size);
int get_file_buf1(const char *fileName, uint8_t **buf);
int get_file_buf2(const char *fileName, uint8_t **buf);
int get_spl_len(uint8_t *buf);
int get_spl_len_sd(uint8_t *buf);

int save_buf_to_file(const char *fileName, uint8_t *buf, long len);

typedef enum {
	SD_BIN,
	NOR_FLASH_BIN,
} UBOOT_TYPE;

#endif

