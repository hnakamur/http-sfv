#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("allocator", "[allocator]") {
  SECTION("global_allocator") {
    void *buf = hsfv_global_allocator.alloc(&hsfv_global_allocator, 8);
    CHECK(buf != NULL);
    buf = hsfv_global_allocator.realloc(&hsfv_global_allocator, buf, 16);
    CHECK(buf != NULL);
    hsfv_global_allocator.free(&hsfv_global_allocator, buf);
  }
}
