#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("serialize parameters", "[serialze][parameters]")
{
#define OK_HELPER(section, input_literal, want)                                                                                    \
    SECTION("section")                                                                                                             \
    {                                                                                                                              \
        hsfv_parameters_t params = input_literal;                                                                                  \
        hsfv_buffer_t buf = (hsfv_buffer_t){0};                                                                                    \
        hsfv_err_t err;                                                                                                            \
        err = hsfv_serialize_parameters(&params, &hsfv_global_allocator, &buf);                                                    \
        CHECK(err == HSFV_OK);                                                                                                     \
        CHECK(buf.bytes.len == strlen(want));                                                                                      \
        CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));                                                                       \
        hsfv_buffer_deinit(&buf, &hsfv_global_allocator);                                                                          \
    }

    hsfv_parameter_t want_params[3];
    want_params[0] = hsfv_parameter_t{
        .key = hsfv_key_t{.base = "foo", .len = 3},
        .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
    };
    want_params[1] = hsfv_parameter_t{
        .key = hsfv_key_t{.base = "*bar", .len = 4},
        .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_STRING, .string = hsfv_token_t{.base = "baz", .len = 3}},
    };
    want_params[2] = hsfv_parameter_t{
        .key = hsfv_key_t{.base = "baz", .len = 3},
        .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = false},
    };

    OK_HELPER("case 1",
              (hsfv_parameters_t{
                  .params = &want_params[0],
                  .len = 3,
                  .capacity = 3,
              }),
              ";foo;*bar=\"baz\";baz=?0");
#undef OK_HELPER
}

TEST_CASE("parse parameters", "[parse][parameters]")
{
#define OK_HELPER(section, input, want_len, want)                                                                                  \
    SECTION(section)                                                                                                               \
    {                                                                                                                              \
        hsfv_parameters_t params;                                                                                                  \
        hsfv_err_t err;                                                                                                            \
        const char *rest;                                                                                                          \
        const char *input_end = input + strlen(input);                                                                             \
        err = hsfv_parse_parameters(&params, &hsfv_global_allocator, input, input_end, &rest);                                     \
        CHECK(err == HSFV_OK);                                                                                                     \
        CHECK(hsfv_parameters_eq(&params, want));                                                                                  \
        CHECK(rest == input + want_len);                                                                                           \
        hsfv_parameters_deinit(&params, &hsfv_global_allocator);                                                                   \
    }

    hsfv_parameter_t want_params[2];
    want_params[0] = hsfv_parameter_t{
        .key = hsfv_key_t{.base = "foo", .len = 3},
        .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
    };
    want_params[1] = hsfv_parameter_t{
        .key = hsfv_key_t{.base = "*bar", .len = 4},
        .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_STRING, .string = hsfv_token_t{.base = "baz", .len = 3}},
    };
    hsfv_parameters_t want = hsfv_parameters_t{
        .params = &want_params[0],
        .len = 2,
        .capacity = 2,
    };

    OK_HELPER("case 1", ";foo=?1;*bar=\"baz\" foo", strlen(";foo=?1;*bar=\"baz\" foo") - strlen(" foo"), &want);
    OK_HELPER("case 2", ";foo;*bar=\"baz\" foo", strlen(";foo;*bar=\"baz\" foo") - strlen(" foo"), &want);
    OK_HELPER("case 3", ";foo=?1;*bar=tok;*bar=\"baz\" foo", strlen(";foo=?1;*bar=tok;*bar=\"baz\" foo") - strlen(" foo"), &want);
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                                                                            \
    SECTION("section")                                                                                                             \
    {                                                                                                                              \
        hsfv_parameters_t params;                                                                                                  \
        hsfv_err_t err;                                                                                                            \
        const char *rest;                                                                                                          \
        const char *input_end = input + strlen(input);                                                                             \
        err = hsfv_parse_parameters(&params, &hsfv_global_allocator, input, input_end, &rest);                                     \
        CHECK(err == want);                                                                                                        \
    }

    NG_HELPER("invalid key", ";é=?0", HSFV_ERR_INVALID);
    NG_HELPER("invalid value", ";foo=é", HSFV_ERR_INVALID);
#undef NG_HELPER
}
