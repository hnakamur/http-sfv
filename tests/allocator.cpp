#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("allocator", "[allocator]")
{
    SECTION("global_allocator")
    {
        void *buf = hsfv_global_allocator.alloc(&hsfv_global_allocator, 8);
        CHECK(buf != NULL);
        buf = hsfv_global_allocator.realloc(&hsfv_global_allocator, buf, 16);
        CHECK(buf != NULL);
        hsfv_global_allocator.free(&hsfv_global_allocator, buf);
    }

    SECTION("counting_allocator")
    {
        hsfv_allocator_t *allocator = &hsfv_counting_allocator.allocator;
        void *buf = allocator->alloc(allocator, 8);
        CHECK(buf != NULL);
        buf = allocator->realloc(allocator, buf, 16);
        CHECK(buf != NULL);
        allocator->free(allocator, buf);
        CHECK(hsfv_counting_allocator.alloc_count == 2);
    }
}
