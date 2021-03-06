#include "hsfv.h"

#define LIST_INITIAL_CAPACITY 8

static bool hsfv_list_member_eq(const hsfv_list_member_t *self, const hsfv_list_member_t *other)
{
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

bool hsfv_list_eq(const hsfv_list_t *self, const hsfv_list_t *other)
{
    if (self->len != other->len) {
        return false;
    }
    for (size_t i = 0; i < self->len; i++) {
        if (!hsfv_list_member_eq(&self->members[i], &other->members[i])) {
            return false;
        }
    }
    return true;
}

static void sfv_list_member_deinit(hsfv_list_member_t *self, hsfv_allocator_t *allocator)
{
    switch (self->type) {
    case HSFV_LIST_MEMBER_TYPE_ITEM:
        hsfv_item_deinit(&self->item, allocator);
        break;
    case HSFV_LIST_MEMBER_TYPE_INNER_LIST:
        hsfv_inner_list_deinit(&self->inner_list, allocator);
        break;
    }
}

void hsfv_list_deinit(hsfv_list_t *self, hsfv_allocator_t *allocator)
{
    for (size_t i = 0; i < self->len; i++) {
        sfv_list_member_deinit(&self->members[i], allocator);
    }
    allocator->free(allocator, self->members);
}

hsfv_err_t hsfv_serialize_list(const hsfv_list_t *list, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    hsfv_err_t err;
    const hsfv_list_member_t *member;

    for (size_t i = 0; i < list->len; ++i) {
        if (i > 0) {
            err = hsfv_buffer_append_bytes(dest, allocator, ", ", 2);
            if (err) {
                return err;
            }
        }

        member = &list->members[i];
        switch (member->type) {
        case HSFV_LIST_MEMBER_TYPE_ITEM:
            err = hsfv_serialize_item(&member->item, allocator, dest);
            if (err) {
                return err;
            }
            break;
        case HSFV_LIST_MEMBER_TYPE_INNER_LIST:
            err = hsfv_serialize_inner_list(&member->inner_list, allocator, dest);
            if (err) {
                return err;
            }
            break;
        }
    }

    return HSFV_OK;
}

static hsfv_err_t hsfv_list_append(hsfv_list_t *self, hsfv_allocator_t *allocator, const hsfv_list_member_t *member)
{
    if (self->len + 1 > self->capacity) {
        size_t new_capacity = hsfv_align(self->len + 1, LIST_INITIAL_CAPACITY);
        void *members2 = allocator->realloc(allocator, self->members, new_capacity * sizeof(hsfv_list_member_t));
        if (members2 == NULL) {
            return HSFV_ERR_OUT_OF_MEMORY;
        }
        self->members = members2;
        self->capacity = new_capacity;
    }
    self->members[self->len] = *member;
    self->len++;
    return HSFV_OK;
}

hsfv_err_t hsfv_parse_list(hsfv_list_t *list, hsfv_allocator_t *allocator, const char *input, const char *input_end,
                           const char **out_rest)
{
    hsfv_err_t err;
    hsfv_list_member_t member;

    *list = (hsfv_list_t){0};
    while (input < input_end) {
        if (*input == '(') {
            err = hsfv_parse_inner_list(&member.inner_list, allocator, input, input_end, &input);
            if (err) {
                goto error2;
            }
            member.type = HSFV_LIST_MEMBER_TYPE_INNER_LIST;
        } else {
            err = hsfv_parse_item(&member.item, allocator, input, input_end, &input);
            if (err) {
                goto error2;
            }
            member.type = HSFV_LIST_MEMBER_TYPE_ITEM;
        }
        err = hsfv_list_append(list, allocator, &member);
        if (err) {
            goto error1;
        }

        hsfv_skip_ows(input, input_end, &input);
        if (input < input_end) {
            if (*input == ',') {
                ++input;
                hsfv_skip_ows(input, input_end, &input);
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
    sfv_list_member_deinit(&member, allocator);

error2:
    hsfv_list_deinit(list, allocator);
    return err;
}
