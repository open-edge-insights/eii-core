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

cdef extern from "eis/config_manager/cfg_mgr.h" nogil:
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

    ctypedef struct base_cfg_t:
        config_t* m_app_config
        config_t* m_app_interface

    ctypedef struct pub_cfg_t:
        config_t* (*cfgmgr_get_msgbus_config_pub)(base_cfg_t* base_cfg)
        config_value_t*(*cfgmgr_get_endpoint_pub)(base_cfg_t* base_cfg)
        config_value_t* (*cfgmgr_get_topics_pub)(base_cfg_t* base_cfg)
        int (*cfgmgr_set_topics_pub)(char** topics_list, int len, base_cfg_t* base_cfg)
        config_value_t* (*cfgmgr_get_allowed_clients_pub)(base_cfg_t* base_cfg)

    ctypedef struct sub_cfg_t:
        config_t* (*cfgmgr_get_msgbus_config_sub)(base_cfg_t* base_cfg)
        config_value_t* (*cfgmgr_get_endpoint_sub)(base_cfg_t* base_cfg)
        config_value_t* (*cfgmgr_get_topics_sub)(base_cfg_t* base_cfg)
        int (*cfgmgr_set_topics_sub)(char** topics_list, int len, base_cfg_t* base_cfg)

    ctypedef struct server_cfg_t:
        config_t* (*cfgmgr_get_msgbus_config_server)(base_cfg_t* base_cfg)
        config_value_t* (*cfgmgr_get_endpoint_server)(base_cfg_t* base_cfg)
        config_value_t* (*cfgmgr_get_allowed_clients_server)(base_cfg_t* base_cfg)

    ctypedef struct client_cfg_t:
        config_t* (*cfgmgr_get_msgbus_config_client)(base_cfg_t* base_cfg)
        config_value_t* (*cfgmgr_get_endpoint_client)(base_cfg_t* base_cfg)

    ctypedef struct app_cfg_t:
        base_cfg_t* base_cfg

    # base_cfg_t APIs
    char* configt_to_char(config_t* config)
    config_t* get_app_config(base_cfg_t* base_cfg) 
    config_t* get_app_interface(base_cfg_t* base_cfg) 
    void base_cfg_config_destroy(base_cfg_t *base_cfg_config)

    # pub_cfg_t APIs
    void pub_cfg_config_destroy(pub_cfg_t *pub_cfg_config)

    # sub_cfg_t APIs
    void sub_cfg_config_destroy(sub_cfg_t *sub_cfg_config)

    # server_cfg_t APIs
    void server_cfg_config_destroy(server_cfg_t *server_cfg_config)

    # client_cfg_t APIs
    void client_cfg_config_destroy(client_cfg_t *cli_cfg_config)

    # app_cfg_t APIs
    pub_cfg_t* cfgmgr_get_publisher_by_name(app_cfg_t* self, const char* name)
    pub_cfg_t* cfgmgr_get_publisher_by_index(app_cfg_t* app_cfg, int index)
    sub_cfg_t* cfgmgr_get_subscriber_by_name(app_cfg_t* self, const char* name)
    sub_cfg_t* cfgmgr_get_subscriber_by_index(app_cfg_t* app_cfg, int index)
    server_cfg_t* cfgmgr_get_server_by_name(app_cfg_t* self, const char* name)
    server_cfg_t* cfgmgr_get_server_by_index(app_cfg_t* app_cfg, int index)
    client_cfg_t* cfgmgr_get_client_by_name(app_cfg_t* self, const char* name)
    client_cfg_t* cfgmgr_get_client_by_index(app_cfg_t* app_cfg, int index)
    app_cfg_t* app_cfg_new()
    void app_cfg_config_destroy(app_cfg_t *app_cfg_config)

    # config_value_t APIs
    size_t config_value_array_len(const config_value_t* arr)
    config_value_t* config_value_array_get(const config_value_t* arr, int idx)
    void config_value_destroy(config_value_t* value)
