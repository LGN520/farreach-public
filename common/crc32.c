// ----------------------------- crc32b --------------------------------

/* This is the basic CRC-32 calculation with some optimization but no
table lookup. The the byte reversal is avoided by shifting the crc reg
right instead of left and by using a reversed 32-bit word to represent
the polynomial.
   When compiled to Cyclops with GCC, this function executes in 8 + 72n
instructions, where n is the number of bytes in the input message. It
should be doable in 4 + 61n instructions.
   If the inner loop is strung out (approx. 5*8 = 40 instructions),
it would take about 6 + 46n instructions. */

#include "crc32.h"
#include <netinet/in.h>

unsigned int crc32(unsigned char* message, uint32_t size) {
    int i, j;
    unsigned int byte, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;
    while (true) {
        byte = message[i];  // Get next byte.
        crc = crc ^ byte;
        for (j = 7; j >= 0; j--) {  // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
        if (i == int(size))
            break;
    }
    return ~crc;
}
uint16_t cksum16(const char* buf, uint32_t len) {
    uint64_t sum = 0;
    const uint64_t *b = reinterpret_cast<const uint64_t *>(buf);
    uint32_t t1, t2;
    uint16_t t3, t4;
    const uint8_t *tail;
    /* Main loop - 8 bytes at a time */
    while (len >= sizeof(uint64_t)) {
      uint64_t s = *b++;
      sum += s;
      if (sum < s) sum++;
      len -= 8;
    }
    /* Handle tail less than 8-bytes long */
    tail = reinterpret_cast<const uint8_t *>(b);
    if (len & 4) {
      uint32_t s = *reinterpret_cast<const uint32_t *>(tail);
      sum += s;
      if (sum < s) sum++;
      tail += 4;
    }
    if (len & 2) {
      uint16_t s = *reinterpret_cast<const uint16_t *>(tail);
      sum += s;
      if (sum < s) sum++;
      tail += 2;
    }
    if (len & 1) {
      uint8_t s = *reinterpret_cast<const uint8_t *>(tail);
      sum += s;
      if (sum < s) sum++;
    }
    /* Fold down to 16 bits */
    t1 = sum;
    t2 = sum >> 32;
    t1 += t2;
    if (t1 < t2) t1++;
    t3 = t1;
    t4 = t1 >> 16;
    t3 += t4;
    if (t3 < t4) t3++;
    return ntohs(~t3);
}


// int main() {
//     unsigned char message[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//                           0x00, 0x00, 0x18, 0x74, 0x51, 0x79, 0x6f, 0xdf, 0x37, 0x38};
    
//     printf("%d\n",cksum16(reinterpret_cast<const char *>(message),sizeof(message))%32768);
// }