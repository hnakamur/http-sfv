#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

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

static hsfv_parameter_t item_params[2] = {
    {
        .key = {.base = "foo", .len = 3},
        .value = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true},
    },
    {
        .key = {.base = "*bar", .len = 4},
        .value = {.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = "tok", .len = 3}},
    },
};

static hsfv_list_member_t members[] = {
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

static hsfv_list_t test_list = {
    .members = members,
    .len = 2,
    .capacity = 2,
};

TEST_CASE("hsfv_list_eq", "[field_value]")
{
    SECTION("different length")
    {
        hsfv_list_t empty = hsfv_list_t{0};
        CHECK(!hsfv_list_eq(&test_list, &empty));
    }

    SECTION("different member type")
    {
        hsfv_list_member_t members2[] = {
            {
                .type = HSFV_LIST_MEMBER_TYPE_ITEM,
                .item =
                    {
                        .bare_item =
                            {
                                .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = true,
                            },
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
        hsfv_list_t test_list2 = {
            .members = members2,
            .len = 2,
            .capacity = 2,
        };

        CHECK(!hsfv_list_eq(&test_list, &test_list2));
    }

    SECTION("invalid member type")
    {
        hsfv_list_member_t bad_members[] = {
            {
                .type = (hsfv_list_member_type_t)(-1),
            },
        };
        hsfv_list_t bad_list = {
            .members = bad_members,
            .len = 1,
            .capacity = 1,
        };

        CHECK(!hsfv_list_eq(&bad_list, &bad_list));
    }
}

static void serialize_list_ok_test(hsfv_list_t input, const char *want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_list(&input, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_list_alloc_error_test(hsfv_list_t input)
{
    hsfv_allocator_t *allocator = &hsfv_failing_allocator.allocator;
    hsfv_failing_allocator.fail_index = -1;
    hsfv_failing_allocator.alloc_count = 0;

    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_list(&input, allocator, &buf);
    CHECK(err == HSFV_OK);
    hsfv_buffer_deinit(&buf, allocator);

    int alloc_count = hsfv_failing_allocator.alloc_count;
    for (int i = 0; i < alloc_count; i++) {
        hsfv_failing_allocator.fail_index = i;
        hsfv_failing_allocator.alloc_count = 0;
        buf = (hsfv_buffer_t){0};
        err = hsfv_serialize_list(&input, allocator, &buf);
        CHECK(err == HSFV_ERR_OUT_OF_MEMORY);
        hsfv_buffer_deinit(&buf, allocator);
    }
}

TEST_CASE("serialize list", "[serialze][list]")
{
    SECTION("case 1")
    {
        serialize_list_ok_test(test_list, "(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok");
    }

    SECTION("alloc error 1")
    {
        hsfv_list_member_t members[] = {
            {
                .type = HSFV_LIST_MEMBER_TYPE_ITEM,
                .item =
                    {
                        .bare_item =
                            {
                                .type = HSFV_BARE_ITEM_TYPE_TOKEN,
                                .token = {.base = "abcdefg", .len = 7},
                            },
                    },
            },
            {
                .type = HSFV_LIST_MEMBER_TYPE_ITEM,
                .item =
                    {
                        .bare_item =
                            {
                                .type = HSFV_BARE_ITEM_TYPE_BOOLEAN,
                                .boolean = false,
                            },
                    },
            },
        };
        serialize_list_alloc_error_test(hsfv_list_t{
            .members = members,
            .len = 2,
            .capacity = 2,
        });
    }
}

static void parse_list_ok_test(const char *input, hsfv_list_t want)
{
    hsfv_list_t list;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_list(&list, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(hsfv_list_eq(&list, &want));
    CHECK(rest == input_end);
    hsfv_list_deinit(&list, &hsfv_global_allocator);
}

static void parse_list_ng_test(const char *input, hsfv_err_t want)
{
    hsfv_list_t innser_list;
    hsfv_err_t err;
    const char *rest;
    const char *input_end = input + strlen(input);
    err = hsfv_parse_list(&innser_list, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse list", "[parse][list]")
{
    SECTION("ok case 1")
    {
        parse_list_ok_test("(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok", test_list);
    }

    SECTION("ng case 1")
    {
        parse_list_ng_test("(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok, ", HSFV_ERR_EOF);
    }
    SECTION("ng case 2")
    {
        parse_list_ng_test("(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71, ?1;foo;*bar=tok, é", HSFV_ERR_INVALID);
    }
}
