#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("serialize list", "[serialze][list]") {
#define OK_HELPER(section, input_literal, want)                                \
  SECTION("section") {                                                         \
    hsfv_list_t list = input_literal;                                          \
    hsfv_buffer_t buf = (hsfv_buffer_t){0};                                    \
    hsfv_err_t err;                                                            \
    err = hsfv_serialize_list(&list, &hsfv_global_allocator, &buf);            \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(buf.bytes.len == strlen(want));                                      \
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));                       \
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);                          \
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
          hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_STRING,
                           .string = hsfv_string_t{.base = "foo", .len = 3}},
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
              .type = HSFV_BARE_ITEM_TYPE_BYTE_SEQ,
              .byte_seq = hsfv_byte_seq_t{.base = &bytes[0], .len = 4},
          },
  };
  items[1] = hsfv_item_t{
      .bare_item =
          hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN,
                           .string = hsfv_token_t{.base = "bar", .len = 3}},
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

  hsfv_list_member_t members[2];
  members[0] = hsfv_list_member_t{
      .type = HSFV_LIST_MEMBER_TYPE_INNER_LIST,
      .inner_list =
          hsfv_inner_list_t{
              .items = &items[0],
              .len = 2,
              .capacity = 2,
              .parameters =
                  hsfv_parameters_t{.params = &param, .len = 1, .capacity = 1},
          },
  };

  hsfv_parameter_t item_params[2];
  item_params[0] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "foo", .len = 3},
      .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true},
  };
  item_params[1] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "*bar", .len = 4},
      .value =
          hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN,
                           .string = hsfv_token_t{.base = "tok", .len = 3}},
  };
  members[1] = hsfv_list_member_t{
      .type = HSFV_LIST_MEMBER_TYPE_ITEM,
      .item =
          hsfv_item_t{
              .bare_item =
                  hsfv_bare_item_t{
                      .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                      .boolean = true,
                  },
              .parameters =
                  hsfv_parameters_t{
                      .params = &item_params[0],
                      .len = 2,
                      .capacity = 2,
                  },
          },
  };

  hsfv_list_t want = hsfv_list_t{
      .members = &members[0],
      .len = 2,
      .capacity = 2,
  };

  OK_HELPER("case 1",
            (hsfv_list_t{
                .members = &members[0],
                .len = 2,
                .capacity = 2,
            }),
            "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok");
#undef OK_HELPER
}

TEST_CASE("parse list", "[parse][list]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    hsfv_list_t list;                                                          \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_list(&list, &hsfv_global_allocator, input, input_end,     \
                          &rest);                                              \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(hsfv_list_eq(&list, want));                                          \
    CHECK(rest == input_end);                                                  \
    hsfv_list_deinit(&list, &hsfv_global_allocator);                           \
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
          hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_STRING,
                           .string = hsfv_string_t{.base = "foo", .len = 3}},
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
              .type = HSFV_BARE_ITEM_TYPE_BYTE_SEQ,
              .byte_seq = hsfv_byte_seq_t{.base = &bytes[0], .len = 4},
          },
  };
  items[1] = hsfv_item_t{
      .bare_item =
          hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN,
                           .string = hsfv_token_t{.base = "bar", .len = 3}},
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

  hsfv_list_member_t members[2];
  members[0] = hsfv_list_member_t{
      .type = HSFV_LIST_MEMBER_TYPE_INNER_LIST,
      .inner_list =
          hsfv_inner_list_t{
              .items = &items[0],
              .len = 2,
              .capacity = 2,
              .parameters =
                  hsfv_parameters_t{.params = &param, .len = 1, .capacity = 1},
          },
  };

  hsfv_parameter_t item_params[2];
  item_params[0] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "foo", .len = 3},
      .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true},
  };
  item_params[1] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "*bar", .len = 4},
      .value =
          hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN,
                           .string = hsfv_token_t{.base = "tok", .len = 3}},
  };
  members[1] = hsfv_list_member_t{
      .type = HSFV_LIST_MEMBER_TYPE_ITEM,
      .item =
          hsfv_item_t{
              .bare_item =
                  hsfv_bare_item_t{
                      .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                      .boolean = true,
                  },
              .parameters =
                  hsfv_parameters_t{
                      .params = &item_params[0],
                      .len = 2,
                      .capacity = 2,
                  },
          },
  };

  hsfv_list_t want = hsfv_list_t{
      .members = &members[0],
      .len = 2,
      .capacity = 2,
  };

  OK_HELPER("case 1",
            "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok",
            &want);
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION("section") {                                                         \
    hsfv_list_t innser_list;                                                   \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_list(&innser_list, &hsfv_global_allocator, input,         \
                          input_end, &rest);                                   \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("case 1",
            "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok, ",
            HSFV_ERR_EOF);
  NG_HELPER("case 1",
            "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok, é",
            HSFV_ERR_INVALID);
#undef NG_HELPER
}
