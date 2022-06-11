#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("field value can be parsed", "[field_value]") {
#define OK_HELPER(section, input, field_type, want)                            \
  SECTION(section) {                                                           \
    hsfv_field_value_t field_value;                                            \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_field_value(&field_value, field_type,                     \
                                 &htsv_global_allocator, input, input_end,     \
                                 &rest);                                       \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(hsfv_field_value_eq(&field_value, want));                            \
    CHECK(rest == input_end);                                                  \
    hsfv_field_value_deinit(&field_value, &htsv_global_allocator);             \
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
              .type = HSFV_BARE_ITEM_TYPE_BINARY,
              .bytes = hsfv_bytes_t{.base = &bytes[0], .len = 4},
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

  hsfv_field_value_t want_list = hsfv_field_value_t{
      .type = HSFV_FIELD_VALUE_TYPE_LIST,
      .list =
          hsfv_list_t{
              .members = &members[0],
              .len = 2,
              .capacity = 2,
          },
  };
  OK_HELPER(
      "list",
      "   (\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok   ",
      HSFV_FIELD_VALUE_TYPE_LIST, &want_list);

  hsfv_dict_member_t dict_members[3];
  dict_members[0] = hsfv_dict_member_t{
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
  dict_members[1] = hsfv_dict_member_t{
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
  dict_members[2] = hsfv_dict_member_t{
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
  hsfv_field_value_t want_dict = hsfv_field_value_t{
      .type = HSFV_FIELD_VALUE_TYPE_DICTIONARY,
      .dictionary =
          hsfv_dictionary_t{
              .members = dict_members,
              .len = 3,
              .capacity = 3,
          },
  };
  OK_HELPER("dict", "   a=?0, b, c; foo=bar  ",
            HSFV_FIELD_VALUE_TYPE_DICTIONARY, &want_dict);

  hsfv_parameter_t want_item_params[2];
  want_item_params[0] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "foo", .len = 3},
      .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true},
  };
  want_item_params[1] = hsfv_parameter_t{
      .key = hsfv_key_t{.base = "*bar", .len = 4},
      .value =
          hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN,
                           .string = hsfv_token_t{.base = "tok", .len = 3}},
  };
  hsfv_field_value_t want_item = hsfv_field_value_t{
      .type = HSFV_FIELD_VALUE_TYPE_ITEM,
      .item =
          hsfv_item_t{
              .bare_item =
                  hsfv_bare_item_t{
                      .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                      .boolean = true,
                  },
              .parameters =
                  hsfv_parameters_t{
                      .params = &want_item_params[0],
                      .len = 2,
                      .capacity = 2,
                  },
          },
  };

  OK_HELPER("item", "  ?1;foo;*bar=tok  ", HSFV_FIELD_VALUE_TYPE_ITEM,
            &want_item);
#undef OK_HELPER

#define NG_HELPER(section, input, field_type, want)                            \
  SECTION(section) {                                                           \
    hsfv_field_value_t field_value;                                            \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    const char *input_end = input + strlen(input);                             \
    err = hsfv_parse_field_value(&field_value, field_type,                     \
                                 &htsv_global_allocator, input, input_end,     \
                                 &rest);                                       \
    CHECK(err == want);                                                        \
  }

  NG_HELPER(
      "list",
      "   (\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok   a",
      HSFV_FIELD_VALUE_TYPE_LIST, HSFV_ERR_INVALID);
  NG_HELPER("dict", "   a=?0, b, c; foo=bar  a",
            HSFV_FIELD_VALUE_TYPE_DICTIONARY, HSFV_ERR_INVALID);
  NG_HELPER("item", "  ?1;foo;*bar=tok  a", HSFV_FIELD_VALUE_TYPE_ITEM,
            HSFV_ERR_INVALID);
#undef NG_HELPER
}
