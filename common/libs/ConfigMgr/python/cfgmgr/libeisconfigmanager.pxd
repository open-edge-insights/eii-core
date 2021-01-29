# Copyright (c) 2020 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
"""All C imports for the EIS ConfigManager
"""

from libc.stdint cimport *

cdef extern from "stdbool.h":
    ctypedef bint bool

cdef extern from "eis/config_manager/cfgmgr.h" nogil:
    ctypedef struct config_t:
        pass

    ctypedef struct kv_store_client_t:
        pass

    ctypedef enum config_value_type_t:
        CVT_INTEGER  = 0
        CVT_FLOATING = 1
        CVT_STRING   = 2
        CVT_BOOLEAN  = 3
        CVT_OBJECT   = 4
        CVT_ARRAY    = 5
        CVT_NONE     = 6

    # Forward declaration
    ctypedef struct config_value_t

    ctypedef struct config_value_object_t:
        void* object
        void (*free)(void* object)

    ctypedef struct config_value_array_t:
        void* array
        size_t length
        config_value_t* (*get)(void*,int)
        void (*free)(void*)

    ctypedef union config_value_type_body_union_t:
        int64_t      integer
        double       floating
        const char*  string
        bool         boolean
        config_value_object_t* object
        config_value_array_t* array

    ctypedef struct config_value_t:
        config_value_type_t type
        config_value_type_body_union_t body

    ctypedef struct cfgmgr_ctx_t:
        char* env_var
        config_t* app_config
        config_t* app_interface
        char* app_name
    
    ctypedef struct cfgmgr_interface_t:
        config_value_t* interface
        cfgmgr_ctx_t* cfg_mgr

    # C callback type definition
    ctypedef void (*cfgmgr_watch_callback_t)(const char* key, config_t* value, void* cb_user_data)

    # cfg_mgr APIs
    bool cfgmgr_is_dev_mode(cfgmgr_ctx_t* cfgmgr)
    config_value_t* cfgmgr_get_appname(cfgmgr_ctx_t* cfgmgr)
    config_t* cfgmgr_get_app_config(cfgmgr_ctx_t* cfgmgr)
    config_t* cfgmgr_get_app_interface(cfgmgr_ctx_t* cfgmgr)
    config_value_t* cfgmgr_get_interface_value(cfgmgr_interface_t* cfgmgr_interface, const char* key)
    config_value_t* cfgmgr_get_app_config_value(cfgmgr_ctx_t* cfgmgr, const char* key)
    config_value_t* cfgmgr_get_app_interface_value(cfgmgr_ctx_t* cfgmgr, const char* key)
    config_t* cfgmgr_get_msgbus_config(cfgmgr_interface_t* ctx)
    config_value_t* cfgmgr_get_endpoint(cfgmgr_interface_t* ctx)
    config_value_t* cfgmgr_get_topics(cfgmgr_interface_t* ctx)
    bool cfgmgr_set_topics(cfgmgr_interface_t* ctx, char** topics_list, int len)
    config_value_t* cfgmgr_get_allowed_clients(cfgmgr_interface_t* ctx)
    int cfgmgr_get_num_publishers(cfgmgr_ctx_t* cfgmgr)
    int cfgmgr_get_num_subscribers(cfgmgr_ctx_t* cfgmgr)
    int cfgmgr_get_num_servers(cfgmgr_ctx_t* cfgmgr)
    int cfgmgr_get_num_clients(cfgmgr_ctx_t* cfgmgr)
    cfgmgr_interface_t* cfgmgr_get_publisher_by_name(cfgmgr_ctx_t* cfgmgr, const char* name)
    cfgmgr_interface_t* cfgmgr_get_publisher_by_index(cfgmgr_ctx_t* cfgmgr, int index)
    cfgmgr_interface_t* cfgmgr_get_subscriber_by_name(cfgmgr_ctx_t* cfgmgr, const char* name)
    cfgmgr_interface_t* cfgmgr_get_subscriber_by_index(cfgmgr_ctx_t* cfgmgr, int index)
    cfgmgr_interface_t* cfgmgr_get_server_by_name(cfgmgr_ctx_t* cfgmgr, const char* name)
    cfgmgr_interface_t* cfgmgr_get_server_by_index(cfgmgr_ctx_t* cfgmgr, int index)
    cfgmgr_interface_t* cfgmgr_get_client_by_name(cfgmgr_ctx_t* cfgmgr, const char* name)
    cfgmgr_interface_t* cfgmgr_get_client_by_index(cfgmgr_ctx_t* cfgmgr, int index)
    cfgmgr_ctx_t* cfgmgr_initialize()
    void cfgmgr_destroy(cfgmgr_ctx_t *cfg_mgr)
    cfgmgr_interface_t* cfgmgr_interface_initialize()
    void cfgmgr_interface_destroy(cfgmgr_interface_t *cfg_mgr_interface)
    
    # CfgMgr Util APIs
    char* configt_to_char(config_t* config)
    char* cvt_to_char(config_value_t* config)

    # watch APIs
    void cfgmgr_watch(cfgmgr_ctx_t* cfgmgr, const char* key, cfgmgr_watch_callback_t watch_callback, void* user_data)
    void cfgmgr_watch_prefix(cfgmgr_ctx_t* cfgmgr, char* prefix, cfgmgr_watch_callback_t watch_callback, void* user_data)

    # config_value_t APIs
    size_t config_value_array_len(const config_value_t* arr)
    config_value_t* config_value_array_get(const config_value_t* arr, int idx)
    void config_value_destroy(config_value_t* value)
    void config_destroy(config_t* config)
