#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

/* Test inner list */

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

static hsfv_inner_list_t test_inner_list = {
    .items = items,
    .len = 2,
    .capacity = 2,
    .parameters = {.params = &param, .len = 1, .capacity = 1},
};

TEST_CASE("serialize inner_list", "[serialze][inner_list]")
{
#define OK_HELPER(section, input, want)                                                                                            \
    SECTION("section")                                                                                                             \
    {                                                                                                                              \
        hsfv_buffer_t buf = (hsfv_buffer_t){0};                                                                                    \
        hsfv_err_t err;                                                                                                            \
        err = hsfv_serialize_inner_list(input, &hsfv_global_allocator, &buf);                                                      \
        CHECK(err == HSFV_OK);                                                                                                     \
        CHECK(buf.bytes.len == strlen(want));                                                                                      \
        CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));                                                                       \
        hsfv_buffer_deinit(&buf, &hsfv_global_allocator);                                                                          \
    }

    OK_HELPER("case 1", &test_inner_list, "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71");
#undef OK_HELPER
}

TEST_CASE("parse inner_list", "[parse][inner_list]")
{
#define OK_HELPER(section, input, want)                                                                                            \
    SECTION(section)                                                                                                               \
    {                                                                                                                              \
        hsfv_inner_list_t inner_list;                                                                                              \
        hsfv_err_t err;                                                                                                            \
        const char *rest;                                                                                                          \
        const char *input_end = input + strlen(input);                                                                             \
        err = hsfv_parse_inner_list(&inner_list, &hsfv_global_allocator, input, input_end, &rest);                                 \
        CHECK(err == HSFV_OK);                                                                                                     \
        CHECK(hsfv_inner_list_eq(&inner_list, want));                                                                              \
        CHECK(rest == input_end);                                                                                                  \
        hsfv_inner_list_deinit(&inner_list, &hsfv_global_allocator);                                                               \
    }

    OK_HELPER("case 1", "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71", &test_inner_list);
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                                                                            \
    SECTION("section")                                                                                                             \
    {                                                                                                                              \
        hsfv_inner_list_t innser_list;                                                                                             \
        hsfv_err_t err;                                                                                                            \
        const char *rest;                                                                                                          \
        const char *input_end = input + strlen(input);                                                                             \
        err = hsfv_parse_inner_list(&innser_list, &hsfv_global_allocator, input, input_end, &rest);                                \
        CHECK(err == want);                                                                                                        \
    }

    NG_HELPER("case 1", "(\"foo\";a;b=1936 bar;y=:AQMBAg==:;é);d=18.71", HSFV_ERR_INVALID);
    NG_HELPER("case 2", "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71;é", HSFV_ERR_INVALID);
#undef NG_HELPER
}
