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

#ifndef NEU_PLUGIN_OPCUA_SERVER_H
#define NEU_PLUGIN_OPCUA_SERVER_H

#include <open62541/server.h>
#include <pthread.h>

#include "neuron.h"

#include "opcua_node_manager.h"

struct neu_plugin {
    neu_plugin_common_t   common;
    neu_events_t         *events;
    UA_Server            *server;
    bool                  running;
    int                   port;
    char                 *username;
    char                 *password;
    neu_event_timer_t    *event;
    UA_NodeId             neuronid;
    opcua_node_manager_t *tag_manager;
    opcua_node_manager_t *group_manager;
    opcua_node_manager_t *driver_manager;
    pthread_mutex_t       mutex;
};

#endif // NEU_PLUGIN_OPCUA_SERVER_H