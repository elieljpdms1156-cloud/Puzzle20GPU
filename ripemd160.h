#ifndef RIPEMD160_H
#define RIPEMD160_H
#include <stdint.h>
#include <stddef.h>
typedef struct {
    uint32_t h[5];
    uint8_t  buf[64];
    uint32_t buf_len;
    uint64_t total_bytes;
} RIPEMD160_CTX;
void ripemd160_init(RIPEMD160_CTX *ctx);
void ripemd160_update(RIPEMD160_CTX *ctx, const void *data, size_t len);
void ripemd160_final(uint8_t out[20], RIPEMD160_CTX *ctx);
void ripemd160_first4(const uint8_t data[32], uint8_t out[4]);
#endif
