#include "hsfv.h"

bool hsfv_is_parsable_boolean(const char *input, const char *input_end, const char **out_rest)
{
    if (input_end < input + 2 || input[0] != '?' || (input[1] != '1' && input[1] != '0')) {
        return false;
    }
    *out_rest = input + 2;
    return true;
}

bool hsfv_is_parsable_number(const char *input, const char *input_end, const char **out_rest)
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
