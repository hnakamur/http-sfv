#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

static void test_is_parsable_boolean(const char *input, bool want)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_is_parsable_boolean(input, input_end, &rest);
    CHECK(got == want);
}

TEST_CASE("is_parsable_boolean", "[parsable][boolean]")
{
    SECTION("false")
    {
        test_is_parsable_boolean("?0", true);
    }
    SECTION("true")
    {
        test_is_parsable_boolean("?1", true);
    }

    SECTION("empty")
    {
        test_is_parsable_boolean("", false);
    }
    SECTION("just question")
    {
        test_is_parsable_boolean("?", false);
    }
    SECTION("not starts with question")
    {
        test_is_parsable_boolean("1", false);
    }
    SECTION("bad char after question")
    {
        test_is_parsable_boolean("?2", false);
    }
}
