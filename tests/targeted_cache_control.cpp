#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

typedef struct st_hsfv_targeted_cache_control_t {
    int64_t max_age;
    bool must_revalidate;
    bool no_store;
    bool no_cache;
    bool private_;
} hsfv_targeted_cache_control_t;

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
                hsfv_err_t err = hsfv_parse_integer(input, input_end, &out_cc->max_age, &input);
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
    SECTION("valid max-age and mulst-revalidate")
    {
        test_parse_targeted_cache_control("max-age=60, must-revalidate", true,
                                          hsfv_targeted_cache_control_t{.max_age = 60, .must_revalidate = true});
    }
    SECTION("max-age overwritten")
    {
        test_parse_targeted_cache_control("max-age=60, max-age=30", true, hsfv_targeted_cache_control_t{.max_age = 30});
    }
    SECTION("private")
    {
        test_parse_targeted_cache_control("private", true, hsfv_targeted_cache_control_t{.private_ = true});
    }
    SECTION("must-revalidate")
    {
        test_parse_targeted_cache_control("must-revalidate", true, hsfv_targeted_cache_control_t{.must_revalidate = true});
    }
    SECTION("no-store")
    {
        test_parse_targeted_cache_control("no-store", true, hsfv_targeted_cache_control_t{.no_store = true});
    }
    SECTION("no-cache")
    {
        test_parse_targeted_cache_control("no-cache", true, hsfv_targeted_cache_control_t{.no_cache = true});
    }
    SECTION("ignore parameters")
    {
        test_parse_targeted_cache_control("private;p=1", true, hsfv_targeted_cache_control_t{.private_ = true});
    }

    SECTION("invalid")
    {
        SECTION("uppercase is not valid key")
        {
            test_parse_targeted_cache_control("Max-Age=60", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("max-age with string value")
        {
            test_parse_targeted_cache_control("max-age=\"60\"", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("max-age with decimal value")
        {
            test_parse_targeted_cache_control("max-age=60.0", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("max-age overwritten with decimal value")
        {
            test_parse_targeted_cache_control("max-age=30, max-age=60.0", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("no-store with token")
        {
            test_parse_targeted_cache_control("no-store=yes", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("must-revalidate with token")
        {
            test_parse_targeted_cache_control("must-revalidate=yes", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("private with string")
        {
            test_parse_targeted_cache_control("private=\"field1\"", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("no-cache with string")
        {
            test_parse_targeted_cache_control("no-cache=\"field1\"", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("true value must be omitted")
        {
            test_parse_targeted_cache_control("private=?1", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("false value not allowed")
        {
            test_parse_targeted_cache_control("private=?0", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("valid true then invalid false")
        {
            test_parse_targeted_cache_control("private, private=?0", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("comma after invalid value")
        {
            test_parse_targeted_cache_control("private=?1, no_store=?0", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("space after invalid value")
        {
            test_parse_targeted_cache_control("private=?1 , no_store=?0", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("tab after invalid value")
        {
            test_parse_targeted_cache_control("private=?1\t, no_store=?0", false, hsfv_targeted_cache_control_t{0});
        }
    }

    SECTION("ignore private with field-name token")
    {
        test_parse_targeted_cache_control("private=field1", false, hsfv_targeted_cache_control_t{0});
    }
    SECTION("ignore no-cache with field-name token")
    {
        test_parse_targeted_cache_control("no-cache=field1", false, hsfv_targeted_cache_control_t{0});
    }
}
