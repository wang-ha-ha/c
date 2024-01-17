#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "openssl/rsa.h"
#include "openssl/err.h"
#include "openssl/evp.h"
#include "openssl/sha.h"

#include "openssl/sha.h"

#include "pss.h"
#include "rsa.h"

#include "scboot_rsa_tool.h"

#define UBOOT_BUF_SIZE 1024*1024

#define SPL_SIG_OFFSET 512
#define RSA_N_OFFSET 768
#define SPL_OFFSET 2*1024

#define NEW_UBOOT_FILE "./u-boot-with-spl_scb.bin"
#define SD_BIN 0
#define SD_CPM_OFFSET  256
#define BLSWAP32(val)													\
    ((uint32_t)((((uint32_t)(val) & (uint32_t)0x000000ffU) << 24) |		\
				(((uint32_t)(val) & (uint32_t)0x0000ff00U) <<	8) |	\
				(((uint32_t)(val) & (uint32_t)0x00ff0000U) >>	8) |	\
				(((uint32_t)(val) & (uint32_t)0xff000000U) >> 24)))

static void show_hex(unsigned char *buf, int len)
{
	int i;
	for(i = 0; i < len; i++) {
		if( i != 0 && i % 4 == 0 )
			printf("\n");
		printf("%02x", buf[i]);
	}
	printf("\n=========================\n");
}

static int get_uboot_mod_n(uint8_t *uboot_buf, int uLen, uint8_t *mod, int mLen)
{
	memcpy(mod, uboot_buf+RSA_N_OFFSET, mLen);
	return 0;
}

static int get_uboot_spl_signature(uint8_t *uboot_buf, int uLen, uint8_t *signature, int sLen)
{
	memcpy(signature, uboot_buf+SPL_SIG_OFFSET, sLen);
	return 0;
}

int main(int argc, char *argv[])
{

	if(argc != 4) {
		printf("please use %s [signed uboot] [pri pem] [type]\n", argv[0]);
		return -1;
	}
	int type = atoi(argv[3]);
    uint8_t *uboot_buf = (uint8_t *)malloc(UBOOT_BUF_SIZE);
    memset(uboot_buf, 0, UBOOT_BUF_SIZE);

    int buflen = get_file_buf(argv[1], uboot_buf, UBOOT_BUF_SIZE);
    if(buflen < 0) {
        return -1;
    }
	printf("uboot buflen: %d\n", buflen);
	int spllen = 0;
	if(type == SD_BIN) {
		spllen = get_spl_len_sd(uboot_buf + SD_CPM_OFFSET);
		printf("SD scboot create\n");
	} else {
		spllen = get_spl_len(uboot_buf);
		printf("sfc scboot create\n");
	}
	if(spllen < 0) {
		printf("spllen error: %d, %d\n", spllen, type);
		return -1;
	}

    uint8_t *spl_buf = (uint8_t *)malloc(spllen);
    uint8_t *spl_sha = (uint8_t *)malloc(SHA256_DIGEST_LENGTH);
    memset(spl_buf, 0, spllen);
    memset(spl_sha, 0, SHA256_DIGEST_LENGTH);
    memcpy(spl_buf, uboot_buf+SPL_OFFSET, spllen);

//get spl sha
    int shalen = get_sha256(spl_buf, spllen, spl_sha, SHA256_DIGEST_LENGTH);
    if(shalen < 0) {
		printf("error, %s, %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

// get mod n
	uint8_t mod_n[256] = {0};
	if(get_uboot_mod_n(uboot_buf, buflen, mod_n, 256) < 0) {
		printf("get uboot mod n error\n");
		exit(-1);
	}
// get signature
	uint8_t signature[256] = {0};
	if(get_uboot_spl_signature(uboot_buf, buflen, signature, 256) < 0) {
		printf("get uboot signature error\n");
		exit(-1);
	}

	int e = 0x10001;

	unsigned int out[64] = {0};
	cache_init();

	f_rsa_public_decrypt(out, (bn_t *)signature, 64, (bn_t *)mod_n, (bn_t *)&e, 64);

	unsigned int tmp[64] = {0};
	memcpy(tmp, out, 64 * sizeof(int));

	int j = 0;
	for(j = 0; j < 64; j++ ) {
		tmp[j] = BLSWAP32(out[j]);
	}

	struct Pss_Info_t pssInfo = {
		.mHash = (char *)spl_sha,
		.lenHash = 32,
		.em = (char *)tmp,
		.lenEm = 256,
		.sha256 = SHA256,
	};

	printf("pss_verify: \n");
	pss_verify(&pssInfo) == 0 ?\
						  printf("success\n") : printf("failed\n");

    return 0;
}
