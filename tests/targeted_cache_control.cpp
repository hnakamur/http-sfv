#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

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
        SECTION("negative max-age value")
        {
            test_parse_targeted_cache_control("max-age=-1", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("max-age without value")
        {
            test_parse_targeted_cache_control("max-age", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("max-age without value and private")
        {
            test_parse_targeted_cache_control("max-age, private", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("uppercase is not valid key")
        {
            test_parse_targeted_cache_control("Max-Age=60", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("invalid dict member value where boolean true or token is expected")
        {
            test_parse_targeted_cache_control("private=,", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("invalid dict member value where boolean true or token is expected")
        {
            test_parse_targeted_cache_control("no-store=,", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("invalid parameters after value of max-age")
        {
            test_parse_targeted_cache_control("max-age=30;?bad", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("bad value for 7 characters key")
        {
            test_parse_targeted_cache_control("a123456=?bad", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("bad value for 8 characters key starts with no-")
        {
            test_parse_targeted_cache_control("no-12345=?bad", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("bad value for 8 characters key")
        {
            test_parse_targeted_cache_control("a1234567=?bad", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("bad value for 15 characters key")
        {
            test_parse_targeted_cache_control("a12345678901234=?bad", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("bad value for key of other length")
        {
            test_parse_targeted_cache_control("a=?bad", false, hsfv_targeted_cache_control_t{0});
        }
        SECTION("ends with comma")
        {
            test_parse_targeted_cache_control("a,", false, hsfv_targeted_cache_control_t{0});
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
