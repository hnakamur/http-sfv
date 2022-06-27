#include "hsfv.h"

static void *global_allocator_alloc(hsfv_allocator_t *_self, size_t size)
{
    return malloc(size);
}

static void *global_allocator_realloc(hsfv_allocator_t *_self, void *ptr, size_t size)
{
    return realloc(ptr, size);
}

static void global_allocator_free(hsfv_allocator_t *_self, void *ptr)
{
    free(ptr);
}

hsfv_allocator_t hsfv_global_allocator = {
    .alloc = global_allocator_alloc,
    .realloc = global_allocator_realloc,
    .free = global_allocator_free,
};

static void *failing_allocator_alloc(hsfv_allocator_t *self, size_t size)
{
    hsfv_failing_allocator_t *ca = (hsfv_failing_allocator_t *)self;
    if (ca->alloc_count == ca->fail_index) {
        return NULL;
    }
    ca->alloc_count++;
    return global_allocator_alloc(self, size);
}

static void *failing_allocator_realloc(hsfv_allocator_t *self, void *ptr, size_t size)
{
    hsfv_failing_allocator_t *ca = (hsfv_failing_allocator_t *)self;
    if (ca->alloc_count == ca->fail_index) {
        return NULL;
    }
    ca->alloc_count++;
    return global_allocator_realloc(self, ptr, size);
}

static void failing_allocator_free(hsfv_allocator_t *self, void *ptr)
{
    global_allocator_free(self, ptr);
}

hsfv_failing_allocator_t hsfv_failing_allocator = {
    .allocator =
        {
            .alloc = failing_allocator_alloc,
            .realloc = failing_allocator_realloc,
            .free = failing_allocator_free,
        },
    .fail_index = -1,
};

hsfv_byte_t *hsfv_bytes_dup(hsfv_allocator_t *allocator, const hsfv_byte_t *src, size_t len)
{
    hsfv_byte_t *copy = allocator->alloc(allocator, len);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, src, len);
    return copy;
}
