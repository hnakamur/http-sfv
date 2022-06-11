#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

static int is_tchar_ref_impl(char c) {
  /* https://www.rfc-editor.org/rfc/rfc7230.html#section-3.2.6 */

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
    return 1;
  default:
    return hsfv_is_digit(c) || hsfv_is_alpha(c);
  }
}

TEST_CASE("is_token_char", "[ctype]") {
  SECTION("result is equal to reference implementation") {
    unsigned char c;

    for (c = '\x00';; ++c) {
      CHECK(hsfv_is_token_char(c) == is_tchar_ref_impl(c));
      if (c == u'\xff') {
        break;
      }
    }
  }
}