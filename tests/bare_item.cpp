#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("booleans can be parsed", "[bare_item][boolean]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    err = hsfv_parse_boolean(&item, input, input_end, &rest);                  \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_BOOLEAN);                           \
    CHECK(item.boolean == want);                                               \
    CHECK(rest == input_end);                                                  \
  }

  OK_HELPER("false", "?0", 0);
  OK_HELPER("true", "?1", 1);
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION("section") {                                                         \
    const char *input_end = input + strlen(input);                             \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    err = hsfv_parse_boolean(&item, input, input_end, &rest);                  \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("unexpected EOF", "?", HSFV_ERR_EOF);
  NG_HELPER("invalid value", "?2", HSFV_ERR_INVALID);
#undef NG_HELPER
}

TEST_CASE("integer can be parsed", "[bare_item][integer]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    err = hsfv_parse_number(&item, input, input_end, &rest);                   \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_INTEGER);                           \
    CHECK(item.integer == want);                                               \
  }

  OK_HELPER("positive", "1871", 1871) OK_HELPER("negative", "-1871", -1871);
  OK_HELPER("positive followed by non number", "1871next", 1871)
  OK_HELPER("minimum", "-999999999999999", HSFV_MIN_INT);
  OK_HELPER("minimum", "999999999999999", HSFV_MAX_INT);
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    err = hsfv_parse_number(&item, input, input_end, &rest);                   \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("not digit", "a", HSFV_ERR_INVALID);
  NG_HELPER("no digit after minus sign", "-", HSFV_ERR_EOF);
  NG_HELPER("integer with too many digits", "1234567890123456",
            HSFV_ERR_NUMBER_OUT_OF_RANGE);
  NG_HELPER("smaller than minimum", "-1000000000000000",
            HSFV_ERR_NUMBER_OUT_OF_RANGE);
  NG_HELPER("larger than maximum", "1000000000000000",
            HSFV_ERR_NUMBER_OUT_OF_RANGE);
#undef NG_HELPER
}

TEST_CASE("decimal can be parsed", "[bare_item][decimal]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    err = hsfv_parse_number(&item, input, input_end, &rest);                   \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_DECIMAL);                           \
    CHECK(item.decimal == want);                                               \
  }

  OK_HELPER("positive", "18.71", 18.71);
  OK_HELPER("negative", "-18.71", -18.71);
  OK_HELPER("negative followed by non number", "-18.71next", -18.71);
  OK_HELPER("three frac digits", "-18.710", -18.71);
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    err = hsfv_parse_number(&item, input, input_end, &rest);                   \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("not digit", "a", HSFV_ERR_INVALID);
  NG_HELPER("starts with digit", ".1", HSFV_ERR_INVALID);
  NG_HELPER("ends with digit", "1.", HSFV_ERR_INVALID);
  NG_HELPER("more than three fraction digits", "10.1234",
            HSFV_ERR_NUMBER_OUT_OF_RANGE);
  NG_HELPER("more than twelve int digits", "1234567890123.0",
            HSFV_ERR_NUMBER_OUT_OF_RANGE);
#undef NG_HELPER
}

TEST_CASE("string can be parsed", "[bare_item][string]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    hsfv_string_t want_s;                                                      \
    err = hsfv_parse_string(&item, &htsv_global_allocator, input, input_end,   \
                            &rest);                                            \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_STRING);                            \
    want_s.base = want;                                                        \
    want_s.len = strlen(want);                                                 \
    CHECK(hsfv_string_eq(&item.string, &want_s));                              \
    hsfv_bare_item_deinit(&item, &htsv_global_allocator);                      \
  }

  OK_HELPER("no escape", "\"foo\"", "foo");
  OK_HELPER("escape", "\"b\\\"a\\\\r\"", "b\"a\\r");
  OK_HELPER("empty", "\"\"", "");
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    const char *rest;                                                          \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    err = hsfv_parse_string(&item, &htsv_global_allocator, input, input_end,   \
                            &rest);                                            \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("empty", "", HSFV_ERR_INVALID);
  NG_HELPER("no double quote", "a", HSFV_ERR_INVALID);
  NG_HELPER("no characer after escape", "\"\\", HSFV_ERR_INVALID);
  NG_HELPER("invalid characer after escape", "\"\\o", HSFV_ERR_INVALID);
  NG_HELPER("invalid control characer", "\x1f", HSFV_ERR_INVALID);
  NG_HELPER("invalid control characer DEL", "\x7f", HSFV_ERR_INVALID);
  NG_HELPER("unclosed string", "\"foo", HSFV_ERR_EOF);
#undef NG_HELPER
}

TEST_CASE("token can be parsed", "[bare_item][token]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    const char *rest;                                                          \
    hsfv_token_t want_t;                                                       \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    err = hsfv_parse_token(&item, &htsv_global_allocator, input, input_end,    \
                           &rest);                                             \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_TOKEN);                             \
    want_t.base = want;                                                        \
    want_t.len = strlen(want);                                                 \
    CHECK(hsfv_token_eq(&item.token, &want_t));                                \
    hsfv_bare_item_deinit(&item, &htsv_global_allocator);                      \
  }

  OK_HELPER("single character", "t", "t");
  OK_HELPER("multipel characters", "tok", "tok");
  OK_HELPER("starts with asterisk", "*t!o&k", "*t!o&k");
  OK_HELPER("starts with alpha followed with equal sign", "t=", "t");
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    const char *rest;                                                          \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    err = hsfv_parse_token(&item, &htsv_global_allocator, input, input_end,    \
                           &rest);                                             \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("empty", "", HSFV_ERR_EOF);
  NG_HELPER("non ASCII character", "é", HSFV_ERR_INVALID);
#undef NG_HELPER
}

TEST_CASE("byte_seq can be parsed", "[bare_item][byte_seq]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    const char *rest;                                                          \
    hsfv_byte_seq_t want_b;                                                    \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    err = hsfv_parse_byte_seq(&item, &htsv_global_allocator, input, input_end, \
                              &rest);                                          \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_BYTE_SEQ);                          \
    want_b.base = want;                                                        \
    want_b.len = strlen(want);                                                 \
    CHECK(hsfv_byte_seq_eq(&item.byte_seq, &want_b));                          \
    hsfv_bare_item_deinit(&item, &htsv_global_allocator);                      \
  }

  OK_HELPER("case 1", ":YWJj:", "abc");
  OK_HELPER("case 2", ":YW55IGNhcm5hbCBwbGVhc3VyZQ==:", "any carnal pleasure");
  OK_HELPER("case 3", ":YW55IGNhcm5hbCBwbGVhc3Vy:", "any carnal pleasur");
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    const char *rest;                                                          \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    err = hsfv_parse_byte_seq(&item, &htsv_global_allocator, input, input_end, \
                              &rest);                                          \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("empty", "", HSFV_ERR_EOF);
  NG_HELPER("just opening colon", ":", HSFV_ERR_EOF);
  NG_HELPER("no closing colon case 1", ":YW55IGNhcm5hbCBwbGVhc3Vy",
            HSFV_ERR_EOF);
  NG_HELPER("no closing colon case 2", ":YW55IGNhcm5hbCBwbGVhc3Vy~",
            HSFV_ERR_INVALID);
  NG_HELPER("bad encoded", ":YW55IGNhcm5hbCBwbGVhc3VyZQ!=:", HSFV_ERR_INVALID);
#undef NG_HELPER
}

TEST_CASE("key can be parsed", "[key]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    const char *rest;                                                          \
    hsfv_key_t want_k;                                                         \
    hsfv_key_t key;                                                            \
    hsfv_err_t err;                                                            \
    err =                                                                      \
        hsfv_parse_key(&key, &htsv_global_allocator, input, input_end, &rest); \
    CHECK(err == HSFV_OK);                                                     \
    want_k.base = want;                                                        \
    want_k.len = strlen(want);                                                 \
    CHECK(hsfv_key_eq(&key, &want_k));                                         \
    hsfv_key_deinit(&key, &htsv_global_allocator);                             \
  }

  OK_HELPER("single character", "t", "t");
  OK_HELPER("multipel characters", "tok", "tok");
  OK_HELPER("starts with asterisk", "*k-.*", "*k-.*");
  OK_HELPER("starts with alpha followed with equal sign", "k=", "k");
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    const char *rest;                                                          \
    hsfv_key_t key;                                                            \
    hsfv_err_t err;                                                            \
    err =                                                                      \
        hsfv_parse_key(&key, &htsv_global_allocator, input, input_end, &rest); \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("empty", "", HSFV_ERR_EOF);
  NG_HELPER("non ASCII character", "é", HSFV_ERR_INVALID);
#undef NG_HELPER
}

TEST_CASE("bare item can be parsed", "[bare_item]") {
#define OK_HELPER(section, input, want_literal)                                \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    hsfv_bare_item_t want = want_literal;                                      \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    err = hsfv_parse_bare_item(&item, &htsv_global_allocator, input,           \
                               input_end, &rest);                              \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(hsfv_bare_item_eq(&item, &want));                                    \
    hsfv_bare_item_deinit(&item, &htsv_global_allocator);                      \
  }

  OK_HELPER(
      "true", "?1",
      (hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = 1}));
  OK_HELPER(
      "false", "?0",
      (hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = 0}));
  OK_HELPER(
      "integer", "22",
      (hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_INTEGER, .integer = 22}));
  OK_HELPER(
      "decimal", "-2.2",
      (hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_DECIMAL, .decimal = -2.2}));
  OK_HELPER("string", "\"foo\"",
            (hsfv_bare_item_t{
                .type = HSFV_BARE_ITEM_TYPE_STRING,
                .string = hsfv_string_t{.base = "foo", .len = strlen("foo")}}));
  OK_HELPER("token case 1", "abc",
            (hsfv_bare_item_t{
                .type = HSFV_BARE_ITEM_TYPE_TOKEN,
                .token = hsfv_token_t{.base = "abc", .len = strlen("abc")}}));
  OK_HELPER("token case 2", "*abc",
            (hsfv_bare_item_t{
                .type = HSFV_BARE_ITEM_TYPE_TOKEN,
                .token = hsfv_token_t{.base = "*abc", .len = strlen("*abc")}}));
  OK_HELPER("byte_seq", ":YWJj:",
            (hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BYTE_SEQ,
                              .byte_seq = hsfv_byte_seq_t{
                                  .base = "abc", .len = strlen("abc")}}));
#undef OK_HELPER

#define NG_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    const char *input_end = input + strlen(input);                             \
    hsfv_bare_item_t item;                                                     \
    hsfv_err_t err;                                                            \
    const char *rest;                                                          \
    err = hsfv_parse_bare_item(&item, &htsv_global_allocator, input,           \
                               input_end, &rest);                              \
    CHECK(err == want);                                                        \
  }

  NG_HELPER("empty", "", HSFV_ERR_EOF);
  NG_HELPER("invalid symbol", "~", HSFV_ERR_INVALID);
#undef NG_HELPER
}

TEST_CASE("serialize boolean", "[serialize][boolean]") {
#define OK_HELPER(section, input, want)                                        \
  SECTION(section) {                                                           \
    hsfv_buffer_t buf = (hsfv_buffer_t){0};                                    \
    hsfv_err_t err;                                                            \
    err = htsv_serialize_boolean(&buf, &htsv_global_allocator, input);         \
    CHECK(err == HSFV_OK);                                                     \
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));                       \
    htsv_buffer_deinit(&buf, &htsv_global_allocator);                          \
  }

  OK_HELPER("false", false, "?0");
  OK_HELPER("true", true, "?1");
#undef OK_HELPER
}