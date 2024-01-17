#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include "scboot_rsa_tool.h"
#define UBOOT_BUF_SIZE 1024*1024

#define SD_UBOOT_OFFSET (0 * 1024)

#define SPL_SIG_OFFSET_DEFAULT 512
#define RSA_N_OFFSET_DEFAULT 768
#define DEFAULT_SPL_OFFSET 2*1024

#define SPL_SIG_OFFSET(type) ((type == SD_BIN ? SD_UBOOT_OFFSET +  SPL_SIG_OFFSET_DEFAULT : SPL_SIG_OFFSET_DEFAULT))
#define RSA_N_OFFSET(type) ((type == SD_BIN ? SD_UBOOT_OFFSET +  RSA_N_OFFSET_DEFAULT : RSA_N_OFFSET_DEFAULT))

#define SD_CPM_OFFSET  (SD_UBOOT_OFFSET + 256)

#define NEW_UBOOT_FILE "./out/u-boot-with-spl-signed.bin"
#define NEW_RSA_N_SHA_FILE "./out/rsa_mod_n_sha256.bin"
#define NEW_RSA_N_FILE "./out/rsa_n.bin"

#define BLSWAP32(val)													\
    ((uint32_t)((((uint32_t)(val) & (uint32_t)0x000000ffU) << 24) |		\
				(((uint32_t)(val) & (uint32_t)0x0000ff00U) <<	8) |	\
				(((uint32_t)(val) & (uint32_t)0x00ff0000U) >>	8) |	\
				(((uint32_t)(val) & (uint32_t)0xff000000U) >> 24)))

static void useage(char *bin)
{
    printf("%s uboot=[file_path] pri_pem=[file_path]  [type] [spl_offset]\n", bin);
}

int main(int argc, char *argv[])
{

    if(argc != 4 && argc != 5) {
        useage(argv[0]);
        return -1;
    }

    char *u_boot_file_path = argv[1];
    char *rsa_pri_pem = argv[2];
    if(u_boot_file_path == NULL || rsa_pri_pem == NULL) {
        printf("there is no file\n");
        return -1;
    }

	int type = atoi(argv[3]);

    uint8_t *uboot_buf = (uint8_t *)malloc(UBOOT_BUF_SIZE);
    memset(uboot_buf, 0, UBOOT_BUF_SIZE);
	int buflen = get_file_buf(u_boot_file_path, uboot_buf, UBOOT_BUF_SIZE);
	if(buflen < 0) {
		printf("get uboot error\n");
		return -1;
	}

	uint32_t spl_offset = argc == 5 ? atoi(argv[4]) : DEFAULT_SPL_OFFSET;

	printf("uboot buflen: %d\n", buflen);
	int spllen = 0;
	if(type == SD_BIN) {
		spllen = get_spl_len_sd(uboot_buf + SD_CPM_OFFSET);
		spl_offset = DEFAULT_SPL_OFFSET + SD_UBOOT_OFFSET;
		printf("SD scboot create\n");
	} else {
		spllen = get_spl_len(uboot_buf);
		printf("sfc scboot create\n");
	}
	if(spllen < 0) {
		printf("spllen error: %d, %d\n", spllen, type);
		return -1;
	}
	printf("spl_offset=%u\n", spl_offset);
	printf("spllen===> %d\n", spllen);
	/*
	 *int spllen = 11072-2048;
	 */
    /* spl buf */
    uint8_t *spl_buf = (uint8_t *)malloc(spllen);
    /* spl 做sha值*/
    uint8_t *spl_sha = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);
    /* 对modulus n做sha值*/
    uint8_t *rsa_n_sha = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);

    memset(spl_buf, 0, spllen);
    memset(spl_sha, 0, SHA256_DIGEST_LENGTH);

    memcpy(spl_buf, uboot_buf+spl_offset, spllen);

    int shalen = get_sha256(spl_buf, spllen, spl_sha, SHA256_DIGEST_LENGTH);
    if(shalen < 0) {
        return -1;
    }
	int i;
	printf("SPL SHA256:\n");
	for(i = 0; i < shalen; i++) {
		printf("0x%02x, ", spl_sha[i]);
		if (i % 16 == 15)
			printf("\n");
	}

    uint8_t module[256] = {0};
    int e = 0;
    uint8_t pribuf[256] = {0};

    int ret = get_module_and_e(rsa_pri_pem, module, sizeof(module), &e);
    if(ret < 0) {
        printf("get module error\n");
        return -1;
    }

	printf("e:%d\n", e);

    ret = pri_encrypt(rsa_pri_pem, spl_sha, shalen, pribuf, sizeof(pribuf));
    if(ret < 0) {
        printf("rsa pri_encrypt error\n");
        return -1;
    }

/*
 *    shalen = get_sha256(module, 256, rsa_n_sha, SHA256_DIGEST_LENGTH);
 *    if(shalen < 0) {
 *        return -1;
 *    }
 *
 */
#if 0
    int j = 0;
    for(; j < 256;j ++) {
        printf("%02x", pribuf[j]);
    }
    printf("\n");
#endif

	//输入转换word大小端
	uint32_t *module_32 = (uint32_t *)module;
	uint32_t *pribuf_32 = (uint32_t *)pribuf;
	for (i = 0; i < 256/4; i++) {
		module_32[i] = BLSWAP32(module_32[i]);
		pribuf_32[i] = BLSWAP32(pribuf_32[i]);
	}

    shalen = get_sha256(module, 256, rsa_n_sha, SHA256_DIGEST_LENGTH);
    if(shalen < 0) {
        return -1;
    }

	uint32_t *rsa_n_sha_32 = (uint32_t *)rsa_n_sha;
	for (i = 0; i < SHA256_DIGEST_LENGTH / 4; i++) {
		rsa_n_sha_32[i] = BLSWAP32(rsa_n_sha_32[i]);
	}

	printf("RSA N SHA256:\n");
	for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		printf("0x%02x, ", rsa_n_sha[i]);
		if (i % 16 == 15)
			printf("\n");
	}

	printf("RSA N MOD:\n");
	for (i = 0; i < 256; i++) {
		printf("0x%02x, ", module[i]);
		if (i % 16 == 15)
			printf("\n");
	}

	printf("SPL encrypted signature:\n");
	for (i = 0; i < 256; i++) {
		printf("0x%02x, ", pribuf[i]);
		if (i % 16 == 15)
			printf("\n");
	}


    memcpy(uboot_buf+SPL_SIG_OFFSET(type), pribuf, 256);
    memcpy(uboot_buf+RSA_N_OFFSET(type), module, 256);

    save_buf_to_file(NEW_UBOOT_FILE, uboot_buf, buflen);
    save_buf_to_file(NEW_RSA_N_SHA_FILE, rsa_n_sha, SHA256_DIGEST_LENGTH);
    save_buf_to_file(NEW_RSA_N_FILE, module, 256);
	printf("signature success\n");

    return 0;
}
