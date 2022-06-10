#include "hsfv.h"

static const char *parse_integer(const char *buf, const char *buf_end, int neg, hsfv_bare_item_t *item, int *ret);
static const char *parse_decimal(const char *buf, const char *buf_end, int dec_sep_off, int neg, hsfv_bare_item_t *item, int *ret);

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

static int is_digit(int ch)
{
    return '0' <= ch && ch <= '9';
}

const char *parse_number(const char *buf, const char *buf_end, hsfv_bare_item_t *item, int *ret)
{
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
        if ((is_integer && size >= HSFV_MAX_INT_LEN)
            || (!is_integer && size - dec_sep_off > HSFV_MAX_DEC_FRAC_LEN)) {
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

static const char *parse_integer(const char *buf, const char *buf_end, int neg, hsfv_bare_item_t *item, int *ret)
{
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

static const char *parse_decimal(const char *buf, const char *buf_end, int dec_sep_off, int neg, hsfv_bare_item_t *item, int *ret)
{
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
