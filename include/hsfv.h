#ifndef hsfv_h
#define hsfv_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HSFV_MAX_INT_LEN 15
#define HSFV_MIN_INT -999999999999999
#define HSFV_MAX_INT 999999999999999

#define HSFV_MAX_DEC_INT_LEN 12
#define HSFV_MAX_DEC_FRAC_LEN 3
#define HSFV_MIN_DEC_INT -999999999999
#define HSFV_MAX_DEC_INT 999999999999

typedef enum {
  HSFV_OK = 0,
  HSFV_ERR = -1,
  HSFV_ERR_OUT_OF_MEMORY = -2,
  HSFV_ERR_EOF = -3,
  HSFV_ERR_INVALID = -4,
  HSFV_ERR_NUMBER_OUT_OF_RANGE = -5,
} hsfv_err_t;

typedef struct st_hsfv_allocator_t hsfv_allocator_t;

struct st_hsfv_allocator_t {
  void *(*alloc)(hsfv_allocator_t *self, size_t size);
  void *(*realloc)(hsfv_allocator_t *self, void *ptr, size_t size);
  void (*free)(hsfv_allocator_t *self, void *ptr);
};

extern hsfv_allocator_t htsv_global_allocator;

#define hsfv_align(d, a) (((d) + (a - 1)) & ~(a - 1))
#define hsfv_max(val1, val2) ((val1 < val2) ? (val2) : (val1))
#define hsfv_min(val1, val2) ((val1 > val2) ? (val2) : (val1))

/**
 * buffer structure compatible with iovec
 */
typedef struct st_hsfv_iovec_t {
  char *base;
  size_t len;
} hsfv_iovec_t;

typedef struct st_hsfv_iovec_const_t {
  const char *base;
  size_t len;
} hsfv_iovec_const_t;

static inline void hsfv_iovec_free(hsfv_allocator_t *allocator,
                                   hsfv_iovec_t *v) {
  allocator->free(allocator, (void *)v->base);
}

static inline void hsfv_iovec_const_free(hsfv_allocator_t *allocator,
                                         hsfv_iovec_const_t *v) {
  allocator->free(allocator, (void *)v->base);
}

typedef hsfv_iovec_const_t hsfv_key_t;
typedef hsfv_iovec_const_t hsfv_string_t;
typedef hsfv_iovec_const_t hsfv_token_t;
typedef hsfv_iovec_const_t hsfv_bytes_t;

#define hsfv_iovec_const_eq(self, other)                                       \
  ((self).len == (other).len && !memcmp((self).base, (other).base, (self).len))

typedef struct st_hsfv_buffer_t {
  hsfv_iovec_t bytes;
  size_t capacity;
} hsfv_buffer_t;

hsfv_err_t htsv_buffer_alloc_bytes(hsfv_allocator_t *allocator,
                                   hsfv_buffer_t *buf, size_t capacity);
hsfv_err_t htsv_buffer_realloc_bytes(hsfv_allocator_t *allocator,
                                     hsfv_buffer_t *buf, size_t capacity);
void htsv_buffer_free_bytes(hsfv_allocator_t *allocator, hsfv_buffer_t *buf);
hsfv_err_t htsv_buffer_ensure_unused_bytes(hsfv_allocator_t *allocator,
                                           hsfv_buffer_t *buf, size_t len);
hsfv_err_t htsv_buffer_ensure_append_byte(hsfv_allocator_t *allocator,
                                          hsfv_buffer_t *buf, const char src);
hsfv_err_t htsv_buffer_ensure_append_bytes(hsfv_allocator_t *allocator,
                                           hsfv_buffer_t *buf, const char *src,
                                           size_t len);

/* Bare Item */

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
    hsfv_string_t string;
    hsfv_token_t token;
    hsfv_bytes_t bytes;
    int boolean;
  };
} hsfv_bare_item_t;

void hsfv_bare_item_deinit(hsfv_allocator_t *allocator,
                           hsfv_bare_item_t *bare_item);

/* Parameters */

typedef struct st_hsfv_parameter_t {
  hsfv_key_t key;
  hsfv_bare_item_t value;
} hsfv_parameter_t;

typedef struct st_hsfv_parameters_t {
  hsfv_parameter_t *params;
  size_t len;
} hsfv_parameters_t;

/* Item */

typedef struct st_hsfv_item_t {
  hsfv_bare_item_t bare_item;
  hsfv_parameters_t parameters;
} hsfv_item_t;

/* Inner List */

typedef struct st_hsfv_inner_list_t {
  hsfv_item_t *items;
  size_t len;
  hsfv_parameters_t parameters;
} hsfv_inner_list_t;

/* List */

typedef enum {
  HSFV_LIST_MEMBER_TYPE_ITEM = 0,
  HSFV_LIST_MEMBER_TYPE_INNER_LIST,
} hsfv_list_member_type_t;

typedef struct st_hsfv_list_member_t {
  hsfv_list_member_type_t type;
  union {
    hsfv_item_t item;
    hsfv_inner_list_t inner_list;
  };
} hsfv_list_member_t;

typedef struct st_hsfv_list_t {
  hsfv_list_member_t *members;
  size_t len;
} hsfv_list_t;

/* Dictionary */

typedef enum {
  HSFV_DICT_MEMBER_TYPE_ITEM = 0,
  HSFV_DICT_MEMBER_TYPE_INNER_LIST,
} hsfv_dict_member_type_t;

typedef struct st_hsfv_dict_member_value_t {
  hsfv_dict_member_type_t type;
  union {
    hsfv_item_t item;
    hsfv_inner_list_t inner_list;
  };
} hsfv_dict_member_value_t;

typedef struct st_hsfv_dict_member_t {
  hsfv_key_t key;
  hsfv_parameters_t parameters;
  hsfv_dict_member_value_t value;
} hsfv_dict_member_t;

typedef struct st_hsfv_dict_t {
  hsfv_dict_member_t *members;
  size_t len;
} hsfv_dict_t;

const char *parse_number(const char *buf, const char *buf_end,
                         hsfv_bare_item_t *item, int *ret);
const char *parse_boolean(const char *buf, const char *buf_end, int *boolean,
                          int *ret);
hsfv_err_t parse_string(hsfv_allocator_t *allocator, const char *input,
                        const char *input_end, hsfv_bare_item_t *item,
                        const char **out_rest);
hsfv_err_t parse_token(hsfv_allocator_t *allocator, const char *input,
                       const char *input_end, hsfv_bare_item_t *item,
                       const char **out_rest);
hsfv_err_t parse_binary(hsfv_allocator_t *allocator, const char *input,
                        const char *input_end, hsfv_bare_item_t *item,
                        const char **out_rest);

hsfv_err_t parse_key(hsfv_allocator_t *allocator, const char *input,
                     const char *input_end, hsfv_key_t *key,
                     const char **out_rest);

#include "hsfv/base64.h"
#include "hsfv/ctype.h"

#ifdef __cplusplus
}
#endif

#endif
