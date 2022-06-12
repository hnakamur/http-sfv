#include "hsfv.h"

hsfv_err_t hsfv_buffer_alloc(hsfv_buffer_t *buf, hsfv_allocator_t *allocator,
                             size_t capacity) {
  buf->bytes.base = allocator->alloc(allocator, capacity);
  if (buf->bytes.base == NULL) {
    return HSFV_ERR_OUT_OF_MEMORY;
  }
  buf->bytes.len = 0;
  buf->capacity = capacity;
  return HSFV_OK;
}

hsfv_err_t hsfv_buffer_realloc(hsfv_buffer_t *buf, hsfv_allocator_t *allocator,
                               size_t capacity) {
  buf->bytes.base = allocator->realloc(allocator, buf->bytes.base, capacity);
  if (buf->bytes.base == NULL) {
    return HSFV_ERR_OUT_OF_MEMORY;
  }
  if (capacity < buf->bytes.len) {
    buf->bytes.len = capacity;
  }
  buf->capacity = capacity;
  return HSFV_OK;
}

void hsfv_buffer_deinit(hsfv_buffer_t *buf, hsfv_allocator_t *allocator) {
  allocator->free(allocator, buf->bytes.base);
  buf->bytes.base = NULL;
  buf->bytes.len = 0;
  buf->capacity = 0;
}

#define BUFFER_CAPACITY_ALIGN 8

hsfv_err_t hsfv_buffer_ensure_unused_bytes(hsfv_buffer_t *buf,
                                           hsfv_allocator_t *allocator,
                                           size_t len) {
  size_t new_capacity;

  if (buf->bytes.len + len > buf->capacity) {
    new_capacity = hsfv_align(buf->bytes.len + len, BUFFER_CAPACITY_ALIGN);
    return hsfv_buffer_realloc(buf, allocator, new_capacity);
  }
  return HSFV_OK;
}

hsfv_err_t hsfv_buffer_append_byte(hsfv_buffer_t *buf,
                                   hsfv_allocator_t *allocator,
                                   const char src) {
  hsfv_err_t err;

  err = hsfv_buffer_ensure_unused_bytes(buf, allocator, 1);
  if (err) {
    return err;
  }

  hsfv_buffer_append_byte_unsafe(buf, src);
  return HSFV_OK;
}

hsfv_err_t hsfv_buffer_append_bytes(hsfv_buffer_t *buf,
                                    hsfv_allocator_t *allocator,
                                    const char *src, size_t len) {
  hsfv_err_t err;

  err = hsfv_buffer_ensure_unused_bytes(buf, allocator, len);
  if (err) {
    return err;
  }

  hsfv_buffer_append_bytes_unsafe(buf, src, len);
  return HSFV_OK;
}
