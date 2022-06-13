#ifndef hsfv_base64_h
#define hsfv_base64_h

#ifdef __cplusplus
extern "C" {
#endif

#include "hsfv.h"

#define HSFV_BASE64_ENCODED_LENGTH(len) (((len + 2) / 3) * 4)
#define HSFV_BASE64_DECODED_LENGTH(len) (((len + 3) / 4) * 3)

void hsfv_encode_base64(hsfv_iovec_t *dst, const hsfv_iovec_const_t *src);
hsfv_err_t hsfv_decode_base64(hsfv_iovec_t *dst, const hsfv_iovec_const_t *src);

#ifdef __cplusplus
}
#endif

#endif
