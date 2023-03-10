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

#include "opcua_node_manager.h"

void node_free(opcua_node_t *node)
{
    free(node->name);
    if (node->prename) {
        free(node->prename);
    }
    free(node);
}

opcua_node_manager_t *opcua_node_manager_create()
{
    opcua_node_manager_t *manager = calloc(1, sizeof(opcua_node_manager_t));
    return manager;
}

void opcua_node_manager_destroy(opcua_node_manager_t *manager)
{
    opcua_node_t *node = NULL, *tmp = NULL;

    HASH_ITER(hh, manager->nodes, node, tmp)
    {
        HASH_DEL(manager->nodes, node);
        node_free(node);
    }

    free(manager);
}

void opcua_node_manager_clear(opcua_node_manager_t *manager)
{
    opcua_node_t *node = NULL, *tmp = NULL;

    HASH_ITER(hh, manager->nodes, node, tmp)
    {
        HASH_DEL(manager->nodes, node);
        node_free(node);
    }

    manager->nodes = NULL;
}

void opcua_node_manager_nodeid_clear(opcua_node_manager_t *manager)
{
    opcua_node_t *node = NULL, *tmp = NULL;

    HASH_ITER(hh, manager->nodes, node, tmp)
    {
        node->nodeid = UA_NODEID_NULL;
    }
}

opcua_node_t *opcua_node_manager_find(opcua_node_manager_t *manager,
                                      const char           *name)
{
    opcua_node_t *node = NULL;

    HASH_FIND_STR(manager->nodes, name, node);
    if (node) {
        return node;
    } else {
        return NULL;
    }
}

opcua_node_t *opcua_node_manager_find_by_prename(opcua_node_manager_t *manager,
                                        const char           *prename)
{
    opcua_node_t *node = NULL, *tmp = NULL;

    HASH_ITER(hh, manager->nodes, node, tmp)
    {
        if (node->prename) {
            if (strcmp(node->prename, prename) == 0) {
                return node;
            }
        }
    }
    return NULL;
}

void opcua_node_manager_add(opcua_node_manager_t *manager, opcua_node_t *node)
{
    opcua_node_t *tmp = NULL;

    HASH_FIND_STR(manager->nodes, node->name, tmp);
    if (!tmp) {
        HASH_ADD_STR(manager->nodes, name, node);
    }
}

void opcua_node_manager_remove(opcua_node_manager_t *manager,
                               opcua_node_t         *node)
{
    opcua_node_t *tmp = NULL;

    HASH_FIND_STR(manager->nodes, node->name, tmp);
    if (tmp) {
        HASH_DEL(manager->nodes, tmp);
        node_free(tmp);
    }
}
