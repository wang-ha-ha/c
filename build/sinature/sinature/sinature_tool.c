#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "sinature_rsa_tool.h"

#define BLSWAP32(val)													\
    ((uint32_t)((((uint32_t)(val) & (uint32_t)0x000000ffU) << 24) |		\
				(((uint32_t)(val) & (uint32_t)0x0000ff00U) <<	8) |	\
				(((uint32_t)(val) & (uint32_t)0x00ff0000U) >>	8) |	\
				(((uint32_t)(val) & (uint32_t)0xff000000U) >> 24)))

static void useage(char *bin)
{
    printf("%s sinature_file=[file_path] pri_pem=[file_path] \n", bin);
}

int main(int argc, char *argv[])
{
    if(argc != 3) {
        useage(argv[0]);
        return -1;
    }

    char *sinature_file_path = argv[1];
    char *rsa_pri_pem = argv[2];
    if(sinature_file_path == NULL || rsa_pri_pem == NULL) {
        printf("there is no file\n");
        return -1;
    }

    uint8_t *sinature_file_buf;
	int buflen = get_file_buf1(sinature_file_path, &sinature_file_buf);
	if(buflen < 0) {
		printf("get sinature_file error\n");
		return -1;
	}

	printf("sinature_file buflen: %d\n", buflen);

    /* spl 做sha值*/
    uint8_t *spl_sha = (uint8_t *)malloc(32);
    memset(spl_sha, 0, 32);

    int shalen = get_sha256(sinature_file_buf+256, buflen - 256, spl_sha, 32);
    if(shalen < 0) {
        return -1;
    }
	int i;
	printf("SPL SHA256[%d]:\n",shalen);
	for(i = 0; i < shalen; i++) {
		printf("0x%02x, ", spl_sha[i]);
		if (i % 16 == 15)
			printf("\n");
	}

    uint8_t pribuf[256] = {0};

    int ret = pri_encrypt(rsa_pri_pem, spl_sha, shalen, pribuf, sizeof(pribuf));
    if(ret < 0) {
        printf("rsa pri_encrypt error\n");
        return -1;
    }

	//输入转换word大小端
	uint32_t *pribuf_32 = (uint32_t *)pribuf;
	for (i = 0; i < 256/4; i++) {
		pribuf_32[i] = BLSWAP32(pribuf_32[i]);
	}

	printf("SPL encrypted signature:\n");
	for (i = 0; i < 256; i++) {
		printf("0x%02x, ", pribuf[i]);
		if (i % 16 == 15)
			printf("\n");
	}

    memcpy(sinature_file_buf, pribuf, 256);

	char sinature_out_file_path[1024] = { 0 };
	sprintf(sinature_out_file_path, "%s-signed",sinature_file_path);
    save_buf_to_file(sinature_out_file_path, sinature_file_buf, buflen);

	printf("signature success\n");

    return 0;
}
