#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("hsfv_iovec_eq", "[allocator]")
{
    SECTION("equal and not equal")
    {
        const char *data = "abc";
        hsfv_byte_t *copy = (hsfv_byte_t *)hsfv_bytes_dup(&hsfv_global_allocator, (const hsfv_byte_t *)data, strlen(data));
        CHECK(copy != NULL);

        hsfv_byte_t *copy2 = (hsfv_byte_t *)hsfv_bytes_dup(&hsfv_global_allocator, (const hsfv_byte_t *)data, strlen(data));
        CHECK(copy2 != NULL);

        hsfv_iovec_t vec1 = hsfv_iovec_t{
            .base = copy,
            .len = strlen(data),
        };
        hsfv_iovec_t vec2 = hsfv_iovec_t{
            .base = copy2,
            .len = strlen(data),
        };
        CHECK(hsfv_iovec_eq(&vec1, &vec2));

        vec2.len--;
        CHECK(!hsfv_iovec_eq(&vec1, &vec2));

        vec2.len++;
        vec2.base[0] = '!';
        CHECK(!hsfv_iovec_eq(&vec1, &vec2));

        hsfv_iovec_deinit(&vec1, &hsfv_global_allocator);
        hsfv_iovec_deinit(&vec2, &hsfv_global_allocator);
    }
}
