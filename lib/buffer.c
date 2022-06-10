#include "hsfv.h"

hsfv_err_t htsv_buffer_alloc_bytes(hsfv_allocator_t *allocator,
                                   hsfv_buffer_t *buf, size_t capacity) {
  buf->bytes.base = allocator->alloc(allocator, capacity);
  if (buf->bytes.base == NULL) {
    return HSFV_ERR_OUT_OF_MEMORY;
  }
  buf->bytes.len = 0;
  buf->capacity = capacity;
  return HSFV_OK;
}

hsfv_err_t htsv_buffer_realloc_bytes(hsfv_allocator_t *allocator,
                                     hsfv_buffer_t *buf, size_t capacity) {
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

void htsv_buffer_free_bytes(hsfv_allocator_t *allocator, hsfv_buffer_t *buf) {
  allocator->free(allocator, buf->bytes.base);
  buf->bytes.base = NULL;
  buf->bytes.len = 0;
  buf->capacity = 0;
}

hsfv_err_t htsv_buffer_ensure_unused_bytes(hsfv_allocator_t *allocator,
                                           hsfv_buffer_t *buf, size_t len) {
  size_t new_capacity;

  if (buf->bytes.len + len > buf->capacity) {
    new_capacity = hsfv_align(buf->bytes.len + len, buf->capacity);
    return htsv_buffer_realloc_bytes(allocator, buf, new_capacity);
  }
  return HSFV_OK;
}

hsfv_err_t htsv_buffer_ensure_append_byte(hsfv_allocator_t *allocator,
                                          hsfv_buffer_t *buf, const char src) {
  hsfv_err_t err;

  err = htsv_buffer_ensure_unused_bytes(allocator, buf, 1);
  if (err) {
    return err;
  }

  buf->bytes.base[buf->bytes.len] = src;
  buf->bytes.len++;
  return HSFV_OK;
}

hsfv_err_t htsv_buffer_ensure_append_bytes(hsfv_allocator_t *allocator,
                                           hsfv_buffer_t *buf, const char *src,
                                           size_t len) {
  hsfv_err_t err;

  err = htsv_buffer_ensure_unused_bytes(allocator, buf, len);
  if (err) {
    return err;
  }

  memcpy(buf->bytes.base + buf->bytes.len, src, len);
  buf->bytes.len += len;
  return HSFV_OK;
}
