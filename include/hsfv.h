#ifndef hsfv_h
#define hsfv_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

enum {
    HSFV_OK = 0,
    HSFV_ERR = -1,
    HSFV_ERR_EOF = -2,
    HSFV_ERR_INVALID = -3,
};

const char *parse_boolean(const char *buf, const char *buf_end, int *boolean, int *ret);

/**
 * buffer structure compatible with iovec
 */
typedef struct st_hsfv_iovec_t {
    char *base;
    size_t len;
} hsfv_iovec_t;

typedef struct st_hsfv_bare_item_t {
    enum {
        HSFV_BARE_ITEM_TYPE_INTEGER = 0,
        HSFV_BARE_ITEM_TYPE_DECIMAL,
        HSFV_BARE_ITEM_TYPE_STRING,
        HSFV_BARE_ITEM_TYPE_TOKEN,
        HSFV_BARE_ITEM_TYPE_BINARY,
        HSFV_BARE_ITEM_TYPE_BOOLEAN,
    } type;

    union {
        int64_t integer;
        double decimal;
        hsfv_iovec_t string;
        hsfv_iovec_t token;
        hsfv_iovec_t bytes;
        int boolean;
    } data;
} hsfv_bare_item_t;

#ifdef __cplusplus
}
#endif

#endif
