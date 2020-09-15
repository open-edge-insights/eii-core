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


cdef extern from "eis/config_manager/c_cfg_mgr.h" nogil:
    ctypedef struct config_t:
        pass

    ctypedef struct kv_store_client_t:
        pass

    # Forward declaration
    ctypedef struct config_value_t

    ctypedef struct base_cfg_t:
        config_t* m_app_config
        config_t* m_app_interface

    ctypedef struct pub_cfg_t:
        config_t* (*get_msgbus_config)(base_cfg_t* base_cfg)
        char* (*get_endpoint)(base_cfg_t* base_cfg)
        char** (*get_topics)(base_cfg_t* base_cfg)
        int (*set_topics)(char** topics_list, base_cfg_t* base_cfg)
        char** (*get_allowed_clients)(base_cfg_t* base_cfg)

    ctypedef struct sub_cfg_t:
        config_t* (*get_msgbus_config_sub)(base_cfg_t* base_cfg)
        char* (*get_endpoint_sub)(base_cfg_t* base_cfg)
        char** (*get_topics_sub)(base_cfg_t* base_cfg)
        int (*set_topics_sub)(char** topics_list, base_cfg_t* base_cfg)

    ctypedef struct server_cfg_t:
        config_t* (*get_msgbus_config_server)(base_cfg_t* base_cfg)
        char* (*get_endpoint_server)(base_cfg_t* base_cfg)
        char** (*get_allowed_clients_server)(base_cfg_t* base_cfg)

    ctypedef struct client_cfg_t:
        config_t* (*get_msgbus_config_client)(base_cfg_t* base_cfg)
        char* (*get_endpoint_client)(base_cfg_t* base_cfg)

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
    pub_cfg_t* get_publisher_by_name(app_cfg_t* self, const char* name)
    pub_cfg_t* get_publisher_by_index(app_cfg_t* app_cfg, int index)
    sub_cfg_t* get_subscriber_by_name(app_cfg_t* self, const char* name)
    sub_cfg_t* get_subscriber_by_index(app_cfg_t* app_cfg, int index)
    server_cfg_t* get_server_by_name(app_cfg_t* self, const char* name)
    server_cfg_t* get_server_by_index(app_cfg_t* app_cfg, int index)
    client_cfg_t* get_client_by_name(app_cfg_t* self, const char* name)
    client_cfg_t* get_client_by_index(app_cfg_t* app_cfg, int index)
    app_cfg_t* app_cfg_new()
