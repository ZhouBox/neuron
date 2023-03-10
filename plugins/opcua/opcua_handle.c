/**
 * NEURON IIoT System for Industry 4.0
 * Copyright (C) 2020-2023 EMQ Technologies Co., Ltd All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#include <open62541/client_subscriptions.h>
#include <open62541/types.h>

#include "opcua_handle.h"
#include "opcua_plugin.h"

#define UNUSED(X) ((void) X)

typedef struct opcua_monitored_item_ctx {
    char                 *driver;
    char                 *group;
    neu_resp_tag_value_t *tag;
    neu_plugin_t         *plugin;
} opcua_monitored_item_ctx_t;

static void data_change_notify_cb(UA_Server       *server,
                                  UA_UInt32        monitored_item_id,
                                  void            *monitored_item_ctx,
                                  const UA_NodeId *nodeid, void *node_ctx,
                                  UA_UInt32           attribute_id,
                                  const UA_DataValue *value)
{
    UNUSED(server);
    UNUSED(monitored_item_id);
    UNUSED(nodeid);
    UNUSED(node_ctx);
    UNUSED(attribute_id);

    neu_reqresp_head_t  header = { 0 };
    neu_req_write_tag_t cmd    = { 0 };

    header.ctx  = monitored_item_ctx;
    header.type = NEU_REQ_WRITE_TAG;

    opcua_monitored_item_ctx_t *mctx =
        (opcua_monitored_item_ctx_t *) monitored_item_ctx;

    if (!value->hasValue) {
        plog_warn(mctx->plugin, "monitored item has no value!");
        return;
    }

    bool s_flag = true;

    strcpy(cmd.driver, mctx->driver);
    strcpy(cmd.group, mctx->group);
    strcpy(cmd.tag, mctx->tag->tag);

    switch (mctx->tag->value.type) {
    case NEU_TYPE_INT8: {
        cmd.value.type     = mctx->tag->value.type;
        cmd.value.value.i8 = *(int8_t *) value->value.data;
        break;
    }
    case NEU_TYPE_UINT8: {
        cmd.value.type     = mctx->tag->value.type;
        cmd.value.value.u8 = *(uint8_t *) value->value.data;
        break;
    }

    case NEU_TYPE_INT16: {
        cmd.value.type      = mctx->tag->value.type;
        cmd.value.value.i16 = *(int16_t *) value->value.data;
        break;
    }
    case NEU_TYPE_WORD:
    case NEU_TYPE_UINT16: {
        cmd.value.type      = mctx->tag->value.type;
        cmd.value.value.u16 = *(uint16_t *) value->value.data;
        break;
    }

    case NEU_TYPE_INT32: {
        cmd.value.type      = mctx->tag->value.type;
        cmd.value.value.i32 = *(int32_t *) value->value.data;
        break;
    }
    case NEU_TYPE_DWORD:
    case NEU_TYPE_UINT32: {
        cmd.value.type      = mctx->tag->value.type;
        cmd.value.value.i32 = *(int32_t *) value->value.data;
        break;
    }

    case NEU_TYPE_INT64: {
        cmd.value.type      = mctx->tag->value.type;
        cmd.value.value.i64 = *(int64_t *) value->value.data;
        break;
    }
    case NEU_TYPE_LWORD:
    case NEU_TYPE_UINT64: {
        cmd.value.type      = mctx->tag->value.type;
        cmd.value.value.u64 = *(uint64_t *) value->value.data;
        break;
    }

    case NEU_TYPE_FLOAT: {
        cmd.value.type      = mctx->tag->value.type;
        cmd.value.value.f32 = *(float *) value->value.data;
        break;
    }

    case NEU_TYPE_DOUBLE: {
        cmd.value.type      = mctx->tag->value.type;
        cmd.value.value.d64 = *(double *) value->value.data;
        break;
    }
    case NEU_TYPE_BIT:
    case NEU_TYPE_BOOL: {
        cmd.value.type          = mctx->tag->value.type;
        cmd.value.value.boolean = *(bool *) value->value.data;
        break;
    }

    case NEU_TYPE_STRING: {
        cmd.value.type = mctx->tag->value.type;
        UA_String *t   = (UA_String *) value->value.data;
        memcpy(cmd.value.value.str, t->data, t->length);
        break;
    }

    case NEU_TYPE_BYTES: {
        cmd.value.type = mctx->tag->value.type;
        UA_String *t   = (UA_String *) value->value.data;
        memcpy(cmd.value.value.bytes, t->data, t->length);
        break;
    }

    default:
        s_flag = false;
        plog_error(mctx->plugin, "invalid tag value type: %d",
                   mctx->tag->value.type);
    }

    if (s_flag && 0 != neu_plugin_op(mctx->plugin, header, &cmd)) {
        plog_error(mctx->plugin, "neu_plugin_op(NEU_REQ_WRITE_TAG) fail");
    }
}

void do_tag_node(neu_plugin_t *plugin, char *driver, char *group,
                 opcua_node_t *group_node, neu_resp_tag_value_t *tag,
                 UA_VariableAttributes *v_attr)
{

    neu_adapter_update_metric_cb_t update_metric =
        plugin->common.adapter_callbacks->update_metric;

    char *tnodeid_str =
        calloc(1, strlen(driver) + strlen(group) + strlen(tag->tag) + 3);

    strcat(tnodeid_str, driver);
    strcat(tnodeid_str, ".");
    strcat(tnodeid_str, group);

    char *gnodeid_str = strdup(tnodeid_str);

    strcat(tnodeid_str, ".");
    strcat(tnodeid_str, tag->tag);

    opcua_node_t *tag_node =
        opcua_node_manager_find(plugin->tag_manager, tnodeid_str);
    if (!tag_node) {

        UA_NodeId tagnodeid;

        v_attr->description = UA_LOCALIZEDTEXT("en-US", tag->tag);
        v_attr->displayName = UA_LOCALIZEDTEXT("en-US", tag->tag);
        v_attr->accessLevel =
            UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        UA_StatusCode ret = UA_Server_addVariableNode(
            plugin->server, UA_NODEID_STRING(0, tnodeid_str),
            group_node->nodeid, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(1, tag->tag),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), *v_attr, NULL,
            &tagnodeid);

        if (UA_STATUSCODE_GOOD != ret) {
            free(tnodeid_str);
            free(gnodeid_str);
            plog_warn(plugin,
                      "opcua server add variable node fail! "
                      "group:%s tag:%s",
                      group, tag->tag);
            return;
        }

        UA_MonitoredItemCreateRequest mon_req =
            UA_MonitoredItemCreateRequest_default(tagnodeid);

        opcua_monitored_item_ctx_t *mctx =
            calloc(1, sizeof(opcua_monitored_item_ctx_t));

        neu_resp_tag_value_t *t_tag = calloc(1, sizeof(neu_resp_tag_value_t));

        memcpy(t_tag, tag, sizeof(neu_resp_tag_value_t));

        mctx->driver = strdup(driver);
        mctx->group  = strdup(group);
        mctx->plugin = plugin;
        mctx->tag    = t_tag;

        mon_req.requestedParameters.samplingInterval = 100;

        UA_MonitoredItemCreateResult mret =
            UA_Server_createDataChangeMonitoredItem(
                plugin->server, UA_TIMESTAMPSTORETURN_SOURCE, mon_req, mctx,
                data_change_notify_cb);

        if (mret.statusCode != UA_STATUSCODE_GOOD) {
            plog_warn(
                plugin,
                "opcua server add data change (tag:%s) monitored item fail!",
                tnodeid_str);
        }

        tag_node          = calloc(1, sizeof(opcua_node_t));
        tag_node->nodeid  = tagnodeid;
        tag_node->name    = tnodeid_str;
        tag_node->prename = gnodeid_str;

        opcua_node_manager_add(plugin->tag_manager, tag_node);

    } else {
        free(tnodeid_str);
        free(gnodeid_str);
        UA_StatusCode ret = UA_Server_writeValue(
            plugin->server, tag_node->nodeid, v_attr->value);

        if (UA_STATUSCODE_GOOD != ret) {
            plog_warn(plugin,
                      "opcua server write variable node fail! "
                      "group:%s "
                      "tag:%s",
                      group, tag->tag);
            return;
        }
    }

    update_metric(plugin->common.adapter, NEU_METRIC_RECV_MSGS_TOTAL, 1, NULL);
}

int handle_trans_data(neu_plugin_t             *plugin,
                      neu_reqresp_trans_data_t *trans_data)
{
    if (plugin->running && !UA_NodeId_isNull(&plugin->neuronid)) {

        char *driver = trans_data->driver;
        char *group  = trans_data->group;

        char *gnodeid_str = calloc(1, strlen(driver) + strlen(group) + 2);
        strcat(gnodeid_str, driver);
        strcat(gnodeid_str, ".");
        strcat(gnodeid_str, group);

        pthread_mutex_lock(&plugin->mutex);
        opcua_node_t *group_node =
            opcua_node_manager_find(plugin->group_manager, gnodeid_str);
        free(gnodeid_str);
        if (group_node) {
            if (!UA_NodeId_isNull(&group_node->nodeid)) {

                for (uint16_t i = 0; i < trans_data->n_tag; ++i) {
                    bool                  tag_continue = false;
                    neu_resp_tag_value_t *tag          = &trans_data->tags[i];
                    UA_VariableAttributes v_attr =
                        UA_VariableAttributes_default;
                    switch (tag->value.type) {
                    case NEU_TYPE_INT8: {
                        UA_SByte tv = tag->value.value.i8;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_SBYTE]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }
                    case NEU_TYPE_UINT8: {
                        UA_Byte tv = tag->value.value.u8;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_BYTE]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }

                    case NEU_TYPE_INT16: {
                        UA_Int16 tv = tag->value.value.i16;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_INT16]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }
                    case NEU_TYPE_WORD:
                    case NEU_TYPE_UINT16: {
                        UA_UInt16 tv = tag->value.value.u16;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_UINT16]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }

                    case NEU_TYPE_INT32: {
                        UA_Int32 tv = tag->value.value.i32;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_INT32]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }
                    case NEU_TYPE_DWORD:
                    case NEU_TYPE_UINT32: {
                        UA_UInt32 tv = tag->value.value.u32;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_UINT32]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }

                    case NEU_TYPE_INT64: {
                        UA_Int64 tv = tag->value.value.i64;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_INT64]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }
                    case NEU_TYPE_LWORD:
                    case NEU_TYPE_UINT64: {
                        UA_UInt64 tv = tag->value.value.u32;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_UINT64]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }

                    case NEU_TYPE_FLOAT: {
                        UA_Float tv = tag->value.value.f32;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_FLOAT]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }

                    case NEU_TYPE_DOUBLE: {
                        UA_Double tv = tag->value.value.d64;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_DOUBLE]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }
                    case NEU_TYPE_BIT:
                    case NEU_TYPE_BOOL: {
                        UA_Boolean tv = tag->value.value.boolean;
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_BOOLEAN]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }

                    case NEU_TYPE_STRING: {
                        UA_String tv = UA_STRING(tag->value.value.str);
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_STRING]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }

                    case NEU_TYPE_BYTES: {
                        UA_String tv =
                            UA_STRING((char *) tag->value.value.bytes);
                        UA_Variant_setScalar(&v_attr.value, &tv,
                                             &UA_TYPES[UA_TYPES_STRING]);
                        do_tag_node(plugin, driver, group, group_node, tag,
                                    &v_attr);
                        break;
                    }

                    default:
                        tag_continue = true;
                        plog_warn(
                            plugin,
                            "opcua server add variable node type no define! "
                            "group:%s tag:%s type:%d",
                            group, tag->tag, tag->value.type);
                        break;
                    }

                    if (tag_continue) {
                        continue;
                    }
                }
            }
        }
    }
    pthread_mutex_unlock(&plugin->mutex);
    return NEU_ERR_SUCCESS;
}

int handle_subscribe_group(neu_plugin_t *plugin, neu_req_subscribe_t *sub_info)
{

    char *group  = sub_info->group;
    char *driver = sub_info->driver;

    pthread_mutex_lock(&plugin->mutex);

    opcua_node_t *driver_node =
        opcua_node_manager_find(plugin->driver_manager, driver);
    if (!driver_node) {

        if (plugin->running) {
            UA_NodeId nodeid;

            UA_ObjectAttributes o_attr = UA_ObjectAttributes_default;
            o_attr.description         = UA_LOCALIZEDTEXT("en-US", driver);
            o_attr.displayName         = UA_LOCALIZEDTEXT("en-US", driver);
            UA_StatusCode ret          = UA_Server_addObjectNode(
                plugin->server, UA_NODEID_STRING(0, driver), plugin->neuronid,
                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                UA_QUALIFIEDNAME(1, driver),
                UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), o_attr, NULL,
                &nodeid);

            if (UA_STATUSCODE_GOOD != ret) {
                plog_warn(plugin,
                          "opcua server add driver:%s object node fail!",
                          driver);
                pthread_mutex_unlock(&plugin->mutex);
                return NEU_ERR_COMMAND_EXECUTION_FAILED;
            }

            driver_node         = calloc(1, sizeof(opcua_node_t));
            driver_node->nodeid = nodeid;
            driver_node->name   = strdup(driver);

            opcua_node_manager_add(plugin->driver_manager, driver_node);

        } else {
            driver_node         = calloc(1, sizeof(opcua_node_t));
            driver_node->nodeid = UA_NODEID_NULL;
            driver_node->name   = strdup(driver);

            opcua_node_manager_add(plugin->driver_manager, driver_node);
        }
    }

    opcua_node_t *group_node =
        opcua_node_manager_find(plugin->group_manager, group);
    if (!group_node) {

        char *gnodeid_str = calloc(1, strlen(driver) + strlen(group) + 2);
        strcat(gnodeid_str, driver);
        strcat(gnodeid_str, ".");
        strcat(gnodeid_str, group);

        if (plugin->running) {

            if (!UA_NodeId_isNull(&driver_node->nodeid)) {

                UA_NodeId nodeid;

                UA_ObjectAttributes o_attr = UA_ObjectAttributes_default;
                o_attr.description         = UA_LOCALIZEDTEXT("en-US", group);
                o_attr.displayName         = UA_LOCALIZEDTEXT("en-US", group);
                UA_StatusCode ret          = UA_Server_addObjectNode(
                    plugin->server, UA_NODEID_STRING(0, gnodeid_str),
                    driver_node->nodeid,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                    UA_QUALIFIEDNAME(1, group),
                    UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), o_attr, NULL,
                    &nodeid);

                if (UA_STATUSCODE_GOOD != ret) {
                    plog_warn(plugin,
                              "opcua server add group:%s object node fail!",
                              group);
                    pthread_mutex_unlock(&plugin->mutex);
                    return NEU_ERR_COMMAND_EXECUTION_FAILED;
                }

                group_node          = calloc(1, sizeof(opcua_node_t));
                group_node->nodeid  = nodeid;
                group_node->name    = gnodeid_str;
                group_node->prename = strdup(driver);

                opcua_node_manager_add(plugin->group_manager, group_node);
            } else {
                group_node          = calloc(1, sizeof(opcua_node_t));
                group_node->nodeid  = UA_NODEID_NULL;
                group_node->name    = gnodeid_str;
                group_node->prename = strdup(driver);

                opcua_node_manager_add(plugin->group_manager, group_node);
            }

        } else {
            group_node          = calloc(1, sizeof(opcua_node_t));
            group_node->nodeid  = UA_NODEID_NULL;
            group_node->name    = gnodeid_str;
            group_node->prename = strdup(driver);

            opcua_node_manager_add(plugin->group_manager, group_node);
        }
    }
    pthread_mutex_unlock(&plugin->mutex);
    return NEU_ERR_SUCCESS;
}

int handle_unsubscribe_group(neu_plugin_t          *plugin,
                             neu_req_unsubscribe_t *unsub_info)
{
    plog_info(plugin, "del sub driver:%s group:%s", unsub_info->driver,
              unsub_info->group);
    char *driver = unsub_info->driver;
    char *group  = unsub_info->group;

    char *gnodeid_str = calloc(1, strlen(driver) + strlen(group) + 2);
    strcat(gnodeid_str, driver);
    strcat(gnodeid_str, ".");
    strcat(gnodeid_str, group);

    pthread_mutex_lock(&plugin->mutex);

    opcua_node_t *group_node =
        opcua_node_manager_find(plugin->group_manager, gnodeid_str);
    free(gnodeid_str);
    if (group_node) {
        if (plugin->running && !UA_NodeId_isNull(&group_node->nodeid)) {
            UA_Boolean    deleteReferences = true;
            UA_StatusCode ret              = UA_Server_deleteNode(
                plugin->server, group_node->nodeid, deleteReferences);
            if (UA_STATUSCODE_GOOD != ret) {
                plog_warn(plugin, "del group(%s) node fail!", group);
                pthread_mutex_unlock(&plugin->mutex);
                return NEU_ERR_COMMAND_EXECUTION_FAILED;
            }
        }
        opcua_node_manager_remove(plugin->group_manager, group_node);
    }

    opcua_node_t *had =
        opcua_node_manager_find_by_prename(plugin->group_manager, driver);

    if (!had) {
        opcua_node_t *driver_node =
            opcua_node_manager_find(plugin->driver_manager, driver);
        if (driver_node && !UA_NodeId_isNull(&driver_node->nodeid)) {
            UA_Boolean    deleteReferences = true;
            UA_StatusCode ret              = UA_Server_deleteNode(
                plugin->server, driver_node->nodeid, deleteReferences);
            if (UA_STATUSCODE_GOOD != ret) {
                plog_warn(plugin, "del driver(%s) node fail!", driver);
                pthread_mutex_unlock(&plugin->mutex);
                return NEU_ERR_COMMAND_EXECUTION_FAILED;
            }
        }
        opcua_node_manager_remove(plugin->driver_manager, driver_node);
    }

    pthread_mutex_unlock(&plugin->mutex);
    return NEU_ERR_SUCCESS;
}