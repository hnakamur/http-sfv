#include <catch2/catch_test_macros.hpp>
#include "hsfv.h"

TEST_CASE("booleans can be parsed", "[bare_item][boolean]") {
    SECTION("parse false") {
        const char *buf = "?0";
        const char *buf_end = buf + strlen(buf);
        int boolean;
        int ret;
        buf = parse_boolean(buf, buf_end, &boolean, &ret);
        CHECK(boolean == 0);
        CHECK(ret == HSFV_OK);
        CHECK(buf == buf_end);
    }
    SECTION("parse true") {
        const char *buf = "?1";
        const char *buf_end = buf + strlen(buf);
        int boolean;
        int ret;
        buf = parse_boolean(buf, buf_end, &boolean, &ret);
        CHECK(boolean == 1);
        CHECK(ret == HSFV_OK);
        CHECK(buf == buf_end);
    }
    SECTION("unexpected EOF") {
        const char *buf = "?";
        const char *buf_end = buf + strlen(buf);
        int boolean;
        int ret;
        buf = parse_boolean(buf, buf_end, &boolean, &ret);
        CHECK(ret == HSFV_ERR_EOF);
        CHECK(buf == NULL);
    }
    SECTION("invalid value") {
        const char *buf = "?2";
        const char *buf_end = buf + strlen(buf);
        int boolean;
        int ret;
        buf = parse_boolean(buf, buf_end, &boolean, &ret);
        CHECK(ret == HSFV_ERR_INVALID);
        CHECK(buf == NULL);
    }
}
