// Copyright (c) 2021 Intel Corporation.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @file
 * @brief Configuration Manager C APIs
 */

#include <ctype.h>
#include "eii/config_manager/kv_store_plugin/kv_store_plugin.h"
#include "eii/config_manager/cfgmgr_util.h"

#define PUBLISHERS "Publishers"
#define SUBSCRIBERS "Subscribers"
#define SERVERS "Servers"
#define CLIENTS "Clients"
#define SOCKET_FILE "socket_file"
#define ENDPOINT "EndPoint"
#define TOPICS "Topics"
#define NAME "Name"
#define ALLOWED_CLIENTS "AllowedClients"
#define TYPE "Type"
#define PUBLIC_KEYS "/Publickeys/"
#define PRIVATE_KEY "/private_key"
#define ZMQ_RECV_HWM "zmq_recv_hwm"
#define BROKER_APPNAME "BrokerAppName"
#define BROKERED "brokered"
#define PUBLISHER_APPNAME "PublisherAppName"
#define SERVER_APPNAME "ServerAppName"

#ifndef _EII_C_CONFIG_MGR_H
#define _EII_C_CONFIG_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ConfigMgr context struct
 */
typedef struct  {

    // Holds the list of /GlobalEnv var
    // to be set in the Go/Py/Cpp bindings
    char* env_var;

    // Application config
    config_t* app_config;

    // Application interface
    config_t* app_interface;

    // Application data store
    config_t* data_store;

    // Application AppName
    char* app_name;

    // Dev mode variable
    int dev_mode;

    // kv_store_client to call kv_store APIs
    kv_store_client_t* kv_store_client;

    // kv_store_handle to hold the kv_store object
    void* kv_store_handle;

} cfgmgr_ctx_t;

/**
 * ConfigMgr interface struct
 */
typedef struct {

    // Holds the interface specific
    // to a CFGMGR_PUBLISHER/CFGMGR_SUBSCRIBER/
    // CFGMGR_SERVER/CFGMGR_CLIENT
    config_value_t* interface;

    // Defines the type of interface
    cfgmgr_iface_type_t type;

    // Holds the cfgmgr context
    cfgmgr_ctx_t* cfg_mgr;

} cfgmgr_interface_t;


/**
 * To check whether environment is dev mode or prod mode
 * @param cfgmgr - cfgmgr_ctx_t object
 *  @return 0 if dev_mode or any errors, prod mode otherwise
 */
bool cfgmgr_is_dev_mode(cfgmgr_ctx_t* cfgmgr);

/**
 * To fetch appname of any service
 * @param cfgmgr - cfgmgr_ctx_t object
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* cfgmgr_get_appname(cfgmgr_ctx_t* cfgmgr);

/**
 * cfgmgr_get_app_config function to return app config
 * @param cfgmgr - cfgmgr_ctx_t object
 *  @return NULL for any errors occured or config_t* on success
 */
config_t* cfgmgr_get_app_config(cfgmgr_ctx_t* cfgmgr);

/**
 * cfgmgr_get_app_interface function to return app interface
 * @param cfgmgr - cfgmgr_ctx_t object
 *  @return NULL for any errors occured or config_t* on success
 */
config_t* cfgmgr_get_app_interface(cfgmgr_ctx_t* cfgmgr);

/**
 * cfgmgr_get_app_config_value function to fetch config value
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param key - value of key to be fetched
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* cfgmgr_get_app_config_value(cfgmgr_ctx_t* cfgmgr, const char* key);

/**
 * cfgmgr_get_app_interface_value function to fetch app interface value
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param key - value of key to be fetched
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* cfgmgr_get_app_interface_value(cfgmgr_ctx_t* cfgmgr, const char* key);

/**
 * cfgmgr_get_msgbus_config function to fetch msgbus config
 * 
 * @param ctx - cfgmgr_interface_t object
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_t* cfgmgr_get_msgbus_config(cfgmgr_interface_t* ctx);

/**
 * get_endpoint_base function to fetch endpoint
 * 
 * @param ctx - cfgmgr_interface_t object
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* cfgmgr_get_endpoint(cfgmgr_interface_t* ctx);

/**
 * cfgmgr_get_topics function to fetch topics
 * 
 * Returns the value mapped to Topics key in the Applications Interface.
 * If "*" is mentioned in topics, then it is replaced by empty string ,
 * as our EIIMessageBus supports the prefix approach, empty prefix considers all/any the topics.
 * 
 * @param ctx - cfgmgr_interface_t object
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* cfgmgr_get_topics(cfgmgr_interface_t* ctx);

/**
 * set_topics_base function to set topics
 * @param ctx - cfgmgr_interface_t object
 * @param topics_list - list of topics to be set
 * @param len - total number of topics
 *  @return false for any errors occured or true on success
 */
bool cfgmgr_set_topics(cfgmgr_interface_t* ctx, char const* const* topics_list, int len);

/**
 * cfgmgr_get_allowed_clients function to fetch allowed clients
 * 
 * 
 * Get Allowed Clients returns the value mapped to AllowedClients key in the Applications Interface.
 * If "*" is mentioned in the allowed clients, the return value will still be "*" notifying user
 * that all the provisioned applications are allowed to get the topics.
 * 
 * @param ctx - cfgmgr_interface_t object
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* cfgmgr_get_allowed_clients(cfgmgr_interface_t* ctx);

/**
 * To fetch number of publishers in an interface
 * @param cfgmgr - cfgmgr_ctx_t object
 *  @return number of publishers if success or -1 for any errors
 */
int cfgmgr_get_num_publishers(cfgmgr_ctx_t* cfgmgr);

/**
 * To fetch number of subscribers in an interface
 * @param cfgmgr - cfgmgr_ctx_t object
 *  @return number of subscribers if success or -1 for any errors
 */
int cfgmgr_get_num_subscribers(cfgmgr_ctx_t* cfgmgr);

/**
 * To fetch number of servers in an interface
 * @param cfgmgr - cfgmgr_ctx_t object
 *  @return number of servers if success or -1 for any errors
 */
int cfgmgr_get_num_servers(cfgmgr_ctx_t* cfgmgr);

/**
 * To fetch number of clients in an interface
 * @param cfgmgr - cfgmgr_ctx_t object
 *  @return number of clients if success or -1 for any errors
 */
int cfgmgr_get_num_clients(cfgmgr_ctx_t* cfgmgr);

// cfgmgr callback type to be used in watch APIs
typedef kv_store_watch_callback_t cfgmgr_watch_callback_t;

/**
 * function to register a callback for a specific key
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param key - key to watch on
 * @param watch_callback - cfgmgr_watch_callback_t object
 * @param user_data - user_data to be sent to callback
 */
void cfgmgr_watch(cfgmgr_ctx_t* cfgmgr, const char* key, cfgmgr_watch_callback_t watch_callback, void* user_data);

/**
 * function to register a callback for a specific key prefix
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param prefix - key prefix to watch on
 * @param watch_callback - cfgmgr_watch_callback_t object
 * @param user_data - user_data to be sent to callback
 */
void cfgmgr_watch_prefix(cfgmgr_ctx_t* cfgmgr, char* prefix, cfgmgr_watch_callback_t watch_callback, void* user_data);

/**
 * cfgmgr_get_interface_value function to fetch interface value
 * @param cfgmgr_interface - cfgmgr_interface_t object
 * @param key - value of key to be fetched
 *  @return NULL for any errors occured or config_value_t* on success
 */
config_value_t* cfgmgr_get_interface_value(cfgmgr_interface_t* cfgmgr_interface, const char* key);

/**
 * cfgmgr_get_publisher_by_name function to fetch a publisher config using its name
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param name - name of config to be fetched
 * @return NULL for any errors occured or cfgmgr_interface_t* on success
 */
cfgmgr_interface_t* cfgmgr_get_publisher_by_name(cfgmgr_ctx_t* cfgmgr, const char* name);

/**
 * cfgmgr_get_publisher_by_index function to fetch a publisher config using its index
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param index - index of config to be fetched
 * @return NULL for any errors occured or cfgmgr_interface_t* on success
 */
cfgmgr_interface_t* cfgmgr_get_publisher_by_index(cfgmgr_ctx_t* cfgmgr, int index);

/**
 * cfgmgr_get_subscriber_by_name function to fetch a subscriber config using its name
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param name - name of config to be fetched
 * @return NULL for any errors occured or cfgmgr_interface_t* on success
 */
cfgmgr_interface_t* cfgmgr_get_subscriber_by_name(cfgmgr_ctx_t* cfgmgr, const char* name);

/**
 * cfgmgr_get_subscriber_by_index function to fetch a subscriber config using its index
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param index - index of config to be fetched
 * @return NULL for any errors occured or cfgmgr_interface_t* on success
 */
cfgmgr_interface_t* cfgmgr_get_subscriber_by_index(cfgmgr_ctx_t* cfgmgr, int index);

/**
 * cfgmgr_get_server_by_name function to fetch a server config using its name
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param name - name of config to be fetched
 * @return NULL for any errors occured or cfgmgr_interface_t* on success
 */
cfgmgr_interface_t* cfgmgr_get_server_by_name(cfgmgr_ctx_t* cfgmgr, const char* name);

/**
 * cfgmgr_get_server_by_index function to fetch a server config using its index
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param index - index of config to be fetched
 * @return NULL for any errors occured or cfgmgr_interface_t* on success
 */
cfgmgr_interface_t* cfgmgr_get_server_by_index(cfgmgr_ctx_t* cfgmgr, int index);

/**
 * cfgmgr_get_client_by_name function to fetch a client config using its name
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param name - name of config to be fetched
 * @return NULL for any errors occured or cfgmgr_interface_t* on success
 */
cfgmgr_interface_t* cfgmgr_get_client_by_name(cfgmgr_ctx_t* cfgmgr, const char* name);

/**
 * cfgmgr_get_client_by_index function to fetch a client config using its index
 * @param cfgmgr - cfgmgr_ctx_t object
 * @param index - index of config to be fetched
 * @return NULL for any errors occured or cfgmgr_interface_t* on success
 */
cfgmgr_interface_t* cfgmgr_get_client_by_index(cfgmgr_ctx_t* cfgmgr, int index);

/**
 * cfgmgr_initialize function to create a new cfgmgr_ctx_t instance
 *  @return NULL for any errors occured or cfgmgr_ctx_t* on success
 */
cfgmgr_ctx_t* cfgmgr_initialize();

/**
 * Destroy cfgmgr_ctx_t* object.
 *
 * @param cfg_mgr - configuration to destroy
 */
void cfgmgr_destroy(cfgmgr_ctx_t *cfg_mgr);

/**
 * cfgmgr_interface_initialize function to create a new cfgmgr_interface_t instance
 *  @return NULL for any errors occured or cfgmgr_interface_t* on success
 */
cfgmgr_interface_t* cfgmgr_interface_initialize();

/**
 * Destroy cfgmgr_ctx_t* object.
 *
 * @param cfg_mgr_interface - configuration to destroy
 */
void cfgmgr_interface_destroy(cfgmgr_interface_t *cfg_mgr_interface);

#ifdef __cplusplus
}
#endif

#endif