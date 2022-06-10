#ifndef hsfv_h
#define hsfv_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define HSFV_MAX_INT_LEN 15
#define HSFV_MIN_INT -999999999999999
#define HSFV_MAX_INT 999999999999999

#define HSFV_MAX_DEC_INT_LEN 12
#define HSFV_MAX_DEC_FRAC_LEN 3
#define HSFV_MIN_DEC_INT -999999999999
#define HSFV_MAX_DEC_INT 999999999999

enum {
    HSFV_OK = 0,
    HSFV_ERR = -1,
    HSFV_ERR_EOF = -2,
    HSFV_ERR_INVALID = -3,
    HSFV_ERR_NUMBER_OUT_OF_RANGE = -4,
};

/**
 * buffer structure compatible with iovec
 */
typedef struct st_hsfv_iovec_t {
    char *base;
    size_t len;
} hsfv_iovec_t;

typedef struct st_hsfv_allocator_t hsfv_allocator_t;

struct st_hsfv_allocator_t {
    void *(*alloc)(hsfv_allocator_t *self, size_t size);
    void *(*realloc)(hsfv_allocator_t *self, void *ptr, size_t size);
    void (*free)(hsfv_allocator_t *self, void *ptr);
};

extern hsfv_allocator_t htsv_global_allocator;

typedef enum {
    HSFV_BARE_ITEM_TYPE_INTEGER = 0,
    HSFV_BARE_ITEM_TYPE_DECIMAL,
    HSFV_BARE_ITEM_TYPE_STRING,
    HSFV_BARE_ITEM_TYPE_TOKEN,
    HSFV_BARE_ITEM_TYPE_BINARY,
    HSFV_BARE_ITEM_TYPE_BOOLEAN,
} hsfv_bare_item_type_t;

typedef struct st_hsfv_bare_item_t {
    hsfv_bare_item_type_t type;

    union {
        int64_t integer;
        double decimal;
        hsfv_iovec_t string;
        hsfv_iovec_t token;
        hsfv_iovec_t bytes;
        int boolean;
    } data;
} hsfv_bare_item_t;

const char *parse_number(const char *buf, const char *buf_end, hsfv_bare_item_t *item, int *ret);
const char *parse_boolean(const char *buf, const char *buf_end, int *boolean, int *ret);

#ifdef __cplusplus
}
#endif

#endif
