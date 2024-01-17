#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "sinature_rsa_tool.h"


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

int main(int argc, char *argv[])
{

	if(argc != 3) {
		printf("please use %s [signed uboot] [pri pem]\n", argv[0]);
		return -1;
	}

	uint8_t *sinature_file_buf;
	int buflen = get_file_buf1(argv[1], &sinature_file_buf);
	if(buflen < 0) {
		printf("get sinature_file error\n");
		return -1;
	}

	printf("sinature buflen: %d\n", buflen);

    uint8_t spl_sha[32] = { 0 };

	//get spl sha
    int shalen = get_sha256(sinature_file_buf + 512, buflen - 512, spl_sha, 32);
    if(shalen < 0) {
		printf("error, %s, %d\n", __FUNCTION__, __LINE__);
        return -1;
    }

	printf("SPL SHA256[%d]:\n",shalen);
	for(int i = 0; i < shalen; i++) {
		printf("0x%02x, ", spl_sha[i]);
		if (i % 16 == 15)
			printf("\n");
	}

	uint8_t signature[256] = {0};
	memcpy(signature,sinature_file_buf+256,256);
	printf("SPL encrypted signature:\n");
	for (int i = 0; i < 256; i++) {
		printf("0x%02x, ", signature[i]);
		if (i % 16 == 15)
			printf("\n");
	}

	uint8_t out[32] = {0};

	pub_decrypt(argv[2],signature,256,out,sizeof(out));

	printf("out SHA256:\n");
	for(int i = 0; i < 32; i++) {
		printf("0x%02x, ", out[i]);
		if (i % 16 == 15)
			printf("\n");
	}

    return 0;
}
