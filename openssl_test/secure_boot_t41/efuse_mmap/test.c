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

// did 存放在scb 长度为9
#define DID_OFFSET  33
#define DID_LEN     9
// mac 存放在scb 长度为12
#define MAC_OFFSET  43
#define MAC_LEN     12
// psk 存放在user 长度16
#define PSK_OFFSET  0
#define PSK_LEN     16

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

int set_did(char *did)
{
    write_efuse(SCB_DATA, DID_OFFSET, did, DID_LEN, 1);

    return 0;
}

int get_did(char *did)
{
    read_efuse(SCB_DATA, DID_OFFSET, did, DID_LEN);

    return 0;
}


int set_mac(char *did)
{
    write_efuse(SCB_DATA, MAC_OFFSET+10, did, MAC_LEN, 1);

    return 0;
}

int get_mac(char *did)
{
    read_efuse(SCB_DATA, MAC_OFFSET+10, did, MAC_LEN);

    return 0;
}

int main(int argc, char *argv[])
{
    int ret = init_efuse(80);
	if(ret < 0) {
		LOG_ERR("init efuse error\n");
		return -1;
	}

    // char buf[256] = {0};
    // set_mac("123456789");
    // get_mac(buf);
    // printf("buf:%s\n",buf);
    scb_info_show();

    return 0;
}