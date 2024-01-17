#ifndef __CRC32_H__
#define __CRC32_H__

#include <linux/module.h>

uint32_t crc32(uint32_t crc, const uint8_t *buf, uint32_t len);

#endif /* __CRC32_H__ */
