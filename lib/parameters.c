#include "hsfv.h"

#define PARAMETERS_INITIAL_CAPACITY 8

bool hsfv_parameter_eq(const hsfv_parameter_t *self,
                       const hsfv_parameter_t *other) {
  return hsfv_key_eq(&self->key, &other->key) &&
         hsfv_bare_item_eq(&self->value, &other->value);
}

bool hsfv_parameters_eq(const hsfv_parameters_t *self,
                        const hsfv_parameters_t *other) {
  if (self->len != other->len) {
    return false;
  }
  for (size_t i = 0; i < self->len; i++) {
    if (!hsfv_parameter_eq(&self->params[i], &other->params[i])) {
      return false;
    }
  }
  return true;
}

void hsfv_parameter_deinit(hsfv_parameter_t *parameter,
                           hsfv_allocator_t *allocator) {
  hsfv_key_deinit(&parameter->key, allocator);
  hsfv_bare_item_deinit(&parameter->value, allocator);
}

void hsfv_parameters_deinit(hsfv_parameters_t *parameters,
                            hsfv_allocator_t *allocator) {
  for (size_t i = 0; i < parameters->len; i++) {
    hsfv_parameter_deinit(&parameters->params[i], allocator);
  }
  allocator->free(allocator, parameters->params);
}

static hsfv_err_t hsfv_parameters_append(hsfv_allocator_t *allocator,
                                         hsfv_parameters_t *parameters,
                                         hsfv_parameter_t *param) {
  if (parameters->len + 1 >= parameters->capacity) {
    size_t new_capacity =
        hsfv_align(parameters->len + 1, PARAMETERS_INITIAL_CAPACITY);
    parameters->params = allocator->realloc(
        allocator, parameters->params, new_capacity * sizeof(hsfv_parameter_t));
    if (parameters->params == NULL) {
      return HSFV_ERR_OUT_OF_MEMORY;
    }
    parameters->capacity = new_capacity;
  }
  parameters->params[parameters->len] = *param;
  parameters->len++;
  return HSFV_OK;
}

size_t hsfv_parameters_index_of(const hsfv_parameters_t *parameters,
                                const hsfv_key_t *key) {
  for (size_t i = 0; i < parameters->len; i++) {
    if (hsfv_key_eq(&parameters->params[i].key, key)) {
      return i;
    }
  }
  return -1;
}

hsfv_err_t hsfv_parse_parameters(hsfv_parameters_t *parameters,
                                 hsfv_allocator_t *allocator, const char *input,
                                 const char *input_end, const char **out_rest) {
  hsfv_err_t err;
  char c;
  hsfv_parameter_t param;
  hsfv_parameters_t temp;
  size_t i;

  parameters->params = NULL;
  parameters->len = 0;
  parameters->capacity = 0;

  temp.params = NULL;
  temp.len = 0;
  temp.capacity = 0;

  while (input < input_end) {
    c = *input;
    if (c != ';') {
      break;
    }
    ++input;

    hsfv_skip_sp(input, input_end);

    err = hsfv_parse_key(&param.key, allocator, input, input_end, &input);
    if (err) {
      goto error3;
    }

    if (input < input_end && *input == '=') {
      ++input;
      err = hsfv_parse_bare_item(&param.value, allocator, input, input_end,
                                 &input);
      if (err) {
        goto error2;
      }
    } else {
      param.value.type = HSFV_BARE_ITEM_TYPE_BOOLEAN;
      param.value.boolean = 1;
    }
    i = hsfv_parameters_index_of(&temp, &param.key);
    if (i == -1) {
      err = hsfv_parameters_append(allocator, &temp, &param);
      if (err) {
        goto error1;
      }
    } else {
      hsfv_parameter_deinit(&temp.params[i], allocator);
      temp.params[i] = param;
    }
  }

  *parameters = temp;
  if (out_rest) {
    *out_rest = input;
  }
  return HSFV_OK;

error1:
  hsfv_bare_item_deinit(&param.value, allocator);

error2:
  hsfv_key_deinit(&param.key, allocator);

error3:
  hsfv_parameters_deinit(&temp, allocator);
  return err;
}
