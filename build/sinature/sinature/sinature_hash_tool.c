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
    printf("%s sinature_file=[file_path]\n", bin);
}

int main(int argc, char *argv[])
{
    if(argc != 2) {
        useage(argv[0]);
        return -1;
    }

    char *sinature_file_path = argv[1];

    if(sinature_file_path == NULL) {
        printf("there is no file\n");
        return -1;
    }

    uint8_t *sinature_file_buf;
	int buflen = get_file_buf2(sinature_file_path, &sinature_file_buf);
	if(buflen < 0) {
		printf("get sinature_file error\n");
		return -1;
	}

	printf("sinature_file buflen: %d\n", buflen);

	int segment_size = (buflen - (16 * 1024)) / 7;

	printf("sinature_file segment_size: %d\n", segment_size);

	char segment_buf[128 * 1024] = {0};

	for(int i = 0;i < 8;i++){
		memcpy(segment_buf + i * (16 * 1024),sinature_file_buf + i * segment_size + 32,16 * 1024);
	}

    /* spl 做sha值*/
    uint8_t *spl_sha = (uint8_t *)malloc(32);
    memset(spl_sha, 0, 32);


    int shalen = get_sha256(segment_buf, sizeof(segment_buf), spl_sha, 32);
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

    memcpy(sinature_file_buf, spl_sha, 32);

	char sinature_out_file_path[1024] = { 0 };
	sprintf(sinature_out_file_path, "%s-signed",sinature_file_path);
    save_buf_to_file(sinature_out_file_path, sinature_file_buf, buflen + 32);

	printf("signature success\n");

    return 0;
}
