#include "hsfv.h"

bool hsfv_parse_ignore_boolean(const char *input, const char *input_end, const char **out_rest)
{
    if (input_end < input + 2 || input[0] != '?' || (input[1] != '1' && input[1] != '0')) {
        return false;
    }
    *out_rest = input + 2;
    return true;
}

bool hsfv_parse_ignore_number(const char *input, const char *input_end, const char **out_rest)
{
    if (input == input_end) {
        return false;
    }

    const char *digit_start = input;
    if (*input == '-') {
        ++digit_start;
    }
    const char *p = digit_start;
    if (p == input_end) {
        return false;
    }
    if (!HSFV_IS_DIGIT(*p)) {
        return false;
    }
    ++p;

    const char *dot = NULL;
    const char *out_of_range = digit_start + HSFV_MAX_INT_LEN;
    while (p < input_end) {
        if (p >= out_of_range) {
            return false;
        }

        char ch = *p;
        if (HSFV_IS_DIGIT(ch)) {
            ++p;
            continue;
        }

        if (ch == '.') {
            if (dot) {
                return false;
            }
            if (p > digit_start + HSFV_MAX_DEC_INT_LEN) {
                return false;
            }
            dot = p;
            ++p;
            out_of_range = p + HSFV_MAX_DEC_FRAC_LEN;
            continue;
        }

        break;
    }

    const char *end = p;
    if (dot && end == dot + 1) {
        return false;
    }

    *out_rest = end;
    return true;
}

bool hsfv_parse_ignore_string(const char *input, const char *input_end, const char **out_rest)
{
    const char *p = input;
    char c = *p;
    if (c != '"') {
        return false;
    }
    ++p;

    for (; p < input_end; ++p) {
        c = *p;
        if (c == '"') {
            *out_rest = ++p;
            return true;
        }
        if (c == '\\') {
            ++p;
            if (p == input_end) {
                break;
            }
            c = *p;
            if (c != '"' && c != '\\') {
                break;
            }
            continue;
        }
        if (c <= '\x1f' || '\x7f' <= c) {
            break;
        }
    }
    return false;
}

bool hsfv_parse_ignore_token(const char *input, const char *input_end, const char **out_rest)
{
    const char *p = input;

    if (p == input_end) {
        return false;
    }
    if (!HSFV_IS_TOKEN_LEADING_CHAR(*p)) {
        return false;
    }

    for (++p; p < input_end; ++p) {
        if (!HSFV_IS_TOKEN_TRAILING_CHAR(*p)) {
            break;
        }
    }
    *out_rest = p;
    return true;
}

bool hsfv_parse_ignore_key(const char *input, const char *input_end, const char **out_rest)
{
    const char *p = input;

    if (p == input_end) {
        return false;
    }
    if (!HSFV_IS_KEY_LEADING_CHAR(*p)) {
        return false;
    }

    for (++p; p < input_end; ++p) {
        if (!HSFV_IS_KEY_TRAILING_CHAR(*p)) {
            break;
        }
    }
    *out_rest = p;
    return true;
}

bool hsfv_parse_ignore_byte_seq(const char *input, const char *input_end, const char **out_rest)
{
    const char *start;
    char c;
    uint64_t encoded_len;
    hsfv_iovec_const_t src;

    if (input == input_end) {
        return false;
    }
    if (*input != ':') {
        return false;
    }
    ++input;
    start = input;

    for (; input < input_end; ++input) {
        c = *input;
        if (c == ':') {
            encoded_len = input - start;
            src.base = (const hsfv_byte_t *)start;
            src.len = encoded_len;
            if (!hsfv_is_base64_decodable(&src)) {
                return false;
            }
            *out_rest = ++input;
            return true;
        }

        if (!HSFV_IS_BASE64_CHAR(c)) {
            return false;
        }
    }
    return false;
}

bool hsfv_parse_ignore_bare_item(const char *input, const char *input_end, const char **out_rest)
{
    char c;
    if (input == input_end) {
        return false;
    }

    c = *input;
    switch (c) {
    case '"':
        return hsfv_parse_ignore_string(input, input_end, out_rest);
    case ':':
        return hsfv_parse_ignore_byte_seq(input, input_end, out_rest);
    case '?':
        return hsfv_parse_ignore_boolean(input, input_end, out_rest);
    default:
        if (c == '-' || HSFV_IS_DIGIT(c)) {
            return hsfv_parse_ignore_number(input, input_end, out_rest);
        }
        if (HSFV_IS_TOKEN_LEADING_CHAR(c)) {
            return hsfv_parse_ignore_token(input, input_end, out_rest);
        }
        return false;
    }
}

bool hsfv_parse_ignore_parameters(const char *input, const char *input_end, const char **out_rest)
{
    while (input < input_end) {
        char c = *input;
        if (c != ';') {
            break;
        }
        ++input;

        HSFV_SKIP_SP(input, input_end);

        if (!hsfv_parse_ignore_key(input, input_end, &input)) {
            return false;
        }

        if (input < input_end && *input == '=') {
            ++input;
            if (!hsfv_parse_ignore_bare_item(input, input_end, &input)) {
                return false;
            }
        }
    }

    *out_rest = input;
    return true;
}

bool hsfv_parse_ignore_item(const char *input, const char *input_end, const char **out_rest)
{
    if (!hsfv_parse_ignore_bare_item(input, input_end, &input)) {
        return false;
    }

    if (!hsfv_parse_ignore_parameters(input, input_end, &input)) {
        return false;
    }

    *out_rest = input;
    return true;
}

bool hsfv_parse_ignore_inner_list(const char *input, const char *input_end, const char **out_rest)
{
    char c;

    if (input == input_end) {
        return false;
    }

    c = *input;
    if (c != '(') {
        return false;
    }
    ++input;

    while (input < input_end) {
        HSFV_SKIP_SP(input, input_end);

        if (input == input_end) {
            return false;
        }
        c = *input;
        if (c == ')') {
            ++input;
            if (!hsfv_parse_ignore_parameters(input, input_end, &input)) {
                return false;
            }
            *out_rest = input;
            return true;
        }

        if (!hsfv_parse_ignore_item(input, input_end, &input)) {
            return false;
        }

        if (input == input_end) {
            return false;
        }
        c = *input;
        if (c != ' ' && c != ')') {
            return false;
        }
    }

    return false;
}

bool hsfv_parse_ignore_dictionary_member_value(const char *input, const char *input_end, const char **out_rest)
{
    if (input < input_end && *input == '=') {
        ++input;
        if (*input == '(') {
            if (!hsfv_parse_ignore_inner_list(input, input_end, &input)) {
                return false;
            }
        } else {
            if (!hsfv_parse_ignore_item(input, input_end, &input)) {
                return false;
            }
        }
    } else {
        if (!hsfv_parse_ignore_parameters(input, input_end, &input)) {
            return false;
        }
    }
    *out_rest = input;
    return true;
}

void hsfv_skip_sp(const char *input, const char *input_end, const char **out_rest)
{
    while (input < input_end && *input == ' ') {
        ++input;
    }
    *out_rest = input;
}

void hsfv_skip_ows(const char *input, const char *input_end, const char **out_rest)
{
    while (input < input_end && (*input == ' ' || *input == '\t')) {
        ++input;
    }
    *out_rest = input;
}

bool hsfv_skip_ows_comma_ows(const char *input, const char *input_end, const char **out_rest)
{
    hsfv_skip_ows(input, input_end, &input);
    if (input < input_end) {
        if (*input == ',') {
            ++input;
            hsfv_skip_ows(input, input_end, &input);
            if (input == input_end) {
                return false;
            }
        } else {
            return false;
        }
    }
    *out_rest = input;
    return true;
}
