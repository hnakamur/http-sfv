#include "hsfv.h"

static bool hsfv_expect_boolean_true_dictionary_member_value(const char *input, const char *input_end, bool *out_bool,
                                                             const char **rest)
{
    if (!hsfv_skip_dictionary_member_value(input, input_end, rest)) {
        return false;
    }

    if (input == input_end || input[0] == ';' || input[0] == ',' || HSFV_IS_OWS(input[0])) {
        *out_bool = true;
        return true;
    }

    return false;
}

static bool hsfv_expect_boolean_true_or_ignore_token_dictionary_member_value(const char *input, const char *input_end,
                                                                             bool *out_bool, const char **rest)
{
    if (!hsfv_skip_dictionary_member_value(input, input_end, rest)) {
        return false;
    }

    if (input == input_end || input[0] == ';' || input[0] == ',' || HSFV_IS_OWS(input[0])) {
        *out_bool = true;
        return true;
    }

    const char *token_end;
    return hsfv_skip_token(input, input_end, &token_end);
}

bool parse_targeted_cache_control(const char *input, const char *input_end, hsfv_targeted_cache_control_t *out_cc,
                                  const char **rest)
{
    hsfv_skip_sp(input, input_end, &input);

    while (input < input_end) {
        const char *key_start = input;
        if (!hsfv_skip_key(key_start, input_end, &input)) {
            return false;
        }

        const char *key_end = input;
        size_t key_len = key_end - key_start;
        // We can use memcmp below because valid keys are always lowercase.
        switch (key_len) {
        case 7:
            if (!memcmp(key_start, "max-age", 7)) {
                if (input < input_end && input[0] == '=') {
                    ++input;
                } else {
                    return false;
                }
                hsfv_err_t err = hsfv_parse_non_negative_integer(input, input_end, &out_cc->max_age, &input);
                if (err) {
                    return false;
                }
                if (!hsfv_skip_parameters(input, input_end, &input)) {
                    return false;
                }
            } else if (!memcmp(key_start, "private", 7)) {
                if (!hsfv_expect_boolean_true_or_ignore_token_dictionary_member_value(input, input_end, &out_cc->private_,
                                                                                      &input)) {
                    return false;
                }
            } else {
                if (!hsfv_skip_dictionary_member_value(input, input_end, &input)) {
                    return false;
                }
            }
            break;
        case 8:
            if (!memcmp(key_start, "no-", 3)) {
                if (!memcmp(key_start + 3, "store", 5)) {
                    if (!hsfv_expect_boolean_true_dictionary_member_value(input, input_end, &out_cc->no_store, &input)) {
                        return false;
                    }
                } else if (!memcmp(key_start + 3, "cache", 5)) {
                    if (!hsfv_expect_boolean_true_or_ignore_token_dictionary_member_value(input, input_end, &out_cc->no_cache,
                                                                                          &input)) {
                        return false;
                    }
                } else {
                    if (!hsfv_skip_dictionary_member_value(input, input_end, &input)) {
                        return false;
                    }
                }
            } else {
                if (!hsfv_skip_dictionary_member_value(input, input_end, &input)) {
                    return false;
                }
            }
            break;
        case 15:
            if (!memcmp(key_start, "must-revalidate", 15)) {
                if (!hsfv_expect_boolean_true_dictionary_member_value(input, input_end, &out_cc->must_revalidate, &input)) {
                    return false;
                }
            } else {
                if (!hsfv_skip_dictionary_member_value(input, input_end, &input)) {
                    return false;
                }
            }
            break;
        default:
            if (!hsfv_skip_dictionary_member_value(input, input_end, &input)) {
                return false;
            }
            break;
        }

        if (!hsfv_skip_ows_comma_ows(input, input_end, &input)) {
            return false;
        }
    }

    hsfv_skip_sp(input, input_end, &input);
    return input == input_end;
}
