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
"""EIS Message Bus Client wrapper object
"""

import json

from .libneweisconfigmgr cimport *
from libc.stdlib cimport malloc
from .util cimport Util


cdef class Client:
    """EIS Message Bus Client object
    """

    def __init__(self):
        """Constructor

        :param config: Configuration object
        :type: dict
        """
        pass

    def __cinit__(self, *args, **kwargs):
        """Cython base constructor
        """
        self.app_cfg = NULL
        self.client_cfg = NULL

    @staticmethod
    cdef create(app_cfg_t* app_cfg, client_cfg_t* client_cfg):
        """Helper method for initializing the client object.

        :param app_cfg: Applications config struct
        :type: struct
        :param client_cfg: Client config struct
        :type: struct
        :return: Client class object
        :rtype: obj
        """
        c = Client()
        c.app_cfg = app_cfg
        c.client_cfg = client_cfg
        return c

    def __dealloc__(self):
        """Cython destructor
        """
        self.destroy()

    def destroy(self):
        """Destroy the client.
        """
        if self.app_cfg != NULL:
            app_cfg_config_destroy(self.app_cfg)
        if self.client_cfg != NULL:
            client_cfg_config_destroy(self.client_cfg)

    def get_msgbus_config(self):
        """Calling the base C get_msgbus_config() API

        :return: Messagebus config
        :rtype: dict
        """
        cdef char* config
        new_config_new = self.client_cfg.cfgmgr_get_msgbus_config_client(self.app_cfg.base_cfg)
        config = configt_to_char(new_config_new)
        config_str = config.decode('utf-8')
        return json.loads(config_str)

    def get_interface_value(self, key):
        """Calling the base C cfgmgr_get_interface_value_client() API

        :param key: Key on which interface value will be extracted
        :type: string
        :return: Interface value
        :rtype: string
        """
        cdef config_value_t* value
        cdef char* config
        value = self.client_cfg.cfgmgr_get_interface_value_client(self.app_cfg.base_cfg, key.encode('utf-8'))
        interface_value = Util.get_cvt_data(value)
        config_value_destroy(value)
        return interface_value

    def get_endpoint(self):
        """Calling the base C get_endpoint() API

        :return: Endpoint config
        :rtype: string
        """
        cdef config_value_t* ep
        ep = self.client_cfg.cfgmgr_get_endpoint_client(self.app_cfg.base_cfg)
        endpoint = ep.body.string.decode('utf-8')
        config_value_destroy(ep)
        return endpoint
