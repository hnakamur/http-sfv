#include "hsfv.h"

bool hsfv_iovec_const_eq(const hsfv_iovec_const_t *self, const hsfv_iovec_const_t *other)
{
    return self->len == other->len && !memcmp(self->base, other->base, self->len);
}

void hsfv_iovec_deinit(hsfv_iovec_t *v, hsfv_allocator_t *allocator)
{
    allocator->free(allocator, (void *)v->base);
}

void hsfv_iovec_const_deinit(hsfv_iovec_const_t *v, hsfv_allocator_t *allocator)
{
    allocator->free(allocator, (void *)v->base);
}
