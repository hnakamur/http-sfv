#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("serialize item", "[serialze][item]") {
#define OK_HELPER(section, input_literal, want)                                \
  SECTION("section") {                                                         \
    hsfv_item_t item = input_literal;                                          \
    hsfv_buffer_t buf = (hsfv_buffer_t){0};                                    \
    hsfv_err_t err;                                                            \
    err = hsfv_serialize_item(&item, &hsfv_global_allocator, &buf);            \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(buf.bytes.len == strlen(want));                                      \
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));                       \
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);                          \
  }

  hsfv_parameter_t want_params[3];
  want_params[0] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "foo", .len = 3},
      .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true},
  };
  want_params[1] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "*bar", .len = 4},
      .value =
          hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_STRING,
                           .string = hsfv_token_t{.base = "baz", .len = 3}},
  };
  want_params[2] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "baz", .len = 3},
      .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = false},
  };

  OK_HELPER("case 1",
            (hsfv_item_t{
                .bare_item =
                    hsfv_bare_item_t{
                        .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                        .boolean = true,
                    },
                .parameters =
                    hsfv_parameters_t{
                        .params = &want_params[0],
                        .len = 3,
                        .capacity = 3,
                    },
            }),
            "?1;foo;*bar=\"baz\";baz=?0");
#undef OK_HELPER
}

TEST_CASE("parse item", "[parse][item]") {
#define OK_HELPER(section, input, want_len, want)                              \
  SECTION(section) {                                                           \
    hsfv_item_t item;                                                          \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_item(&item, &hsfv_global_allocator, input, input_end,     \
                          &rest);                                              \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(hsfv_item_eq(&item, want));                                          \
    CHECK(rest == input + want_len);                                           \
    hsfv_item_deinit(&item, &hsfv_global_allocator);                           \
  }

  hsfv_parameter_t want_params[2];
  want_params[0] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "foo", .len = 3},
      .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true},
  };
  want_params[1] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "*bar", .len = 4},
      .value =
          hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN,
                           .string = hsfv_token_t{.base = "tok", .len = 3}},
  };
  hsfv_item_t want = hsfv_item_t{
      .bare_item =
          hsfv_bare_item_t{
              .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
              .boolean = true,
          },
      .parameters =
          hsfv_parameters_t{
              .params = &want_params[0],
              .len = 2,
              .capacity = 2,
          },
  };

  OK_HELPER("case 1", "?1;foo;*bar=tok", strlen("?1;foo;*bar=tok"), &want);
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION("section") {                                                         \
    hsfv_item_t item;                                                          \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_item(&item, &hsfv_global_allocator, input, input_end,     \
                          &rest);                                              \
    CHECK(err == want);                                                        \
  }

#if 0
  NG_HELPER("invalid bare item", "é", HSFV_ERR_INVALID);
#endif
  NG_HELPER("invalid parameter", "tok;é", HSFV_ERR_INVALID);
#undef NG_HELPER
}
