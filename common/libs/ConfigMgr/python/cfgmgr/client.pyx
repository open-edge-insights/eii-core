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
"""EII Message Bus Client wrapper object
"""

import json

from .libeiiconfigmanager cimport *
from libc.stdlib cimport malloc
from libc.stdlib cimport free
from .util cimport Util


cdef class Client:
    """EII Message Bus Client object
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
        self.cfgmgr_interface = NULL

    @staticmethod
    cdef create(cfgmgr_interface_t* cfgmgr_interface):
        """Helper method for initializing the client object.

        :param cfgmgr_interface: Client config struct
        :type: struct
        :return: Client class object
        :rtype: obj
        """
        c = Client()
        c.cfgmgr_interface = cfgmgr_interface
        return c

    def __dealloc__(self):
        """Cython destructor
        """
        self.destroy()

    def destroy(self):
        """Destroy the client.
        """
        if self.cfgmgr_interface != NULL:
            cfgmgr_interface_destroy(self.cfgmgr_interface)

    def get_msgbus_config(self):
        """Constructs message bus config for Client

        :return: Messagebus config
        :rtype: dict
        """
        cdef char* config
        cdef config_t* msgbus_config
        try:
            msgbus_config = cfgmgr_get_msgbus_config(self.cfgmgr_interface)
            if msgbus_config is NULL:
                raise Exception("[Client] Getting msgbus config from base c layer failed")
        
            config = configt_to_char(msgbus_config)
            if config is NULL:
                raise Exception("[Client] config failed to get converted to char")

            config_str = config.decode('utf-8')
            free(config)
            config_destroy(msgbus_config)
            return json.loads(config_str)
        except Exception as ex:
            raise ex

    def get_interface_value(self, key):
        """To fetch particular interface value from Client interface config

        :param key: Key on which interface value will be extracted
        :type: string
        :return: Interface value
        :rtype: string
        """
        cdef config_value_t* value
        cdef char* config
        try:
            interface_value = None
            value = cfgmgr_get_interface_value(self.cfgmgr_interface, key.encode('utf-8'))
            if value is NULL:
                raise Exception("[Client] Getting interface value from base c layer failed")
        
            interface_value = Util.get_cvt_data(value)
            if interface_value is None:
                config_value_destroy(value)
                raise Exception("[Client] Getting cvt data failed")
                
            config_value_destroy(value)
            return interface_value
        except Exception as ex:
            raise ex

    def get_endpoint(self):
        """To fetch Endpoint for particular client from its interface config

        :return: Endpoint config
        :rtype: string
        """
        cdef config_value_t* ep
        cdef char* c_endpoint
        try:
            ep = cfgmgr_get_endpoint(self.cfgmgr_interface)
            if ep is NULL:
                raise Exception("[Client] Getting end point from base c layer failed")

            if(ep.type == CVT_OBJECT):
                config = cvt_to_char(ep);
                if config is NULL:
                    config_value_destroy(ep)
                    raise Exception("[Client] Config cvt to char conversion failed")
            
                config_str = config.decode('utf-8')
                endpoint = json.loads(config_str)
            elif(ep.type == CVT_STRING):
                c_endpoint = ep.body.string
                if c_endpoint is NULL:
                    raise Exception("[Client] Endpoint getting string value failed")
                endpoint = c_endpoint.decode('utf-8')
            else:
                endpoint = None
                config_value_destroy(ep)
                raise TypeError("[Client] Type mismatch: EndPoint should be string or dict type")

            config_value_destroy(ep)
            return endpoint
        except TypeError as type_ex:
            raise type_ex
        except Exception as ex:
            raise ex
