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

static hsfv_byte_t bytes[] = {'\1', '\3', '\1', '\2'};
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

TEST_CASE("hsfv_inner_list_eq", "[eq][inner_list]")
{
    SECTION("different length")
    {
        hsfv_inner_list_t empty = hsfv_inner_list_t{0};

        CHECK(!hsfv_inner_list_eq(&empty, &test_inner_list));
    }

    SECTION("different member")
    {
        hsfv_item_t items2[] = {
            {
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "foo", .len = 3}},
                .parameters = {.params = params0params, .len = 2, .capacity = 2},
            },
            {
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = "baz", .len = 3}},
                .parameters = {.params = &param1, .len = 1, .capacity = 1},
            },
        };
        hsfv_inner_list_t test_inner_list2 = {
            .items = items2,
            .len = 2,
            .capacity = 2,
            .parameters = {.params = &param, .len = 1, .capacity = 1},
        };
        CHECK(!hsfv_inner_list_eq(&test_inner_list, &test_inner_list2));
    }
}

static void serialize_inner_list_ok_test(hsfv_inner_list_t input, const char *want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_inner_list(&input, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_inner_list_alloc_error_test(hsfv_inner_list_t input)
{
    hsfv_allocator_t *allocator = &hsfv_failing_allocator.allocator;
    hsfv_failing_allocator.fail_index = -1;
    hsfv_failing_allocator.alloc_count = 0;

    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_inner_list(&input, allocator, &buf);
    CHECK(err == HSFV_OK);
    hsfv_buffer_deinit(&buf, allocator);

    int alloc_count = hsfv_failing_allocator.alloc_count;
    for (int i = 0; i < alloc_count; i++) {
        hsfv_failing_allocator.fail_index = i;
        hsfv_failing_allocator.alloc_count = 0;
        buf = (hsfv_buffer_t){0};
        err = hsfv_serialize_inner_list(&input, allocator, &buf);
        CHECK(err == HSFV_ERR_OUT_OF_MEMORY);
        hsfv_buffer_deinit(&buf, allocator);
    }
}

TEST_CASE("serialize inner_list", "[serialze][inner_list]")
{
    SECTION("case 1")
    {
        serialize_inner_list_ok_test(test_inner_list, "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71");
    }

    SECTION("alloc error 1")
    {
        hsfv_item_t items[] = {
            {
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "abcd", .len = 4}},
            },
            {
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = false},
            },
        };

        serialize_inner_list_alloc_error_test(hsfv_inner_list_t{
            .items = items,
            .len = 2,
            .capacity = 2,
        });
    }

    SECTION("alloc error 2")
    {
        hsfv_item_t items[] = {
            {
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_TOKEN, .string = {.base = "abcdefg", .len = 7}},
            },
            {
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_INTEGER, .integer = 123456789},
            },
        };

        serialize_inner_list_alloc_error_test(hsfv_inner_list_t{
            .items = items,
            .len = 2,
            .capacity = 2,
        });
    }
}

static void parse_inner_list_ok_test(const char *input, hsfv_inner_list_t want)
{
    hsfv_inner_list_t inner_list;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_inner_list(&inner_list, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(hsfv_inner_list_eq(&inner_list, &want));
    CHECK(rest == input_end);
    hsfv_inner_list_deinit(&inner_list, &hsfv_global_allocator);
}

static void parse_inner_list_ng_test(const char *input, hsfv_err_t want)
{
    hsfv_inner_list_t innser_list;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_inner_list(&innser_list, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

static void parse_inner_list_alloc_error_test(const char *input)
{
    hsfv_allocator_t *allocator = &hsfv_failing_allocator.allocator;
    hsfv_failing_allocator.fail_index = -1;
    hsfv_failing_allocator.alloc_count = 0;

    const char *input_end = input + strlen(input);
    hsfv_inner_list_t inner_list;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_inner_list(&inner_list, allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    hsfv_inner_list_deinit(&inner_list, allocator);

    int alloc_count = hsfv_failing_allocator.alloc_count;
    for (int i = 0; i < alloc_count; i++) {
        hsfv_failing_allocator.fail_index = i;
        hsfv_failing_allocator.alloc_count = 0;
        err = hsfv_parse_inner_list(&inner_list, allocator, input, input_end, &rest);
        CHECK(err == HSFV_ERR_OUT_OF_MEMORY);
    }
}

TEST_CASE("parse inner_list", "[parse][inner_list]")
{
    SECTION("ok case 1")
    {
        parse_inner_list_ok_test("(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71", test_inner_list);
    }

    SECTION("ng case 1")
    {
        parse_inner_list_ng_test("(\"foo\";a;b=1936 bar;y=:AQMBAg==:;é);d=18.71", HSFV_ERR_INVALID);
    }
    SECTION("ng case 2")
    {
        parse_inner_list_ng_test("(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71;é", HSFV_ERR_INVALID);
    }
    SECTION("no input")
    {
        parse_inner_list_ng_test("", HSFV_ERR_EOF);
    }
    SECTION("not start open paren")
    {
        parse_inner_list_ng_test("a", HSFV_ERR_INVALID);
    }
    SECTION("unexpected EOF")
    {
        parse_inner_list_ng_test("(a ", HSFV_ERR_EOF);
    }

    SECTION("alloc error")
    {
        parse_inner_list_alloc_error_test("(a b c d e f g h i)");
    }
}
