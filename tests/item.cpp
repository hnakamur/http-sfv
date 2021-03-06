#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

static void serialize_item_ok_test(hsfv_item_t input, const char *want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_item(&input, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_item_ng_test(hsfv_item_t input, hsfv_err_t want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_item(&input, &hsfv_global_allocator, &buf);
    CHECK(err == want);
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

TEST_CASE("serialize item", "[serialze][item]")
{
    SECTION("ok case 1")
    {
        hsfv_parameter_t params[] = {
            {
                .key = hsfv_key_t{.base = "foo", .len = 3},
                .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
            },
            {
                .key = hsfv_key_t{.base = "*bar", .len = 4},
                .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "baz", .len = 3}},
            },
            {
                .key = hsfv_key_t{.base = "baz", .len = 3},
                .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = false},
            },
        };

        hsfv_item_t input = {
            .bare_item =
                hsfv_bare_item_t{
                    .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                    .boolean = true,
                },
            .parameters =
                hsfv_parameters_t{
                    .params = params,
                    .len = 3,
                    .capacity = 3,
                },
        };

        serialize_item_ok_test(input, "?1;foo;*bar=\"baz\";baz=?0");
    }

    SECTION("invalid bare_item type")
    {
        serialize_item_ng_test(
            hsfv_item_t{
                .bare_item = {.type = (hsfv_bare_item_type_t)(-1)},
            },
            HSFV_ERR_INVALID);
    }

    SECTION("invalid parameter key")
    {
        hsfv_parameter_t params[] = {
            {
                .key = hsfv_key_t{.base = "foo?", .len = 4},
                .value = hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
            },
        };

        hsfv_item_t input = {
            .bare_item =
                hsfv_bare_item_t{
                    .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                    .boolean = true,
                },
            .parameters =
                hsfv_parameters_t{
                    .params = params,
                    .len = 1,
                    .capacity = 1,
                },
        };

        serialize_item_ng_test(input, HSFV_ERR_INVALID);
    }
}

static void parse_item_ok_test(const char *input, size_t want_len, hsfv_item_t want)
{
    hsfv_item_t item;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_item(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(hsfv_item_eq(&item, &want));
    CHECK(rest == input + want_len);
    hsfv_item_deinit(&item, &hsfv_global_allocator);
}

static void parse_item_ng_test(const char *input, hsfv_err_t want)
{
    hsfv_item_t item;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_item(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse item", "[parse][item]")
{
    hsfv_parameter_t want_params[] = {
        {
            .key = {.base = "foo", .len = 3},
            .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
        },
        {
            .key = {.base = "*bar", .len = 4},
            .value = {.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = "tok", .len = 3}},
        },
    };

    hsfv_item_t want = {
        .bare_item =
            {
                .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                .boolean = true,
            },
        .parameters =
            {
                .params = want_params,
                .len = 2,
                .capacity = 2,
            },
    };

    SECTION("case 1")
    {
        parse_item_ok_test("?1;foo;*bar=tok", strlen("?1;foo;*bar=tok"), want);
    }

    SECTION("invalid bare item")
    {
        parse_item_ng_test("??", HSFV_ERR_INVALID);
    }
    SECTION("invalid parameter")
    {
        parse_item_ng_test("tok;??", HSFV_ERR_INVALID);
    }
}
