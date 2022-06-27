#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("hsfv_parameters_eq", "[eq][parameters]")
{
    SECTION("different length")
    {
        hsfv_parameters_t parameters1 = hsfv_parameters_t{0};

        hsfv_parameter_t params2[] = {
            {
                .key = {.base = "foo", .len = 3},
                .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
            },
        };
        hsfv_parameters_t parameters2 = hsfv_parameters_t{
            .params = params2,
            .len = 1,
            .capacity = 1,
        };
        CHECK(!hsfv_parameters_eq(&parameters1, &parameters2));
    }

    SECTION("different parameter")
    {
        hsfv_parameter_t params1[] = {
            {
                .key = {.base = "foo", .len = 3},
                .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = false},
            },
        };
        hsfv_parameters_t parameters1 = hsfv_parameters_t{
            .params = params1,
            .len = 1,
            .capacity = 1,
        };

        hsfv_parameter_t params2[] = {
            {
                .key = {.base = "foo", .len = 3},
                .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
            },
        };
        hsfv_parameters_t parameters2 = hsfv_parameters_t{
            .params = params2,
            .len = 1,
            .capacity = 1,
        };
        CHECK(!hsfv_parameters_eq(&parameters1, &parameters2));
    }
}

static void serialize_parameters_ok_test(hsfv_parameters_t input, const char *want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_parameters(&input, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_parameters_alloc_error_test(hsfv_parameters_t input)
{
    hsfv_allocator_t *allocator = &hsfv_failing_allocator.allocator;
    hsfv_failing_allocator.fail_index = -1;
    hsfv_failing_allocator.alloc_count = 0;

    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_parameters(&input, allocator, &buf);
    CHECK(err == HSFV_OK);
    hsfv_buffer_deinit(&buf, allocator);

    int alloc_count = hsfv_failing_allocator.alloc_count;
    for (int i = 0; i < alloc_count; i++) {
        hsfv_failing_allocator.fail_index = i;
        hsfv_failing_allocator.alloc_count = 0;
        buf = (hsfv_buffer_t){0};
        err = hsfv_serialize_parameters(&input, allocator, &buf);
        CHECK(err == HSFV_ERR_OUT_OF_MEMORY);
        hsfv_buffer_deinit(&buf, allocator);
    }
}

TEST_CASE("serialize parameters", "[serialze][parameters]")
{

    hsfv_parameter_t want_params[] = {
        {
            .key = {.base = "foo", .len = 3},
            .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
        },
        {
            .key = {.base = "*bar", .len = 4},
            .value = {.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "baz", .len = 3}},
        },
        {
            .key = {.base = "baz", .len = 3},
            .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = false},
        },
    };

    SECTION("case 1")
    {
        serialize_parameters_ok_test(
            hsfv_parameters_t{
                .params = want_params,
                .len = 3,
                .capacity = 3,
            },
            ";foo;*bar=\"baz\";baz=?0");
    }

    SECTION("alloc error")
    {
        hsfv_parameter_t params[] = {
            {
                .key = {.base = "foo", .len = 3},
                .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
            },
            {
                .key = {.base = "barbarbarbar", .len = 12},
                .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = false},
            },
            {
                .key = {.base = "baz", .len = 3},
                .value = {.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "bazbazbazbazbazbaz", .len = 15}},
            },
        };

        serialize_parameters_alloc_error_test(hsfv_parameters_t{
            .params = params,
            .len = 3,
            .capacity = 3,
        });
    }
}

static void parse_parameters_ok_test(const char *input, size_t want_len, hsfv_parameters_t want)
{
    hsfv_parameters_t params;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_parameters(&params, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(hsfv_parameters_eq(&params, &want));
    CHECK(rest == input + want_len);
    hsfv_parameters_deinit(&params, &hsfv_global_allocator);
}

static void parse_parameters_alloc_error_test(const char *input)
{
    hsfv_allocator_t *allocator = &hsfv_failing_allocator.allocator;
    hsfv_failing_allocator.fail_index = -1;
    hsfv_failing_allocator.alloc_count = 0;

    hsfv_parameters_t params;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_parameters(&params, allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    hsfv_parameters_deinit(&params, allocator);

    int alloc_count = hsfv_failing_allocator.alloc_count;
    for (int i = 0; i < alloc_count; i++) {
        hsfv_failing_allocator.fail_index = i;
        hsfv_failing_allocator.alloc_count = 0;
        err = hsfv_parse_parameters(&params, allocator, input, input_end, &rest);
        CHECK(err == HSFV_ERR_OUT_OF_MEMORY);
    }
}

static void parse_parameters_ng_test(const char *input, hsfv_err_t want)
{
    hsfv_parameters_t params;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_parameters(&params, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse parameters", "[parse][parameters]")
{
    hsfv_parameter_t want_params[] = {
        {
            .key = {.base = "foo", .len = 3},
            .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
        },
        {
            .key = {.base = "*bar", .len = 4},
            .value = {.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "baz", .len = 3}},
        },
    };
    hsfv_parameters_t want = {
        .params = &want_params[0],
        .len = 2,
        .capacity = 2,
    };

    SECTION("case 1")
    {
        parse_parameters_ok_test(";foo=?1;*bar=\"baz\" foo", strlen(";foo=?1;*bar=\"baz\" foo") - strlen(" foo"), want);
    }
    SECTION("case 2")
    {
        parse_parameters_ok_test(";foo;*bar=\"baz\" foo", strlen(";foo;*bar=\"baz\" foo") - strlen(" foo"), want);
    }
    SECTION("case 3")
    {
        parse_parameters_ok_test(";foo=?1;*bar=tok;*bar=\"baz\" foo", strlen(";foo=?1;*bar=tok;*bar=\"baz\" foo") - strlen(" foo"),
                                 want);
    }

    SECTION("invalid key")
    {
        parse_parameters_ng_test(";é=?0", HSFV_ERR_INVALID);
    }
    SECTION("invalid value")
    {
        parse_parameters_ng_test(";foo=é", HSFV_ERR_INVALID);
    }

    SECTION("alloc error")
    {
        parse_parameters_alloc_error_test(";foo=?1;*bar=\"baz\"");
    }
}
