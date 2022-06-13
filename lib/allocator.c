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

char *hsfv_strndup(hsfv_allocator_t *allocator, const char *src, size_t len)
{
    char *copy = allocator->alloc(allocator, len);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, src, len);
    return copy;
}
