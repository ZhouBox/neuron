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

#ifndef NEURON_OPCUA_SERVER_NODE_MANAGER_H
#define NEURON_OPCUA_SERVER_NODE_MANAGER_H

#include <open62541/types.h>

#include "utils/utextend.h"

struct opcua_node {
    char     *name;
    char     *prename;
    UA_NodeId nodeid;

    UT_hash_handle hh;
};

typedef struct opcua_node opcua_node_t;

typedef struct opcua_node_manager {
    opcua_node_t *nodes;
} opcua_node_manager_t;

opcua_node_manager_t *opcua_node_manager_create();

void opcua_node_manager_destroy(opcua_node_manager_t *manager);

opcua_node_t *opcua_node_manager_find(opcua_node_manager_t *manager,
                                      const char           *name);

opcua_node_t * opcua_node_manager_find_by_prename(opcua_node_manager_t *manager,
                                        const char           *prename);

void opcua_node_manager_add(opcua_node_manager_t *manager, opcua_node_t *node);

void opcua_node_manager_remove(opcua_node_manager_t *manager,
                               opcua_node_t         *node);

void opcua_node_manager_clear(opcua_node_manager_t *manager);

void opcua_node_manager_nodeid_clear(opcua_node_manager_t *manager);

#endif // NEURON_OPCUA_SERVER_NODE_MANAGER_H
