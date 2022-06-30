#include "hsfv.h"

#include <fenv.h>
#include <math.h>

// clang-format off

const char hsfv_key_leading_char_map[256] = {
    ['*'] = '\1',
    ['a'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', // to 'z'
};

const char hsfv_key_trailing_char_map[256] = {
    ['*'] = '\1',
    ['-'] = '\1', ['.'] = '\1',
    ['0'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', // to '9'
    ['_'] = '\1',
    ['a'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', // to 'z'
};

const char hsfv_token_leading_char_map[256] = {
    ['*'] = '\1',
    ['A'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', // to 'Z'
    ['a'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', // to 'z'
};

/*
 * token_trailing_char = tchar / ":" / "/"
 * tchar is defined at
 * https://www.rfc-editor.org/rfc/rfc7230.html#section-3.2.6
 */
const char hsfv_token_trailing_char_map[256] = {
    ['!'] = '\1',
    ['#'] = '\1', ['$'] = '\1', ['%'] = '\1', ['&'] = '\1', ['\''] = '\1',
    ['*'] = '\1', ['+'] = '\1',
    ['-'] = '\1', ['.'] = '\1', ['/'] = '\1',
    ['0'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', // to '9'
    [':'] = '\1',
    ['A'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', // to 'Z'
    ['^'] = '\1', ['_'] = '\1', ['`'] = '\1',
    ['a'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', // to 'z'
    ['|'] = '\1', ['~'] = '\1',
};

const char hsfv_base64_char_map[256] = {
    ['+'] = '\1',
    ['/'] = '\1',
    ['0'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', // to '9'
    ['='] = '\1',
    ['A'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', // to 'Z'
    ['a'] = '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1', '\1',
            '\1', '\1', '\1', '\1', '\1', '\1', // to 'z'
};

// clang-format on

bool hsfv_string_eq(const hsfv_string_t *self, const hsfv_string_t *other)
{
    return self->len == other->len && !memcmp(self->base, other->base, self->len);
}

bool hsfv_key_eq(const hsfv_key_t *self, const hsfv_key_t *other)
{
    return self->len == other->len && !memcmp(self->base, other->base, self->len);
}

bool hsfv_token_eq(const hsfv_token_t *self, const hsfv_token_t *other)
{
    return self->len == other->len && !memcmp(self->base, other->base, self->len);
}

bool hsfv_byte_seq_eq(const hsfv_byte_seq_t *self, const hsfv_byte_seq_t *other)
{
    return self->len == other->len && !memcmp(self->base, other->base, self->len);
}

void hsfv_key_deinit(hsfv_key_t *v, hsfv_allocator_t *allocator)
{
    allocator->free(allocator, (void *)v->base);
}

void hsfv_string_deinit(hsfv_string_t *v, hsfv_allocator_t *allocator)
{
    allocator->free(allocator, (void *)v->base);
}

void hsfv_token_deinit(hsfv_token_t *v, hsfv_allocator_t *allocator)
{
    allocator->free(allocator, (void *)v->base);
}

void hsfv_byte_seq_deinit(hsfv_byte_seq_t *v, hsfv_allocator_t *allocator)
{
    allocator->free(allocator, (void *)v->base);
}

bool hsfv_bare_item_eq(const hsfv_bare_item_t *self, const hsfv_bare_item_t *other)
{
    if (self->type != other->type) {
        return false;
    }
    switch (self->type) {
    case HSFV_BARE_ITEM_TYPE_INTEGER:
        return self->integer == other->integer;
    case HSFV_BARE_ITEM_TYPE_DECIMAL:
        return self->decimal == other->decimal;
    case HSFV_BARE_ITEM_TYPE_STRING:
        return hsfv_string_eq(&self->string, &other->string);
    case HSFV_BARE_ITEM_TYPE_TOKEN:
        return hsfv_token_eq(&self->token, &other->token);
    case HSFV_BARE_ITEM_TYPE_BYTE_SEQ:
        return hsfv_byte_seq_eq(&self->byte_seq, &other->byte_seq);
    case HSFV_BARE_ITEM_TYPE_BOOLEAN:
        return self->boolean == other->boolean;
    default:
        return false;
    }
}

void hsfv_bare_item_deinit(hsfv_bare_item_t *bare_item, hsfv_allocator_t *allocator)
{
    switch (bare_item->type) {
    case HSFV_BARE_ITEM_TYPE_STRING:
        hsfv_string_deinit(&bare_item->string, allocator);
        break;
    case HSFV_BARE_ITEM_TYPE_TOKEN:
        hsfv_token_deinit(&bare_item->token, allocator);
        break;
    case HSFV_BARE_ITEM_TYPE_BYTE_SEQ:
        hsfv_byte_seq_deinit(&bare_item->byte_seq, allocator);
        break;
    default:
        /* do nothing */
        break;
    }
}

/* Boolean */

hsfv_err_t hsfv_serialize_boolean(bool boolean, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    hsfv_err_t err;

    err = hsfv_buffer_ensure_unused_bytes(dest, allocator, 2);
    if (err) {
        return err;
    }

    hsfv_buffer_append_byte_unchecked(dest, '?');
    hsfv_buffer_append_byte_unchecked(dest, boolean ? '1' : '0');
    return HSFV_OK;
}

hsfv_err_t hsfv_parse_boolean(hsfv_bare_item_t *item, const char *input, const char *input_end, const char **out_rest)
{
    if (input == input_end) {
        return HSFV_ERR_EOF;
    }
    if (*input != '?') {
        return HSFV_ERR_INVALID;
    }
    ++input;

    if (input == input_end) {
        return HSFV_ERR_EOF;
    }
    if (*input == '1') {
        item->type = HSFV_BARE_ITEM_TYPE_BOOLEAN;
        item->boolean = true;
        if (out_rest) {
            *out_rest = ++input;
        }
        return HSFV_OK;
    }
    if (*input == '0') {
        item->type = HSFV_BARE_ITEM_TYPE_BOOLEAN;
        item->boolean = false;
        if (out_rest) {
            *out_rest = ++input;
        }
        return HSFV_OK;
    }
    return HSFV_ERR_INVALID;
}

/* Number */

static const size_t integer_tmp_bufsize = 17;

hsfv_err_t hsfv_serialize_integer(int64_t integer, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    if (integer < HSFV_MIN_INT || HSFV_MAX_INT < integer) {
        return HSFV_ERR_INVALID;
    }

    char tmp[integer_tmp_bufsize];
    int n = snprintf(tmp, integer_tmp_bufsize, "%ld", integer);
    hsfv_err_t err = hsfv_buffer_ensure_unused_bytes(dest, allocator, n);
    if (err) {
        return err;
    }

    hsfv_buffer_append_bytes_unchecked(dest, tmp, n);
    return HSFV_OK;
}

static const size_t decimal_tmp_bufsize = 1 + HSFV_MAX_DEC_INT_LEN + 1 + HSFV_MAX_DEC_FRAC_LEN + 1;

#pragma STDC FENV_ACCESS ON
hsfv_err_t hsfv_serialize_decimal(double decimal, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    int prev_rounding = fegetround();
    if (prev_rounding != FE_TONEAREST && fesetround(FE_TONEAREST)) {
        return HSFV_ERR_FLOAT_ROUNDING_MODE;
    }

    double rounded = rint(decimal * 1000);

    if (prev_rounding != FE_TONEAREST && fesetround(prev_rounding)) {
        return HSFV_ERR_FLOAT_ROUNDING_MODE;
    }

    char tmp[decimal_tmp_bufsize];
    int n = snprintf(tmp, decimal_tmp_bufsize, "%.3f", rounded / 1000);
    if (n > decimal_tmp_bufsize) {
        return HSFV_ERR_INVALID;
    }
    const char *start = tmp;
    if (*start == '-') {
        ++start;
    }
    const char *pos = strchr(tmp, '.');
    if (pos - start > HSFV_MAX_DEC_INT_LEN) {
        return HSFV_ERR_INVALID;
    }
    const char *end = &tmp[n - 1];
    for (; end > pos + 1; end--) {
        if (*end != '0') {
            break;
        }
    }

    size_t len = end + 1 - tmp;
    hsfv_err_t err = hsfv_buffer_ensure_unused_bytes(dest, allocator, len);
    if (err) {
        return err;
    }

    hsfv_buffer_append_bytes_unchecked(dest, tmp, len);
    return HSFV_OK;
}
#pragma STDC FENV_ACCESS OFF

hsfv_err_t hsfv_parse_number(hsfv_bare_item_t *item, const char *input, const char *input_end, const char **out_rest)
{
    if (input == input_end) {
        return HSFV_ERR_EOF;
    }

    const char *digit_start = input;
    if (*input == '-') {
        ++digit_start;
    }
    const char *p = digit_start;
    if (p == input_end) {
        return HSFV_ERR_EOF;
    }
    if (!HSFV_IS_DIGIT(*p)) {
        return HSFV_ERR_INVALID;
    }
    ++p;

    const char *dot = NULL;
    const char *out_of_range = digit_start + HSFV_MAX_INT_LEN;
    while (p < input_end) {
        if (p >= out_of_range) {
            return HSFV_ERR_NUMBER_OUT_OF_RANGE;
        }

        char ch = *p;
        if (HSFV_IS_DIGIT(ch)) {
            ++p;
            continue;
        }

        if (ch == '.') {
            if (dot) {
                return HSFV_ERR_INVALID;
            }
            if (p > digit_start + HSFV_MAX_DEC_INT_LEN) {
                return HSFV_ERR_NUMBER_OUT_OF_RANGE;
            }
            dot = p;
            ++p;
            out_of_range = p + HSFV_MAX_DEC_FRAC_LEN;
            continue;
        }

        break;
    }
    const char *end = p;

    if (dot) {
        if (end == dot + 1) {
            return HSFV_ERR_INVALID;
        }

        char temp[1 + HSFV_MAX_DEC_INT_LEN + 1 + HSFV_MAX_DEC_FRAC_LEN + 1];
        size_t input_len = end - input;
        memcpy(temp, input, input_len);
        temp[input_len] = '\0';
        item->decimal = strtod(temp, NULL);
        item->type = HSFV_BARE_ITEM_TYPE_DECIMAL;
    } else {
        char temp[1 + HSFV_MAX_INT_LEN + 1];
        size_t input_len = end - input;
        memcpy(temp, input, input_len);
        temp[input_len] = '\0';
        item->integer = strtoll(temp, NULL, 10);
        item->type = HSFV_BARE_ITEM_TYPE_INTEGER;
    }
    if (out_rest) {
        *out_rest = end;
    }

    return HSFV_OK;
}

hsfv_err_t hsfv_parse_integer(const char *input, const char *input_end, int64_t *out_integer, const char **out_rest)
{
    if (input == input_end) {
        return HSFV_ERR_EOF;
    }

    const char *digit_start = input;
    if (*digit_start == '-') {
        ++digit_start;
    }
    const char *p = digit_start;
    if (p == input_end) {
        return HSFV_ERR_EOF;
    }

    if (!HSFV_IS_DIGIT(*p)) {
        return HSFV_ERR_INVALID;
    }
    ++p;

    const char *out_of_range = digit_start + HSFV_MAX_INT_LEN;
    for (; p < input_end; ++p) {
        if (p >= out_of_range) {
            return HSFV_ERR_NUMBER_OUT_OF_RANGE;
        }

        char ch = *p;
        if (HSFV_IS_DIGIT(ch)) {
            continue;
        }

        if (ch == '.') {
            return HSFV_ERR_INVALID;
        }

        break;
    }

    const char *end = p;
    if (out_integer) {
        char temp[1 + HSFV_MAX_INT_LEN + 1];
        size_t input_len = end - input;
        memcpy(temp, input, input_len);
        temp[input_len] = '\0';
        *out_integer = strtoll(temp, NULL, 10);
    }
    if (out_rest) {
        *out_rest = end;
    }
    return HSFV_OK;
}

hsfv_err_t hsfv_parse_decimal(const char *input, const char *input_end, double *out_decimal, const char **out_rest)
{
    if (input == input_end) {
        return HSFV_ERR_EOF;
    }

    const char *digit_start = input;
    if (*input == '-') {
        ++digit_start;
    }
    const char *p = digit_start;
    if (p == input_end) {
        return HSFV_ERR_EOF;
    }

    if (!HSFV_IS_DIGIT(*p)) {
        return HSFV_ERR_INVALID;
    }
    ++p;

    const char *dot = NULL;
    const char *out_of_range = digit_start + HSFV_MAX_DEC_INT_LEN;
    for (; p < input_end; ++p) {
        if (p >= out_of_range) {
            return HSFV_ERR_NUMBER_OUT_OF_RANGE;
        }

        char ch = *p;
        if (HSFV_IS_DIGIT(ch)) {
            continue;
        }

        if (ch == '.') {
            if (dot) {
                return HSFV_ERR_INVALID;
            }
            dot = p;
            out_of_range = dot + 1 + HSFV_MAX_DEC_FRAC_LEN;
            continue;
        }

        break;
    }

    const char *end = p;
    if (!dot || end == dot + 1) {
        return HSFV_ERR_INVALID;
    }

    if (out_decimal) {
        char temp[1 + HSFV_MAX_DEC_INT_LEN + 1 + HSFV_MAX_DEC_FRAC_LEN + 1];
        size_t input_len = end - input;
        memcpy(temp, input, input_len);
        temp[input_len] = '\0';
        *out_decimal = strtod(temp, NULL);
    }
    if (out_rest) {
        *out_rest = end;
    }
    return HSFV_OK;
}

/* String */

hsfv_err_t hsfv_serialize_string(const hsfv_string_t *string, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    hsfv_err_t err;
    size_t escape_count = 0;

    for (const char *p = string->base; p < string->base + string->len; ++p) {
        if (*p <= '\x1f' || '\x7f' <= *p) {
            return HSFV_ERR_INVALID;
        }
        if (*p == '\\' || *p == '"') {
            escape_count++;
        }
    }

    err = hsfv_buffer_ensure_unused_bytes(dest, allocator, string->len + escape_count + 2);
    if (err) {
        return err;
    }

    hsfv_buffer_append_byte_unchecked(dest, '"');
    for (const char *p = string->base; p < string->base + string->len; ++p) {
        if (*p == '\\' || *p == '"') {
            hsfv_buffer_append_byte_unchecked(dest, '\\');
        }
        hsfv_buffer_append_byte_unchecked(dest, *p);
    }
    hsfv_buffer_append_byte_unchecked(dest, '"');

    return HSFV_OK;
}

#define STRING_INITIAL_CAPACITY 8

hsfv_err_t hsfv_parse_string(hsfv_bare_item_t *item, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                             const char **out_rest)
{
    hsfv_err_t err;
    hsfv_buffer_t buf;
    char c;

    err = hsfv_buffer_alloc(&buf, allocator, STRING_INITIAL_CAPACITY);
    if (err) {
        return err;
    }

    c = *input;
    if (c != '"') {
        err = HSFV_ERR_INVALID;
        goto error;
    }
    ++input;

    for (; input < input_end; ++input) {
        c = *input;
        if (c == '\\') {
            ++input;
            if (input == input_end) {
                err = HSFV_ERR_INVALID;
                goto error;
            }
            c = *input;
            if (c != '"' && c != '\\') {
                err = HSFV_ERR_INVALID;
                goto error;
            }
            err = hsfv_buffer_append_byte(&buf, allocator, c);
            if (err) {
                goto error;
            }
        } else if (c == '"') {
            item->type = HSFV_BARE_ITEM_TYPE_STRING;
            item->string.base = (const char *)buf.bytes.base;
            item->string.len = buf.bytes.len;
            if (out_rest) {
                *out_rest = ++input;
            }
            return HSFV_OK;
        } else if (c <= '\x1f' || '\x7f' <= c) {
            err = HSFV_ERR_INVALID;
            goto error;
        } else {
            err = hsfv_buffer_append_byte(&buf, allocator, c);
            if (err) {
                goto error;
            }
        }
    }

    err = HSFV_ERR_EOF;

error:
    hsfv_buffer_deinit(&buf, allocator);
    return err;
}

/* Token */

hsfv_err_t hsfv_serialize_token(const hsfv_token_t *token, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    const char *p;
    hsfv_err_t err;

    p = token->base;
    if (!HSFV_IS_TOKEN_LEADING_CHAR(*p)) {
        return HSFV_ERR_INVALID;
    }
    for (++p; p < token->base + token->len; ++p) {
        if (!HSFV_IS_TOKEN_TRAILING_CHAR(*p)) {
            return HSFV_ERR_INVALID;
        }
    }

    err = hsfv_buffer_ensure_unused_bytes(dest, allocator, token->len);
    if (err) {
        return err;
    }

    hsfv_buffer_append_bytes_unchecked(dest, token->base, token->len);

    return HSFV_OK;
}

#define TOKEN_INITIAL_CAPACITY 8

hsfv_err_t hsfv_parse_token(hsfv_bare_item_t *item, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                            const char **out_rest)
{
    const char *p = input;

    if (p == input_end) {
        return HSFV_ERR_EOF;
    }
    if (!HSFV_IS_TOKEN_LEADING_CHAR(*p)) {
        return HSFV_ERR_INVALID;
    }

    for (++p; p < input_end; ++p) {
        if (!HSFV_IS_TOKEN_TRAILING_CHAR(*p)) {
            break;
        }
    }

    item->token.base = (const char *)hsfv_bytes_dup(allocator, (const hsfv_byte_t *)input, p - input);
    if (item->token.base == NULL) {
        return HSFV_ERR_OUT_OF_MEMORY;
    }
    item->token.len = p - input;
    item->type = HSFV_BARE_ITEM_TYPE_TOKEN;
    if (out_rest) {
        *out_rest = p;
    }
    return HSFV_OK;
}

/* Key */

hsfv_err_t hsfv_serialize_key(const hsfv_key_t *key, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    const char *p = key->base;
    if (!p || key->len == 0 || !HSFV_IS_KEY_LEADING_CHAR(*p)) {
        return HSFV_ERR_INVALID;
    }
    for (++p; p < key->base + key->len; ++p) {
        if (!HSFV_IS_KEY_TRAILING_CHAR(*p)) {
            return HSFV_ERR_INVALID;
        }
    }

    return hsfv_buffer_append_bytes(dest, allocator, key->base, key->len);
}

#define KEY_INITIAL_CAPACITY 8

hsfv_err_t hsfv_parse_key(hsfv_key_t *key, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                          const char **out_rest)
{
    const char *p = input;

    if (p == input_end) {
        return HSFV_ERR_EOF;
    }
    if (!HSFV_IS_KEY_LEADING_CHAR(*p)) {
        return HSFV_ERR_INVALID;
    }

    for (++p; p < input_end; ++p) {
        if (!HSFV_IS_KEY_TRAILING_CHAR(*p)) {
            break;
        }
    }

    key->base = (const char *)hsfv_bytes_dup(allocator, (const hsfv_byte_t *)input, p - input);
    if (key->base == NULL) {
        return HSFV_ERR_OUT_OF_MEMORY;
    }
    key->len = p - input;
    if (out_rest) {
        *out_rest = p;
    }
    return HSFV_OK;
}

/* Byte sequence */

hsfv_err_t hsfv_serialize_byte_seq(const hsfv_byte_seq_t *byte_seq, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    size_t encoded_len = HSFV_BASE64_ENCODED_LENGTH(byte_seq->len);
    hsfv_err_t err;

    err = hsfv_buffer_ensure_unused_bytes(dest, allocator, encoded_len + 2);
    if (err) {
        return err;
    }

    hsfv_buffer_append_byte_unchecked(dest, ':');

    hsfv_iovec_const_t src_vec = {.base = byte_seq->base, .len = byte_seq->len};
    hsfv_iovec_t dest_vec = {.base = &dest->bytes.base[dest->bytes.len], .len = encoded_len};
    hsfv_encode_base64(&dest_vec, &src_vec);
    dest->bytes.len += encoded_len;

    hsfv_buffer_append_byte_unchecked(dest, ':');
    return HSFV_OK;
}

#define BINARY_INITIAL_CAPACITY 8

hsfv_err_t hsfv_parse_byte_seq(hsfv_bare_item_t *item, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                               const char **out_rest)
{
    hsfv_err_t err;
    const char *start;
    char c;
    hsfv_iovec_t temp;
    uint64_t encoded_len, decoded_len;
    hsfv_iovec_const_t src;

    if (input == input_end) {
        return HSFV_ERR_EOF;
    }
    if (*input != ':') {
        return HSFV_ERR_INVALID;
    }
    ++input;
    start = input;

    for (; input < input_end; ++input) {
        c = *input;
        if (c == ':') {
            encoded_len = input - start;
            decoded_len = HSFV_BASE64_DECODED_LENGTH(encoded_len);
            temp.base = allocator->alloc(allocator, decoded_len);
            if (temp.base == NULL) {
                return HSFV_ERR_OUT_OF_MEMORY;
            }
            temp.len = decoded_len;

            src.base = (const hsfv_byte_t *)start;
            src.len = encoded_len;
            err = hsfv_decode_base64(&temp, &src);
            if (err) {
                allocator->free(allocator, temp.base);
                return HSFV_ERR_INVALID;
            }
            item->type = HSFV_BARE_ITEM_TYPE_BYTE_SEQ;
            item->byte_seq.base = temp.base;
            item->byte_seq.len = temp.len;
            if (out_rest) {
                *out_rest = ++input;
            }
            return HSFV_OK;
        }

        if (!HSFV_IS_BASE64_CHAR(c)) {
            return HSFV_ERR_INVALID;
        }
    }
    return HSFV_ERR_EOF;
}

/* Bare item */

hsfv_err_t hsfv_serialize_bare_item(const hsfv_bare_item_t *item, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    switch (item->type) {
    case HSFV_BARE_ITEM_TYPE_INTEGER:
        return hsfv_serialize_integer(item->integer, allocator, dest);
    case HSFV_BARE_ITEM_TYPE_DECIMAL:
        return hsfv_serialize_decimal(item->decimal, allocator, dest);
    case HSFV_BARE_ITEM_TYPE_STRING:
        return hsfv_serialize_string(&item->string, allocator, dest);
    case HSFV_BARE_ITEM_TYPE_TOKEN:
        return hsfv_serialize_token(&item->token, allocator, dest);
    case HSFV_BARE_ITEM_TYPE_BYTE_SEQ:
        return hsfv_serialize_byte_seq(&item->byte_seq, allocator, dest);
    case HSFV_BARE_ITEM_TYPE_BOOLEAN:
        return hsfv_serialize_boolean(item->boolean, allocator, dest);
    default:
        return HSFV_ERR_INVALID;
    }
}

hsfv_err_t hsfv_parse_bare_item(hsfv_bare_item_t *item, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                                const char **out_rest)
{
    char c;
    if (input == input_end) {
        return HSFV_ERR_EOF;
    }

    c = *input;
    switch (c) {
    case '"':
        return hsfv_parse_string(item, allocator, input, input_end, out_rest);
    case ':':
        return hsfv_parse_byte_seq(item, allocator, input, input_end, out_rest);
    case '?':
        return hsfv_parse_boolean(item, input, input_end, out_rest);
    default:
        if (c == '-' || HSFV_IS_DIGIT(c)) {
            return hsfv_parse_number(item, input, input_end, out_rest);
        }
        if (HSFV_IS_TOKEN_LEADING_CHAR(c)) {
            return hsfv_parse_token(item, allocator, input, input_end, out_rest);
        }
        return HSFV_ERR_INVALID;
    }
}
