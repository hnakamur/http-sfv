#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

static int is_tchar_ref_impl(char c)
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
        return 1;
    default:
        return HSFV_IS_DIGIT(c) || HSFV_IS_ALPHA(c);
    }
}

TEST_CASE("is_token_char", "[ctype]")
{
    SECTION("result is equal to reference implementation")
    {
        for (int c = '\x00'; c <= u'\xff'; ++c) {
            CHECK(HSFV_IS_TOKEN_TRAILING_CHAR(c) == is_tchar_ref_impl(c));
        }
    }
}
