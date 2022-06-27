#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("decode base64", "[base64]")
{
    SECTION("invalid char")
    {
        hsfv_iovec_const_t src = {
            .base = (const hsfv_byte_t *)"\t",
            .len = 1,
        };
        hsfv_err_t err = hsfv_decode_base64(NULL, &src);
        CHECK(err == HSFV_ERR);
    }

    SECTION("invalid length")
    {
        hsfv_iovec_const_t src = {
            .base = (const hsfv_byte_t *)"abc",
            .len = 1,
        };
        hsfv_err_t err = hsfv_decode_base64(NULL, &src);
        CHECK(err == HSFV_ERR);
    }
}
