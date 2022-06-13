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
        return ('0' <= c && c <= '9') || ('A' <= c && c <= 'Z' || 'a' <= c && c <= 'z');
    }
}

TEST_CASE("is_token_trailing_char", "[ctype]")
{
    SECTION("result is equal to reference implementation")
    {
        for (int c = '\x00'; c <= u'\xff'; ++c) {
            CHECK(HSFV_IS_TOKEN_TRAILING_CHAR(c) == is_token_trailing_char_ref_impl(c));
        }
    }
}
