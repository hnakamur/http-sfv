#include "hsfv.h"
#include <catch2/catch_test_macros.hpp>
#include <linux/limits.h>
#include <yyjson.h>

#define PATH_SEPARATOR '/'

static hsfv_err_t build_expected_item(yyjson_val *expected, hsfv_item_t *out_item)
{
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
    default:
        return HSFV_ERR_INVALID;
    }

    return HSFV_OK;
}

static hsfv_err_t build_expected_field_value(yyjson_val *expected, hsfv_field_value_type_t field_type, hsfv_allocator_t *allocator,
                                             hsfv_field_value_t *out_value)
{
    switch (field_type) {
    case HSFV_FIELD_VALUE_TYPE_ITEM:
        out_value->type = field_type;
        return build_expected_item(expected, &out_value->item);
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

TEST_CASE("httpwg tests", "[httpwg]")
{
    const char *test_dir = getenv("HTTPWG_TEST_DIR");

    SECTION("boolean.json")
    {
        char path[PATH_MAX];
        sprintf(path, "%s%c%s", test_dir, PATH_SEPARATOR, "boolean.json");

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

                    hsfv_field_value_t got;
                    const char *input_end = input + strlen(input);
                    const char *rest;
                    err = hsfv_parse_field_value(&got, field_type, alc, input, input_end, &rest);
                    if (must_fail) {
                        CHECK(err != HSFV_OK);
                    } else {
                        CHECK(err == HSFV_OK);
                        CHECK(hsfv_field_value_eq(&got, &want));
                    }
                    hsfv_field_value_deinit(&got, alc);
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
}
