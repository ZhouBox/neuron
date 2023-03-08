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

#ifndef NEU_PLUGIN_OPCUA_SERVER_HANDLE_H
#define NEU_PLUGIN_OPCUA_SERVER_HANDLE_H

#include "neuron.h"

int handle_trans_data(neu_plugin_t             *plugin,
                      neu_reqresp_trans_data_t *trans_data);

int handle_subscribe_group(neu_plugin_t *plugin, neu_req_subscribe_t *sub_info);
int handle_unsubscribe_group(neu_plugin_t          *plugin,
                             neu_req_unsubscribe_t *unsub_info);

#endif // NEU_PLUGIN_OPCUA_SERVER_HANDLE_H