#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

static bool is_token_trailing_char_ref_impl(char c)
{
    switch (c) {
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '*':
    case '+':
    case '-':
    case '.':
    case '^':
    case '_':
    case '`':
    case '|':
    case '~':
    case ':':
    case '/':
        return true;
    default:
        return ('0' <= c && c <= '9') || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
    }
}

TEST_CASE("is_token_trailing_char", "[string]")
{
    SECTION("result is equal to reference implementation")
    {
        for (int c = '\x00'; c <= u'\xff'; ++c) {
            CHECK(HSFV_IS_TOKEN_TRAILING_CHAR(c) == is_token_trailing_char_ref_impl(c));
        }
    }
}

TEST_CASE("hsfv_is_ascii_string", "[string]")
{
    SECTION("ok")
    {
        const char *input = "abc";
        const char *input_end = input + strlen(input);
        CHECK(hsfv_is_ascii_string(input, input_end));
    }

    SECTION("ng")
    {
        const char *input = "ab\x80";
        const char *input_end = input + strlen(input);
        CHECK(!hsfv_is_ascii_string(input, input_end));
    }
}

TEST_CASE("hsfv_strncasecmp", "[string]")
{
    SECTION("equal")
    {
        CHECK(hsfv_strncasecmp("abc", "abc", 3) == 0);
        CHECK(hsfv_strncasecmp("Max-Age", "max-age", 7) == 0);
        CHECK(hsfv_strncasecmp("ABC", "abcd", 3) == 0);
        CHECK(hsfv_strncasecmp("ABCD", "abce", 3) == 0);
        CHECK(hsfv_strncasecmp("a", "b", 0) == 0);
        CHECK(hsfv_strncasecmp("\x80\xff", "\x80\xff", 0) == 0);
    }

    SECTION("shorter than n")
    {
        CHECK(hsfv_strncasecmp("ab", "ab", 3) == 0);
    }

    SECTION("shorter than n for strncasecmp")
    {
        CHECK(strncasecmp("ab", "ab", 3) == 0);
    }

    SECTION("not equal")
    {
        CHECK(hsfv_strncasecmp("b", "a", 1) > 0);
        CHECK(hsfv_strncasecmp("b", "A", 1) > 0);
        CHECK(hsfv_strncasecmp("a", "b", 1) < 0);
        CHECK(hsfv_strncasecmp("A", "b", 1) < 0);
    }
}
