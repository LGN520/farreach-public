#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

unsigned int crc32(unsigned char *message, uint32_t size);

uint16_t cksum16(const char* buf, uint32_t len);

#endif
