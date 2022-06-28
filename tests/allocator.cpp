#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("allocator", "[allocator]")
{
    SECTION("global_allocator")
    {
        void *buf = hsfv_global_allocator.alloc(&hsfv_global_allocator, 8);
        CHECK(buf != NULL);
        void *buf2 = hsfv_global_allocator.realloc(&hsfv_global_allocator, buf, 16);
        CHECK(buf2 != NULL);
        buf = buf2;
        hsfv_global_allocator.free(&hsfv_global_allocator, buf);
    }

    SECTION("failing_allocator")
    {
        hsfv_allocator_t *allocator = &hsfv_failing_allocator.allocator;
        void *buf = allocator->alloc(allocator, 8);
        CHECK(buf != NULL);
        void *buf2 = allocator->realloc(allocator, buf, 16);
        CHECK(buf2 != NULL);
        buf = buf2;
        allocator->free(allocator, buf);
        CHECK(hsfv_failing_allocator.alloc_count == 2);

        hsfv_failing_allocator.fail_index = 0;
        hsfv_failing_allocator.alloc_count = 0;
        CHECK(allocator->alloc(allocator, 8) == NULL);

        hsfv_failing_allocator.fail_index = 1;
        hsfv_failing_allocator.alloc_count = 0;
        buf2 = allocator->alloc(allocator, 8);
        CHECK(buf2 != NULL);
        buf = buf2;
        CHECK(allocator->realloc(allocator, buf, 16) == NULL);
        allocator->free(allocator, buf);
    }
}

TEST_CASE("hsfv_bytes_dup", "[allocator]")
{
    SECTION("ok")
    {
        const char *input = "abc";
        const char *copy = (const char *)hsfv_bytes_dup(&hsfv_global_allocator, (const hsfv_byte_t *)input, strlen(input));
        CHECK(copy != NULL);
        CHECK(copy != input);

        hsfv_global_allocator.free(&hsfv_global_allocator, (void *)copy);
    }

    SECTION("ng")
    {
        hsfv_allocator_t *allocator = &hsfv_failing_allocator.allocator;
        hsfv_failing_allocator.fail_index = 0;
        hsfv_failing_allocator.alloc_count = 0;

        const char *input = "abc";
        const char *copy = (const char *)hsfv_bytes_dup(allocator, (const hsfv_byte_t *)input, strlen(input));
        CHECK(copy == NULL);
    }
}
