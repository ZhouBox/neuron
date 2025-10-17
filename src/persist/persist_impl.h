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

#ifndef NEU_PERSIST_PERSIST_IMPL
#define NEU_PERSIST_PERSIST_IMPL

#ifdef __cplusplus
extern "C" {
#endif

#include "persist/persist.h"

struct neu_persister_vtbl_s;

typedef struct {
    struct neu_persister_vtbl_s *vtbl;
} neu_persister_t;

struct neu_persister_vtbl_s {
    /**
     * Destroy perister.
     */
    void (*destroy)(neu_persister_t *self);

    /**
     * @return implementation defined underlying handle.
     */
    void *(*native_handle)(neu_persister_t *self);

    /**
     * Persist nodes.
     * @param node_info                 neu_persist_node_info_t.
     * @return 0 on success, non-zero on failure
     */
    int (*store_node)(neu_persister_t *self, neu_persist_node_info_t *info);

    /**
     * Load node infos.
     * @param[out] node_infos           used to return pointer to heap allocated
     *                                  vector of neu_persist_node_info_t.
     * @return 0 on success, none-zero on failure
     */
    int (*load_nodes)(neu_persister_t *self, UT_array **node_infos);

    /**
     * Delete node.
     * @param node_name                 name of the node to delete.
     * @return 0 on success, none-zero on failure
     */
    int (*delete_node)(neu_persister_t *self, const char *node_name);

    /**
     * Update node name.
     * @param node_name                 name of the node to update.
     * @param new_name                  new name of the adapter.
     * @return 0 on success, none-zero on failure
     */
    int (*update_node)(neu_persister_t *self, const char *node_name,
                       const char *new_name);

    /**
     * Update node state.
     * @param node_name                 name of the node to update.
     * @param state                     state of the adapter.
     * @return 0 on success, none-zero on failure
     */
    int (*update_node_state)(neu_persister_t *self, const char *node_name,
                             int state);

    /**
     * Persist node setting.
     * @param adapter_name              name of the adapter who owns the
     * setting.
     * @param setting                   node setting string.
     * @return 0 on success, non-zero otherwise
     */
    int (*store_node_setting)(neu_persister_t *self, const char *node_name,
                              const char *setting);

    /**
     * Load node setting.
     * @param node_name                 name of the node.
     * @param[out] setting              used to return node setting string.
     * @return 0 on success, non-zero otherwise
     */
    int (*load_node_setting)(neu_persister_t *self, const char *node_name,
                             const char **const setting);

    /**
     * Delete node setting.
     * @param node_name                 name of the node.
     * @return 0 on success, none-zero on failure
     */
    int (*delete_node_setting)(neu_persister_t *self, const char *node_name);

    /**
     * Persist node tag.
     * @param driver_name               name of the driver who owns the tags
     * @param group_name                name of the group
     * @param tag                       the tag to store
     * @return 0 on success, non-zero otherwise
     */
    int (*store_tag)(neu_persister_t *self, const char *driver_name,
                     const char *group_name, const neu_datatag_t *tag);

    /**
     * Persist node tags.
     * @param driver_name               name of the driver who owns the tags
     * @param group_name                name of the group
     * @param tags                      list of tags to store
     * @param n                         number of tags to store
     * @return 0 on success, non-zero otherwise
     */
    int (*store_tags)(neu_persister_t *self, const char *driver_name,
                      const char *group_name, const neu_datatag_t *tags,
                      size_t n);

    /**
     * Load node tag infos.
     * @param node_name                 name of the node who owns the tags
     * @param group_name                name of the group
     * @param[out] tag_infos            used to return pointer to heap allocated
     *                                  vector of neu_datatag_t
     * @return 0 on success, non-zero otherwise
     */
    int (*load_tags)(neu_persister_t *self, const char *driver_name,
                     const char *group_name, UT_array **tag_infos);

    /**
     * Update node tags.
     * @param driver_name               name of the driver who owns the tags
     * @param group_name                name of the group
     * @param tag                       the tag to update
     * @return 0 on success, non-zero otherwise
     */
    int (*update_tag)(neu_persister_t *self, const char *driver_name,
                      const char *group_name, const neu_datatag_t *tag);

    /**
     * Update node tag value.
     * @param driver_name               name of the driver who owns the tags
     * @param group_name                name of the group
     * @param tag                       the tag to update
     * @return 0 on success, non-zero otherwise
     */
    int (*update_tag_value)(neu_persister_t *self, const char *driver_name,
                            const char *group_name, const neu_datatag_t *tag);

    /**
     * Delete node tags.
     * @param driver_name               name of the driver who owns the tags
     * @param group_name                name of the group
     * @param tag_name                  name of the tag
     * @return 0 on success, non-zero otherwise
     */
    int (*delete_tag)(neu_persister_t *self, const char *driver_name,
                      const char *group_name, const char *tag_name);

    /**
     * Persist subscriptions.
     * @param app_name                  name of the app node
     * @param driver_name               name of the driver node
     * @param group_name                name of the group
     * @param params                    subscription params
     * @return 0 on success, non-zero otherwise
     */
    int (*store_subscription)(neu_persister_t *self, const char *app_name,
                              const char *driver_name, const char *group_name,
                              const char *params, const char *static_tags);

    /**
     * Update subscriptions.
     * @param app_name                  name of the app node
     * @param driver_name               name of the driver node
     * @param group_name                name of the group
     * @param params                    subscription params
     * @return 0 on success, non-zero otherwise
     */
    int (*update_subscription)(neu_persister_t *self, const char *app_name,
                               const char *driver_name, const char *group_name,
                               const char *params, const char *static_tags);

    /**
     * Load adapter subscriptions.
     * @param app_name                  name of the app node
     * @param[out] subscription_infos   used to return pointer to heap allocated
     *                                  vector of
     * neu_persist_subscription_info_t.
     * @return 0 on success, non-zero otherwise
     */
    int (*load_subscriptions)(neu_persister_t *self, const char *app_name,
                              UT_array **subscription_infos);

    /**
     * Persist subscriptions.
     * @param app_name                  name of the app node
     * @param driver_name               name of the driver node
     * @param group_name                name of the group
     * @return 0 on success, non-zero otherwise
     */
    int (*delete_subscription)(neu_persister_t *self, const char *app_name,
                               const char *driver_name, const char *group_name);

    /**
     * Persist group config.
     * @param driver_name               name of the driver who owns the group
     * @param group_info                group info to persist.
     * @return 0 on success, non-zero otherwise
     */
    int (*store_group)(neu_persister_t *self, const char *driver_name,
                       neu_persist_group_info_t *group_info,
                       const char *              context);

    /**
     * Update group config.
     * @param driver_name               name of the driver who owns the group
     * @param group_info                group info to persist.
     * @return 0 on success, non-zero otherwise
     */
    int (*update_group)(neu_persister_t *self, const char *driver_name,
                        const char *              group_name,
                        neu_persist_group_info_t *group_info);

    /**
     * Load all group config infos under an adapter.
     * @param driver_name               name of the driver who owns the group
     * @param[out] group_infos          used to return pointer to heap allocated
     *                                  vector of neu_persist_group_info_t.
     * @return 0 on success, non-zero otherwise
     */
    int (*load_groups)(neu_persister_t *self, const char *driver_name,
                       UT_array **group_infos);
    /**
     * Delete group config.
     * @param driver_name               name of the driver who owns the group
     * @param group_name                name of the group.
     * @return 0 on success, none-zero on failure
     */
    int (*delete_group)(neu_persister_t *self, const char *driver_name,
                        const char *group_name);

    /**
     * Load all user infos.
     * @param[out] user_infos           used to return pointer to heap allocated
     *                                  vector of neu_persist_user_info_t.
     * @return 0 on success, non-zero otherwise
     */
    int (*load_users)(neu_persister_t *self, UT_array **user_infos);

    /**
     * Save user info.
     * @param user                      user info
     * @return 0 on success, none-zero on failure
     */
    int (*store_user)(neu_persister_t *              self,
                      const neu_persist_user_info_t *user);

    /**
     * Update user info.
     * @param user                      user info
     * @return 0 on success, none-zero on failure
     */
    int (*update_user)(neu_persister_t *              self,
                       const neu_persist_user_info_t *user);

    /**
     * Load user info.
     * @param user_name                 name of the user.
     * @param user_p                    output parameter of user info
     * @return 0 on success, none-zero on failure
     */
    int (*load_user)(neu_persister_t *self, const char *user_name,
                     neu_persist_user_info_t **user_p);

    /**
     * Delete user info.
     * @param user_name                 name of the user.
     * @return 0 on success, none-zero on failure
     */
    int (*delete_user)(neu_persister_t *self, const char *user_name);

    /**
     * Save server certificate.
     * @param cert_info                 server certificate info
     * @return 0 on success, none-zero on failure
     */
    int (*store_server_cert)(neu_persister_t *                     self,
                             const neu_persist_server_cert_info_t *cert_info);

    /**
     * Update server certificate.
     * @param cert_info                 server certificate info
     * @return 0 on success, none-zero on failure
     */
    int (*update_server_cert)(neu_persister_t *                     self,
                              const neu_persist_server_cert_info_t *cert_info);

    /**
     * Load server certificate by app name.
     * @param app_name                  name of the application
     * @param cert_info_p               output parameter of certificate info
     * @return 0 on success, none-zero on failure
     */
    int (*load_server_cert)(neu_persister_t *self, const char *app_name,
                            neu_persist_server_cert_info_t **cert_info_p);

    /**
     * Save client certificate.
     * @param cert_info                 client certificate info
     * @return 0 on success, none-zero on failure
     */
    int (*store_client_cert)(neu_persister_t *                     self,
                             const neu_persist_client_cert_info_t *cert_info);

    /**
     * Update client certificate.
     * @param cert_info                 client certificate info
     * @return 0 on success, none-zero on failure
     */
    int (*update_client_cert)(neu_persister_t *                     self,
                              const neu_persist_client_cert_info_t *cert_info);

    /**
     * Load client certificates by app name.
     * @param app_name                  name of the application
     * @param[out] cert_infos           used to return pointer to heap allocated
     *                                  vector of
     * neu_persist_client_cert_info_t.
     * @return 0 on success, non-zero otherwise
     */
    int (*load_client_certs_by_app)(neu_persister_t *self, const char *app_name,
                                    UT_array **cert_infos);

    /**
     * Load all client certificates.
     * @param[out] cert_infos           used to return pointer to heap allocated
     *                                  vector of
     * neu_persist_client_cert_info_t.
     * @return 0 on success, non-zero otherwise
     */
    int (*load_client_certs)(neu_persister_t *self, UT_array **cert_infos);

    /**
     * Delete client certificate by id.
     * @param cert_id                   id of the certificate
     * @return 0 on success, none-zero on failure
     */
    int (*delete_client_cert)(neu_persister_t *self, const char *fingerprint);

    /**
     * Save security policy.
     * @param policy_info               security policy info
     * @return 0 on success, none-zero on failure
     */
    int (*store_security_policy)(
        neu_persister_t *                         self,
        const neu_persist_security_policy_info_t *policy_info);

    /**
     * Update security policy.
     * @param policy_info               security policy info
     * @return 0 on success, none-zero on failure
     */
    int (*update_security_policy)(
        neu_persister_t *                         self,
        const neu_persist_security_policy_info_t *policy_info);

    /**
     * Load security policy by app name.
     * @param app_name                  name of the application
     * @param policy_info_p             output parameter of security policy info
     * @return 0 on success, none-zero on failure
     */
    int (*load_security_policy)(
        neu_persister_t *self, const char *app_name,
        neu_persist_security_policy_info_t **policy_info_p);

    /**
     * Load all security policies.
     * @param[out] policy_infos         used to return pointer to heap allocated
     *                                  vector of
     * neu_persist_security_policy_info_t.
     * @return 0 on success, non-zero otherwise
     */
    int (*load_security_policies)(neu_persister_t *self,
                                  UT_array **      policy_infos);

    /**
     * Save authentication setting.
     * @param auth_info                 authentication setting info
     * @return 0 on success, none-zero on failure
     */
    int (*store_auth_setting)(neu_persister_t *                      self,
                              const neu_persist_auth_setting_info_t *auth_info);

    /**
     * Update authentication setting.
     * @param auth_info                 authentication setting info
     * @return 0 on success, none-zero on failure
     */
    int (*update_auth_setting)(
        neu_persister_t *                      self,
        const neu_persist_auth_setting_info_t *auth_info);

    /**
     * Load authentication setting by app name.
     * @param app_name                  name of the application
     * @param auth_info_p               output parameter of auth setting info
     * @return 0 on success, none-zero on failure
     */
    int (*load_auth_setting)(neu_persister_t *self, const char *app_name,
                             neu_persist_auth_setting_info_t **auth_info_p);

    /**
     * Save authentication user.
     * @param user_info                 authentication user info
     * @return 0 on success, none-zero on failure
     */
    int (*store_auth_user)(neu_persister_t *                   self,
                           const neu_persist_auth_user_info_t *user_info);

    /**
     * Update authentication user.
     * @param user_info                 authentication user info
     * @return 0 on success, none-zero on failure
     */
    int (*update_auth_user)(neu_persister_t *                   self,
                            const neu_persist_auth_user_info_t *user_info);

    /**
     * Load authentication user by app name and username.
     * @param app_name                  name of the application
     * @param username                  name of the user
     * @param user_info_p               output parameter of auth user info
     * @return 0 on success, none-zero on failure
     */
    int (*load_auth_user)(neu_persister_t *self, const char *app_name,
                          const char *                   username,
                          neu_persist_auth_user_info_t **user_info_p);

    /**
     * Load authentication users by app name.
     * @param app_name                  name of the application
     * @param[out] user_infos           used to return pointer to heap allocated
     *                                  vector of neu_persist_auth_user_info_t.
     * @return 0 on success, non-zero otherwise
     */
    int (*load_auth_users_by_app)(neu_persister_t *self, const char *app_name,
                                  UT_array **user_infos);

    /**
     * Delete authentication user by app name and username.
     * @param app_name                  name of the application
     * @param username                  name of the user
     * @return 0 on success, none-zero on failure
     */
    int (*delete_auth_user)(neu_persister_t *self, const char *app_name,
                            const char *username);
};

// read all file contents as string
int read_file_string(const char *fn, char **out);

#ifdef __cplusplus
}
#endif

#endif
