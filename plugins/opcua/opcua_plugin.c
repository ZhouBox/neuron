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

#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/server_config_default.h>

#include "opcua_handle.h"
#include "opcua_node_manager.h"
#include "opcua_plugin.h"

#define UNUSED(X) ((void) X)

static neu_plugin_t *driver_open(void);

static int driver_close(neu_plugin_t *plugin);
static int driver_init(neu_plugin_t *plugin);
static int driver_uninit(neu_plugin_t *plugin);
static int driver_start(neu_plugin_t *plugin);
static int driver_stop(neu_plugin_t *plugin);
static int driver_setting(neu_plugin_t *plugin, const char *config);
static int driver_request(neu_plugin_t *plugin, neu_reqresp_head_t *head,
                          void *data);
static int driver_group_timer(neu_plugin_t *plugin, neu_plugin_group_t *group);
static int driver_validate_tag(neu_plugin_t *plugin, neu_datatag_t *tag);
static int driver_write_tag(neu_plugin_t *plugin, void *req, neu_datatag_t *tag,
                            neu_value_u value);

static int opcua_server_start(neu_plugin_t *plugin);
static int opcua_server_stop(neu_plugin_t *plugin);

static const neu_plugin_intf_funs_t plugin_intf_funs = {
    .open    = driver_open,
    .close   = driver_close,
    .init    = driver_init,
    .uninit  = driver_uninit,
    .start   = driver_start,
    .stop    = driver_stop,
    .setting = driver_setting,
    .request = driver_request,

    .driver.group_timer  = driver_group_timer,
    .driver.validate_tag = driver_validate_tag,
    .driver.write_tag    = driver_write_tag
};

const neu_plugin_module_t neu_plugin_module = {
    .version         = NEURON_PLUGIN_VER_2_0,
    .schema          = "opcuaserver",
    .module_name     = "opcuaserver",
    .module_descr    = "opcua server",
    .module_descr_zh = "opcua服务器",
    .intf_funs       = &plugin_intf_funs,
    .kind            = NEU_PLUGIN_KIND_SYSTEM,
    .type            = NEU_NA_TYPE_APP,
    .display         = true,
    .single          = false,

};

static neu_plugin_t *driver_open(void)
{
    neu_plugin_t *plugin = calloc(1, sizeof(neu_plugin_t));

    if (0 != pthread_mutex_init(&plugin->mutex, NULL)) {
        free(plugin);
        return NULL;
    }

    neu_plugin_common_init(&plugin->common);

    return plugin;
}

static int driver_close(neu_plugin_t *plugin)
{
    pthread_mutex_destroy(&plugin->mutex);
    free(plugin);
    return 0;
}

static int driver_init(neu_plugin_t *plugin)
{
    plugin->events         = neu_event_new();
    plugin->running        = false;
    plugin->port           = -1;
    plugin->server         = NULL;
    plugin->username       = NULL;
    plugin->password       = NULL;
    plugin->tag_manager    = opcua_node_manager_create();
    plugin->group_manager  = opcua_node_manager_create();
    plugin->driver_manager = opcua_node_manager_create();
    plugin->neuronid       = UA_NODEID_NULL;
    return 0;
}

static int driver_uninit(neu_plugin_t *plugin)
{
    plugin->server = NULL;
    neu_event_close(plugin->events);
    opcua_node_manager_destroy(plugin->tag_manager);
    opcua_node_manager_destroy(plugin->group_manager);
    opcua_node_manager_destroy(plugin->driver_manager);
    plugin->neuronid = UA_NODEID_NULL;
    plugin->events   = NULL;
    if (plugin->username) {
        free(plugin->username);
        plugin->username = NULL;
    }
    if (plugin->password) {
        free(plugin->password);
        plugin->password = NULL;
    }
    return 0;
}

static int driver_start(neu_plugin_t *plugin)
{
    plugin->server = UA_Server_new();
    opcua_server_start(plugin);
    return 0;
}

static int driver_stop(neu_plugin_t *plugin)
{
    opcua_server_stop(plugin);
    UA_Server_delete(plugin->server);
    return 0;
}

static int driver_setting(neu_plugin_t *plugin, const char *config)
{
    neu_json_elem_t port      = { .name = "port", .t = NEU_JSON_INT };
    neu_json_elem_t username  = { .name = "username", .t = NEU_JSON_STR };
    neu_json_elem_t password  = { .name = "password", .t = NEU_JSON_STR };
    char           *err_param = NULL;

    int ret =
        neu_parse_param(config, &err_param, 3, &port, &username, &password);
    if (ret != 0) {
        plog_warn(plugin, "config: %s, decode error: %s", config, err_param);
        free(err_param);

        if (username.v.val_str) {
            free(username.v.val_str);
        }

        if (password.v.val_str) {
            free(password.v.val_str);
        }

        return -1;
    }

    plugin->port = port.v.val_int;

    if (plugin->username) {
        free(plugin->username);
        plugin->username = NULL;
    }
    if (plugin->password) {
        free(plugin->password);
        plugin->password = NULL;
    }

    plugin->username = username.v.val_str;
    plugin->password = password.v.val_str;

    return 0;
}
static int driver_request(neu_plugin_t *plugin, neu_reqresp_head_t *head,
                          void *data)
{
    neu_err_code_e error = NEU_ERR_SUCCESS;

    switch (head->type) {
    case NEU_RESP_ERROR:
        break;
    case NEU_REQRESP_TRANS_DATA:
        error = handle_trans_data(plugin, data);
        break;
    case NEU_REQ_SUBSCRIBE_GROUP:
        error = handle_subscribe_group(plugin, data);
        break;
    case NEU_REQ_UNSUBSCRIBE_GROUP:
        error = handle_unsubscribe_group(plugin, data);
        break;
    default:
        error = NEU_ERR_COMMAND_EXECUTION_FAILED;
        break;
    }

    return error;
}
static int driver_group_timer(neu_plugin_t *plugin, neu_plugin_group_t *group)
{
    UNUSED(plugin);
    UNUSED(group);
    return 0;
}
static int driver_validate_tag(neu_plugin_t *plugin, neu_datatag_t *tag)
{
    plog_debug(plugin, "validate tag success, name:%s, address:%s,type:%d ",
               tag->name, tag->address, tag->type);
    return 0;
}
static int driver_write_tag(neu_plugin_t *plugin, void *req, neu_datatag_t *tag,
                            neu_value_u value)
{
    UNUSED(plugin);
    UNUSED(req);
    UNUSED(tag);
    UNUSED(value);
    return 0;
}

static int opcua_server_mainloop_cb(void *usr_data)
{
    neu_plugin_t *plugin = (neu_plugin_t *) usr_data;

    pthread_mutex_lock(&plugin->mutex);
    opcua_node_t *dnode = NULL, *dtmp = NULL;

    HASH_ITER(hh, plugin->driver_manager->nodes, dnode, dtmp)
    {
        if (UA_NodeId_isNull(&dnode->nodeid)) {
            UA_NodeId dnodeid;

            UA_ObjectAttributes d_attr = UA_ObjectAttributes_default;
            d_attr.description         = UA_LOCALIZEDTEXT("en-US", dnode->name);
            d_attr.displayName         = UA_LOCALIZEDTEXT("en-US", dnode->name);
            UA_StatusCode ret          = UA_Server_addObjectNode(
                plugin->server, UA_NODEID_STRING(0, dnode->name),
                plugin->neuronid, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                UA_QUALIFIEDNAME(1, dnode->name),
                UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), d_attr, NULL,
                &dnodeid);

            if (UA_STATUSCODE_GOOD != ret) {
                plog_warn(plugin,
                          "opcua server add driver(%s) object node fail!",
                          dnode->name);
            } else {
                dnode->nodeid = dnodeid;
            }
        }
    }

    opcua_node_t *gnode = NULL, *gtmp = NULL;

    HASH_ITER(hh, plugin->group_manager->nodes, gnode, gtmp)
    {

        if (UA_NodeId_isNull(&gnode->nodeid)) {
            opcua_node_t *pnode =
                opcua_node_manager_find(plugin->driver_manager, gnode->prename);

            char *gnodeid_str =
                calloc(1, strlen(gnode->prename) + strlen(gnode->name) + 2);

            strcat(gnodeid_str, gnode->prename);
            strcat(gnodeid_str, ".");
            strcat(gnodeid_str, gnode->name);

            UA_NodeId gnodeid;

            UA_ObjectAttributes d_attr = UA_ObjectAttributes_default;
            d_attr.description         = UA_LOCALIZEDTEXT("en-US", gnode->name);
            d_attr.displayName         = UA_LOCALIZEDTEXT("en-US", gnode->name);
            UA_StatusCode ret          = UA_Server_addObjectNode(
                plugin->server, UA_NODEID_STRING(0, gnodeid_str), pnode->nodeid,
                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                UA_QUALIFIEDNAME(1, gnode->name),
                UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), d_attr, NULL,
                &gnodeid);

            free(gnodeid_str);

            if (UA_STATUSCODE_GOOD != ret) {
                plog_warn(plugin,
                          "opcua server add group(%s) object node fail!",
                          gnode->name);
            } else {
                gnode->nodeid = gnodeid;
            }
        }
    }

    pthread_mutex_unlock(&plugin->mutex);

    UA_Boolean wait_internal = true;

    UA_Server_run_iterate(plugin->server, wait_internal);

    return 0;
}

static int opcua_server_start(neu_plugin_t *plugin)
{
    if (plugin->running || plugin->port == -1 || !plugin->username ||
        !plugin->password) {
        return -1;
    }

    UA_ServerConfig *config = UA_Server_getConfig(plugin->server);
    UA_ServerConfig_setMinimal(config, plugin->port, NULL);

    config->accessControl.clear(&config->accessControl);
    static UA_UsernamePasswordLogin logins[1] = {};
    logins[0].username                        = UA_STRING(plugin->username);
    logins[0].password                        = UA_STRING(plugin->password);

    UA_StatusCode ret = UA_AccessControl_default(
        config, false, NULL,
        &config->securityPolicies[config->securityPoliciesSize - 1].policyUri,
        1, logins);
    if (ret != UA_STATUSCODE_GOOD) {
        plog_warn(plugin, "opcua server access setting fail!");
        return -1;
    }

    UA_NodeId neuronid;

    UA_ObjectAttributes o_attr = UA_ObjectAttributes_default;
    o_attr.description = UA_LOCALIZEDTEXT("en-US", "neuron object node");
    o_attr.displayName = UA_LOCALIZEDTEXT("en-US", "neuron");
    ret                = UA_Server_addObjectNode(
        plugin->server, UA_NODEID_STRING(0, "neuron"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES), UA_QUALIFIEDNAME(0, "neuron"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), o_attr, NULL, &neuronid);

    if (UA_STATUSCODE_GOOD != ret) {
        plog_warn(plugin, "opcua server add neuron top object node fail!");
        return -1;
    }

    plugin->neuronid = neuronid;

    ret = UA_Server_run_startup(plugin->server);
    if (ret != UA_STATUSCODE_GOOD) {
        plog_warn(plugin, "opcua server run startup fail!");
        return -1;
    }

    neu_event_timer_param_t timer_loop = {
        .second      = 0,
        .millisecond = 10,
        .cb          = opcua_server_mainloop_cb,
        .type        = NEU_EVENT_TIMER_BLOCK,
        .usr_data    = (void *) plugin,
    };

    plugin->event = neu_event_add_timer(plugin->events, timer_loop);

    plugin->running = true;

    plugin->common.link_state = NEU_NODE_LINK_STATE_CONNECTED;

    plog_info(plugin, "opcua server start! port:%d", plugin->port);

    return 0;
}

static int opcua_server_stop(neu_plugin_t *plugin)
{
    if (plugin->running) {
        plugin->running = false;
        neu_event_del_timer(plugin->events, plugin->event);
        plugin->event             = NULL;
        plugin->common.link_state = NEU_NODE_LINK_STATE_DISCONNECTED;
        opcua_node_manager_clear(plugin->tag_manager);
        opcua_node_manager_nodeid_clear(plugin->group_manager);
        opcua_node_manager_nodeid_clear(plugin->driver_manager);

        UA_Boolean    deleteReferences = true;
        UA_StatusCode ret              = UA_Server_deleteNode(
            plugin->server, plugin->neuronid, deleteReferences);
        if (UA_STATUSCODE_GOOD != ret) {
            plog_warn(plugin, "del top neuron node fail!");
        }
        ret = UA_Server_run_shutdown(plugin->server);
        if (UA_STATUSCODE_GOOD != ret) {
            plog_warn(plugin, "shutdown server fail!");
        }
        plugin->neuronid = UA_NODEID_NULL;
        plog_info(plugin, "opcua server stop!");
    }
    return 0;
}
