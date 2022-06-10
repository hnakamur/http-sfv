#ifndef hsfv_base64_h
#define hsfv_base64_h

#ifdef __cplusplus
extern "C" {
#endif

#include "hsfv.h"

#define hfsv_base64_encoded_length(len) (((len + 2) / 3) * 4)
#define hfsv_base64_decoded_length(len) (((len + 3) / 4) * 3)

void hsfv_encode_base64(hsfv_iovec_t *dst, hsfv_iovec_const_t *src);
hsfv_err_t hsfv_decode_base64(hsfv_iovec_t *dst, hsfv_iovec_const_t *src);

#ifdef __cplusplus
}
#endif

#endif
