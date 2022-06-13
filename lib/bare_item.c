#include "hsfv.h"

// clang-format off
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
// clang-format on

static hsfv_err_t parse_integer(hsfv_bare_item_t *item, const char *input, const char *input_end, bool neg, const char **out_rest);
static hsfv_err_t parse_decimal(hsfv_bare_item_t *item, const char *input, const char *input_end, int dec_sep_off, bool neg,
                                const char **out_rest);

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

hsfv_err_t hsfv_serialize_integer(int64_t integer, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    if (integer < HSFV_MIN_INT || HSFV_MAX_INT < integer) {
        return HSFV_ERR_INVALID;
    }

    const size_t tmp_bufsize = 17;
    char tmp[tmp_bufsize];
    int n = snprintf(tmp, tmp_bufsize, "%ld", integer);
    if (n > tmp_bufsize) {
        return HSFV_ERR_INVALID;
    }

    hsfv_err_t err = hsfv_buffer_ensure_unused_bytes(dest, allocator, n);
    if (err) {
        return err;
    }

    hsfv_buffer_append_bytes_unchecked(dest, tmp, n);
    return HSFV_OK;
}

hsfv_err_t hsfv_serialize_decimal(double decimal, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    const size_t tmp_bufsize = 18;
    char tmp[tmp_bufsize];
    int n = snprintf(tmp, tmp_bufsize, "%.3f", decimal);
    if (n > tmp_bufsize) {
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

hsfv_err_t hsfv_parse_number(hsfv_bare_item_t *item, const char *input, const char *input_end, const char **out_rest)
{
    bool neg = false, is_integer = true;
    int dec_sep_off = 0, size, ch;
    const char *start;

    if (*input == '-') {
        neg = 1;
        ++input;
    }

    if (input == input_end) {
        return HSFV_ERR_EOF;
    }
    if (!HSFV_IS_DIGIT(*input)) {
        return HSFV_ERR_INVALID;
    }

    start = input;
    while (input < input_end) {
        size = input - start;
        if ((is_integer && size >= HSFV_MAX_INT_LEN) || (!is_integer && size - dec_sep_off > HSFV_MAX_DEC_FRAC_LEN)) {
            return HSFV_ERR_NUMBER_OUT_OF_RANGE;
        }

        ch = *input;
        if (HSFV_IS_DIGIT(ch)) {
            ++input;
            continue;
        }

        if (is_integer && ch == '.') {
            if (size > HSFV_MAX_DEC_INT_LEN) {
                return HSFV_ERR_NUMBER_OUT_OF_RANGE;
            }
            is_integer = false;
            dec_sep_off = input - start;
            ++input;
            continue;
        }

        break;
    }

    if (is_integer) {
        return parse_integer(item, start, input, neg, out_rest);
    }
    return parse_decimal(item, start, input, dec_sep_off, neg, out_rest);
}

static hsfv_err_t parse_integer(hsfv_bare_item_t *item, const char *input, const char *input_end, bool neg, const char **out_rest)
{
    int64_t i;
    char *end;

    i = strtoll(input, &end, 10);
    if (end != input_end) {
        return HSFV_ERR_INVALID;
    }
    if (i < HSFV_MIN_INT || HSFV_MAX_INT < i) {
        return HSFV_ERR_NUMBER_OUT_OF_RANGE;
    }

    item->type = HSFV_BARE_ITEM_TYPE_INTEGER;
    item->integer = neg ? -i : i;
    if (out_rest) {
        *out_rest = end;
    }
    return HSFV_OK;
}

static hsfv_err_t parse_decimal(hsfv_bare_item_t *item, const char *input, const char *input_end, int dec_sep_off, bool neg,
                                const char **out_rest)
{
    double d;
    char *end;

    if (input + dec_sep_off == input_end - 1) {
        return HSFV_ERR_INVALID;
    }

    d = strtod(input, &end);
    if (end != input_end) {
        return HSFV_ERR_INVALID;
    }

    item->type = HSFV_BARE_ITEM_TYPE_DECIMAL;
    item->decimal = neg ? -d : d;
    if (out_rest) {
        *out_rest = end;
    }
    return HSFV_OK;
}

/* String */

hsfv_err_t hsfv_serialize_string(const hsfv_string_t *string, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    const char *p;
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
            if (input == input_end) {
                err = HSFV_ERR_INVALID;
                goto error;
            }
            c = *++input;
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
            item->string.base = buf.bytes.base;
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

    item->token.base = hsfv_strndup(allocator, input, p - input);
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
    hsfv_err_t err;
    const char *p = key->base;
    if (!p || key->len == 0 || !HSFV_IS_KEY_LEAADING_CHAR(*p)) {
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
    if (!HSFV_IS_KEY_LEAADING_CHAR(*p)) {
        return HSFV_ERR_INVALID;
    }

    for (++p; p < input_end; ++p) {
        if (!HSFV_IS_KEY_TRAILING_CHAR(*p)) {
            break;
        }
    }

    key->base = hsfv_strndup(allocator, input, p - input);
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

            src.base = start;
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

        if (!HSFV_IS_ALPHA(c) && !HSFV_IS_DIGIT(c) && c != '+' && c != '/' && c != '=') {
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
