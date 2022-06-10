#include <catch2/catch_test_macros.hpp>
#include "hsfv.h"

TEST_CASE("allocator", "[allocator]") {
    SECTION("global_allocator") {
        void *buf = htsv_global_allocator.alloc(&htsv_global_allocator, 8);
        CHECK(buf != NULL);
        buf = htsv_global_allocator.realloc(&htsv_global_allocator, buf, 16);
        CHECK(buf != NULL);
        htsv_global_allocator.free(&htsv_global_allocator, buf);
    }
}
