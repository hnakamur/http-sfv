#include "hsfv.h"

#define LIST_INITIAL_CAPACITY 8

static bool hsfv_list_member_eq(const hsfv_list_member_t *self,
                                const hsfv_list_member_t *other) {
  if (self->type != other->type) {
    return false;
  }
  switch (self->type) {
  case HSFV_LIST_MEMBER_TYPE_ITEM:
    return hsfv_item_eq(&self->item, &other->item);
  case HSFV_LIST_MEMBER_TYPE_INNER_LIST:
    return hsfv_inner_list_eq(&self->inner_list, &other->inner_list);
  default:
    return false;
  }
}

bool hsfv_list_eq(const hsfv_list_t *self, const hsfv_list_t *other) {
  if (self->len != other->len) {
    printf("hsfv_list_eq len mismatch, self=%d, other=%d\n", self->len,
           other->len);
    return false;
  }
  for (size_t i = 0; i < self->len; i++) {
    if (!hsfv_list_member_eq(&self->members[i], &other->members[i])) {
      printf("hsfv_list_eq member %d mismatch\n", i);
      return false;
    }
  }
  return true;
}

static void sfv_list_member_deinit(hsfv_list_member_t *self,
                                   hsfv_allocator_t *allocator) {
  printf("sfv_list_member_deinit start self=%p\n", self);
  switch (self->type) {
  case HSFV_LIST_MEMBER_TYPE_ITEM:
    hsfv_item_deinit(&self->item, allocator);
    break;
  case HSFV_LIST_MEMBER_TYPE_INNER_LIST:
    printf("sfv_list_member_deinit %p, calling hsfv_inner_list_deinit=%p\n",
           self, &self->inner_list);
    hsfv_inner_list_deinit(&self->inner_list, allocator);
    break;
  }
}

void hsfv_list_deinit(hsfv_list_t *self, hsfv_allocator_t *allocator) {
  printf("hsfv_list_deinit start self=%p\n", self);
  for (size_t i = 0; i < self->len; i++) {
    printf("hsfv_list_deinit calling sfv_list_member_deinit, %ld,%p\n", i,
           &self->members[i]);
    sfv_list_member_deinit(&self->members[i], allocator);
  }
  allocator->free(allocator, self->members);
}

static hsfv_err_t hsfv_list_append(hsfv_list_t *self,
                                   hsfv_allocator_t *allocator,
                                   const hsfv_list_member_t *member) {
  printf("hsfv_list_append start self=%p\n", self);
  if (self->len + 1 >= self->capacity) {
    size_t new_capacity = hsfv_align(self->len + 1, LIST_INITIAL_CAPACITY);
    self->members = allocator->realloc(
        allocator, self->members, new_capacity * sizeof(hsfv_list_member_t));
    printf("hsfv_list_append %p, allocated members=%p\n", self, self->members);
    if (self->members == NULL) {
      return HSFV_ERR_OUT_OF_MEMORY;
    }
    self->capacity = new_capacity;
  }
  self->members[self->len] = *member;
  printf("hsfv_list_append %p copied member at %ld, dest=%p, src=%p\n", self,
         self->len, &self->members[self->len], member);
  self->len++;
  return HSFV_OK;
}

hsfv_err_t hsfv_parse_list(hsfv_list_t *list, hsfv_allocator_t *allocator,
                           const char *input, const char *input_end,
                           const char **out_rest) {
  printf("hsfv_parse_list start self=%p\n", list);
  hsfv_err_t err;
  char c;
  hsfv_list_member_t member;

  list->members = NULL;
  list->len = 0;
  list->capacity = 0;

  while (input < input_end) {
    printf("hsfv_parse_list loop start rest=%s\n", input);
    if (*input == '(') {
      err = hsfv_parse_inner_list(&member.inner_list, allocator, input,
                                  input_end, &input);
      printf("hsfv_parse_list after inner_list, err=%d, rest=%s\n", err, input);
      if (err) {
        goto error2;
      }
      member.type = HSFV_LIST_MEMBER_TYPE_INNER_LIST;
    } else {
      err = hsfv_parse_item(&member.item, allocator, input, input_end, &input);
      printf("hsfv_parse_list after item, err=%d, rest=%s\n", err, input);
      if (err) {
        goto error2;
      }
      member.type = HSFV_LIST_MEMBER_TYPE_ITEM;
    }
    err = hsfv_list_append(list, allocator, &member);
    if (err) {
      goto error1;
    }

    hsfv_skip_ows(input, input_end);
    if (input < input_end && *input == ',') {
      ++input;
      hsfv_skip_ows(input, input_end);
      if (input == input_end) {
        err = HSFV_ERR_EOF;
        goto error2;
      }
    }
  }

  if (out_rest) {
    *out_rest = input;
  }
  return HSFV_OK;

error1:
  sfv_list_member_deinit(&member, allocator);

error2:
  hsfv_list_deinit(list, allocator);
  return err;
}
