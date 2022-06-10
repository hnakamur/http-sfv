/*
 * Copyright (C) Austin Appleby
 * Copied from
 * https://github.com/nginx/nginx/blob/release-1.22.0/src/core/ngx_murmurhash.c
 */

#include "hsfv.h"

uint32_t
hsfv_murmur_hash2(uint8_t *data, size_t len)
{
    uint32_t  h, k;

    h = 0 ^ len;

    while (len >= 4) {
        k  = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;

        k *= 0x5bd1e995;
        k ^= k >> 24;
        k *= 0x5bd1e995;

        h *= 0x5bd1e995;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
    case 3:
        h ^= data[2] << 16;
        /* fall through */
    case 2:
        h ^= data[1] << 8;
        /* fall through */
    case 1:
        h ^= data[0];
        h *= 0x5bd1e995;
    }

    h ^= h >> 13;
    h *= 0x5bd1e995;
    h ^= h >> 15;

    return h;
}
