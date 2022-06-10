#include "hsfv.h"

static const char *token_char_map =
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\1\0\1\1\1\1\1\0\0\1\1\0\1\1\0\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0"
    "\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\1"
    "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\1\0\1\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

#define is_digit(c) ('0' <= (c) && (c) <= '9')
#define is_lcalpha(c) (('a' <= (c) && (c) <= 'z'))
#define is_alpha(c) (('A' <= (c) && (c) <= 'Z') || ('a' <= (c) && (c) <= 'z'))
#define is_token_char(c) token_char_map[(unsigned char)(c)]

void hsfv_bare_item_deinit(hsfv_allocator_t *allocator,
                           hsfv_bare_item_t *bare_item) {
  switch (bare_item->type) {
  case HSFV_BARE_ITEM_TYPE_STRING:
    allocator->free(allocator, (void *)bare_item->data.string.base);
    break;
  case HSFV_BARE_ITEM_TYPE_TOKEN:
    allocator->free(allocator, (void *)bare_item->data.token.base);
    break;
  case HSFV_BARE_ITEM_TYPE_BINARY:
    allocator->free(allocator, (void *)bare_item->data.bytes.base);
    break;
  }
}

static const char *parse_integer(const char *buf, const char *buf_end, int neg,
                                 hsfv_bare_item_t *item, int *ret);
static const char *parse_decimal(const char *buf, const char *buf_end,
                                 int dec_sep_off, int neg,
                                 hsfv_bare_item_t *item, int *ret);

const char *parse_boolean(const char *buf, const char *buf_end, int *boolean,
                          int *ret) {
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

const char *parse_number(const char *buf, const char *buf_end,
                         hsfv_bare_item_t *item, int *ret) {
  int neg = 0, is_integer = 1, dec_sep_off = 0, size, ch;
  const char *start;

  if (*buf == '-') {
    neg = 1;
    ++buf;
  }

  if (buf == buf_end) {
    *ret = HSFV_ERR_EOF;
    return NULL;
  }
  if (!is_digit(*buf)) {
    *ret = HSFV_ERR_INVALID;
    return NULL;
  }

  start = buf;
  while (buf < buf_end) {
    size = buf - start;
    if ((is_integer && size >= HSFV_MAX_INT_LEN) ||
        (!is_integer && size - dec_sep_off > HSFV_MAX_DEC_FRAC_LEN)) {
      *ret = HSFV_ERR_NUMBER_OUT_OF_RANGE;
      return NULL;
    }

    ch = *buf;
    if (is_digit(ch)) {
      ++buf;
      continue;
    }

    if (is_integer && ch == '.') {
      if (size > HSFV_MAX_DEC_INT_LEN) {
        *ret = HSFV_ERR_NUMBER_OUT_OF_RANGE;
        return NULL;
      }
      is_integer = 0;
      dec_sep_off = buf - start;
      ++buf;
      continue;
    }

    break;
  }

  if (is_integer) {
    return parse_integer(start, buf, neg, item, ret);
  }
  return parse_decimal(start, buf, dec_sep_off, neg, item, ret);
}

static const char *parse_integer(const char *buf, const char *buf_end, int neg,
                                 hsfv_bare_item_t *item, int *ret) {
  int64_t i;
  char *end;

  i = strtoll(buf, &end, 10);
  if (end != buf_end) {
    *ret = HSFV_ERR_INVALID;
    return NULL;
  }
  if (i < HSFV_MIN_INT || HSFV_MAX_INT < i) {
    *ret = HSFV_ERR_NUMBER_OUT_OF_RANGE;
    return NULL;
  }

  item->type = HSFV_BARE_ITEM_TYPE_INTEGER;
  item->data.integer = neg ? -i : i;
  *ret = HSFV_OK;
  return buf_end;
}

static const char *parse_decimal(const char *buf, const char *buf_end,
                                 int dec_sep_off, int neg,
                                 hsfv_bare_item_t *item, int *ret) {
  double d;
  char *end;

  if (buf + dec_sep_off == buf_end - 1) {
    *ret = HSFV_ERR_INVALID;
    return NULL;
  }

  d = strtod(buf, &end);
  if (end != buf_end) {
    *ret = HSFV_ERR_INVALID;
    return NULL;
  }

  item->type = HSFV_BARE_ITEM_TYPE_DECIMAL;
  item->data.decimal = neg ? -d : d;
  *ret = HSFV_OK;
  return buf_end;
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
      item->data.string.base = buf.bytes.base;
      item->data.string.len = buf.bytes.len;
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
  if (!is_alpha(c) && c != '*') {
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
    if (!is_token_char(c)) {
      break;
    }

    err = htsv_buffer_ensure_append_byte(allocator, &buf, c);
    if (err) {
      goto error;
    }
  }

  item->type = HSFV_BARE_ITEM_TYPE_TOKEN;
  item->data.token.base = buf.bytes.base;
  item->data.token.len = buf.bytes.len;
  if (out_rest) {
    *out_rest = input;
  }
  return HSFV_OK;

error:
  htsv_buffer_free_bytes(allocator, &buf);
  return err;
}
