#include "hsfv.h"

static bool hsfv_dict_member_value_eq(const hsfv_dict_member_value_t *self,
                                      const hsfv_dict_member_value_t *other) {
  if (self->type != other->type) {
    return false;
  }
  switch (self->type) {
  case HSFV_DICT_MEMBER_TYPE_ITEM:
    return hsfv_item_eq(&self->item, &other->item);
  case HSFV_DICT_MEMBER_TYPE_INNER_LIST:
    return hsfv_inner_list_eq(&self->inner_list, &other->inner_list);
  default:
    return false;
  }
}

static bool hsfv_dict_member_eq(const hsfv_dict_member_t *self,
                                const hsfv_dict_member_t *other) {
  return hsfv_key_eq(&self->key, &other->key) &&
         hsfv_parameters_eq(&self->parameters, &other->parameters) &&
         hsfv_dict_member_value_eq(&self->value, &other->value);
}

bool hsfv_dictionary_eq(const hsfv_dictionary_t *self,
                        const hsfv_dictionary_t *other) {
  if (self->len != other->len) {
    return false;
  }
  for (size_t i = 0; i < self->len; i++) {
    if (!hsfv_dict_member_eq(&self->members[i], &other->members[i])) {
      return false;
    }
  }
  return true;
}

static void hsfv_dict_member_value_deinit(hsfv_dict_member_value_t *self,
                                          hsfv_allocator_t *allocator) {
  switch (self->type) {
  case HSFV_DICT_MEMBER_TYPE_ITEM:
    hsfv_item_deinit(&self->item, allocator);
    break;
  case HSFV_DICT_MEMBER_TYPE_INNER_LIST:
    hsfv_inner_list_deinit(&self->inner_list, allocator);
    break;
  }
}

static void hsfv_dict_member_deinit(hsfv_dict_member_t *self,
                                    hsfv_allocator_t *allocator) {
  hsfv_key_deinit(&self->key, allocator);
  hsfv_parameters_deinit(&self->parameters, allocator);
  hsfv_dict_member_value_deinit(&self->value, allocator);
}

void hsfv_dictionary_deinit(hsfv_dictionary_t *self,
                            hsfv_allocator_t *allocator) {
  for (size_t i = 0; i < self->len; i++) {
    hsfv_dict_member_deinit(&self->members[i], allocator);
  }
  allocator->free(allocator, self->members);
}

#define DICT_INITIAL_CAPACITY 8

static hsfv_err_t hsfv_dictionary_append(hsfv_dictionary_t *dictionary,
                                         hsfv_allocator_t *allocator,
                                         hsfv_dict_member_t *member) {
  if (dictionary->len + 1 >= dictionary->capacity) {
    size_t new_capacity =
        hsfv_align(dictionary->len + 1, DICT_INITIAL_CAPACITY);
    dictionary->members =
        allocator->realloc(allocator, dictionary->members,
                           new_capacity * sizeof(hsfv_parameter_t));
    if (dictionary->members == NULL) {
      return HSFV_ERR_OUT_OF_MEMORY;
    }
    dictionary->capacity = new_capacity;
  }
  dictionary->members[dictionary->len] = *member;
  dictionary->len++;
  return HSFV_OK;
}

size_t hsfv_dictionary_index_of(const hsfv_dictionary_t *dictionary,
                                const hsfv_key_t *key) {
  for (size_t i = 0; i < dictionary->len; i++) {
    if (hsfv_key_eq(&dictionary->members[i].key, key)) {
      return i;
    }
  }
  return -1;
}

hsfv_err_t hsfv_parse_dictionary(hsfv_dictionary_t *dictionary,
                                 hsfv_allocator_t *allocator, const char *input,
                                 const char *input_end, const char **out_rest) {
  hsfv_err_t err;
  hsfv_dict_member_t member;
  size_t i;

  *dictionary = (hsfv_dictionary_t){0};
  while (input < input_end) {
    member = (hsfv_dict_member_t){0};
    err = hsfv_parse_key(&member.key, allocator, input, input_end, &input);
    if (err) {
      goto error2;
    }

    if (input < input_end && *input == '=') {
      ++input;
      if (*input == '(') {
        err = hsfv_parse_inner_list(&member.value.inner_list, allocator, input,
                                    input_end, &input);
        if (err) {
          goto error1;
        }
        member.value.type = HSFV_LIST_MEMBER_TYPE_INNER_LIST;
      } else {
        err = hsfv_parse_item(&member.value.item, allocator, input, input_end,
                              &input);
        if (err) {
          goto error1;
        }
        member.value.type = HSFV_LIST_MEMBER_TYPE_ITEM;
      }
    } else {
      member.value.type = HSFV_LIST_MEMBER_TYPE_ITEM;
      member.value.item.bare_item.type = HSFV_BARE_ITEM_TYPE_BOOLEAN;
      member.value.item.bare_item.boolean = true;
      err = hsfv_parse_parameters(&member.parameters, allocator, input,
                                  input_end, &input);
      if (err) {
        goto error1;
      }
    }
    i = hsfv_dictionary_index_of(dictionary, &member.key);
    if (i == -1) {
      err = hsfv_dictionary_append(dictionary, allocator, &member);
      if (err) {
        goto error1;
      }
    } else {
      hsfv_dict_member_deinit(&dictionary->members[i], allocator);
      dictionary->members[i] = member;
    }

    hsfv_skip_ows(input, input_end);
    if (input < input_end) {
      if (*input == ',') {
        ++input;
        hsfv_skip_ows(input, input_end);
        if (input == input_end) {
          err = HSFV_ERR_EOF;
          goto error2;
        }
      } else {
        err = HSFV_ERR_INVALID;
        goto error2;
      }
    }
  }

  if (out_rest) {
    *out_rest = input;
  }
  return HSFV_OK;

error1:
  hsfv_dict_member_deinit(&member, allocator);

error2:
  hsfv_dictionary_deinit(dictionary, allocator);
  return err;
}