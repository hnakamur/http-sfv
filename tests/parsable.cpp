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

static void test_is_parsable_number(const char *input, bool want)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_is_parsable_number(input, input_end, &rest);
    CHECK(got == want);
}

TEST_CASE("is_parsable_number", "[parsable][number]")
{
    SECTION("positive integer")
    {
        test_is_parsable_number("1871", true);
    }
    SECTION("negative integer")
    {
        test_is_parsable_number("-1871", true);
    }
    SECTION("positive integer followed by non number")
    {
        test_is_parsable_number("1871next", true);
    }
    SECTION("minimum integer")
    {
        test_is_parsable_number("-999999999999999", true);
    }
    SECTION("minimum integer")
    {
        test_is_parsable_number("999999999999999", true);
    }
    SECTION("positive decimal")
    {
        test_is_parsable_number("18.71", true);
    }
    SECTION("negative decimal")
    {
        test_is_parsable_number("-18.71", true);
    }
    SECTION("negative decimal followed by non number")
    {
        test_is_parsable_number("-18.71next", true);
    }
    SECTION("three frac digits, last zero")
    {
        test_is_parsable_number("-18.710", true);
    }
    SECTION("three frac digits, last non-zero")
    {
        test_is_parsable_number("-18.712", true);
    }

    SECTION("empty")
    {
        test_is_parsable_number("", false);
    }
    SECTION("not digit")
    {
        test_is_parsable_number("a", false);
    }
    SECTION("eof after minus sign")
    {
        test_is_parsable_number("-", false);
    }
    SECTION("not digit after minus sign")
    {
        test_is_parsable_number("-a", false);
    }
    SECTION("integer with too many digits")
    {
        test_is_parsable_number("1234567890123456", false);
    }
    SECTION("smaller than minimum")
    {
        test_is_parsable_number("-1000000000000000", false);
    }
    SECTION("larger than maximum")
    {
        test_is_parsable_number("1000000000000000", false);
    }
    SECTION("multiple dots case 1")
    {
        test_is_parsable_number("1..0", false);
    }
    SECTION("multiple dots case 2")
    {
        test_is_parsable_number("1.2.0", false);
    }
    SECTION("starts with digit")
    {
        test_is_parsable_number(".1", false);
    }
    SECTION("ends with digit")
    {
        test_is_parsable_number("1.", false);
    }
    SECTION("more than three fraction digits")
    {
        test_is_parsable_number("10.1234", false);
    }
    SECTION("decimal with more than twelve int digits")
    {
        test_is_parsable_number("1234567890123.0", false);
    }
}
