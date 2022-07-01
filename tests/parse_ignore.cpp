#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

static void test_parse_ignore_boolean_ok(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_boolean(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end);
}

static void test_parse_ignore_boolean_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_boolean(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_boolean", "[parse_ignore][boolean]")
{
    SECTION("false")
    {
        test_parse_ignore_boolean_ok("?0");
    }
    SECTION("true")
    {
        test_parse_ignore_boolean_ok("?1");
    }

    SECTION("empty")
    {
        test_parse_ignore_boolean_ng("");
    }
    SECTION("just question")
    {
        test_parse_ignore_boolean_ng("?");
    }
    SECTION("not starts with question")
    {
        test_parse_ignore_boolean_ng("1");
    }
    SECTION("bad char after question")
    {
        test_parse_ignore_boolean_ng("?2");
    }
}

static void test_parse_ignore_number_ok(const char *input, size_t want_rest_len)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_number(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end - want_rest_len);
}

static void test_parse_ignore_number_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_number(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_number", "[parse_ignore][number]")
{
    SECTION("positive integer")
    {
        test_parse_ignore_number_ok("1871", 0);
    }
    SECTION("negative integer")
    {
        test_parse_ignore_number_ok("-1871", 0);
    }
    SECTION("positive integer followed by non number")
    {
        test_parse_ignore_number_ok("1871next", 4);
    }
    SECTION("minimum integer")
    {
        test_parse_ignore_number_ok("-999999999999999", 0);
    }
    SECTION("minimum integer")
    {
        test_parse_ignore_number_ok("999999999999999", 0);
    }
    SECTION("positive decimal")
    {
        test_parse_ignore_number_ok("18.71", 0);
    }
    SECTION("negative decimal")
    {
        test_parse_ignore_number_ok("-18.71", 0);
    }
    SECTION("negative decimal followed by non number")
    {
        test_parse_ignore_number_ok("-18.71next", 4);
    }
    SECTION("three frac digits, last zero")
    {
        test_parse_ignore_number_ok("-18.710", 0);
    }
    SECTION("three frac digits, last non-zero")
    {
        test_parse_ignore_number_ok("-18.712", 0);
    }

    SECTION("empty")
    {
        test_parse_ignore_number_ng("");
    }
    SECTION("not digit")
    {
        test_parse_ignore_number_ng("a");
    }
    SECTION("eof after minus sign")
    {
        test_parse_ignore_number_ng("-");
    }
    SECTION("not digit after minus sign")
    {
        test_parse_ignore_number_ng("-a");
    }
    SECTION("integer with too many digits")
    {
        test_parse_ignore_number_ng("1234567890123456");
    }
    SECTION("smaller than minimum")
    {
        test_parse_ignore_number_ng("-1000000000000000");
    }
    SECTION("larger than maximum")
    {
        test_parse_ignore_number_ng("1000000000000000");
    }
    SECTION("multiple dots case 1")
    {
        test_parse_ignore_number_ng("1..0");
    }
    SECTION("multiple dots case 2")
    {
        test_parse_ignore_number_ng("1.2.0");
    }
    SECTION("starts with digit")
    {
        test_parse_ignore_number_ng(".1");
    }
    SECTION("ends with digit")
    {
        test_parse_ignore_number_ng("1.");
    }
    SECTION("more than three fraction digits")
    {
        test_parse_ignore_number_ng("10.1234");
    }
    SECTION("decimal with more than twelve int digits")
    {
        test_parse_ignore_number_ng("1234567890123.0");
    }
}

static void test_parse_ignore_string_ok(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_string(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end);
}

static void test_parse_ignore_string_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_string(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_string", "[parse_ignore][string]")
{
    SECTION("no escape")
    {
        test_parse_ignore_string_ok("\"foo\"");
    }
    SECTION("escape")
    {
        test_parse_ignore_string_ok("\"b\\\"a\\\\r\"");
    }
    SECTION("empty")
    {
        test_parse_ignore_string_ok("\"\"");
    }

    SECTION("empty")
    {
        test_parse_ignore_string_ng("");
    }
    SECTION("no double quote")
    {
        test_parse_ignore_string_ng("a");
    }
    SECTION("no characer after escape")
    {
        test_parse_ignore_string_ng("\"\\");
    }
    SECTION("invalid characer after escape")
    {
        test_parse_ignore_string_ng("\"\\o");
    }
    SECTION("invalid control characer")
    {
        test_parse_ignore_string_ng("\x1f");
    }
    SECTION("invalid control characer DEL")
    {
        test_parse_ignore_string_ng("\x7f");
    }
    SECTION("unclosed string")
    {
        test_parse_ignore_string_ng("\"foo");
    }
}

static void test_parse_ignore_token_ok(const char *input, size_t want_rest_len)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_token(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end - want_rest_len);
}

static void test_parse_ignore_token_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_token(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_token", "[parse_ignore][token]")
{
    SECTION("single character")
    {
        test_parse_ignore_token_ok("t", 0);
    }
    SECTION("multipel characters")
    {
        test_parse_ignore_token_ok("tok", 0);
    }
    SECTION("starts with asterisk")
    {
        test_parse_ignore_token_ok("*t!o&k", 0);
    }
    SECTION("starts with alpha followed with equal sign")
    {
        test_parse_ignore_token_ok("t=", 1);
    }
    SECTION("contains colon and slash")
    {
        test_parse_ignore_token_ok("a/b:c", 0);
    }

    SECTION("empty")
    {
        test_parse_ignore_token_ng("");
    }
    SECTION("non ASCII character")
    {
        test_parse_ignore_token_ng("é");
    }
}

static void test_parse_ignore_key_ok(const char *input, size_t want_rest_len)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_key(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end - want_rest_len);
}

static void test_parse_ignore_key_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_key(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_key", "[parse_ignore][key]")
{
    SECTION("single character")
    {
        test_parse_ignore_key_ok("t", 0);
    }
    SECTION("multipel characters")
    {
        test_parse_ignore_key_ok("tok", 0);
    }
    SECTION("starts with asterisk")
    {
        test_parse_ignore_key_ok("*k-.*", 0);
    }
    SECTION("starts with alpha followed with equal sign")
    {
        test_parse_ignore_key_ok("k=", 1);
    }

    SECTION("empty")
    {
        test_parse_ignore_key_ng("");
    }
    SECTION("non ASCII character")
    {
        test_parse_ignore_key_ng("é");
    }
}

static void test_parse_ignore_byte_seq_ok(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_byte_seq(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end);
}

static void test_parse_ignore_byte_seq_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_byte_seq(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_byte_seq", "[parse_ignore][byte_seq]")
{
    SECTION("case 1")
    {
        test_parse_ignore_byte_seq_ok(":YWJj:");
    }
    SECTION("case 2")
    {
        test_parse_ignore_byte_seq_ok(":YW55IGNhcm5hbCBwbGVhc3VyZQ==:");
    }
    SECTION("case 3")
    {
        test_parse_ignore_byte_seq_ok(":YW55IGNhcm5hbCBwbGVhc3Vy:");
    }
    SECTION("empty")
    {
        test_parse_ignore_byte_seq_ok("::");
    }

    SECTION("empty")
    {
        test_parse_ignore_byte_seq_ng("");
    }
    SECTION("just opening colon")
    {
        test_parse_ignore_byte_seq_ng(":");
    }
    SECTION("no closing colon case 1")
    {
        test_parse_ignore_byte_seq_ng(":YW55IGNhcm5hbCBwbGVhc3Vy");
    }
    SECTION("not start with colon")
    {
        test_parse_ignore_byte_seq_ng("a");
    }
    SECTION("bad base64 encode")
    {
        test_parse_ignore_byte_seq_ng(":a:");
    }
    SECTION("no closing colon case 2")
    {
        test_parse_ignore_byte_seq_ng(":YW55IGNhcm5hbCBwbGVhc3Vy~");
    }
    SECTION("bad encoded")
    {
        test_parse_ignore_byte_seq_ng(":YW55IGNhcm5hbCBwbGVhc3VyZQ!=:");
    }
}

static void test_parse_ignore_bare_item_ok(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_bare_item(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end);
}

static void test_parse_ignore_bare_item_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_bare_item(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_bare_item", "[parse_ignore][bare_item]")
{
    SECTION("true")
    {
        test_parse_ignore_bare_item_ok("?1");
    }
    SECTION("false")
    {
        test_parse_ignore_bare_item_ok("?0");
    }
    SECTION("integer")
    {
        test_parse_ignore_bare_item_ok("22");
    }
    SECTION("decimal")
    {
        test_parse_ignore_bare_item_ok("-2.2");
    }
    SECTION("string")
    {
        test_parse_ignore_bare_item_ok("\"foo\"");
    }
    SECTION("token case 1")
    {
        test_parse_ignore_bare_item_ok("abc");
    }
    SECTION("token case 2")
    {
        test_parse_ignore_bare_item_ok("*abc");
    }
    SECTION("byte_seq")
    {
        test_parse_ignore_bare_item_ok(":YWJj:");
    }

    SECTION("empty")
    {
        test_parse_ignore_bare_item_ng("");
    }
    SECTION("invalid symbol")
    {
        test_parse_ignore_bare_item_ng("~");
    }
}

static void test_parse_ignore_parameters_ok(const char *input, size_t want_rest_len)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_parameters(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end - want_rest_len);
}

static void test_parse_ignore_parameters_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_parameters(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_parameters", "[parse_ignore][parameters]")
{
    SECTION("case 1")
    {
        test_parse_ignore_parameters_ok(";foo=?1;*bar=\"baz\" foo", 4);
    }
    SECTION("case 2")
    {
        test_parse_ignore_parameters_ok(";foo;*bar=\"baz\" foo", 4);
    }
    SECTION("case 3")
    {
        test_parse_ignore_parameters_ok(";foo=?1;*bar=tok;*bar=\"baz\" foo", 4);
    }

    SECTION("invalid key")
    {
        test_parse_ignore_parameters_ng(";é=?0");
    }
    SECTION("invalid value")
    {
        test_parse_ignore_parameters_ng(";foo=é");
    }
}

static void test_parse_ignore_item_ok(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_item(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end);
}

static void test_parse_ignore_item_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_item(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_item", "[parse_ignore][item]")
{
    SECTION("case 1")
    {
        test_parse_ignore_item_ok("?1;foo;*bar=tok");
    }

    SECTION("invalid bare item")
    {
        test_parse_ignore_item_ng("é");
    }
    SECTION("invalid parameter")
    {
        test_parse_ignore_item_ng("tok;é");
    }
}

static void test_parse_ignore_inner_list_ok(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_inner_list(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end);
}

static void test_parse_ignore_inner_list_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_inner_list(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_inner_list", "[parse_ignore][inner_list]")
{
    SECTION("ok case 1")
    {
        test_parse_ignore_inner_list_ok("(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71");
    }

    SECTION("ng case 1")
    {
        test_parse_ignore_inner_list_ng("(\"foo\";a;b=1936 bar;y=:AQMBAg==:;é);d=18.71");
    }
    SECTION("ng case 2")
    {
        test_parse_ignore_inner_list_ng("(\"foo\";a;b=1936 bar;y=:AQMBAg==:);d=18.71;é");
    }
    SECTION("no input")
    {
        test_parse_ignore_inner_list_ng("");
    }
    SECTION("not start open paren")
    {
        test_parse_ignore_inner_list_ng("a");
    }
    SECTION("unexpected EOF")
    {
        test_parse_ignore_inner_list_ng("(a ");
    }
}

static void test_parse_ignore_dictionary_member_value_ok(const char *input, size_t want_rest_len)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_dictionary_member_value(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end - want_rest_len);
}

static void test_parse_ignore_dictionary_member_value_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_parse_ignore_dictionary_member_value(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("parse_ignore_dictionary_member_value", "[parse_ignore][dictionary_member_value]")
{
    SECTION("ok case 1")
    {
        test_parse_ignore_dictionary_member_value_ok(",", 1);
    }
    SECTION("boolean false")
    {
        test_parse_ignore_dictionary_member_value_ok("=?0", 0);
    }
    SECTION("boolean true")
    {
        test_parse_ignore_dictionary_member_value_ok("=?1", 0);
    }
    SECTION("empty value with parameters")
    {
        test_parse_ignore_dictionary_member_value_ok(";foo=bar", 0);
    }
    SECTION("token with parameters")
    {
        test_parse_ignore_dictionary_member_value_ok("=token;foo=bar", 0);
    }
    SECTION("inner list with parameters")
    {
        test_parse_ignore_dictionary_member_value_ok("=(a b);foo=bar", 0);
    }

    SECTION("ends with equal")
    {
        test_parse_ignore_dictionary_member_value_ng("=");
    }

    SECTION("comma after equal")
    {
        test_parse_ignore_dictionary_member_value_ng("=,");
    }
}

static void test_skip_ows_comma_ows_ok(const char *input, size_t want_rest_len)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_skip_ows_comma_ows(input, input_end, &rest);
    CHECK(got);
    CHECK(rest == input_end - want_rest_len);
}

static void test_skip_ows_comma_ows_ng(const char *input)
{
    const char *input_end = input + strlen(input);
    const char *rest;
    bool got = hsfv_skip_ows_comma_ows(input, input_end, &rest);
    CHECK(!got);
}

TEST_CASE("skip_ows_comma_ows", "[skip][ows_comma_ows]")
{
    SECTION("empty")
    {
        test_skip_ows_comma_ows_ok("", 0);
    }
    SECTION("just comma")
    {
        test_skip_ows_comma_ows_ok(",a", 1);
    }
    SECTION("ows after comma")
    {
        test_skip_ows_comma_ows_ok(", a", 1);
    }
    SECTION("two ows after comma")
    {
        test_skip_ows_comma_ows_ok(", \ta", 1);
    }
    SECTION("ows before comma")
    {
        test_skip_ows_comma_ows_ok(" ,a", 1);
    }
    SECTION("two ows before comma")
    {
        test_skip_ows_comma_ows_ok(" \t,a", 1);
    }
    SECTION("ows around comma")
    {
        test_skip_ows_comma_ows_ok(" , a", 1);
    }
    SECTION("two ows around comma")
    {
        test_skip_ows_comma_ows_ok("\t , \ta", 1);
    }

    SECTION("not comma")
    {
        test_skip_ows_comma_ows_ng("a");
    }
    SECTION("not comma after ows")
    {
        test_skip_ows_comma_ows_ng(" a");
    }
    SECTION("ends with comma")
    {
        test_skip_ows_comma_ows_ng(" ,");
    }
    SECTION("ends with comma and ows")
    {
        test_skip_ows_comma_ows_ng(" , ");
    }
}
