#include "hsfv.h"

#define INNER_LIST_INITIAL_CAPACITY 8

bool hsfv_inner_list_eq(const hsfv_inner_list_t *self,
                        const hsfv_inner_list_t *other) {
  if (self->len != other->len) {
    return false;
  }
  for (size_t i = 0; i < self->len; i++) {
    if (!hsfv_item_eq(&self->items[i], &other->items[i])) {
      return false;
    }
  }
  return hsfv_parameters_eq(&self->parameters, &other->parameters);
}

void hsfv_inner_list_deinit(hsfv_inner_list_t *self,
                            hsfv_allocator_t *allocator) {
  for (size_t i = 0; i < self->len; i++) {
    hsfv_item_deinit(&self->items[i], allocator);
  }
  allocator->free(allocator, self->items);
  hsfv_parameters_deinit(&self->parameters, allocator);
}

static hsfv_err_t hsfv_inner_list_append(hsfv_inner_list_t *self,
                                         hsfv_allocator_t *allocator,
                                         const hsfv_item_t *item) {
  if (self->len + 1 >= self->capacity) {
    size_t new_capacity =
        hsfv_align(self->len + 1, INNER_LIST_INITIAL_CAPACITY);
    self->items = allocator->realloc(allocator, self->items,
                                     new_capacity * sizeof(hsfv_item_t));
    if (self->items == NULL) {
      return HSFV_ERR_OUT_OF_MEMORY;
    }
    self->capacity = new_capacity;
  }
  self->items[self->len] = *item;
  self->len++;
  return HSFV_OK;
}

hsfv_err_t hsfv_parse_inner_list(hsfv_inner_list_t *inner_list,
                                 hsfv_allocator_t *allocator, const char *input,
                                 const char *input_end, const char **out_rest) {
  hsfv_err_t err;
  char c;
  hsfv_item_t item;

  if (input == input_end) {
    return HSFV_ERR_EOF;
  }

  c = *input;
  if (c != '(') {
    return HSFV_ERR_INVALID;
  }
  ++input;

  inner_list->items = NULL;
  inner_list->len = 0;
  inner_list->capacity = 0;
  inner_list->parameters.params = NULL;
  inner_list->parameters.len = 0;
  inner_list->parameters.capacity = 0;

  while (input < input_end) {
    while (input < input_end && *input == ' ') {
      ++input;
    }

    if (input == input_end) {
      err = HSFV_ERR_EOF;
      goto error2;
    }
    c = *input;
    if (c == ')') {
      ++input;
      err = hsfv_parse_parameters(&inner_list->parameters, allocator, input,
                                  input_end, &input);
      if (err) {
        goto error2;
      }
      if (out_rest) {
        *out_rest = input;
      }
      return HSFV_OK;
    }

    err = hsfv_parse_item(&item, allocator, input, input_end, &input);
    if (err) {
      goto error2;
    }

    err = hsfv_inner_list_append(inner_list, allocator, &item);
    if (err) {
      goto error1;
    }
  }

  err = HSFV_ERR_EOF;

error1:
  hsfv_item_deinit(&item, allocator);

error2:
  hsfv_inner_list_deinit(inner_list, allocator);
  return err;
}