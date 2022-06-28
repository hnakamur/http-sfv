#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("st_hsfv_buffer_t", "[allocator]")
{
    SECTION("shrink")
    {
        hsfv_buffer_t buf = hsfv_buffer_t{0};

        hsfv_err_t err = hsfv_buffer_append_bytes(&buf, &hsfv_global_allocator, "abc", 3);
        CHECK(err == HSFV_OK);
        CHECK(buf.bytes.len == 3);
        CHECK(!memcmp(buf.bytes.base, "abc", 3));

        err = hsfv_buffer_realloc(&buf, &hsfv_global_allocator, 1);
        CHECK(err == HSFV_OK);
        CHECK(buf.capacity == 1);
        CHECK(buf.bytes.len == 1);
        CHECK(!memcmp(buf.bytes.base, "a", 1));

        hsfv_buffer_deinit(&buf, &hsfv_global_allocator);
    }
}
