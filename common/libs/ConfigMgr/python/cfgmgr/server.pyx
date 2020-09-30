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
"""EIS Message Bus Server wrapper object
"""

import json

from .libneweisconfigmgr cimport *
from libc.stdlib cimport malloc
from .util cimport Util


cdef class Server:
    """EIS Message Bus Server object
    """

    def __init__(self):
        """Constructor
        """
        pass

    def __cinit__(self, *args, **kwargs):
        """Cython base constructor
        """
        self.app_cfg = NULL
        self.server_cfg = NULL

    @staticmethod
    cdef create(app_cfg_t* app_cfg, server_cfg_t* server_cfg):
        """Helper method for initializing the client object.

        :param app_cfg: Applications config struct
        :type: struct
        :param server_cfg: Server config struct
        :type: struct
        :return: Server class object
        :rtype: obj
        """
        s = Server()
        s.app_cfg = app_cfg
        s.server_cfg = server_cfg
        return s

    def __dealloc__(self):
        """Cython destructor
        """
        self.destroy()

    def destroy(self):
        """Destroy the server.
        """
        if self.app_cfg != NULL:
            app_cfg_config_destroy(self.app_cfg)
        if self.server_cfg != NULL:
            server_cfg_config_destroy(self.server_cfg)

    def get_msgbus_config(self):
        """Calling the base C cfgmgr_get_msgbus_config_server() API

        :return: Messagebus config
        :rtype: dict
        """
        cdef char* config
        new_config_new = self.server_cfg.cfgmgr_get_msgbus_config_server(self.app_cfg.base_cfg)
        config = configt_to_char(new_config_new)
        config_str = config.decode('utf-8')
        return json.loads(config_str)

    def get_interface_value(self, key):
        """Calling the base C cfgmgr_get_interface_value_server() API

        :param key: Key on which interface value will be extracted
        :type: string
        :return: Interface value
        :rtype: string
        """
        cdef config_value_t* value
        cdef char* config
        value = self.server_cfg.cfgmgr_get_interface_value_server(self.app_cfg.base_cfg, key.encode('utf-8'))
        interface_value = Util.get_cvt_data(value)
        config_value_destroy(value)
        return interface_value

    def get_endpoint(self):
        """Calling the base C get_endpoint() API

        :return: Endpoint config
        :rtype: string
        """
        cdef config_value_t* ep
        ep = self.server_cfg.cfgmgr_get_endpoint_server(self.app_cfg.base_cfg)
        if(ep.type == CVT_OBJECT):
            config = cvt_to_char(ep);
            config_str = config.decode('utf-8')
            endpoint = json.loads(config_str)
        elif(ep.type == CVT_STRING):
            endpoint = ep.body.string.decode('utf-8')
        else:
            endpoint = None
            raise TypeError("Type mismatch: EndPoint should be string or dict type")
        config_value_destroy(ep)
        return endpoint

    def get_allowed_clients(self):
        """Calling the base C cfgmgr_get_allowed_clients_server() API
        
        :return: List of clients
        :rtype: List
        """
        # Calling the base C cfgmgr_get_allowed_clients_server() API
        clients_list = []
        cdef config_value_t* clients
        clients = self.server_cfg.cfgmgr_get_allowed_clients_server(self.app_cfg.base_cfg)
        cdef config_value_t* client_value
        for i in range(config_value_array_len(clients)):
            client_value = config_value_array_get(clients, i)
            clients_list.append(client_value.body.string.decode('utf-8'))
            config_value_destroy(client_value)
        config_value_destroy(clients)
        return clients_list