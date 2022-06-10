#include "hsfv.h"

/*
 * TCHAR is defined at https://www.rfc-editor.org/rfc/rfc7230.html#section-3.2.6
 */
const char *hsfv_token_char_map =
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\1\0\1\1\1\1\1\0\0\1\1\0\1\1\0\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0"
    "\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\1"
    "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\1\0\1\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

static hsfv_err_t parse_integer(const char *input, const char *input_end,
                                int neg, hsfv_bare_item_t *item,
                                const char **out_rest);
static hsfv_err_t parse_decimal(const char *input, const char *input_end,
                                int dec_sep_off, int neg,
                                hsfv_bare_item_t *item, const char **out_rest);

void hsfv_bare_item_deinit(hsfv_allocator_t *allocator,
                           hsfv_bare_item_t *bare_item) {
  switch (bare_item->type) {
  case HSFV_BARE_ITEM_TYPE_STRING:
    allocator->free(allocator, (void *)bare_item->string.base);
    break;
  case HSFV_BARE_ITEM_TYPE_TOKEN:
    allocator->free(allocator, (void *)bare_item->token.base);
    break;
  case HSFV_BARE_ITEM_TYPE_BINARY:
    allocator->free(allocator, (void *)bare_item->bytes.base);
    break;
  }
}

hsfv_err_t parse_boolean(const char *input, const char *input_end,
                         hsfv_bare_item_t *item, const char **out_rest) {
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
    item->boolean = 1;
    if (out_rest) {
      *out_rest = ++input;
    }
    return HSFV_OK;
  }
  if (*input == '0') {
    item->type = HSFV_BARE_ITEM_TYPE_BOOLEAN;
    item->boolean = 0;
    if (out_rest) {
      *out_rest = ++input;
    }
    return HSFV_OK;
  }
  return HSFV_ERR_INVALID;
}

hsfv_err_t parse_number(const char *input, const char *input_end,
                        hsfv_bare_item_t *item, const char **out_rest) {
  int neg = 0, is_integer = 1, dec_sep_off = 0, size, ch;
  const char *start;

  if (*input == '-') {
    neg = 1;
    ++input;
  }

  if (input == input_end) {
    return HSFV_ERR_EOF;
  }
  if (!hsfv_is_digit(*input)) {
    return HSFV_ERR_INVALID;
  }

  start = input;
  while (input < input_end) {
    size = input - start;
    if ((is_integer && size >= HSFV_MAX_INT_LEN) ||
        (!is_integer && size - dec_sep_off > HSFV_MAX_DEC_FRAC_LEN)) {
      return HSFV_ERR_NUMBER_OUT_OF_RANGE;
    }

    ch = *input;
    if (hsfv_is_digit(ch)) {
      ++input;
      continue;
    }

    if (is_integer && ch == '.') {
      if (size > HSFV_MAX_DEC_INT_LEN) {
        return HSFV_ERR_NUMBER_OUT_OF_RANGE;
      }
      is_integer = 0;
      dec_sep_off = input - start;
      ++input;
      continue;
    }

    break;
  }

  if (is_integer) {
    return parse_integer(start, input, neg, item, out_rest);
  }
  return parse_decimal(start, input, dec_sep_off, neg, item, out_rest);
}

static hsfv_err_t parse_integer(const char *input, const char *input_end,
                                int neg, hsfv_bare_item_t *item,
                                const char **out_rest) {
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

static hsfv_err_t parse_decimal(const char *input, const char *input_end,
                                int dec_sep_off, int neg,
                                hsfv_bare_item_t *item, const char **out_rest) {
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

#define STRING_INITIAL_CAPACITY 8

hsfv_err_t parse_string(hsfv_allocator_t *allocator, const char *input,
                        const char *input_end, hsfv_bare_item_t *item,
                        const char **out_rest) {
  hsfv_err_t err;
  hsfv_buffer_t buf;
  char c;

  err = htsv_buffer_alloc_bytes(allocator, &buf, STRING_INITIAL_CAPACITY);
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
      err = htsv_buffer_ensure_append_byte(allocator, &buf, c);
      if (err) {
        goto error;
      }
    } else if (c == '"') {
      item->type = HSFV_BARE_ITEM_TYPE_STRING;
      item->string.base = buf.bytes.base;
      item->string.len = buf.bytes.len;
      if (out_rest) {
        *out_rest = input;
      }
      return HSFV_OK;
    } else if (c <= '\x1f' || '\x7f' <= c) {
      err = HSFV_ERR_INVALID;
      goto error;
    } else {
      err = htsv_buffer_ensure_append_byte(allocator, &buf, c);
      if (err) {
        goto error;
      }
    }
  }

  err = HSFV_ERR_EOF;

error:
  htsv_buffer_free_bytes(allocator, &buf);
  return err;
}

#define TOKEN_INITIAL_CAPACITY 8

hsfv_err_t parse_token(hsfv_allocator_t *allocator, const char *input,
                       const char *input_end, hsfv_bare_item_t *item,
                       const char **out_rest) {
  hsfv_err_t err;
  hsfv_buffer_t buf;
  char c;

  err = htsv_buffer_alloc_bytes(allocator, &buf, TOKEN_INITIAL_CAPACITY);
  if (err) {
    return err;
  }

  if (input == input_end) {
    return HSFV_ERR_EOF;
  }

  c = *input;
  if (!hsfv_is_alpha(c) && c != '*') {
    err = HSFV_ERR_INVALID;
    goto error;
  }
  err = htsv_buffer_ensure_append_byte(allocator, &buf, c);
  if (err) {
    goto error;
  }
  ++input;

  for (; input < input_end; ++input) {
    c = *input;
    if (!hsfv_is_token_char(c)) {
      break;
    }

    err = htsv_buffer_ensure_append_byte(allocator, &buf, c);
    if (err) {
      goto error;
    }
  }

  item->type = HSFV_BARE_ITEM_TYPE_TOKEN;
  item->token.base = buf.bytes.base;
  item->token.len = buf.bytes.len;
  if (out_rest) {
    *out_rest = input;
  }
  return HSFV_OK;

error:
  htsv_buffer_free_bytes(allocator, &buf);
  return err;
}

#define KEY_INITIAL_CAPACITY 8

hsfv_err_t parse_key(hsfv_allocator_t *allocator, const char *input,
                     const char *input_end, hsfv_key_t *key,
                     const char **out_rest) {
  hsfv_err_t err;
  hsfv_buffer_t buf;
  char c;

  err = htsv_buffer_alloc_bytes(allocator, &buf, KEY_INITIAL_CAPACITY);
  if (err) {
    return err;
  }

  if (input == input_end) {
    return HSFV_ERR_EOF;
  }

  c = *input;
  if (!hsfv_is_lcalpha(c) && c != '*') {
    err = HSFV_ERR_INVALID;
    goto error;
  }
  err = htsv_buffer_ensure_append_byte(allocator, &buf, c);
  if (err) {
    goto error;
  }
  ++input;

  for (; input < input_end; ++input) {
    c = *input;
    if (!hsfv_is_key_char(c)) {
      break;
    }

    err = htsv_buffer_ensure_append_byte(allocator, &buf, c);
    if (err) {
      goto error;
    }
  }

  key->base = buf.bytes.base;
  key->len = buf.bytes.len;
  if (out_rest) {
    *out_rest = input;
  }
  return HSFV_OK;

error:
  htsv_buffer_free_bytes(allocator, &buf);
  return err;
}

#define BINARY_INITIAL_CAPACITY 8

hsfv_err_t parse_binary(hsfv_allocator_t *allocator, const char *input,
                        const char *input_end, hsfv_bare_item_t *item,
                        const char **out_rest) {
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
      decoded_len = hfsv_base64_decoded_length(encoded_len);
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
      item->type = HSFV_BARE_ITEM_TYPE_BINARY;
      item->bytes.base = temp.base;
      item->bytes.len = temp.len;
      if (out_rest) {
        *out_rest = ++input;
      }
      return HSFV_OK;
    }

    if (!hsfv_is_alpha(c) && !hsfv_is_digit(c) && c != '+' && c != '/' &&
        c != '=') {
      return HSFV_ERR_INVALID;
    }
  }
  return HSFV_ERR_EOF;
}
