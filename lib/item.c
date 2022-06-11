#include "hsfv.h"

bool hsfv_item_eq(const hsfv_item_t *self, const hsfv_item_t *other) {
  return hsfv_bare_item_eq(&self->bare_item, &other->bare_item) &&
         hsfv_parameters_eq(&self->parameters, &other->parameters);
}

void hsfv_item_deinit(hsfv_item_t *item, hsfv_allocator_t *allocator) {
  hsfv_bare_item_deinit(&item->bare_item, allocator);
  hsfv_parameters_deinit(&item->parameters, allocator);
}

hsfv_err_t hsfv_parse_item(hsfv_item_t *item, hsfv_allocator_t *allocator,
                           const char *input, const char *input_end,
                           const char **out_rest) {
  hsfv_err_t err;

  err = hsfv_parse_bare_item(&item->bare_item, allocator, input, input_end,
                             &input);
  if (err) {
    return err;
  }

  err = hsfv_parse_parameters(&item->parameters, allocator, input, input_end,
                              &input);
  if (err) {
    goto error;
  }

  if (out_rest) {
    *out_rest = input;
  }
  return HSFV_OK;

error:
  hsfv_bare_item_deinit(&item->bare_item, allocator);
  return err;
}
