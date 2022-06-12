#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("serialize dictionary", "[serialze][dictionary]") {
#define OK_HELPER(section, input_literal, want)                                \
  SECTION("section") {                                                         \
    hsfv_dictionary_t dictionary = input_literal;                              \
    hsfv_buffer_t buf = (hsfv_buffer_t){0};                                    \
    hsfv_err_t err;                                                            \
    err =                                                                      \
        hsfv_serialize_dictionary(&dictionary, &hsfv_global_allocator, &buf);  \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(buf.bytes.len == strlen(want));                                      \
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));                       \
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);                          \
  }

  hsfv_dict_member_t members[3];
  members[0] = hsfv_dict_member_t{
      .key = hsfv_key_t{.base = "a", .len = 1},
      .value =
          hsfv_dict_member_value_t{
              .type = HSFV_DICT_MEMBER_TYPE_ITEM,
              .item =
                  hsfv_item_t{
                      .bare_item =
                          hsfv_bare_item_t{
                              .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                              .boolean = false,
                          },
                  },
          },
  };
  members[1] = hsfv_dict_member_t{
      .key = hsfv_key_t{.base = "b", .len = 1},
      .value =
          hsfv_dict_member_value_t{
              .type = HSFV_DICT_MEMBER_TYPE_ITEM,
              .item =
                  hsfv_item_t{
                      .bare_item =
                          hsfv_bare_item_t{
                              .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                              .boolean = true,
                          },
                  },
          },
  };
  hsfv_parameter_t c_param = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "foo", .len = 3},
      .value =
          hsfv_bare_item_t{
              .type = HSFV_BARE_ITEM_TYPE_TOKEN,
              .token = hsfv_token_t{.base = "bar", .len = 3},
          },
  };
  members[2] = hsfv_dict_member_t{
      .key = hsfv_key_t{.base = "c", .len = 1},
      .parameters =
          hsfv_parameters_t{
              .params = &c_param,
              .len = 1,
              .capacity = 1,
          },
      .value =
          hsfv_dict_member_value_t{
              .type = HSFV_DICT_MEMBER_TYPE_ITEM,
              .item =
                  hsfv_item_t{
                      .bare_item =
                          hsfv_bare_item_t{
                              .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                              .boolean = true,
                          },
                  },
          },
  };

  OK_HELPER("case 1",
            (hsfv_dictionary_t{
                .members = members,
                .len = 3,
                .capacity = 3,
            }),
            "a=?0, b, c;foo=bar");
#undef OK_HELPER
}

TEST_CASE("parse dictionary", "[parse][dictionary]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    hsfv_dictionary_t dictionary;                                              \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_dictionary(&dictionary, &hsfv_global_allocator, input,    \
                                input_end, &rest);                             \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(hsfv_dictionary_eq(&dictionary, want));                              \
    CHECK(rest == input_end);                                                  \
    hsfv_dictionary_deinit(&dictionary, &hsfv_global_allocator);               \
  }

  hsfv_dict_member_t members[3];
  members[0] = hsfv_dict_member_t{
      .key = hsfv_key_t{.base = "a", .len = 1},
      .value =
          hsfv_dict_member_value_t{
              .type = HSFV_DICT_MEMBER_TYPE_ITEM,
              .item =
                  hsfv_item_t{
                      .bare_item =
                          hsfv_bare_item_t{
                              .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                              .boolean = false,
                          },
                  },
          },
  };
  members[1] = hsfv_dict_member_t{
      .key = hsfv_key_t{.base = "b", .len = 1},
      .value =
          hsfv_dict_member_value_t{
              .type = HSFV_DICT_MEMBER_TYPE_ITEM,
              .item =
                  hsfv_item_t{
                      .bare_item =
                          hsfv_bare_item_t{
                              .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                              .boolean = true,
                          },
                  },
          },
  };
  hsfv_parameter_t c_param = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "foo", .len = 3},
      .value =
          hsfv_bare_item_t{
              .type = HSFV_BARE_ITEM_TYPE_TOKEN,
              .token = hsfv_token_t{.base = "bar", .len = 3},
          },
  };
  members[2] = hsfv_dict_member_t{
      .key = hsfv_key_t{.base = "c", .len = 1},
      .parameters =
          hsfv_parameters_t{
              .params = &c_param,
              .len = 1,
              .capacity = 1,
          },
      .value =
          hsfv_dict_member_value_t{
              .type = HSFV_DICT_MEMBER_TYPE_ITEM,
              .item =
                  hsfv_item_t{
                      .bare_item =
                          hsfv_bare_item_t{
                              .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                              .boolean = true,
                          },
                  },
          },
  };
  hsfv_dictionary_t want = hsfv_dictionary_t{
      .members = members,
      .len = 3,
      .capacity = 3,
  };
  OK_HELPER("case 1", "a=?0, b, c; foo=bar", &want);
  OK_HELPER("case 2", "a, b, a=?0, c; foo=bar", &want);
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    hsfv_dictionary_t dictionary;                                              \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_dictionary(&dictionary, &hsfv_global_allocator, input,    \
                                input_end, &rest);                             \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("case 1", "a=?0, b, c; foo=bar, ", HSFV_ERR_EOF);
  NG_HELPER("case 2", "a=?0, b, c; foo=bar, é", HSFV_ERR_INVALID);
#undef NG_HELPER
}
