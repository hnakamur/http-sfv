#ifndef hsfv_h
#define hsfv_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
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
    HSFV_ERR_FLOAT_ROUNDING_MODE = -6,
} hsfv_err_t;

typedef struct st_hsfv_allocator_t hsfv_allocator_t;

struct st_hsfv_allocator_t {
    void *(*alloc)(hsfv_allocator_t *self, size_t size);
    void *(*realloc)(hsfv_allocator_t *self, void *ptr, size_t size);
    void (*free)(hsfv_allocator_t *self, void *ptr);
};

extern hsfv_allocator_t hsfv_global_allocator;

typedef struct {
    hsfv_allocator_t allocator;
    int fail_index;
    int alloc_count;
} hsfv_failing_allocator_t;

extern hsfv_failing_allocator_t hsfv_failing_allocator;

typedef unsigned char hsfv_byte_t;

hsfv_byte_t *hsfv_bytes_dup(hsfv_allocator_t *allocator, const hsfv_byte_t *src, size_t len);

#define hsfv_align(d, a) (((d) + (a - 1)) & ~(a - 1))
#define hsfv_roundup(d, a) (((d) + (a - 1)) / (a) * (a))
#define hsfv_max(val1, val2) ((val1 < val2) ? (val2) : (val1))
#define hsfv_min(val1, val2) ((val1 > val2) ? (val2) : (val1))

/**
 * buffer structure compatible with iovec
 */
typedef struct st_hsfv_iovec_t {
    hsfv_byte_t *base;
    size_t len;
} hsfv_iovec_t;

bool hsfv_iovec_eq(const hsfv_iovec_t *self, const hsfv_iovec_t *other);
void hsfv_iovec_deinit(hsfv_iovec_t *v, hsfv_allocator_t *allocator);

typedef struct st_hsfv_iovec_const_t {
    const hsfv_byte_t *base;
    size_t len;
} hsfv_iovec_const_t;

bool hsfv_iovec_const_eq(const hsfv_iovec_const_t *self, const hsfv_iovec_const_t *other);
void hsfv_iovec_const_deinit(hsfv_iovec_const_t *v, hsfv_allocator_t *allocator);

typedef struct st_hsfv_key_t {
    const char *base;
    size_t len;
} hsfv_key_t;

typedef struct st_hsfv_string_t {
    const char *base;
    size_t len;
} hsfv_string_t;

typedef struct st_hsfv_token_t {
    const char *base;
    size_t len;
} hsfv_token_t;

typedef struct st_hsfv_byte_seq_t {
    const hsfv_byte_t *base;
    size_t len;
} hsfv_byte_seq_t;

bool hsfv_string_eq(const hsfv_string_t *self, const hsfv_string_t *other);
bool hsfv_key_eq(const hsfv_key_t *self, const hsfv_key_t *other);
bool hsfv_token_eq(const hsfv_token_t *self, const hsfv_token_t *other);
bool hsfv_byte_seq_eq(const hsfv_byte_seq_t *self, const hsfv_byte_seq_t *other);

void hsfv_key_deinit(hsfv_key_t *v, hsfv_allocator_t *allocator);
void hsfv_string_deinit(hsfv_string_t *v, hsfv_allocator_t *allocator);
void hsfv_token_deinit(hsfv_token_t *v, hsfv_allocator_t *allocator);
void hsfv_byte_seq_deinit(hsfv_byte_seq_t *v, hsfv_allocator_t *allocator);

typedef struct st_hsfv_buffer_t {
    hsfv_iovec_t bytes;
    size_t capacity;
} hsfv_buffer_t;

hsfv_err_t hsfv_buffer_alloc(hsfv_buffer_t *buf, hsfv_allocator_t *allocator, size_t capacity);
hsfv_err_t hsfv_buffer_realloc(hsfv_buffer_t *buf, hsfv_allocator_t *allocator, size_t capacity);
void hsfv_buffer_deinit(hsfv_buffer_t *buf, hsfv_allocator_t *allocator);
hsfv_err_t hsfv_buffer_ensure_unused_bytes(hsfv_buffer_t *buf, hsfv_allocator_t *allocator, size_t len);
hsfv_err_t hsfv_buffer_append_byte(hsfv_buffer_t *buf, hsfv_allocator_t *allocator, const char src);
hsfv_err_t hsfv_buffer_append_bytes(hsfv_buffer_t *buf, hsfv_allocator_t *allocator, const char *src, size_t len);

static inline void hsfv_buffer_append_byte_unchecked(hsfv_buffer_t *buf, const char src)
{
    buf->bytes.base[buf->bytes.len++] = src;
}

static inline void hsfv_buffer_append_bytes_unchecked(hsfv_buffer_t *buf, const char *src, size_t len)
{
    memcpy(buf->bytes.base + buf->bytes.len, src, len);
    buf->bytes.len += len;
}

/* Bare Item */

typedef enum {
    HSFV_BARE_ITEM_TYPE_INTEGER = 0,
    HSFV_BARE_ITEM_TYPE_DECIMAL,
    HSFV_BARE_ITEM_TYPE_STRING,
    HSFV_BARE_ITEM_TYPE_TOKEN,
    HSFV_BARE_ITEM_TYPE_BYTE_SEQ,
    HSFV_BARE_ITEM_TYPE_BOOLEAN,
} hsfv_bare_item_type_t;

typedef struct st_hsfv_bare_item_t {
    hsfv_bare_item_type_t type;
    union {
        int64_t integer;
        double decimal;
        hsfv_string_t string;
        hsfv_token_t token;
        hsfv_byte_seq_t byte_seq;
        bool boolean;
    };
} hsfv_bare_item_t;

bool hsfv_bare_item_eq(const hsfv_bare_item_t *self, const hsfv_bare_item_t *other);

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
    hsfv_dict_member_value_t value;
} hsfv_dict_member_t;

typedef struct st_hsfv_dict_t {
    hsfv_dict_member_t *members;
    size_t len;
    size_t capacity;
} hsfv_dictionary_t;

/* Field Value */

typedef enum {
    HSFV_FIELD_VALUE_TYPE_LIST = 0,
    HSFV_FIELD_VALUE_TYPE_DICTIONARY,
    HSFV_FIELD_VALUE_TYPE_ITEM,
} hsfv_field_value_type_t;

typedef struct st_hsfv_field_value_t {
    hsfv_field_value_type_t type;
    union {
        hsfv_list_t list;
        hsfv_dictionary_t dictionary;
        hsfv_item_t item;
    };
} hsfv_field_value_t;

bool hsfv_field_value_is_empty(const hsfv_field_value_t *self);

bool hsfv_field_value_eq(const hsfv_field_value_t *self, const hsfv_field_value_t *other);
bool hsfv_dictionary_eq(const hsfv_dictionary_t *self, const hsfv_dictionary_t *other);
bool hsfv_list_eq(const hsfv_list_t *self, const hsfv_list_t *other);
bool hsfv_inner_list_eq(const hsfv_inner_list_t *self, const hsfv_inner_list_t *other);
bool hsfv_item_eq(const hsfv_item_t *self, const hsfv_item_t *other);
bool hsfv_parameter_eq(const hsfv_parameter_t *self, const hsfv_parameter_t *other);
bool hsfv_parameters_eq(const hsfv_parameters_t *self, const hsfv_parameters_t *other);

void hsfv_field_value_deinit(hsfv_field_value_t *self, hsfv_allocator_t *allocator);
void hsfv_dictionary_deinit(hsfv_dictionary_t *self, hsfv_allocator_t *allocator);
void hsfv_list_deinit(hsfv_list_t *self, hsfv_allocator_t *allocator);
void hsfv_inner_list_deinit(hsfv_inner_list_t *self, hsfv_allocator_t *allocator);
void hsfv_item_deinit(hsfv_item_t *item, hsfv_allocator_t *allocator);
void hsfv_parameters_deinit(hsfv_parameters_t *parameters, hsfv_allocator_t *allocator);
void hsfv_parameter_deinit(hsfv_parameter_t *parameter, hsfv_allocator_t *allocator);
void hsfv_bare_item_deinit(hsfv_bare_item_t *bare_item, hsfv_allocator_t *allocator);

hsfv_err_t hsfv_parse_field_value(hsfv_field_value_t *field_value, hsfv_field_value_type_t field_type, hsfv_allocator_t *allocator,
                                  const char *input, const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_dictionary(hsfv_dictionary_t *dictionary, hsfv_allocator_t *allocator, const char *input,
                                 const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_list(hsfv_list_t *list, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                           const char **out_rest);
hsfv_err_t hsfv_parse_inner_list(hsfv_inner_list_t *inner_list, hsfv_allocator_t *allocator, const char *input,
                                 const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_item(hsfv_item_t *item, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                           const char **out_rest);
hsfv_err_t hsfv_parse_parameters(hsfv_parameters_t *parameters, hsfv_allocator_t *allocator, const char *input,
                                 const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_bare_item(hsfv_bare_item_t *item, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                                const char **out_rest);
hsfv_err_t hsfv_parse_boolean(hsfv_bare_item_t *item, const char *input, const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_number(hsfv_bare_item_t *item, const char *input, const char *input_end, const char **out_rest);
hsfv_err_t hsfv_parse_string(hsfv_bare_item_t *item, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                             const char **out_rest);
hsfv_err_t hsfv_parse_token(hsfv_bare_item_t *item, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                            const char **out_rest);
hsfv_err_t hsfv_parse_byte_seq(hsfv_bare_item_t *item, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                               const char **out_rest);
hsfv_err_t hsfv_parse_key(hsfv_key_t *key, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                          const char **out_rest);

hsfv_err_t hsfv_parse_integer(const char *input, const char *input_end, int64_t *out_integer, const char **out_rest);
hsfv_err_t hsfv_parse_decimal(const char *input, const char *input_end, double *out_decimal, const char **out_rest);

hsfv_err_t hsfv_serialize_field_value(const hsfv_field_value_t *field_value, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_dictionary(const hsfv_dictionary_t *dictionary, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_list(const hsfv_list_t *list, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_item(const hsfv_item_t *item, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_inner_list(const hsfv_inner_list_t *inner_list, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_parameters(const hsfv_parameters_t *parameters, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_key(const hsfv_key_t *key, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_bare_item(const hsfv_bare_item_t *item, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_boolean(bool boolean, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_byte_seq(const hsfv_byte_seq_t *byte_seq, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_token(const hsfv_token_t *token, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_string(const hsfv_string_t *string, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_decimal(double decimal, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);
hsfv_err_t hsfv_serialize_integer(int64_t integer, hsfv_allocator_t *allocator, hsfv_buffer_t *dest);

bool hsfv_skip_boolean(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_number(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_string(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_token(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_key(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_byte_seq(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_bare_item(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_parameters(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_item(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_inner_list(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_dictionary_member_value(const char *input, const char *input_end, const char **out_rest);

void hsfv_skip_sp(const char *input, const char *input_end, const char **out_rest);
void hsfv_skip_ows(const char *input, const char *input_end, const char **out_rest);
bool hsfv_skip_ows_comma_ows(const char *input, const char *input_end, const char **out_rest);

#define HSFV_IS_OWS(c) ((c) == ' ' || (c) == '\t')
#define HSFV_IS_DIGIT(c) ('0' <= (c) && (c) <= '9')
#define HSFV_IS_ASCII(c) ((c) <= '\x7f')

bool hsfv_is_ascii_string(const char *input, const char *input_end);
int hsfv_strncasecmp(const char *s1, const char *s2, size_t n);

extern const char hsfv_base64_char_map[256];

#define HSFV_IS_BASE64_CHAR(c) hsfv_base64_char_map[(unsigned char)(c)]

#define HSFV_BASE64_ENCODED_LENGTH(len) (((len + 2) / 3) * 4)
#define HSFV_BASE64_DECODED_LENGTH(len) (((len + 3) / 4) * 3)

void hsfv_encode_base64(hsfv_iovec_t *dst, const hsfv_iovec_const_t *src);
hsfv_err_t hsfv_decode_base64(hsfv_iovec_t *dst, const hsfv_iovec_const_t *src);
bool hsfv_is_base64_decodable(const hsfv_iovec_const_t *src);

extern const char hsfv_key_leading_char_map[256];
extern const char hsfv_key_trailing_char_map[256];

#define HSFV_IS_KEY_LEADING_CHAR(c) hsfv_key_leading_char_map[(unsigned char)(c)]
#define HSFV_IS_KEY_TRAILING_CHAR(c) hsfv_key_trailing_char_map[(unsigned char)(c)]

extern const char hsfv_token_leading_char_map[256];
extern const char hsfv_token_trailing_char_map[256];

#define HSFV_IS_TOKEN_LEADING_CHAR(c) hsfv_token_leading_char_map[(unsigned char)(c)]
#define HSFV_IS_TOKEN_TRAILING_CHAR(c) hsfv_token_trailing_char_map[(unsigned char)(c)]

#ifdef __cplusplus
}
#endif

#endif
