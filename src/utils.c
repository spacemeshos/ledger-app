#include "utils.h"

void bin2hex(char *dst, uint8_t *data, uint64_t inlen) {
    static char const hex[] = "0123456789abcdef";
    for (uint64_t i = 0; i < inlen; i++) {
        dst[2*i+0] = hex[(data[i]>>4) & 0x0F];
        dst[2*i+1] = hex[(data[i]>>0) & 0x0F];
    }
    dst[2*inlen] = '\0';
}

int bin2dec(char *dst, uint64_t n) {
    if (n == 0) {
        dst[0] = '0';
        dst[1] = '\0';
        return 1;
    }
    // determine final length
    int len = 0;
    for (uint64_t nn = n; nn != 0; nn /= 10) {
        len++;
    }
    // write digits in big-endian order
    for (int i = len-1; i >= 0; i--) {
        dst[i] = (n % 10) + '0';
        n /= 10;
    }
    dst[len] = '\0';
    return len;
}
