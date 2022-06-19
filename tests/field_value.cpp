#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

/* List test data */

static hsfv_parameter_t params0params[] = {

    {
        .key = {.base = "a", .len = 1},
        .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
    },
    {
        .key = {.base = "b", .len = 1},
        .value = {.type = HSFV_BARE_ITEM_TYPE_INTEGER, .integer = 1936},
    },
};

static char bytes[] = {'\1', '\3', '\1', '\2'};

static hsfv_parameter_t param1 = {
    .key = {.base = "y", .len = 1},
    .value =
        {
            .type = HSFV_BARE_ITEM_TYPE_BYTE_SEQ,
            .byte_seq = {.base = bytes, .len = 4},
        },
};

static hsfv_item_t items[] = {
    {
        .bare_item = {.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "foo", .len = 3}},
        .parameters = {.params = params0params, .len = 2, .capacity = 2},
    },
    {
        .bare_item = {.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = "bar", .len = 3}},
        .parameters = {.params = &param1, .len = 1, .capacity = 1},
    },
};

static hsfv_parameter_t param = {
    .key = {.base = "d", .len = 1},
    .value =
        {
            .type = HSFV_BARE_ITEM_TYPE_DECIMAL,
            .decimal = 18.71,
        },
};

static hsfv_parameter_t item_params[] = {
    {
        .key = {.base = "foo", .len = 3},
        .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
    },
    {
        .key = {.base = "*bar", .len = 4},
        .value = {.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = "tok", .len = 3}},
    },
};

static hsfv_list_member_t members[2] = {
    {
        .type = HSFV_LIST_MEMBER_TYPE_INNER_LIST,
        .inner_list =
            {
                .items = items,
                .len = 2,
                .capacity = 2,
                .parameters = {.params = &param, .len = 1, .capacity = 1},
            },
    },

    {
        .type = HSFV_LIST_MEMBER_TYPE_ITEM,
        .item =
            {
                .bare_item =
                    {
                        .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                        .boolean = true,
                    },
                .parameters =
                    {
                        .params = item_params,
                        .len = 2,
                        .capacity = 2,
                    },
            },
    },
};

static hsfv_field_value_t test_list = {
    .type = HSFV_FIELD_VALUE_TYPE_LIST,
    .list =
        {
            .members = members,
            .len = 2,
            .capacity = 2,
        },
};

/* Dictionary test data */

static hsfv_parameter_t c_param = {
    .key = {.base = "foo", .len = 3},
    .value =
        {
            .type = HSFV_BARE_ITEM_TYPE_TOKEN,
            .token = {.base = "bar", .len = 3},
        },
};

static hsfv_dict_member_t dict_members[3] = {
    {
        .key = {.base = "a", .len = 1},
        .value =
            {
                .type = HSFV_DICT_MEMBER_TYPE_ITEM,
                .item =
                    {
                        .bare_item =
                            {
                                .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = false,
                            },
                    },
            },
    },
    {
        .key = {.base = "b", .len = 1},
        .value =
            {
                .type = HSFV_DICT_MEMBER_TYPE_ITEM,
                .item =
                    {
                        .bare_item =
                            {
                                .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true,
                            },
                    },
            },
    },
    {
        .key = {.base = "c", .len = 1},
        .value =
            {
                .type = HSFV_DICT_MEMBER_TYPE_ITEM,
                .item =
                    {
                        .bare_item =
                            {
                                .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true,
                            },
                        .parameters =
                            {
                                .params = &c_param,
                                .len = 1,
                                .capacity = 1,
                            },
                    },
            },
    },
};

static hsfv_field_value_t test_dict = {
    .type = HSFV_FIELD_VALUE_TYPE_DICTIONARY,
    .dictionary =
        {
            .members = dict_members,
            .len = 3,
            .capacity = 3,
        },
};

/* Item test data */

static hsfv_parameter_t input_item_params[] = {
    {
        .key = {.base = "foo", .len = 3},
        .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
    },
    {
        .key = {.base = "*bar", .len = 4},
        .value = {.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = "tok", .len = 3}},
    },
};

static hsfv_field_value_t test_item = {
    .type = HSFV_FIELD_VALUE_TYPE_ITEM,
    .item =
        {
            .bare_item =
                {
                    .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                    .boolean = true,
                },
            .parameters =
                {
                    .params = input_item_params,
                    .len = 2,
                    .capacity = 2,
                },
        },
};

TEST_CASE("check field_value is_empty", "[is_empty][field_value]")
{
#define OK_HELPER(section, input_literal, want)                                                                                    \
    SECTION(section)                                                                                                               \
    {                                                                                                                              \
        hsfv_field_value_t field_value = input_literal;                                                                            \
        bool got = hsfv_field_value_is_empty(&field_value);                                                                        \
        CHECK(got == want);                                                                                                        \
    }

    hsfv_dict_member_t dict_member = {
        .key = {.base = "foo", .len = 3},
        .value =
            {
                .type = HSFV_DICT_MEMBER_TYPE_ITEM,
                .item =
                    {
                        .bare_item =
                            {
                                .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true,
                            },
                    },
            },
    };

    hsfv_list_member_t list_member = {
        .type = HSFV_LIST_MEMBER_TYPE_ITEM,
        .item =
            {
                .bare_item =
                    {
                        .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                        .boolean = true,
                    },
            },
    };

    OK_HELPER("empty dict",
              (hsfv_field_value_t{
                  .type = HSFV_FIELD_VALUE_TYPE_DICTIONARY,
                  .dictionary = {.members = NULL, .len = 0, .capacity = 8},
              }),
              true);
    OK_HELPER("non empty dict",
              (hsfv_field_value_t{
                  .type = HSFV_FIELD_VALUE_TYPE_DICTIONARY,
                  .dictionary = {.members = &dict_member, .len = 1, .capacity = 8},
              }),
              false);
    OK_HELPER("empty list",
              (hsfv_field_value_t{
                  .type = HSFV_FIELD_VALUE_TYPE_LIST,
                  .list = {.members = NULL, .len = 0, .capacity = 8},
              }),
              true);
    OK_HELPER("non empty list",
              (hsfv_field_value_t{
                  .type = HSFV_FIELD_VALUE_TYPE_LIST,
                  .list = {.members = &list_member, .len = 1, .capacity = 8},
              }),
              false);
    OK_HELPER("item",
              (hsfv_field_value_t{
                  .type = HSFV_FIELD_VALUE_TYPE_ITEM,
                  .item =
                      {
                          .bare_item =
                              {
                                  .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                  .boolean = true,
                              },
                      },
              }),
              false);
#undef OK_HELPER
}

TEST_CASE("serialize field_value", "[serialze][field_value]")
{
#define OK_HELPER(section, input, want)                                                                                            \
    SECTION("section")                                                                                                             \
    {                                                                                                                              \
        hsfv_buffer_t buf = (hsfv_buffer_t){0};                                                                                    \
        hsfv_err_t err;                                                                                                            \
        err = hsfv_serialize_field_value(input, &hsfv_global_allocator, &buf);                                                     \
        CHECK(err == HSFV_OK);                                                                                                     \
        CHECK(buf.bytes.len == strlen(want));                                                                                      \
        CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));                                                                       \
        hsfv_buffer_deinit(&buf, &hsfv_global_allocator);                                                                          \
    }

    OK_HELPER("list", &test_list, "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok");
    OK_HELPER("dict", &test_dict, "a=?0, b, c;foo=bar");
    OK_HELPER("item", &test_item, "?1;foo;*bar=tok");
#undef OK_HELPER
}

TEST_CASE("parse field_value", "[parse][field_value]")
{
#define OK_HELPER(section, input, field_type, want)                                                                                \
    SECTION(section)                                                                                                               \
    {                                                                                                                              \
        hsfv_field_value_t field_value;                                                                                            \
        hsfv_err_t err;                                                                                                            \
        const char *rest;                                                                                                          \
        const char *input_end = input + strlen(input);                                                                             \
        err = hsfv_parse_field_value(&field_value, field_type, &hsfv_global_allocator, input, input_end, &rest);                   \
        CHECK(err == HSFV_OK);                                                                                                     \
        CHECK(hsfv_field_value_eq(&field_value, want));                                                                            \
        CHECK(rest == input_end);                                                                                                  \
        hsfv_field_value_deinit(&field_value, &hsfv_global_allocator);                                                             \
    }

    OK_HELPER("list", "   (\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok   ", HSFV_FIELD_VALUE_TYPE_LIST, &test_list);
    OK_HELPER("dict", "   a=?0, b, c; foo=bar  ", HSFV_FIELD_VALUE_TYPE_DICTIONARY, &test_dict);
    OK_HELPER("item", "  ?1;foo;*bar=tok  ", HSFV_FIELD_VALUE_TYPE_ITEM, &test_item);
#undef OK_HELPER

#define NG_HELPER(section, input, field_type, want)                                                                                \
    SECTION(section)                                                                                                               \
    {                                                                                                                              \
        hsfv_field_value_t field_value;                                                                                            \
        hsfv_err_t err;                                                                                                            \
        const char *rest;                                                                                                          \
        const char *input_end = input + strlen(input);                                                                             \
        err = hsfv_parse_field_value(&field_value, field_type, &hsfv_global_allocator, input, input_end, &rest);                   \
        CHECK(err == want);                                                                                                        \
    }

    NG_HELPER("list", "   (\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok   a", HSFV_FIELD_VALUE_TYPE_LIST,
              HSFV_ERR_INVALID);
    NG_HELPER("dict", "   a=?0, b, c; foo=bar  a", HSFV_FIELD_VALUE_TYPE_DICTIONARY, HSFV_ERR_INVALID);
    NG_HELPER("item", "  ?1;foo;*bar=tok  a", HSFV_FIELD_VALUE_TYPE_ITEM, HSFV_ERR_INVALID);
#undef NG_HELPER
}
