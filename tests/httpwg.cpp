#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>
#include <linux/limits.h>
extern "C" {
#include <baseencode.h>
}
#include <yyjson.h>

#define PATH_SEPARATOR "/"

static hsfv_err_t build_expected_item(yyjson_val *expected, hsfv_allocator_t *allocator, hsfv_item_t *out_item)
{
    hsfv_err_t err;
    const char *input, *input_end;

    if (!yyjson_is_arr(expected) || yyjson_arr_size(expected) != 2) {
        return HSFV_ERR_INVALID;
    }
    yyjson_val *bare_item_val = yyjson_arr_get(expected, 0);
    yyjson_type bare_item_type = yyjson_get_type(bare_item_val);
    switch (bare_item_type) {
    case YYJSON_TYPE_BOOL: {
        bool b = yyjson_get_bool(bare_item_val);
        *out_item = {.bare_item = {.type = HSFV_BARE_ITEM_TYPE_BOOLEAN, .boolean = b}};
    } break;
    case YYJSON_TYPE_NUM: {
        if (yyjson_is_int(bare_item_val)) {
            *out_item = hsfv_item_t{
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_INTEGER, .integer = yyjson_get_sint(bare_item_val)},
            };
        } else {
            *out_item = hsfv_item_t{
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_DECIMAL, .decimal = yyjson_get_real(bare_item_val)},
            };
        }
    } break;
    case YYJSON_TYPE_STR: {
        const char *value = yyjson_get_str(bare_item_val);
        const char *str_value = hsfv_strndup(allocator, value, strlen(value));
        if (!str_value) {
            return HSFV_ERR_OUT_OF_MEMORY;
        }
        *out_item = hsfv_item_t{
            .bare_item = {.type = HSFV_BARE_ITEM_TYPE_STRING, .string = {.base = str_value, .len = strlen(value)}},
        };
    } break;
    case YYJSON_TYPE_OBJ: {
        const char *override_type = yyjson_get_str(yyjson_obj_get(bare_item_val, "__type"));
        if (!strcmp(override_type, "token")) {
            const char *value = yyjson_get_str(yyjson_obj_get(bare_item_val, "value"));
            const char *token_value = hsfv_strndup(allocator, value, strlen(value));
            if (!token_value) {
                return HSFV_ERR_OUT_OF_MEMORY;
            }
            *out_item = hsfv_item_t{
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_TOKEN, .token = {.base = token_value, .len = strlen(value)}},
            };
        } else if (!strcmp(override_type, "binary")) {
            const char *value = yyjson_get_str(yyjson_obj_get(bare_item_val, "value"));
            size_t decoded_len = 0;
            char *decoded = NULL;
            if (strlen(value) > 0) {
                baseencode_error_t err;
                char *decoded_cstr = (char *)base32_decode(value, strlen(value), &err);
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
            *out_item = hsfv_item_t{
                .bare_item = {.type = HSFV_BARE_ITEM_TYPE_BYTE_SEQ, .byte_seq = {.base = decoded, .len = decoded_len}},
            };
        }
    } break;
    default:
        return HSFV_ERR_INVALID;
    }

    return HSFV_OK;
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
            fprintf(stderr, "inner_list not implemented yet\n");
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

static void run_test_for_json_file(const char *json_rel_path)
{
    const char *test_dir = getenv("HTTPWG_TEST_DIR");

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
            printf("name=%s\n", name);

            hsfv_err_t err;
            hsfv_field_value_type_t field_type;
            err = get_header_type(case_obj, &field_type);
            REQUIRE(err == HSFV_OK);

            yyjson_val *expected = yyjson_obj_get(case_obj, "expected");
            hsfv_allocator_t *alc = &hsfv_global_allocator;
            hsfv_field_value_t want;
            if (expected) {
                err = build_expected_field_value(expected, field_type, alc, &want);
                REQUIRE(err == HSFV_OK);
            }

            yyjson_val *raw = yyjson_obj_get(case_obj, "raw");
            yyjson_arr_iter raw_iter;
            yyjson_arr_iter_init(raw, &raw_iter);
            yyjson_val *raw_elem;
            while ((raw_elem = yyjson_arr_iter_next(&raw_iter))) {
                const char *input = yyjson_get_str(raw_elem);
                printf("input=%s\n", input);

                hsfv_field_value_t got;
                const char *input_end = input + strlen(input);
                const char *rest;
                err = hsfv_parse_field_value(&got, field_type, alc, input, input_end, &rest);
                if (must_fail) {
                    CHECK(err != HSFV_OK);
                } else {
                    CHECK(err == HSFV_OK);
                    CHECK(hsfv_field_value_eq(&got, &want));
                    hsfv_field_value_deinit(&got, alc);
                }
            }
            if (expected) {
                hsfv_field_value_deinit(&want, alc);
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
    SECTION_HELPER("number.json");
    SECTION_HELPER("number-generated.json");
    SECTION_HELPER("string.json");
    SECTION_HELPER("token.json");
#undef SECTION_HELPER
}
