#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("hsfv_bare_item_eq", "[eq][bare_item]")
{
    SECTION("different bare_item type")
    {
        hsfv_bare_item_t item1 = hsfv_bare_item_t{
            .type = HSFV_BARE_ITEM_TYPE_INTEGER,
        };
        hsfv_bare_item_t item2 = hsfv_bare_item_t{
            .type = HSFV_BARE_ITEM_TYPE_DECIMAL,
        };
        CHECK(!hsfv_bare_item_eq(&item1, &item2));
    }

    SECTION("invalid bare_item type")
    {
        hsfv_bare_item_t bad = hsfv_bare_item_t{
            .type = (hsfv_bare_item_type_t)(-1),
        };
        CHECK(!hsfv_bare_item_eq(&bad, &bad));
    }
}

/* Boolean */

static void serialize_boolean_ok_test(bool input, const char *want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_boolean(input, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

TEST_CASE("serialize boolean", "[serialize][boolean]")
{
    SECTION("false")
    {
        serialize_boolean_ok_test(false, "?0");
    }

    SECTION("true")
    {
        serialize_boolean_ok_test(true, "?1");
    }
}

static void parse_boolean_ok_test(const char *input, bool want)
{
    const char *input_end = input + strlen(input);
    hsfv_bare_item_t item;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_boolean(&item, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_BOOLEAN);
    CHECK(item.boolean == want);
    CHECK(rest == input_end);
}

static void parse_boolean_ng_test(const char *input, hsfv_err_t want)
{
    const char *input_end = input + strlen(input);
    hsfv_bare_item_t item;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_boolean(&item, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse boolean", "[parse][boolean]")
{
    SECTION("false")
    {
        parse_boolean_ok_test("?0", 0);
    }
    SECTION("true")
    {
        parse_boolean_ok_test("?1", 1);
    }

    SECTION("unexpected EOF with no char")
    {
        parse_boolean_ng_test("", HSFV_ERR_EOF);
    }
    SECTION("unexpected EOF after question")
    {
        parse_boolean_ng_test("?", HSFV_ERR_EOF);
    }
    SECTION("not start with question")
    {
        parse_boolean_ng_test("0", HSFV_ERR_INVALID);
    }
    SECTION("invalid value")
    {
        parse_boolean_ng_test("?2", HSFV_ERR_INVALID);
    }
}

/* Integer */

static void serialize_integer_ok_test(int64_t input, const char *want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_integer(input, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_integer_ng_test(int64_t input, hsfv_err_t want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_integer(input, &hsfv_global_allocator, &buf);
    CHECK(err == want);
}

TEST_CASE("serialize integer", "[serialize][integer]")
{
    SECTION("case 1")
    {
        serialize_integer_ok_test(12, "12");
    }
    SECTION("case 2")
    {
        serialize_integer_ok_test(-12, "-12");
    }
    SECTION("min")
    {
        serialize_integer_ok_test(-999999999999999, "-999999999999999");
    }
    SECTION("max")
    {
        serialize_integer_ok_test(999999999999999, "999999999999999");
    }

    SECTION("case 1")
    {
        serialize_integer_ng_test(1000000000000000, HSFV_ERR_INVALID);
    }
    SECTION("case 2")
    {
        serialize_integer_ng_test(-1000000000000000, HSFV_ERR_INVALID);
    }
}

static void parse_integer_ok_test(const char *input, int64_t want)
{
    const char *input_end = input + strlen(input);
    hsfv_bare_item_t item;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_number(&item, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_INTEGER);
    CHECK(item.integer == want);
}

static void parse_integer_ng_test(const char *input, hsfv_err_t want)
{
    const char *input_end = input + strlen(input);
    hsfv_bare_item_t item;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_number(&item, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse integer", "[parse][integer]")
{
    SECTION("positive")
    {
        parse_integer_ok_test("1871", 1871);
    }
    SECTION("negative")
    {
        parse_integer_ok_test("-1871", -1871);
    }
    SECTION("positive followed by non number")
    {
        parse_integer_ok_test("1871next", 1871);
    }
    SECTION("minimum")
    {
        parse_integer_ok_test("-999999999999999", HSFV_MIN_INT);
    }
    SECTION("minimum")
    {
        parse_integer_ok_test("999999999999999", HSFV_MAX_INT);
    }

    SECTION("not digit")
    {
        parse_integer_ng_test("a", HSFV_ERR_INVALID);
    }
    SECTION("no digit after minus sign")
    {
        parse_integer_ng_test("-", HSFV_ERR_EOF);
    }
    SECTION("integer with too many digits")
    {
        parse_integer_ng_test("1234567890123456", HSFV_ERR_NUMBER_OUT_OF_RANGE);
    }
    SECTION("smaller than minimum")
    {
        parse_integer_ng_test("-1000000000000000", HSFV_ERR_NUMBER_OUT_OF_RANGE);
    }
    SECTION("larger than maximum")
    {
        parse_integer_ng_test("1000000000000000", HSFV_ERR_NUMBER_OUT_OF_RANGE);
    }
}

static void serialize_decimal_ok_test(double input, const char *want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_decimal(input, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    if (memcmp(buf.bytes.base, want, buf.bytes.len)) {
        printf("got=%.*s, want=%s\n", (int)buf.bytes.len, buf.bytes.base, want);
    };
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_decimal_ng_test(double input, hsfv_err_t want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_decimal(input, &hsfv_global_allocator, &buf);
    CHECK(err == want);
}

/* Decimal */

TEST_CASE("serialize decimal", "[serialize][decimal]")
{
    SECTION("case 1")
    {
        serialize_decimal_ok_test(12, "12.0");
    }
    SECTION("case 2")
    {
        serialize_decimal_ok_test(12.000, "12.0");
    }
    SECTION("case 3")
    {
        serialize_decimal_ok_test(12.0004, "12.0");
    }
    SECTION("case 4")
    {
        serialize_decimal_ok_test(12.3456, "12.346");
    }
    SECTION("case 5")
    {
        serialize_decimal_ok_test(-12.3456, "-12.346");
    }
    SECTION("case 6")
    {
        serialize_decimal_ok_test(18.71, "18.71");
    }
    SECTION("min")
    {
        serialize_decimal_ok_test(-999999999999.999, "-999999999999.999");
    }
    SECTION("max")
    {
        serialize_decimal_ok_test(999999999999.999, "999999999999.999");
    }

    SECTION("case 1")
    {
        serialize_decimal_ng_test(1000000000000, HSFV_ERR_INVALID);
    }
    SECTION("case 2")
    {
        serialize_decimal_ng_test(-1000000000000, HSFV_ERR_INVALID);
    }
    SECTION("case 3")
    {
        serialize_decimal_ng_test(100000000000000.999, HSFV_ERR_INVALID);
    }
    SECTION("case 4")
    {
        serialize_decimal_ng_test(-10000000000000.999, HSFV_ERR_INVALID);
    }
}

static void parse_decimal_ok_test(const char *input, double want)
{
    const char *input_end = input + strlen(input);
    hsfv_bare_item_t item;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_number(&item, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_DECIMAL);
    CHECK(item.decimal == want);
}

static void parse_decimal_ng_test(const char *input, hsfv_err_t want)
{
    const char *input_end = input + strlen(input);
    hsfv_bare_item_t item;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_number(&item, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse decimal", "[parse][decimal]")
{
    SECTION("positive")
    {
        parse_decimal_ok_test("18.71", 18.71);
    }
    SECTION("negative")
    {
        parse_decimal_ok_test("-18.71", -18.71);
    }
    SECTION("negative followed by non number")
    {
        parse_decimal_ok_test("-18.71next", -18.71);
    }
    SECTION("three frac digits")
    {
        parse_decimal_ok_test("-18.710", -18.71);
    }

    SECTION("not digit")
    {
        parse_decimal_ng_test("a", HSFV_ERR_INVALID);
    }
    SECTION("starts with digit")
    {
        parse_decimal_ng_test(".1", HSFV_ERR_INVALID);
    }
    SECTION("ends with digit")
    {
        parse_decimal_ng_test("1.", HSFV_ERR_INVALID);
    }
    SECTION("more than three fraction digits")
    {
        parse_decimal_ng_test("10.1234", HSFV_ERR_NUMBER_OUT_OF_RANGE);
    }
    SECTION("more than twelve int digits")
    {
        parse_decimal_ng_test("1234567890123.0", HSFV_ERR_NUMBER_OUT_OF_RANGE);
    }
}

static void serialize_string_ok_test(const char *input, const char *want)
{
    hsfv_string_t input_s = (hsfv_string_t){.base = input, .len = strlen(input)};
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_string(&input_s, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_string_ng_test(const char *input, hsfv_err_t want)
{
    hsfv_string_t input_s = (hsfv_string_t){.base = input, .len = strlen(input)};
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_string(&input_s, &hsfv_global_allocator, &buf);
    CHECK(err == want);
}

/* String */

TEST_CASE("serialize string", "[serialize][string]")
{
    SECTION("empty")
    {
        serialize_string_ok_test("", "\"\"");
    }
    SECTION("no escape")
    {
        serialize_string_ok_test("foo", "\"foo\"");
    }
    SECTION("escape")
    {
        serialize_string_ok_test("\\\"", "\"\\\\\\\"\"");
    }

    SECTION("case 1")
    {
        serialize_string_ng_test("\x1f", HSFV_ERR_INVALID);
    }
    SECTION("case 2")
    {
        serialize_string_ng_test("\x7f", HSFV_ERR_INVALID);
    }
}

static void parse_string_ok_test(const char *input, const char *want)
{
    const char *input_end = input + strlen(input);
    hsfv_bare_item_t item;
    hsfv_err_t err;
    const char *rest;
    hsfv_string_t want_s;
    err = hsfv_parse_string(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_STRING);
    want_s.base = want;
    want_s.len = strlen(want);
    CHECK(hsfv_string_eq(&item.string, &want_s));
    hsfv_bare_item_deinit(&item, &hsfv_global_allocator);
}

static void parse_string_ng_test(const char *input, hsfv_err_t want)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    hsfv_bare_item_t item;
    hsfv_err_t err;
    err = hsfv_parse_string(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse string", "[parse][string]")
{
    SECTION("no escape")
    {
        parse_string_ok_test("\"foo\"", "foo");
    }
    SECTION("escape")
    {
        parse_string_ok_test("\"b\\\"a\\\\r\"", "b\"a\\r");
    }
    SECTION("empty")
    {
        parse_string_ok_test("\"\"", "");
    }

    SECTION("empty")
    {
        parse_string_ng_test("", HSFV_ERR_INVALID);
    }
    SECTION("no double quote")
    {
        parse_string_ng_test("a", HSFV_ERR_INVALID);
    }
    SECTION("no characer after escape")
    {
        parse_string_ng_test("\"\\", HSFV_ERR_INVALID);
    }
    SECTION("invalid characer after escape")
    {
        parse_string_ng_test("\"\\o", HSFV_ERR_INVALID);
    }
    SECTION("invalid control characer")
    {
        parse_string_ng_test("\x1f", HSFV_ERR_INVALID);
    }
    SECTION("invalid control characer DEL")
    {
        parse_string_ng_test("\x7f", HSFV_ERR_INVALID);
    }
    SECTION("unclosed string")
    {
        parse_string_ng_test("\"foo", HSFV_ERR_EOF);
    }
}

/* Token */

static void serialize_token_ok_test(const char *input, const char *want)
{
    hsfv_token_t input_t = (hsfv_token_t){.base = input, .len = strlen(input)};
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_token(&input_t, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_token_ng_test(const char *input, hsfv_err_t want)
{
    hsfv_token_t input_t = (hsfv_token_t){.base = input, .len = strlen(input)};
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_token(&input_t, &hsfv_global_allocator, &buf);
    CHECK(err == want);
}

TEST_CASE("serialize token", "[serialize][token]")
{
    SECTION("case 1")
    {
        serialize_token_ok_test("*t!o&k", "*t!o&k");
    }
    SECTION("case 2")
    {
        serialize_token_ok_test("a/b:c", "a/b:c");
    }

    SECTION("case 1")
    {
        serialize_token_ng_test("/", HSFV_ERR_INVALID);
    }
    SECTION("case 2")
    {
        serialize_token_ng_test("a?", HSFV_ERR_INVALID);
    }
}

static void parse_token_ok_test(const char *input, const char *want)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    hsfv_token_t want_t;
    hsfv_bare_item_t item;
    hsfv_err_t err;
    err = hsfv_parse_token(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_TOKEN);
    want_t.base = want;
    want_t.len = strlen(want);
    CHECK(hsfv_token_eq(&item.token, &want_t));
    hsfv_bare_item_deinit(&item, &hsfv_global_allocator);
}

static void parse_token_ng_test(const char *input, hsfv_err_t want)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    hsfv_bare_item_t item;
    hsfv_err_t err;
    err = hsfv_parse_token(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse token", "[parse][token]")
{
    SECTION("single character")
    {
        parse_token_ok_test("t", "t");
    }
    SECTION("multipel characters")
    {
        parse_token_ok_test("tok", "tok");
    }
    SECTION("starts with asterisk")
    {
        parse_token_ok_test("*t!o&k", "*t!o&k");
    }
    SECTION("starts with alpha followed with equal sign")
    {
        parse_token_ok_test("t=", "t");
    }
    SECTION("contains colon and slash")
    {
        parse_token_ok_test("a/b:c", "a/b:c");
    }

    SECTION("empty")
    {
        parse_token_ng_test("", HSFV_ERR_EOF);
    }
    SECTION("non ASCII character")
    {
        parse_token_ng_test("é", HSFV_ERR_INVALID);
    }
}

static void serialize_byte_seq_ok_test(const char *input, const char *want)
{
    hsfv_byte_seq_t input_b = (hsfv_byte_seq_t){.base = (const hsfv_byte_t *)input, .len = strlen(input)};
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_byte_seq(&input_b, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

/* Byte sequnce */

TEST_CASE("serialize byte_seq", "[serialize][byte_seq]")
{
    SECTION("case 1")
    {
        serialize_byte_seq_ok_test("abc", ":YWJj:");
    }
    SECTION("case 2")
    {
        serialize_byte_seq_ok_test("any carnal pleasure", ":YW55IGNhcm5hbCBwbGVhc3VyZQ==:");
    }
    SECTION("case 3")
    {
        serialize_byte_seq_ok_test("any carnal pleasur", ":YW55IGNhcm5hbCBwbGVhc3Vy:");
    }
}

static void parse_byte_seq_ok_test(const char *input, const char *want)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    hsfv_byte_seq_t want_b;
    hsfv_bare_item_t item;
    hsfv_err_t err;
    err = hsfv_parse_byte_seq(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(item.type == HSFV_BARE_ITEM_TYPE_BYTE_SEQ);
    want_b.base = (const hsfv_byte_t *)want;
    want_b.len = strlen(want);
    CHECK(hsfv_byte_seq_eq(&item.byte_seq, &want_b));
    hsfv_bare_item_deinit(&item, &hsfv_global_allocator);
}

static void parse_byte_seq_ng_test(const char *input, hsfv_err_t want)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    hsfv_bare_item_t item;
    hsfv_err_t err;
    err = hsfv_parse_byte_seq(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse byte_seq", "[parse][byte_seq]")
{
    SECTION("case 1")
    {
        parse_byte_seq_ok_test(":YWJj:", "abc");
    }
    SECTION("case 2")
    {
        parse_byte_seq_ok_test(":YW55IGNhcm5hbCBwbGVhc3VyZQ==:", "any carnal pleasure");
    }
    SECTION("case 3")
    {
        parse_byte_seq_ok_test(":YW55IGNhcm5hbCBwbGVhc3Vy:", "any carnal pleasur");
    }
    SECTION("empty")
    {
        parse_byte_seq_ok_test("::", "");
    }

    SECTION("empty")
    {
        parse_byte_seq_ng_test("", HSFV_ERR_EOF);
    }
    SECTION("just opening colon")
    {
        parse_byte_seq_ng_test(":", HSFV_ERR_EOF);
    }
    SECTION("no closing colon case 1")
    {
        parse_byte_seq_ng_test(":YW55IGNhcm5hbCBwbGVhc3Vy", HSFV_ERR_EOF);
    }
    SECTION("not start with colon")
    {
        parse_byte_seq_ng_test("a", HSFV_ERR_INVALID);
    }
    SECTION("bad base64 encode")
    {
        parse_byte_seq_ng_test(":a:", HSFV_ERR_INVALID);
    }
    SECTION("no closing colon case 2")
    {
        parse_byte_seq_ng_test(":YW55IGNhcm5hbCBwbGVhc3Vy~", HSFV_ERR_INVALID);
    }
    SECTION("bad encoded")
    {
        parse_byte_seq_ng_test(":YW55IGNhcm5hbCBwbGVhc3VyZQ!=:", HSFV_ERR_INVALID);
    }
}

/* Key */

static void serialize_key_ok_test(const char *input, const char *want)
{
    hsfv_key_t input_k = (hsfv_key_t){.base = input, .len = strlen(input)};
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_key(&input_k, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_key_ng_test(const char *input, hsfv_err_t want)
{
    hsfv_key_t input_k = (hsfv_key_t){.base = input, .len = strlen(input)};
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_key(&input_k, &hsfv_global_allocator, &buf);
    CHECK(err == want);
}

TEST_CASE("serialize key", "[serialize][key]")
{
    SECTION("case 1")
    {
        serialize_key_ok_test("foo", "foo");
    }
    SECTION("case 2")
    {
        serialize_key_ok_test("*a_-.*", "*a_-.*");
    }

    SECTION("case 1")
    {
        serialize_key_ng_test("A", HSFV_ERR_INVALID);
    }
    SECTION("case 2")
    {
        serialize_key_ng_test("a?", HSFV_ERR_INVALID);
    }
}

static void parse_key_ok_test(const char *input, const char *want)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    hsfv_key_t want_k;
    hsfv_key_t key;
    hsfv_err_t err;
    err = hsfv_parse_key(&key, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    want_k.base = want;
    want_k.len = strlen(want);
    CHECK(hsfv_key_eq(&key, &want_k));
    hsfv_key_deinit(&key, &hsfv_global_allocator);
}

static void parse_key_ng_test(const char *input, hsfv_err_t want)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    hsfv_key_t key;
    hsfv_err_t err;
    err = hsfv_parse_key(&key, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse key", "[parse][key]")
{
    SECTION("single character")
    {
        parse_key_ok_test("t", "t");
    }
    SECTION("multipel characters")
    {
        parse_key_ok_test("tok", "tok");
    }
    SECTION("starts with asterisk")
    {
        parse_key_ok_test("*k-.*", "*k-.*");
    }
    SECTION("starts with alpha followed with equal sign")
    {
        parse_key_ok_test("k=", "k");
    }

    SECTION("empty")
    {
        parse_key_ng_test("", HSFV_ERR_EOF);
    }
    SECTION("non ASCII character")
    {
        parse_key_ng_test("é", HSFV_ERR_INVALID);
    }
}

/* Bare item */

static void serialize_bare_item_ok_test(hsfv_bare_item_t input, const char *want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_bare_item(&input, &hsfv_global_allocator, &buf);
    CHECK(err == HSFV_OK);
    CHECK(buf.bytes.len == strlen(want));
    CHECK(!memcmp(buf.bytes.base, want, buf.bytes.len));
    hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
}

static void serialize_bare_item_ng_test(hsfv_bare_item_t input, hsfv_err_t want)
{
    hsfv_buffer_t buf = (hsfv_buffer_t){0};
    hsfv_err_t err;
    err = hsfv_serialize_bare_item(&input, &hsfv_global_allocator, &buf);
    CHECK(err == want);
}

TEST_CASE("serialize bare_item", "[serialize][bare_item]")
{
    SECTION("boolean")
    {
        serialize_bare_item_ok_test(hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = true}, "?1");
    }
    SECTION("integer")
    {
        serialize_bare_item_ok_test(hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_INTEGER, .integer = 123}, "123");
    }
    SECTION("decimal")
    {
        serialize_bare_item_ok_test(hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_DECIMAL, .decimal = 123.456}, "123.456");
    }
    SECTION("string")
    {
        serialize_bare_item_ok_test(hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "foo", .len = 3}},
                                    "\"foo\"");
    }
    SECTION("token")
    {
        serialize_bare_item_ok_test(hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = "foo", .len = 3}}, "foo");
    }
    SECTION("byte_seq")
    {
        serialize_bare_item_ok_test(
            hsfv_bare_item_t{
                .type = HSFV_BARE_ITEM_TYPE_BYTE_SEQ,
                .byte_seq = hsfv_byte_seq_t{.base = (const hsfv_byte_t *)"abc", .len = 3},
            },
            ":YWJj:");
    }

    SECTION("invalid type")
    {
        serialize_bare_item_ng_test(hsfv_bare_item_t{.type = (hsfv_bare_item_type_t)(-1)}, HSFV_ERR_INVALID);
    }
}

static void parse_bare_item_ok_test(const char *input, hsfv_bare_item_t want)
{
    const char *input_end = input + strlen(input);
    hsfv_bare_item_t item;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_bare_item(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == HSFV_OK);
    CHECK(hsfv_bare_item_eq(&item, &want));
    hsfv_bare_item_deinit(&item, &hsfv_global_allocator);
}

static void parse_bare_item_ng_test(const char *input, hsfv_err_t want)
{
    const char *input_end = input + strlen(input);
    hsfv_bare_item_t item;
    hsfv_err_t err;
    const char *rest;
    err = hsfv_parse_bare_item(&item, &hsfv_global_allocator, input, input_end, &rest);
    CHECK(err == want);
}

TEST_CASE("parse bare_item", "[parse][bare_item]")
{
    SECTION("true")
    {
        parse_bare_item_ok_test("?1", hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = 1});
    }
    SECTION("false")
    {
        parse_bare_item_ok_test("?0", hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = 0});
    }
    SECTION("integer")
    {
        parse_bare_item_ok_test("22", hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_INTEGER, .integer = 22});
    }
    SECTION("decimal")
    {
        parse_bare_item_ok_test("-2.2", hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_DECIMAL, .decimal = -2.2});
    }
    SECTION("string")
    {
        parse_bare_item_ok_test(
            "\"foo\"", hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = "foo", .len = strlen("foo")}});
    }
    SECTION("token case 1")
    {
        parse_bare_item_ok_test(
            "abc", hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = "abc", .len = strlen("abc")}});
    }
    SECTION("token case 2")
    {
        parse_bare_item_ok_test(
            "*abc", hsfv_bare_item_t{.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = "*abc", .len = strlen("*abc")}});
    }
    SECTION("byte_seq")
    {
        parse_bare_item_ok_test(":YWJj:", hsfv_bare_item_t{
                                              .type = HSFV_BARE_ITEM_TYPE_BYTE_SEQ,
                                              .byte_seq = {.base = (const hsfv_byte_t *)"abc", .len = strlen("abc")},
                                          });
    }

    SECTION("empty")
    {
        parse_bare_item_ng_test("", HSFV_ERR_EOF);
    }
    SECTION("invalid symbol")
    {
        parse_bare_item_ng_test("~", HSFV_ERR_INVALID);
    }
}
