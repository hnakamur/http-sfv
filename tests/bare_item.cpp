#include <catch2/catch_test_macros.hpp>
#include "hsfv.h"

TEST_CASE("booleans can be parsed", "[bare_item][boolean]") {
    #define OK_HELPER(section, input, want)                      \
        SECTION("section") {                                     \
            const char *buf;                                     \
            const char *buf_end = input + strlen(input);         \
            int boolean, ret;                                    \
            buf = parse_boolean(input, buf_end, &boolean, &ret); \
            CHECK(boolean == want);                              \
            CHECK(ret == HSFV_OK);                               \
            CHECK(buf == buf_end);                               \
        }

        OK_HELPER("false", "?0", 0)
        OK_HELPER("true", "?1", 1)
    #undef OK_HELPER

    #define NG_HELPER(section, input, want)                      \
        SECTION("section") {                                     \
            const char *buf;                                     \
            const char *buf_end = input + strlen(input);         \
            int boolean, ret;                                    \
            buf = parse_boolean(input, buf_end, &boolean, &ret); \
            CHECK(ret == want);                                  \
        }

        NG_HELPER("unexpected EOF", "?", HSFV_ERR_EOF)
        NG_HELPER("invalid value", "?2", HSFV_ERR_INVALID)
    #undef NG_HELPER
}

TEST_CASE("integer can be parsed", "[bare_item][integer]") {
    #define OK_HELPER(section, input, want)                  \
        SECTION(section) {                                   \
            const char *buf;                                 \
            const char *buf_end = input + strlen(input);     \
            hsfv_bare_item_t item;                           \
            int ret;                                         \
            buf = parse_number(input, buf_end, &item, &ret); \
            CHECK(ret == HSFV_OK);                           \
            CHECK(item.type == HSFV_BARE_ITEM_TYPE_INTEGER); \
            CHECK(item.data.integer == want);                \
        }

        OK_HELPER("positive", "1871", 1871)
        OK_HELPER("negative", "-1871", -1871)
        OK_HELPER("positive followed by non number", "1871next", 1871)
        OK_HELPER("minimum", "-999999999999999", HSFV_MIN_INT)
        OK_HELPER("minimum", "999999999999999", HSFV_MAX_INT)
    #undef OK_HELPER

    #define NG_HELPER(section, input, want)                  \
        SECTION(section) {                                   \
            const char *buf;                                 \
            const char *buf_end = input + strlen(input);     \
            hsfv_bare_item_t item;                           \
            int ret;                                         \
            buf = parse_number(input, buf_end, &item, &ret); \
            CHECK(ret == want);                              \
        }

        NG_HELPER("not digit", "a", HSFV_ERR_INVALID)
        NG_HELPER("no digit after minus sign", "-", HSFV_ERR_EOF)
        NG_HELPER("integer with too many digits", "1234567890123456", HSFV_ERR_NUMBER_OUT_OF_RANGE)
        NG_HELPER("smaller than minimum", "-1000000000000000", HSFV_ERR_NUMBER_OUT_OF_RANGE)
        NG_HELPER("larger than maximum", "1000000000000000", HSFV_ERR_NUMBER_OUT_OF_RANGE)
    #undef NG_HELPER
}

TEST_CASE("decimal can be parsed", "[bare_item][decimal]") {
    #define OK_HELPER(section, input, want)                  \
        SECTION(section) {                                   \
            const char *buf;                                 \
            const char *buf_end = input + strlen(input);     \
            hsfv_bare_item_t item;                           \
            int ret;                                         \
            buf = parse_number(input, buf_end, &item, &ret); \
            CHECK(ret == HSFV_OK);                           \
            CHECK(item.type == HSFV_BARE_ITEM_TYPE_DECIMAL); \
            CHECK(item.data.decimal == want);                \
        }

        OK_HELPER("positive", "18.71", 18.71)
        OK_HELPER("negative", "-18.71", -18.71)
        OK_HELPER("negative followed by non number", "-18.71next", -18.71)
        OK_HELPER("three frac digits", "-18.710", -18.71)
    #undef OK_HELPER

    #define NG_HELPER(section, input, want)                  \
        SECTION(section) {                                   \
            const char *buf;                                 \
            const char *buf_end = input + strlen(input);     \
            hsfv_bare_item_t item;                           \
            int ret;                                         \
            buf = parse_number(input, buf_end, &item, &ret); \
            CHECK(ret == want);                              \
        }

        NG_HELPER("not digit", "a", HSFV_ERR_INVALID)
        NG_HELPER("starts with digit", ".1", HSFV_ERR_INVALID)
        NG_HELPER("ends with digit", "1.", HSFV_ERR_INVALID)
        NG_HELPER("more than three fraction digits", "10.1234", HSFV_ERR_NUMBER_OUT_OF_RANGE)
        NG_HELPER("more than twelve int digits", "1234567890123.0", HSFV_ERR_NUMBER_OUT_OF_RANGE)
    #undef NG_HELPER
}

TEST_CASE("string can be parsed", "[bare_item][string]") {
    #define OK_HELPER(section, input, want)                                                 \
        SECTION(section) {                                                                  \
            const char *rest;                                                               \
            hsfv_string_t s, want_s;                                                        \
            hsfv_bare_item_t item;                                                          \
            hsfv_err_t err;                                                                 \
            s.base = input;                                                                 \
            s.len = strlen(input);                                                          \
            err = parse_string(&htsv_global_allocator, input, input + s.len, &item, &rest); \
            CHECK(err == HSFV_OK);                                                          \
            CHECK(item.type == HSFV_BARE_ITEM_TYPE_STRING);                                 \
            want_s.base = want;                                                             \
            want_s.len = strlen(want);                                                      \
            CHECK(hsfv_string_eq(item.data.string, want_s));                                \
            hsfv_bare_item_deinit(&htsv_global_allocator, &item);                           \
        }

        OK_HELPER("no escape", "\"foo\"", "foo")
        OK_HELPER("escape", "\"b\\\"a\\\\r\"", "b\"a\\r")
        OK_HELPER("empty", "\"\"", "")
    #undef OK_HELPER

    #define NG_HELPER(section, input, want)                                                 \
        SECTION(section) {                                                                  \
            const char *rest;                                                               \
            hsfv_string_t s;                                                                \
            hsfv_bare_item_t item;                                                          \
            hsfv_err_t err;                                                                 \
            s.base = input;                                                                 \
            s.len = strlen(input);                                                          \
            err = parse_string(&htsv_global_allocator, input, input + s.len, &item, &rest); \
            CHECK(err == want);                                                             \
        }

        NG_HELPER("empty", "", HSFV_ERR_INVALID)
        NG_HELPER("no double quote", "a", HSFV_ERR_INVALID)
        NG_HELPER("no characer after escape", "\"\\", HSFV_ERR_INVALID)
        NG_HELPER("invalid characer after escape", "\"\\o", HSFV_ERR_INVALID)
        NG_HELPER("invalid control characer", "\x1f", HSFV_ERR_INVALID)
        NG_HELPER("invalid control characer DEL", "\x7f", HSFV_ERR_INVALID)
        NG_HELPER("unclosed string", "\"foo", HSFV_ERR_EOF)
    #undef NG_HELPER
}
