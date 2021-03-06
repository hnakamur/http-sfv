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

TEST_CASE("hsfv_dicreionary_eq", "[eq][dicreionary]")
{
    SECTION("different length")
    {
        hsfv_dictionary_t empty = hsfv_dictionary_t{0};

        CHECK(!hsfv_dictionary_eq(&empty, &test_dict));
    }

    SECTION("different member type")
    {
        hsfv_item_t items2[] = {
            {
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "foo", .len = 3}},
            },
        };

        hsfv_dict_member_t members2[] = {
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
                        .type = HSFV_DICT_MEMBER_TYPE_INNER_LIST,
                        .inner_list =
                            {
                                .items = items2,
                                .len = 1,
                                .capacity = 1,
                            },
                    },
            },
        };
        static hsfv_dictionary_t test_dict2 = {
            .members = members2,
            .len = 3,
            .capacity = 3,
        };
        CHECK(!hsfv_dictionary_eq(&test_dict, &test_dict2));
    }

    SECTION("invalid member type")
    {
        hsfv_dict_member_t bad_members[] = {
            {
                .key = {.base = "a", .len = 1},
                .value =
                    {
                        .type = (hsfv_dict_member_type_t)(-1),
                    },
            },
        };
        static hsfv_dictionary_t bad_dict = {
            .members = bad_members,
            .len = 3,
            .capacity = 3,
        };
        CHECK(!hsfv_dictionary_eq(&bad_dict, &bad_dict));
    }
}

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

static void serialize_dictionary_alloc_error_test(hsfv_dictionary_t input)
{
    hsfv_allocator_t *allocator = &hsfv_failing_allocator.allocator;
    hsfv_failing_allocator.fail_index = -1;
    hsfv_failing_allocator.alloc_count = 0;

    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_dictionary(&input, allocator, &buf);
    CHECK(err == HSFV_OK);
    hsfv_buffer_deinit(&buf, allocator);

    int alloc_count = hsfv_failing_allocator.alloc_count;
    for (int i = 0; i < alloc_count; i++) {
        hsfv_failing_allocator.fail_index = i;
        hsfv_failing_allocator.alloc_count = 0;
        buf = (hsfv_buffer_t){0};
        err = hsfv_serialize_dictionary(&input, allocator, &buf);
        CHECK(err == HSFV_ERR_OUT_OF_MEMORY);
        hsfv_buffer_deinit(&buf, allocator);
    }
}

TEST_CASE("serialize dictionary", "[serialze][dictionary]")
{
    SECTION("case 1")
    {
        serialize_dictionary_ok_test(test_dict, "a=?0, b, c;foo=bar");
    }

    SECTION("alloc error 1")
    {
        hsfv_item_t items[] = {
            {
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "foofoofoo", .len = 9}},
            },
        };

        hsfv_dict_member_t members[] = {
            {
                .key = {.base = "abcdefg", .len = 7},
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
                .key = {.base = "bcdefghi", .len = 8},
                .value =
                    {
                        .type = HSFV_DICT_MEMBER_TYPE_INNER_LIST,
                        .inner_list =
                            {
                                .items = items,
                                .len = 1,
                                .capacity = 1,
                            },
                    },
            },
        };
        serialize_dictionary_alloc_error_test(hsfv_dictionary_t{
            .members = members,
            .len = 2,
            .capacity = 2,
        });
    }

    SECTION("alloc error 2")
    {
        hsfv_parameter_t param = {
            .key = {.base = "foo", .len = 3},
            .value =
                {
                    .type = HSFV_BARE_ITEM_TYPE_TOKEN,
                    .token = {.base = "barbarbar", .len = 9},
                },
        };

        hsfv_dict_member_t members[] = {
            {
                .key = {.base = "abcdefgi", .len = 8},
                .value =
                    {
                        .type = HSFV_DICT_MEMBER_TYPE_ITEM,
                        .item =
                            {
                                .bare_item =
                                    {
                                        .type = HSFV_BARE_ITEM_TYPE_INTEGER,
                                        .integer = 123456789,
                                    },
                            },
                    },
            },
            {
                .key = {.base = "bcdefghi", .len = 8},
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
                                        .params = &param,
                                        .len = 1,
                                        .capacity = 1,
                                    },
                            },
                    },
            },
        };
        serialize_dictionary_alloc_error_test(hsfv_dictionary_t{
            .members = members,
            .len = 2,
            .capacity = 2,
        });
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

static void parse_dictionary_alloc_error_test(const char *input)
{
    hsfv_allocator_t *allocator = &hsfv_failing_allocator.allocator;
    hsfv_failing_allocator.fail_index = -1;
    hsfv_failing_allocator.alloc_count = 0;

    const char *input_end = input + strlen(input);
    hsfv_dictionary_t dictionary;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_dictionary(&dictionary, allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    hsfv_dictionary_deinit(&dictionary, allocator);

    int alloc_count = hsfv_failing_allocator.alloc_count;
    for (int i = 0; i < alloc_count; i++) {
        hsfv_failing_allocator.fail_index = i;
        hsfv_failing_allocator.alloc_count = 0;
        err = hsfv_parse_dictionary(&dictionary, allocator, input, input_end, &rest);
        CHECK(err == HSFV_ERR_OUT_OF_MEMORY);
    }
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
        parse_dictionary_ng_test("a=?0, b, c; foo=bar, ??", HSFV_ERR_INVALID);
    }

    SECTION("alloc error 1")
    {
        parse_dictionary_alloc_error_test("a=?0, b, c, d, e, f, g, h");
    }

    SECTION("alloc error 2")
    {
        parse_dictionary_alloc_error_test("a=(a b c d e f g h i), b;a=1;b=2;c=3;d=4;e=5;f=6;g=7;h=8;i=9");
    }
}
