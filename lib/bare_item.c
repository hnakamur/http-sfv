#include "hsfv.h"

const char *parse_boolean(const char *buf, const char *buf_end, int *boolean, int *ret)
{
    if (buf + 2 > buf_end) {
        *ret = HSFV_ERR_EOF;
        return NULL;
    }
    if (*buf != '?') {
        *ret = HSFV_ERR_INVALID;
        return NULL;
    }
    ++buf;

    if (*buf == '0') {
        *boolean = 0;
        *ret = HSFV_OK;
        return ++buf;
    } else if (*buf == '1') {
        *boolean = 1;
        *ret = HSFV_OK;
        return ++buf;
    }
    *ret = HSFV_ERR_INVALID;
    return NULL;
}
