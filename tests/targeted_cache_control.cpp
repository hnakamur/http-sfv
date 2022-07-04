#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

typedef struct st_hsfv_targeted_cache_control_t {
    int64_t max_age;
    bool must_revalidate;
    bool no_store;
    bool no_cache;
    bool private_;
} hsfv_targeted_cache_control_t;

static bool hsfv_skip_boolean_true_dictionary_member_value(const char *input, const char *input_end, const char **rest)
{
    if (!hsfv_skip_dictionary_member_value(input, input_end, rest)) {
        return false;
    }

    return input == input_end || input[0] == ';' || input[0] == ',' || HSFV_IS_OWS(input[0]);
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
        switch (key_len) {
        case 7:
            if (!hsfv_strncasecmp(key_start, "max-age", 7)) {
                if (input < input_end && input[0] == '=') {
                    ++input;
                } else {
                    return false;
                }
                hsfv_err_t err = hsfv_parse_integer(input, input_end, &out_cc->max_age, &input);
                if (err) {
                    return false;
                }
                if (!hsfv_skip_parameters(input, input_end, &input)) {
                    return false;
                }
            } else if (!hsfv_strncasecmp(key_start, "private", 7)) {
                if (!hsfv_skip_boolean_true_dictionary_member_value(input, input_end, &input)) {
                    return false;
                }
                out_cc->private_ = true;
            } else {
                if (!hsfv_skip_dictionary_member_value(input, input_end, &input)) {
                    return false;
                }
            }
            break;
        case 8:
            if (!hsfv_strncasecmp(key_start, "no-store", 7)) {
                if (!hsfv_skip_boolean_true_dictionary_member_value(input, input_end, &input)) {
                    return false;
                }
                out_cc->no_store = true;
            } else if (!hsfv_strncasecmp(key_start, "no-cache", 7)) {
                if (!hsfv_skip_boolean_true_dictionary_member_value(input, input_end, &input)) {
                    return false;
                }
                out_cc->no_cache = true;
            } else {
                if (!hsfv_skip_dictionary_member_value(input, input_end, &input)) {
                    return false;
                }
            }
            break;
        case 15:
            if (!hsfv_strncasecmp(key_start, "must-revalidate", 15)) {
                if (!hsfv_skip_boolean_true_dictionary_member_value(input, input_end, &input)) {
                    return false;
                }
                out_cc->must_revalidate = true;
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

static void test_parse_targeted_cache_control(const char *input, bool want_valid, hsfv_targeted_cache_control_t want_cc)
{
    const char *input_end = input + strlen(input);
    hsfv_targeted_cache_control_t cc = {0};
    const char *rest;
    bool got_valid = parse_targeted_cache_control(input, input_end, &cc, &rest);
    CHECK(got_valid == want_valid);
    if (got_valid) {
        CHECK(cc.max_age == want_cc.max_age);
        CHECK(cc.must_revalidate == want_cc.must_revalidate);
        CHECK(cc.no_store == want_cc.no_store);
        CHECK(cc.no_cache == want_cc.no_cache);
        CHECK(cc.private_ == want_cc.private_);
    }
}

TEST_CASE("parse_targeted_cache_control", "[parse][cache_control]")
{
    SECTION("false")
    {
        test_parse_targeted_cache_control("max-age=60, must-revalidate", true,
                                          hsfv_targeted_cache_control_t{.max_age = 60, .must_revalidate = true});
        test_parse_targeted_cache_control("private", true, hsfv_targeted_cache_control_t{.private_ = true});
        test_parse_targeted_cache_control("must-revalidate", true, hsfv_targeted_cache_control_t{.must_revalidate = true});
        test_parse_targeted_cache_control("no-store", true, hsfv_targeted_cache_control_t{.no_store = true});
        test_parse_targeted_cache_control("no-cache", true, hsfv_targeted_cache_control_t{.no_cache = true});
        test_parse_targeted_cache_control("private;p=1", true, hsfv_targeted_cache_control_t{.private_ = true});

        test_parse_targeted_cache_control("private=yes", false, hsfv_targeted_cache_control_t{0});
        test_parse_targeted_cache_control("private=?1", false, hsfv_targeted_cache_control_t{0});
        test_parse_targeted_cache_control("private=?0", false, hsfv_targeted_cache_control_t{0});
        test_parse_targeted_cache_control("private=?1, no_store=?0", false, hsfv_targeted_cache_control_t{0});
        test_parse_targeted_cache_control("private=?1 , no_store=?0", false, hsfv_targeted_cache_control_t{0});
        test_parse_targeted_cache_control("private=?1\t, no_store=?0", false, hsfv_targeted_cache_control_t{0});
    }
}
