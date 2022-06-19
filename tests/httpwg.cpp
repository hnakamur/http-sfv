#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>
#include <linux/limits.h>
extern "C" {
#include <baseencode.h>
}
#include <yyjson.h>

#define PATH_SEPARATOR "/"

static hsfv_err_t build_expected_bare_item(yyjson_val *bare_item_val, hsfv_allocator_t *allocator, hsfv_bare_item_t *out_item)
{
    hsfv_err_t err;
    const char *input, *input_end;

    yyjson_type bare_item_type = yyjson_get_type(bare_item_val);
    switch (bare_item_type) {
    case YYJSON_TYPE_BOOL: {
        bool b = yyjson_get_bool(bare_item_val);
        *out_item = (hsfv_bare_item_t){.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = b};
    } break;
    case YYJSON_TYPE_NUM: {
        if (yyjson_is_int(bare_item_val)) {
            *out_item = (hsfv_bare_item_t){.type = HSFV_BARE_ITEM_TYPE_INTEGER, .integer = yyjson_get_sint(bare_item_val)};
        } else {
            *out_item = (hsfv_bare_item_t){.type = HSFV_BARE_ITEM_TYPE_DECIMAL, .decimal = yyjson_get_real(bare_item_val)};
        }
    } break;
    case YYJSON_TYPE_STR: {
        const char *value = yyjson_get_str(bare_item_val);
        size_t value_len = yyjson_get_len(bare_item_val);
        const char *str_value = hsfv_strndup(allocator, value, value_len);
        if (!str_value) {
            return HSFV_ERR_OUT_OF_MEMORY;
        }
        *out_item = (hsfv_bare_item_t){.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = str_value, .len = value_len}};
    } break;
    case YYJSON_TYPE_OBJ: {
        const char *override_type = yyjson_get_str(yyjson_obj_get(bare_item_val, "__type"));
        if (!strcmp(override_type, "token")) {
            yyjson_val *value_val = yyjson_obj_get(bare_item_val, "value");
            const char *value = yyjson_get_str(value_val);
            size_t value_len = yyjson_get_len(value_val);
            const char *token_value = hsfv_strndup(allocator, value, value_len);
            if (!token_value) {
                return HSFV_ERR_OUT_OF_MEMORY;
            }
            *out_item = (hsfv_bare_item_t){.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = token_value, .len = value_len}};
        } else if (!strcmp(override_type, "binary")) {
            yyjson_val *value_val = yyjson_obj_get(bare_item_val, "value");
            const char *value = yyjson_get_str(value_val);
            size_t value_len = yyjson_get_len(value_val);
            size_t decoded_len = 0;
            char *decoded = NULL;
            if (value_len > 0) {
                baseencode_error_t err;
                char *decoded_cstr = (char *)base32_decode(value, value_len, &err);
                if (err != SUCCESS) {
                    return err == MEMORY_ALLOCATION ? HSFV_ERR_OUT_OF_MEMORY : HSFV_ERR_INVALID;
                }
                decoded_len = strlen(decoded_cstr);
                decoded = hsfv_strndup(allocator, decoded_cstr, decoded_len);
                if (!decoded) {
                    return HSFV_ERR_OUT_OF_MEMORY;
                }
                free(decoded_cstr);
            }
            *out_item = (hsfv_bare_item_t){.type = HSFV_BARE_ITEM_TYPE_BYTE_SEQ, .byte_seq = {.base = decoded, .len = decoded_len}};
        }
    } break;
    default:
        return HSFV_ERR_INVALID;
    }

    return HSFV_OK;
}

static hsfv_err_t build_expected_key(yyjson_val *expected, hsfv_allocator_t *allocator, hsfv_key_t *out_key)
{
    const char *value = yyjson_get_str(expected);
    size_t value_len = yyjson_get_len(expected);
    const char *base = hsfv_strndup(allocator, value, value_len);
    if (!base) {
        *out_key = (hsfv_key_t){0};
        return HSFV_ERR_OUT_OF_MEMORY;
    }
    *out_key = (hsfv_key_t){.base = base, .len = value_len};
    return HSFV_OK;
}

static hsfv_err_t build_expected_parameters(yyjson_val *expected, hsfv_allocator_t *allocator, hsfv_parameters_t *out_parameters)
{
    hsfv_err_t err;

    if (!yyjson_is_arr(expected)) {
        return HSFV_ERR_INVALID;
    }

    size_t len = yyjson_arr_size(expected);
    if (len == 0) {
        *out_parameters = (hsfv_parameters_t){0};
        return HSFV_OK;
    }

    hsfv_parameter_t *params = (hsfv_parameter_t *)allocator->alloc(allocator, len * sizeof(hsfv_parameter_t));
    if (!params) {
        return HSFV_ERR_OUT_OF_MEMORY;
    }
    *out_parameters = (hsfv_parameters_t){.params = params, .len = 0, .capacity = len};

    size_t idx, max;
    yyjson_val *param_val;
    yyjson_arr_foreach(expected, idx, max, param_val)
    {
        if (!yyjson_is_arr(param_val) || yyjson_arr_size(param_val) != 2) {
            return HSFV_ERR_INVALID;
        }
        yyjson_val *key_val = yyjson_arr_get(param_val, 0);
        yyjson_val *bare_item_val = yyjson_arr_get(param_val, 1);

        hsfv_parameter_t *param = &params[idx];
        err = build_expected_key(key_val, allocator, &param->key);
        if (err) {
            goto error;
        }

        err = build_expected_bare_item(bare_item_val, allocator, &param->value);
        if (err) {
            hsfv_key_deinit(&param->key, allocator);
            goto error;
        }

        out_parameters->len++;
    }

    return HSFV_OK;

error:
    hsfv_parameters_deinit(out_parameters, allocator);
    return err;
}

static hsfv_err_t build_expected_item(yyjson_val *expected, hsfv_allocator_t *allocator, hsfv_item_t *out_item)
{
    hsfv_err_t err;
    const char *input, *input_end;

    if (!yyjson_is_arr(expected) || yyjson_arr_size(expected) != 2) {
        return HSFV_ERR_INVALID;
    }

    yyjson_val *bare_item_val = yyjson_arr_get(expected, 0);
    yyjson_val *parameters_val = yyjson_arr_get(expected, 1);

    out_item->parameters = (hsfv_parameters_t){0};
    err = build_expected_bare_item(bare_item_val, allocator, &out_item->bare_item);
    if (err) {
        goto error;
    }

    err = build_expected_parameters(parameters_val, allocator, &out_item->parameters);
    if (err) {
        goto error;
    }

    return HSFV_OK;

error:
    hsfv_item_deinit(out_item, allocator);
    return err;
}

static hsfv_err_t build_expected_inner_list(yyjson_val *expected, hsfv_allocator_t *allocator, hsfv_inner_list_t *out_inner_list)
{
    if (!yyjson_is_arr(expected) || yyjson_arr_size(expected) != 2) {
        return HSFV_ERR_INVALID;
    }

    yyjson_val *members_val = yyjson_arr_get(expected, 0);
    yyjson_val *parameters_val = yyjson_arr_get(expected, 1);

    size_t n = yyjson_arr_size(members_val);
    hsfv_item_t *items = (hsfv_item_t *)allocator->alloc(allocator, n * sizeof(hsfv_item_t));
    if (!items) {
        return HSFV_ERR_OUT_OF_MEMORY;
    }
    *out_inner_list = hsfv_inner_list_t{.items = items, .len = 0, .capacity = n};

    hsfv_err_t err;
    size_t idx, max;
    yyjson_val *member_val;
    yyjson_arr_foreach(members_val, idx, max, member_val)
    {
        if (!yyjson_is_arr(member_val) || yyjson_arr_size(member_val) != 2) {
            err = HSFV_ERR_INVALID;
            goto error;
        }

        printf("build_expected_inner_list before build_expected_item idx=%zd\n", idx);
        err = build_expected_item(member_val, allocator, &items[idx]);
        printf("build_expected_inner_list after build_expected_item idx=%zd, err=%d\n", idx, err);
        if (err) {
            goto error;
        }
        out_inner_list->len++;
    }

    err = build_expected_parameters(parameters_val, allocator, &out_inner_list->parameters);
    if (err) {
        goto error;
    }

    return HSFV_OK;

error:
    hsfv_inner_list_deinit(out_inner_list, allocator);
    return err;
}

static hsfv_err_t build_expected_list(yyjson_val *expected, hsfv_allocator_t *allocator, hsfv_list_t *out_list)
{
    if (!yyjson_is_arr(expected)) {
        return HSFV_ERR_INVALID;
    }

    size_t n = yyjson_arr_size(expected);
    hsfv_list_member_t *members = (hsfv_list_member_t *)allocator->alloc(allocator, n * sizeof(hsfv_list_member_t));
    if (!members) {
        return HSFV_ERR_OUT_OF_MEMORY;
    }
    *out_list = hsfv_list_t{.members = members, .len = 0, .capacity = n};

    hsfv_err_t err;
    size_t idx, max;
    yyjson_val *list_member_val;
    yyjson_arr_foreach(expected, idx, max, list_member_val)
    {
        if (!yyjson_is_arr(list_member_val) || yyjson_arr_size(list_member_val) != 2) {
            err = HSFV_ERR_INVALID;
            goto error;
        }
        hsfv_list_member_t *member = &members[idx];
        yyjson_val *item_or_inner_list_val = yyjson_arr_get(list_member_val, 0);
        if (yyjson_is_arr(item_or_inner_list_val)) {
            member->type = HSFV_LIST_MEMBER_TYPE_INNER_LIST;
            err = build_expected_inner_list(list_member_val, allocator, &member->inner_list);
            if (err) {
                goto error;
            }
        } else {
            member->type = HSFV_LIST_MEMBER_TYPE_ITEM;
            err = build_expected_item(list_member_val, allocator, &member->item);
            if (err) {
                goto error;
            }
        }
        out_list->len++;
    }

    return HSFV_OK;

error:
    hsfv_list_deinit(out_list, allocator);
    return err;
}

static hsfv_err_t build_expected_dict(yyjson_val *expected, hsfv_allocator_t *allocator, hsfv_dictionary_t *out_dict)
{
    if (!yyjson_is_arr(expected)) {
        return HSFV_ERR_INVALID;
    }

    size_t n = yyjson_arr_size(expected);
    hsfv_dict_member_t *members = (hsfv_dict_member_t *)allocator->alloc(allocator, n * sizeof(hsfv_dict_member_t));
    if (!members) {
        return HSFV_ERR_OUT_OF_MEMORY;
    }
    *out_dict = hsfv_dictionary_t{.members = members, .len = 0, .capacity = n};

    hsfv_err_t err;
    size_t idx, max;
    yyjson_val *dict_member_val;
    yyjson_arr_foreach(expected, idx, max, dict_member_val)
    {
#if 0
        if (!yyjson_is_arr(dict_member_val) || yyjson_arr_size(dict_member_val) != 2) {
            err = HSFV_ERR_INVALID;
            goto error;
        }
        hsfv_list_member_t *member = &members[idx];
        yyjson_val *item_or_inner_list_val = yyjson_arr_get(list_member_val, 0);
        if (yyjson_is_arr(item_or_inner_list_val)) {
            member->type = HSFV_LIST_MEMBER_TYPE_INNER_LIST;
            fprintf(stderr, "inner_list not implemented yet\n");
        } else {
            member->type = HSFV_LIST_MEMBER_TYPE_ITEM;
            err = build_expected_item(list_member_val, allocator, &member->item);
            if (err) {
                goto error;
            }
        }
#endif
        out_dict->len++;
    }

    return HSFV_OK;

error:
    hsfv_dictionary_deinit(out_dict, allocator);
    return err;
}

static hsfv_err_t build_expected_field_value(yyjson_val *expected, hsfv_field_value_type_t field_type, hsfv_allocator_t *allocator,
                                             hsfv_field_value_t *out_value)
{
    switch (field_type) {
    case HSFV_FIELD_VALUE_TYPE_ITEM:
        out_value->type = field_type;
        return build_expected_item(expected, allocator, &out_value->item);
    case HSFV_FIELD_VALUE_TYPE_LIST:
        out_value->type = field_type;
        return build_expected_list(expected, allocator, &out_value->list);
    default:
        return HSFV_ERR_INVALID;
    }
}

static hsfv_err_t get_header_type(yyjson_val *case_obj, hsfv_field_value_type_t *out_type)
{
    const char *header_type = yyjson_get_str(yyjson_obj_get(case_obj, "header_type"));
    if (!strcmp(header_type, "item")) {
        *out_type = HSFV_FIELD_VALUE_TYPE_ITEM;
    } else if (!strcmp(header_type, "dictionary")) {
        *out_type = HSFV_FIELD_VALUE_TYPE_DICTIONARY;
    } else if (!strcmp(header_type, "list")) {
        *out_type = HSFV_FIELD_VALUE_TYPE_LIST;
    } else {
        return HSFV_ERR_INVALID;
    }
    return HSFV_OK;
}

static hsfv_err_t combine_field_lines(yyjson_val *raw, hsfv_allocator_t *allocator, const char **combined, size_t *combined_len)
{
    size_t total_len = 0;
    size_t idx, max;
    yyjson_val *val;
    yyjson_arr_foreach(raw, idx, max, val)
    {
        const char *s = yyjson_get_str(val);
        total_len += yyjson_get_len(val) + (idx < max - 1 ? strlen(", ") : 0);
    }

    char *temp = (char *)allocator->alloc(allocator, total_len);
    if (!temp) {
        return HSFV_ERR_OUT_OF_MEMORY;
    }

    char *p = temp;
    yyjson_arr_foreach(raw, idx, max, val)
    {
        const char *s = yyjson_get_str(val);
        size_t len = yyjson_get_len(val);
        memcpy(p, s, len);
        p += len;
        if (idx < max - 1) {
            memcpy(p, ", ", strlen(", "));
            p += strlen(", ");
        }
    }
    *combined = temp;
    *combined_len = total_len;
    return HSFV_OK;
}

static void run_test_for_json_file(const char *json_rel_path)
{
    const char *test_dir = getenv("HTTPWG_TEST_DIR");
    if (!test_dir) {
        test_dir = "./_deps/httpwg_tests-src";
    }

    char path[PATH_MAX];
    sprintf(path, "%s%s%s", test_dir, PATH_SEPARATOR, json_rel_path);
    printf("=== %s start ===\n", json_rel_path);

    yyjson_read_flag flg = YYJSON_READ_NOFLAG;
    yyjson_read_err err;
    yyjson_doc *doc = yyjson_read_file(path, flg, NULL, &err);
    if (doc) {
        yyjson_val *arr = yyjson_doc_get_root(doc);
        yyjson_arr_iter iter;
        yyjson_arr_iter_init(arr, &iter);
        yyjson_val *case_obj;
        while ((case_obj = yyjson_arr_iter_next(&iter))) {
            const char *name = yyjson_get_str(yyjson_obj_get(case_obj, "name"));
            bool must_fail = yyjson_get_bool(yyjson_obj_get(case_obj, "must_fail"));
            // if (strcmp(name, "two line list")) {
            //     continue;
            // }
            printf("name=%s\n", name);

            hsfv_err_t err;
            hsfv_field_value_type_t field_type;
            err = get_header_type(case_obj, &field_type);
            REQUIRE(err == HSFV_OK);

            yyjson_val *expected = yyjson_obj_get(case_obj, "expected");
            hsfv_allocator_t *allocator = &hsfv_global_allocator;
            hsfv_field_value_t want;
            if (expected) {
                err = build_expected_field_value(expected, field_type, allocator, &want);
                REQUIRE(err == HSFV_OK);
            }

            yyjson_val *raw = yyjson_obj_get(case_obj, "raw");
            REQUIRE(yyjson_is_arr(raw));
            REQUIRE(yyjson_arr_size(raw) >= 1);
            bool multiple_headers = yyjson_arr_size(raw) > 1;
            const char *input;
            size_t input_len;
            if (multiple_headers) {
                REQUIRE(field_type == HSFV_FIELD_VALUE_TYPE_LIST);
                err = combine_field_lines(raw, allocator, &input, &input_len);
                REQUIRE(err == HSFV_OK);
            } else {
                yyjson_val *raw0 = yyjson_arr_get(raw, 0);
                input = yyjson_get_str(raw0);
                input_len = yyjson_get_len(raw0);
            }
            printf("input=%.*s, input_len=%zd\n", (int)input_len, input, input_len);

            hsfv_field_value_t got;
            const char *input_end = input + input_len;
            const char *rest;
            err = hsfv_parse_field_value(&got, field_type, allocator, input, input_end, &rest);
            if (must_fail) {
                CHECK(err != HSFV_OK);
            } else {
                CHECK(err == HSFV_OK);
                CHECK(hsfv_field_value_eq(&got, &want));
                hsfv_field_value_deinit(&got, allocator);
            }
            if (multiple_headers) {
                allocator->free(allocator, (void *)input);
            }
            if (expected) {
                hsfv_field_value_deinit(&want, allocator);
            }
        }
    } else {
        fprintf(stderr, "read error (%u): %s at position: %ld\n", err.code, err.msg, err.pos);
    }
    yyjson_doc_free(doc);
}

TEST_CASE("httpwg tests", "[httpwg]")
{

#define SECTION_HELPER(json_rel_path)                                                                                              \
    SECTION(json_rel_path)                                                                                                         \
    {                                                                                                                              \
        run_test_for_json_file(json_rel_path);                                                                                     \
    }

    SECTION_HELPER("binary.json");
    SECTION_HELPER("boolean.json");
    SECTION_HELPER("list.json");
    SECTION_HELPER("listlist.json");
    SECTION_HELPER("number.json");
    SECTION_HELPER("number-generated.json");
    SECTION_HELPER("string.json");
    SECTION_HELPER("string-generated.json");
    SECTION_HELPER("token.json");
    SECTION_HELPER("token-generated.json");
#undef SECTION_HELPER
}
