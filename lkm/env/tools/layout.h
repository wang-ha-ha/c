#ifndef __LAYOUT_H__
#define __LAYOUT_H__

/* 布局
------------- uboot -------------
IPC 4KB空闲，中间是uboot的env
------------------------------------
------------- 缓冲区 ---------------
4KB的缓冲区
------------------------------------
------------- factory -------------
0-3:   crc32
4-n:   env (字符串形式的kv键值对，每个键值对以0结尾)
------------------------------------
------------- user-backup -------------
0-3:   crc32
4-n:   env (字符串形式的kv键值对，每个键值对以0结尾)
------------------------------------
------------- user -------------
0-3:   crc32
4-n:   env (字符串形式的kv键值对，每个键值对以0结尾)
------------------------------------

两个32KB扇区，第一个扇区只在工厂写号阶段去擦除写入，第二个扇区是应用程序读写的区域

*/

#define ENV_WO      (0)
#define ENV_RO      (1) /* read only */

typedef struct
{
    char * name;
    unsigned int offset;
    unsigned int size;
    unsigned int flag;
} env_layout_t;

static env_layout_t g_env_layout[] =
{
    /* the first 4KB of "Config" is reserved for uboot. */
    // {
    //     .name = "uboot",
    //     .offset = 0x0,
    //     .size = 0x1000, // 4KB
    //     .flag = ENV_RO,
    // },
    /* 和 uboot 中间预留了 4KB 的缓冲区 */
    {
        .name = "factory",
        .offset = 0x2000,
        .size = 0x1000, // 4KB
        .flag = ENV_RO,
    },
    {
        .name = "user-backup",
        .offset = 0x3000,
        .size = 0x2000, // 8KB
        .flag = ENV_RO,
    },
    {
        .name = "user",
        .offset = 0x8000,
        .size = 0x2000, // 8KB
        .flag = ENV_WO,
    },
    {
        .name = "userb",
        .offset = 0xA000,
        .size = 0x2000, // 8KB
        .flag = ENV_WO,
    },
    {
        .name = "point",
        .offset = 0xC000,
        .size = 0x1000, // 4KB
        .flag = ENV_WO,
    },
};

static int g_env_layout_num = sizeof(g_env_layout)/sizeof(env_layout_t);

#endif /* __LAYOUT_H__ */


