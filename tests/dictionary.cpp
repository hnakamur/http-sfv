#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

/* Dictionary test data */

static hsfv_parameter_t c_param = {
    .key = {.base = "foo", .len = 3},
    .value =
        {
            .type = HSFV_BARE_ITEM_TYPE_TOKEN,
            .token = {.base = "bar", .len = 3},
        },
};

static hsfv_dict_member_t members[] = {
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

static hsfv_dictionary_t test_dict = {
    .members = members,
    .len = 3,
    .capacity = 3,
};

static void serialize_dictionary_ok_test(hsfv_dictionary_t input, const char *want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_dictionary(&input, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

TEST_CASE("serialize dictionary", "[serialze][dictionary]")
{
    SECTION("case 1")
    {
        serialize_dictionary_ok_test(test_dict, "a=?0, b, c;foo=bar");
    }
}

static void parse_dictionary_ok_test(const char *input, hsfv_dictionary_t want)
{
    hsfv_dictionary_t dictionary;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_dictionary(&dictionary, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(hsfv_dictionary_eq(&dictionary, &want));
    CHECK(rest == input_end);
    hsfv_dictionary_deinit(&dictionary, &hsfv_global_allocator);
}

static void parse_dictionary_ng_test(const char *input, hsfv_err_t want)
{
    hsfv_dictionary_t dictionary;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_dictionary(&dictionary, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse dictionary", "[parse][dictionary]")
{
    SECTION("ok case 1")
    {
        parse_dictionary_ok_test("a=?0, b, c; foo=bar", test_dict);
    }
    SECTION("ok case 2")
    {
        parse_dictionary_ok_test("a, b, a=?0, c; foo=bar", test_dict);
    }

    SECTION("ng case 1")
    {
        parse_dictionary_ng_test("a=?0, b, c; foo=bar, ", HSFV_ERR_EOF);
    }
    SECTION("ng case 2")
    {
        parse_dictionary_ng_test("a=?0, b, c; foo=bar, é", HSFV_ERR_INVALID);
    }
}
