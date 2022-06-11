#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("inner list can be parsed", "[inner_list]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    hsfv_inner_list_t inner_list;                                              \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_inner_list(&inner_list, &htsv_global_allocator, input,    \
                                input_end, &rest);                             \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(hsfv_inner_list_eq(&inner_list, want));                              \
    CHECK(rest == input_end);                                                  \
    hsfv_inner_list_deinit(&inner_list, &htsv_global_allocator);               \
  }

  hsfv_item_t items[2];

  hsfv_parameter_t params0params[2];
  params0params[0] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "a", .len = 1},
      .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true},
  };
  params0params[1] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "b", .len = 1},
      .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_INTEGER,
                                .integer = 1936},
  };
  items[0] = hsfv_item_t{
      .bare_item =
          hsfv_bare_item_t{
              .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_STRING,
                                        .string = hsfv_string_t{.base = "foo",
                                                                .len = 3}},
          },
      .parameters = {.params = &params0params[0], .len = 2, .capacity = 2},
  };

  char bytes[4];
  bytes[0] = '\1';
  bytes[1] = '\3';
  bytes[2] = '\1';
  bytes[3] = '\2';
  hsfv_parameter_t param1 = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "y", .len = 1},
      .value =
          hsfv_bare_item_t{
              .type = HSFV_BARE_ITEM_TYPE_BINARY,
              .bytes = hsfv_bytes_t{.base = &bytes[0], .len = 4},
          },
  };
  items[1] = hsfv_item_t{
      .bare_item =
          hsfv_bare_item_t{
              .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN,
                                        .string = hsfv_token_t{.base = "bar",
                                                               .len = 3}},
          },
      .parameters =
          hsfv_parameters_t{.params = &param1, .len = 1, .capacity = 1},
  };

  hsfv_parameter_t param = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "d", .len = 1},
      .value =
          hsfv_bare_item_t{
              .type = HSFV_BARE_ITEM_TYPE_DECIMAL,
              .decimal = 18.71,
          },
  };
  hsfv_inner_list_t want = hsfv_inner_list_t{
      .items = &items[0],
      .len = 2,
      .capacity = 2,
      .parameters =
          hsfv_parameters_t{.params = &param, .len = 1, .capacity = 1},
  };

  OK_HELPER("case 1", "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71", &want);
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION("section") {                                                         \
    hsfv_inner_list_t innser_list;                                             \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_inner_list(&innser_list, &htsv_global_allocator, input,   \
                                input_end, &rest);                             \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("case 1", "(\"foo\";a;b=1936 bar;y=:AQMBAg==:;é);d=18.71",
            HSFV_ERR_INVALID);
  NG_HELPER("case 2", "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71;é",
            HSFV_ERR_INVALID);
#undef NG_HELPER
}
