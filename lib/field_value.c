#include "hsfv.h"

bool hsfv_field_value_eq(const hsfv_field_value_t *self, const hsfv_field_value_t *other)
{
    if (self->type != other->type) {
        return false;
    }
    switch (self->type) {
    case HSFV_FIELD_VALUE_TYPE_LIST:
        return hsfv_list_eq(&self->list, &other->list);
    case HSFV_FIELD_VALUE_TYPE_DICTIONARY:
        return hsfv_dictionary_eq(&self->dictionary, &other->dictionary);
    case HSFV_FIELD_VALUE_TYPE_ITEM:
        return hsfv_item_eq(&self->item, &other->item);
    default:
        return false;
    }
}

void hsfv_field_value_deinit(hsfv_field_value_t *self, hsfv_allocator_t *allocator)
{
    switch (self->type) {
    case HSFV_FIELD_VALUE_TYPE_LIST:
        hsfv_list_deinit(&self->list, allocator);
        break;
    case HSFV_FIELD_VALUE_TYPE_DICTIONARY:
        hsfv_dictionary_deinit(&self->dictionary, allocator);
        break;
    case HSFV_FIELD_VALUE_TYPE_ITEM:
        hsfv_item_deinit(&self->item, allocator);
        break;
    }
}

bool hsfv_field_value_is_empty(const hsfv_field_value_t *self)
{
    switch (self->type) {
    case HSFV_FIELD_VALUE_TYPE_LIST:
        return self->list.len == 0;
    case HSFV_FIELD_VALUE_TYPE_DICTIONARY:
        return self->dictionary.len == 0;
    default:
        return false;
    }
}

hsfv_err_t hsfv_serialize_field_value(const hsfv_field_value_t *field_value, hsfv_allocator_t *allocator, hsfv_buffer_t *dest)
{
    if (hsfv_field_value_is_empty(field_value)) {
        return HSFV_ERR_INVALID;
    }

    switch (field_value->type) {
    case HSFV_FIELD_VALUE_TYPE_LIST:
        return hsfv_serialize_list(&field_value->list, allocator, dest);
    case HSFV_FIELD_VALUE_TYPE_DICTIONARY:
        return hsfv_serialize_dictionary(&field_value->dictionary, allocator, dest);
    case HSFV_FIELD_VALUE_TYPE_ITEM:
        return hsfv_serialize_item(&field_value->item, allocator, dest);
    default:
        return HSFV_ERR_INVALID;
    }
}

hsfv_err_t hsfv_parse_field_value(hsfv_field_value_t *field_value, hsfv_field_value_type_t field_type, hsfv_allocator_t *allocator,
                                  const char *input, const char *input_end, const char **out_rest)
{
    hsfv_err_t err;

    if (!hsfv_is_ascii_string(input, input_end)) {
        return HSFV_ERR_INVALID;
    }

    hsfv_skip_sp(input, input_end);
    switch (field_type) {
    case HSFV_FIELD_VALUE_TYPE_LIST:
        err = hsfv_parse_list(&field_value->list, allocator, input, input_end, &input);
        if (err) {
            return err;
        }
        field_value->type = HSFV_FIELD_VALUE_TYPE_LIST;
        break;
    case HSFV_FIELD_VALUE_TYPE_DICTIONARY:
        err = hsfv_parse_dictionary(&field_value->dictionary, allocator, input, input_end, &input);
        if (err) {
            return err;
        }
        field_value->type = HSFV_FIELD_VALUE_TYPE_DICTIONARY;
        break;
    case HSFV_FIELD_VALUE_TYPE_ITEM:
        err = hsfv_parse_item(&field_value->item, allocator, input, input_end, &input);
        if (err) {
            return err;
        }
        field_value->type = HSFV_FIELD_VALUE_TYPE_ITEM;
        break;
    }

    hsfv_skip_sp(input, input_end);

    if (input < input_end) {
        err = HSFV_ERR_INVALID;
        goto error;
    }

    if (out_rest) {
        *out_rest = input;
    }
    return HSFV_OK;

error:
    hsfv_field_value_deinit(field_value, allocator);
    return err;
}
