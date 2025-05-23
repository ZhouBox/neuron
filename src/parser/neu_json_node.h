/**
 * NEURON IIoT System for Industry 4.0
 * Copyright (C) 2020-2022 EMQ Technologies Co., Ltd All rights reserved.
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

/*
 * DO NOT EDIT THIS FILE MANUALLY!
 * It was automatically generated by `json-autotype`.
 */

#ifndef _NEU_JSON_API_NEU_JSON_NODE_H_
#define _NEU_JSON_API_NEU_JSON_NODE_H_

#include "define.h"
#include "json/json.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *name;
    char *plugin;
    char *setting;
} neu_json_add_node_req_t;

int  neu_json_encode_add_node_req(void *json_object, void *param);
int  neu_json_decode_add_node_req_json(void *                   json_obj,
                                       neu_json_add_node_req_t *req);
int  neu_json_decode_add_node_req(char *buf, neu_json_add_node_req_t **result);
void neu_json_decode_add_node_req_fini(neu_json_add_node_req_t *req);
void neu_json_decode_add_node_req_free(neu_json_add_node_req_t *req);

typedef struct {
    char *name;
} neu_json_del_node_req_t;

int  neu_json_encode_del_node_req(void *json_object, void *param);
int  neu_json_decode_del_node_req(char *buf, neu_json_del_node_req_t **result);
void neu_json_decode_del_node_req_free(neu_json_del_node_req_t *req);

typedef struct {
    int64_t  running;
    int64_t  link;
    int64_t  rtt;
    uint16_t sub_group_count;
    bool     is_driver;
    char *   log_level;
    char *   core_level;
} neu_json_get_node_state_resp_t;

int neu_json_encode_get_node_state_resp(void *json_object, void *param);

typedef struct {
    char *   name;
    int64_t  running;
    int64_t  link;
    int64_t  rtt;
    uint16_t sub_group_count;
    bool     is_driver;
    char *   log_level;
} neu_json_get_nodes_state_t;

typedef struct {
    uint16_t                    n_node;
    neu_json_get_nodes_state_t *nodes;
    char *                      core_level;
} neu_json_get_nodes_state_resp_t;

int neu_json_encode_get_nodes_state_resp(void *json_object, void *param);

typedef struct {
    char *name;
    char *plugin;
} neu_json_get_nodes_resp_node_t;

typedef struct {
    int                             n_node;
    neu_json_get_nodes_resp_node_t *nodes;
} neu_json_get_nodes_resp_t;

int  neu_json_encode_get_nodes_resp(void *json_object, void *param);
int  neu_json_decode_get_nodes_resp(char *                      buf,
                                    neu_json_get_nodes_resp_t **result);
int  neu_json_decode_get_nodes_resp_json(void *                      json_obj,
                                         neu_json_get_nodes_resp_t **result);
void neu_json_decode_get_nodes_resp_free(neu_json_get_nodes_resp_t *result);

typedef struct {
    char *name;
    char *new_name;
} neu_json_update_node_req_t;

int  neu_json_encode_update_node_req(void *json_object, void *param);
int  neu_json_decode_update_node_req(char *                       buf,
                                     neu_json_update_node_req_t **result);
void neu_json_decode_update_node_req_free(neu_json_update_node_req_t *req);

typedef struct {
    char *  node;
    int64_t cmd;
} neu_json_node_ctl_req_t;

int  neu_json_encode_node_ctl_req(void *json_object, void *param);
int  neu_json_decode_node_ctl_req(char *buf, neu_json_node_ctl_req_t **result);
void neu_json_decode_node_ctl_req_free(neu_json_node_ctl_req_t *req);

typedef struct {
    char *node;
    char *setting;
} neu_json_node_setting_req_t;

int  neu_json_encode_node_setting_req(void *json_object, void *param);
int  neu_json_decode_node_setting_req(char *                        buf,
                                      neu_json_node_setting_req_t **result);
void neu_json_decode_node_setting_req_free(neu_json_node_setting_req_t *req);

typedef struct {
    char *node;
    char *setting;
} neu_json_get_node_setting_resp_t;

int neu_json_encode_get_node_setting_resp(void *json_object, void *param);

#ifdef __cplusplus
}
#endif

#endif
