#ifndef hsfv_h
#define hsfv_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
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
#define hsfv_roundup(d, a) (((d) + (a - 1)) / (a) * (a))
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

static inline void hsfv_iovec_deinit(hsfv_iovec_t *v,
                                     hsfv_allocator_t *allocator) {
  allocator->free(allocator, (void *)v->base);
}

static inline void hsfv_iovec_const_deinit(hsfv_iovec_const_t *v,
                                           hsfv_allocator_t *allocator) {
  allocator->free(allocator, (void *)v->base);
}

typedef hsfv_iovec_const_t hsfv_key_t;
typedef hsfv_iovec_const_t hsfv_string_t;
typedef hsfv_iovec_const_t hsfv_token_t;
typedef hsfv_iovec_const_t hsfv_bytes_t;

static inline bool hsfv_iovec_const_eq(const hsfv_iovec_const_t *self,
                                       const hsfv_iovec_const_t *other) {
  return self->len == other->len && !memcmp(self->base, other->base, self->len);
}

#define hsfv_key_eq hsfv_iovec_const_eq
#define hsfv_string_eq hsfv_iovec_const_eq
#define hsfv_token_eq hsfv_iovec_const_eq
#define hsfv_bytes_eq hsfv_iovec_const_eq

#define hsfv_key_deinit hsfv_iovec_const_deinit
#define hsfv_string_deinit hsfv_iovec_const_deinit
#define hsfv_token_deinit hsfv_iovec_const_deinit
#define hsfv_bytes_deinit hsfv_iovec_const_deinit

typedef struct st_hsfv_buffer_t {
  hsfv_iovec_t bytes;
  size_t capacity;
} hsfv_buffer_t;

hsfv_err_t htsv_buffer_alloc(hsfv_buffer_t *buf, hsfv_allocator_t *allocator,
                             size_t capacity);
hsfv_err_t htsv_buffer_realloc(hsfv_buffer_t *buf, hsfv_allocator_t *allocator,
                               size_t capacity);
void htsv_buffer_deinit(hsfv_buffer_t *buf, hsfv_allocator_t *allocator);
hsfv_err_t htsv_buffer_ensure_unused_bytes(hsfv_buffer_t *buf,
                                           hsfv_allocator_t *allocator,
                                           size_t len);
hsfv_err_t htsv_buffer_append_byte(hsfv_buffer_t *buf,
                                   hsfv_allocator_t *allocator, const char src);
hsfv_err_t htsv_buffer_append_bytes(hsfv_buffer_t *buf,
                                    hsfv_allocator_t *allocator,
                                    const char *src, size_t len);

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
    bool boolean;
  };
} hsfv_bare_item_t;

bool hsfv_bare_item_eq(const hsfv_bare_item_t *self,
                       const hsfv_bare_item_t *other);

/* Parameters */

typedef struct st_hsfv_parameter_t {
  hsfv_key_t key;
  hsfv_bare_item_t value;
} hsfv_parameter_t;

typedef struct st_hsfv_parameters_t {
  hsfv_parameter_t *params;
  size_t len;
  size_t capacity;
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
  size_t capacity;
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
  size_t capacity;
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
  size_t capacity;
} hsfv_dict_t;

bool hsfv_inner_list_eq(const hsfv_inner_list_t *self,
                        const hsfv_inner_list_t *other);
bool hsfv_item_eq(const hsfv_item_t *self, const hsfv_item_t *other);
bool hsfv_parameter_eq(const hsfv_parameter_t *self,
                       const hsfv_parameter_t *other);
bool hsfv_parameters_eq(const hsfv_parameters_t *self,
                        const hsfv_parameters_t *other);

void hsfv_inner_list_deinit(hsfv_inner_list_t *self,
                            hsfv_allocator_t *allocator);
void hsfv_item_deinit(hsfv_item_t *item, hsfv_allocator_t *allocator);
void hsfv_parameters_deinit(hsfv_parameters_t *parameters,
                            hsfv_allocator_t *allocator);
void hsfv_parameter_deinit(hsfv_parameter_t *parameter,
                           hsfv_allocator_t *allocator);
void hsfv_bare_item_deinit(hsfv_bare_item_t *bare_item,
                           hsfv_allocator_t *allocator);

hsfv_err_t hsfv_parse_inner_list(hsfv_inner_list_t *inner_list,
                                 hsfv_allocator_t *allocator, const char *input,
                                 const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_item(hsfv_item_t *item, hsfv_allocator_t *allocator,
                           const char *input, const char *input_end,
                           const char **out_rest);
hsfv_err_t hsfv_parse_parameters(hsfv_parameters_t *parameters,
                                 hsfv_allocator_t *allocator, const char *input,
                                 const char *input_end,

                                 const char **out_rest);
hsfv_err_t hsfv_parse_bare_item(hsfv_bare_item_t *item,
                                hsfv_allocator_t *allocator, const char *input,
                                const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_boolean(hsfv_bare_item_t *item, const char *input,
                              const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_number(hsfv_bare_item_t *item, const char *input,
                             const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_string(hsfv_bare_item_t *item,
                             hsfv_allocator_t *allocator, const char *input,
                             const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_token(hsfv_bare_item_t *item, hsfv_allocator_t *allocator,
                            const char *input, const char *input_end,
                            const char **out_rest);
hsfv_err_t hsfv_parse_binary(hsfv_bare_item_t *item,
                             hsfv_allocator_t *allocator, const char *input,
                             const char *input_end, const char **out_rest);

hsfv_err_t hsfv_parse_key(hsfv_key_t *key, hsfv_allocator_t *allocator,
                          const char *input, const char *input_end,
                          const char **out_rest);

#include "hsfv/base64.h"
#include "hsfv/ctype.h"

#ifdef __cplusplus
}
#endif

#endif
