#include "hsfv/base64.h"

static void hsfv_encode_base64_internal(hsfv_iovec_t *dst,
                                        hsfv_iovec_const_t *src,
                                        const u_char *basis, u_int64_t padding);
static hsfv_err_t hsfv_decode_base64_internal(hsfv_iovec_t *dst,
                                              hsfv_iovec_const_t *src,
                                              const u_char *basis);

void hsfv_encode_base64(hsfv_iovec_t *dst, hsfv_iovec_const_t *src) {
  static u_char basis64[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  hsfv_encode_base64_internal(dst, src, basis64, 1);
}

static void hsfv_encode_base64_internal(hsfv_iovec_t *dst,
                                        hsfv_iovec_const_t *src,
                                        const u_char *basis,
                                        u_int64_t padding) {
  const char *s;
  char *d;
  size_t len;

  len = src->len;
  s = src->base;
  d = dst->base;

  while (len > 2) {
    *d++ = basis[(s[0] >> 2) & 0x3f];
    *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
    *d++ = basis[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
    *d++ = basis[s[2] & 0x3f];

    s += 3;
    len -= 3;
  }

  if (len) {
    *d++ = basis[(s[0] >> 2) & 0x3f];

    if (len == 1) {
      *d++ = basis[(s[0] & 3) << 4];
      if (padding) {
        *d++ = '=';
      }

    } else {
      *d++ = basis[((s[0] & 3) << 4) | (s[1] >> 4)];
      *d++ = basis[(s[1] & 0x0f) << 2];
    }

    if (padding) {
      *d++ = '=';
    }
  }

  dst->len = d - dst->base;
}

hsfv_err_t hsfv_decode_base64(hsfv_iovec_t *dst, hsfv_iovec_const_t *src) {
  static u_char basis64[] = {
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
      52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
      77, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
      77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
      41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
      77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77};

  return hsfv_decode_base64_internal(dst, src, basis64);
}

static hsfv_err_t hsfv_decode_base64_internal(hsfv_iovec_t *dst,
                                              hsfv_iovec_const_t *src,
                                              const u_char *basis) {
  size_t len;
  const char *s;
  char *d;

  for (len = 0; len < src->len; len++) {
    if (src->base[len] == '=') {
      break;
    }

    if (basis[src->base[len]] == 77) {
      return HSFV_ERR;
    }
  }

  if (len % 4 == 1) {
    return HSFV_ERR;
  }

  s = src->base;
  d = dst->base;

  while (len > 3) {
    *d++ = (u_char)(basis[s[0]] << 2 | basis[s[1]] >> 4);
    *d++ = (u_char)(basis[s[1]] << 4 | basis[s[2]] >> 2);
    *d++ = (u_char)(basis[s[2]] << 6 | basis[s[3]]);

    s += 4;
    len -= 4;
  }

  if (len > 1) {
    *d++ = (u_char)(basis[s[0]] << 2 | basis[s[1]] >> 4);
  }

  if (len > 2) {
    *d++ = (u_char)(basis[s[1]] << 4 | basis[s[2]] >> 2);
  }

  dst->len = d - dst->base;

  return HSFV_OK;
}
