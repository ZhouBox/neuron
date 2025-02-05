#include <jansson.h>

#include "schema.h"

int mqtt_schema_validate(const char *schema, mqtt_schema_vt_t **vts,
                         size_t *vts_len)
{
    json_t *root = json_loads(schema, 0, NULL);
    if (root == NULL) {
        return -1;
    }

    *vts_len = 0;
    *vts     = NULL;

    const char *key   = NULL;
    json_t *    value = NULL;

    json_object_foreach(root, key, value)
    {
        if (!json_is_string(value)) {
            json_decref(root);
            if (*vts_len > 0) {
                free(*vts);
            }
            return -1;
        }

        const char *str_val = json_string_value(value);
        if (str_val == NULL) {
            json_decref(root);
            if (*vts_len > 0) {
                free(*vts);
            }
            return -1;
        }

        *vts_len += 1;
        *vts = realloc(*vts, *vts_len * sizeof(mqtt_schema_vt_t));
        mqtt_schema_vt_t *vt = &(*vts)[*vts_len - 1];

        memset(vt, 0, sizeof(mqtt_schema_vt_t));
        strcpy(vt->name, key);

        if (strcmp(str_val, "${timestamp}") == 0) {
            vt->vt = MQTT_SCHEMA_TIMESTAMP;
        } else if (strcmp(str_val, "${node}") == 0) {
            vt->vt = MQTT_SCHEMA_NODE_NAME;
        } else if (strcmp(str_val, "${group}") == 0) {
            vt->vt = MQTT_SCHEMA_GROUP_NAME;
        } else if (strcmp(str_val, "${tags}") == 0) {
            vt->vt = MQTT_SCHEMA_TAGS;
        } else if (strcmp(str_val, "${static_tags}") == 0) {
            vt->vt = MQTT_SCHEMA_STATIC_TAGS;
        } else if (strcmp(str_val, "${tag_values}") == 0) {
            vt->vt = MQTT_SCHEMA_TAGVALUES;
        } else if (strcmp(str_val, "${static_tag_values}") == 0) {
            vt->vt = MQTT_SCHEMA_STATIC_TAGVALUES;
        } else if (strcmp(str_val, "${tag_error_values}") == 0) {
            vt->vt = MQTT_SCHEMA_TAG_ERROR_VALUES;
        } else if (strcmp(str_val, "${tag_errors}") == 0) {
            vt->vt = MQTT_SCHEMA_TAG_ERRORS;
        } else {
            if (str_val[0] == '$' && str_val[1] == '{' &&
                str_val[strlen(str_val) - 1] == '}') {
                json_decref(root);
                free(*vts);
                return -1;
            } else {
                vt->vt = MQTT_SCHEMA_UD;
                strcpy(vt->ud, str_val);
            }
        }
    }

    json_decref(root);
    return 0;
}

int mqtt_schema_encode(char *driver, char *group, neu_json_read_resp_t *tags,
                       mqtt_schema_vt_t *vts, size_t n_vts,
                       mqtt_static_vt_t *s_tags, size_t n_s_tags,
                       char **result_str)
{
    void *root = neu_json_encode_new();

    for (size_t i = 0; i < n_vts; i++) {
        neu_json_elem_t elem = {
            .name = vts[i].name,
        };
        switch (vts[i].vt) {
        case MQTT_SCHEMA_TIMESTAMP:
            elem.t         = NEU_JSON_INT;
            elem.v.val_int = global_timestamp;
            break;
        case MQTT_SCHEMA_NODE_NAME:
            elem.t         = NEU_JSON_STR;
            elem.v.val_str = driver;
            break;
        case MQTT_SCHEMA_GROUP_NAME:
            elem.t         = NEU_JSON_STR;
            elem.v.val_str = group;
            break;
        case MQTT_SCHEMA_TAGS: {
            void *values_array = neu_json_array();

            neu_json_read_resp_tag_t *p_tag = tags->tags;

            for (int j = 0; j < tags->n_tag; j++) {
                neu_json_elem_t tag_elems[2 + NEU_TAG_META_SIZE] = { 0 };

                if (p_tag->error == 0) {
                    tag_elems[0].name      = "name";
                    tag_elems[0].t         = NEU_JSON_STR;
                    tag_elems[0].v.val_str = p_tag->name;

                    tag_elems[1].name      = "value";
                    tag_elems[1].t         = p_tag->t;
                    tag_elems[1].v         = p_tag->value;
                    tag_elems[1].precision = p_tag->precision;

                    for (int k = 0; k < p_tag->n_meta; k++) {
                        tag_elems[2 + k].name = p_tag->metas[k].name;
                        tag_elems[2 + k].t    = p_tag->metas[k].t;
                        tag_elems[2 + k].v    = p_tag->metas[k].value;
                    }

                    values_array = neu_json_encode_array(
                        values_array, tag_elems, 2 + p_tag->n_meta);
                }

                p_tag++;
            }

            elem.t            = NEU_JSON_OBJECT;
            elem.v.val_object = values_array;
            break;
        }
        case MQTT_SCHEMA_TAGVALUES: {
            void *values_array = neu_json_encode_new();

            neu_json_read_resp_tag_t *p_tag = tags->tags;

            for (int j = 0; j < tags->n_tag; j++) {
                neu_json_elem_t tag_elems[1 + NEU_TAG_META_SIZE] = { 0 };

                if (p_tag->error == 0) {
                    tag_elems[0].name = p_tag->name;
                    tag_elems[0].t    = p_tag->t;
                    tag_elems[0].v    = p_tag->value;

                    for (int k = 0; k < p_tag->n_meta; k++) {
                        tag_elems[1 + k].name = p_tag->metas[k].name;
                        tag_elems[1 + k].t    = p_tag->metas[k].t;
                        tag_elems[1 + k].v    = p_tag->metas[k].value;
                    }

                    neu_json_encode_field(values_array, tag_elems,
                                          1 + p_tag->n_meta);
                }

                p_tag++;
            }

            elem.t            = NEU_JSON_OBJECT;
            elem.v.val_object = values_array;
            break;
        }
        case MQTT_SCHEMA_STATIC_TAGS: {
            void *static_array = neu_json_array();

            for (size_t k = 0; k < n_s_tags; k++) {
                neu_json_elem_t tag_elems[2 + NEU_TAG_META_SIZE] = { 0 };

                tag_elems[0].name      = "name";
                tag_elems[0].t         = NEU_JSON_STR;
                tag_elems[0].v.val_str = s_tags[k].name;

                tag_elems[1].name = "value";
                tag_elems[1].t    = s_tags[k].jtype;
                tag_elems[1].v    = s_tags[k].jvalue;

                static_array =
                    neu_json_encode_array(static_array, tag_elems, 2);
            }

            elem.t            = NEU_JSON_OBJECT;
            elem.v.val_object = static_array;

            break;
        }
        case MQTT_SCHEMA_STATIC_TAGVALUES: {
            void *static_array = neu_json_encode_new();

            for (size_t k = 0; k < n_s_tags; k++) {
                neu_json_elem_t tag_elems[1 + NEU_TAG_META_SIZE] = { 0 };

                tag_elems[0].name = s_tags[k].name;
                tag_elems[0].t    = s_tags[k].jtype;
                tag_elems[0].v    = s_tags[k].jvalue;

                neu_json_encode_field(static_array, tag_elems, 1);
            }

            elem.t            = NEU_JSON_OBJECT;
            elem.v.val_object = static_array;

            break;
        }
        case MQTT_SCHEMA_TAG_ERRORS: {
            void *errors_array = neu_json_array();

            neu_json_read_resp_tag_t *p_tag = tags->tags;

            for (int k = 0; k < tags->n_tag; k++) {
                neu_json_elem_t tag_elems[2 + NEU_TAG_META_SIZE] = { 0 };

                if (p_tag->error != 0) {
                    tag_elems[0].name      = "name";
                    tag_elems[0].t         = NEU_JSON_STR;
                    tag_elems[0].v.val_str = p_tag->name;

                    tag_elems[1].name      = "error";
                    tag_elems[1].t         = NEU_JSON_INT;
                    tag_elems[1].v.val_int = p_tag->error;

                    errors_array =
                        neu_json_encode_array(errors_array, tag_elems, 2);
                }

                p_tag++;
            }

            elem.t            = NEU_JSON_OBJECT;
            elem.v.val_object = errors_array;
            break;
        }
        case MQTT_SCHEMA_TAG_ERROR_VALUES: {
            void *errors_array = neu_json_encode_new();

            neu_json_read_resp_tag_t *p_tag = tags->tags;

            for (int k = 0; k < tags->n_tag; k++) {
                neu_json_elem_t tag_elems[1 + NEU_TAG_META_SIZE] = { 0 };

                if (p_tag->error != 0) {
                    tag_elems[0].name      = p_tag->name;
                    tag_elems[0].t         = NEU_JSON_INT;
                    tag_elems[0].v.val_int = p_tag->error;

                    neu_json_encode_field(errors_array, tag_elems, 1);
                }

                p_tag++;
            }

            elem.t            = NEU_JSON_OBJECT;
            elem.v.val_object = errors_array;
            break;
        }
        case MQTT_SCHEMA_UD:
            elem.t         = NEU_JSON_STR;
            elem.v.val_str = vts[i].ud;
            break;
        }

        neu_json_encode_field(root, &elem, 1);
    }

    int ret = neu_json_encode(root, result_str);
    neu_json_decode_free(root);
    return ret;
}

int mqtt_static_validate(const char *static_tags, mqtt_static_vt_t **vts,
                         size_t *vts_len)
{
    json_t *root = json_loads(static_tags, 0, NULL);
    if (root == NULL) {
        return -1;
    }

    *vts_len = 0;
    *vts     = NULL;

    const char *key   = NULL;
    json_t *    value = NULL;

    json_t *child = json_object_get(root, "static_tags");
    if (child == NULL) {
        json_decref(root);
        return -1;
    }

    json_object_foreach(child, key, value)
    {
        *vts_len += 1;
        *vts = realloc(*vts, *vts_len * sizeof(mqtt_static_vt_t));
        mqtt_static_vt_t *vt = &(*vts)[*vts_len - 1];

        memset(vt, 0, sizeof(mqtt_static_vt_t));
        strcpy(vt->name, key);

        if (json_is_string(value)) {
            vt->jtype          = NEU_JSON_STR;
            vt->jvalue.val_str = strdup(json_string_value(value));
        } else if (json_is_real(value)) {
            vt->jtype             = NEU_JSON_DOUBLE;
            vt->jvalue.val_double = json_real_value(value);
        } else if (json_is_integer(value)) {
            vt->jtype          = NEU_JSON_INT;
            vt->jvalue.val_int = json_integer_value(value);
        } else if (json_is_boolean(value)) {
            vt->jtype           = NEU_JSON_BOOL;
            vt->jvalue.val_bool = json_boolean_value(value);
        } else {
            vt->jtype          = NEU_JSON_STR;
            vt->jvalue.val_str = json_dumps(value, JSON_ENCODE_ANY);
        }
    }

    json_decref(root);
    return 0;
}

void mqtt_static_free(mqtt_static_vt_t *vts, size_t vts_len)
{
    for (size_t i = 0; i < vts_len; i++) {
        if (vts[i].jtype == NEU_JSON_STR) {
            free(vts[i].jvalue.val_str);
        }
    }

    free(vts);
}