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
"""EIS ConfigManager Python mapping
"""

# Python imports
from .exc import *
import json
import logging
import os

# Cython imports
from .libneweisconfigmgr cimport *
from .publisher cimport Publisher
from .subscriber cimport Subscriber
from .app_config import AppCfg
from .server cimport Server
from .client cimport Client


cdef class ConfigMgr:
    """EIS ConfigManager context object
    """
    cdef app_cfg_t* app_cfg
    cdef pub_cfg_t* pub_cfg
    cdef sub_cfg_t* sub_cfg
    cdef server_cfg_t* server_cfg
    cdef client_cfg_t* client_cfg

    def __init__(self):
        """Constructor
        """
        # Initializing cdef variables
        cdef char* env_var

        # Initializing app_cfg object
        log = logging.getLogger('config_manager')
        try:
            self.app_cfg = app_cfg_new()
            if self.app_cfg == NULL:
                raise Exception("app_cfg is NULL in config_manager")
            # Setting /GlobalEnv/ env variables
            env_var = self.app_cfg.env_var
            if env_var is NULL:
                log.info("env_var is not set in config manager base c layer,"
                         " continuing without setting env vars...")
            else:
                # Converting c string to py string
                config_str = env_var.decode('utf-8')
                # Converting py string to json object
                config_json = json.loads(config_str)
                # Iterating through and setting key, value pairs
                # of config_json in env
                for key, value in config_json.items():
                    os.environ[key] = value
        except Exception as ex:
            raise ex

    def __cinit__(self, *args, **kwargs):
        """Basic C init
        """
        pass

    def __dealloc__(self):
        """Deconstructor
        """
        if self.app_cfg != NULL:
            app_cfg_config_destroy(self.app_cfg)

    def get_app_config(self):
        """Calling base C get_app_config in order to get the 
        respective applications config

        :return: Return object of class AppCfg
        :rtype: obj
        """
        cdef config_t* conf
        cdef char* config
        try: 
            conf = get_app_config(self.app_cfg.base_cfg)
            if conf is NULL:
                raise Exception("[GetAppConfig] Conf received from base c layer is NULL")

            config = configt_to_char(conf)
            if config is NULL:
                raise Exception("[GetAppConfig] Configt to char conversion failed")
                
            config_str = config.decode('utf-8')
            cfg = json.loads(config_str)

            obj = AppCfg(cfg)
            return obj
        except Exception as ex:
            raise ex


    def is_dev_mode(self):
        """Calling bace c cfgmgr_is_dev_mode_base in order to get
        the dev mode variable
        
        :return: Whether dev mode is set
        :rtype: bool
        """
        # Calling the base C API to fetch appname
        try:
            ret = cfgmgr_is_dev_mode_base(self.app_cfg.base_cfg)
            if ret == 0:
                return True
            return False
        except Exception as ex:
            raise ex


    def get_app_name(self):
        """Calling bace c cfgmgr_get_appname_base in order to get
        the app name
        
        :return: App name
        :rtype: str
        """
        cdef config_value_t* appname
        try:
            # Calling the base C API to fetch appname
            appname = cfgmgr_get_appname_base(self.app_cfg.base_cfg)
            if appname is NULL:
                raise Exception("Getting AppName from base c layer failed")

            app_name = appname.body.string
            if app_name is NULL:
                raise Exception("Extraction of string from appname cvt failed")

            # Returning python string of appname
            return app_name.decode("utf-8")
        except Exception as ex:
            raise ex


    def get_publisher_by_name(self, name):
        """Calling bace c get_publisher_by_name in order to get
        respective publisher config

        :param name: Name of the publisher 
        :type: string
        :return: Publisher class object
        :rtype : obj
        """
        try:
            bname = bytes(name, 'utf-8')
            self.pub_cfg = cfgmgr_get_publisher_by_name(self.app_cfg, bname)
            if self.pub_cfg == NULL:
                raise Exception("pub_cfg is NULL in config_manager base c layer")

            # Create & return Publisher object
            return Publisher.create(self.app_cfg, self.pub_cfg)
        except Exception as ex:
            raise ex


    def get_publisher_by_index(self, index):
        """Calling bace c get_publisher_by_index in order to get
        respective publisher config

        :param index: Index of the publisher 
        :type: int
        :return: Publisher class object
        :rtype : obj
        """
        try:
            self.pub_cfg = cfgmgr_get_publisher_by_index(self.app_cfg, index)
            if self.pub_cfg == NULL:
                raise Exception("pub_cfg is NULL in config_manager base c layer")

            # Create & return Publisher object
            return Publisher.create(self.app_cfg, self.pub_cfg)
        except Exception as ex:
            raise ex

    def get_subscriber_by_name(self, name):
        """Calling bace c get_subscriber_by_name in order to get
        respective publisher config

        :param name: Name of the subscriber
        :type: string
        :return: Subscriber class object
        :rtype : obj
        """
        try:
            bname = bytes(name, 'utf-8')
            self.sub_cfg = cfgmgr_get_subscriber_by_name(self.app_cfg, bname)
            if self.sub_cfg == NULL:
                raise Exception("sub_cfg is NULL in config_manager base c layer")

            # Create & return Subscriber object
            return Subscriber.create(self.app_cfg, self.sub_cfg)
        except Exception as ex:
            raise ex
        

    def get_subscriber_by_index(self, index):
        """Calling bace c get_subscriber_by_index in order to get
        respective subscriber config

        :param index: Index of the subscriber 
        :type: int
        :return: Subscriber class object
        :rtype : obj
        """
        try:
            self.sub_cfg = cfgmgr_get_subscriber_by_index(self.app_cfg, index)
            if self.sub_cfg == NULL:
                raise Exception("sub_cfg is NULL in config_manager in base c layer")

            # Create & return Subscriber object
            return Subscriber.create(self.app_cfg, self.sub_cfg)
        except Exception as ex:
            raise ex


    def get_server_by_name(self, name):
        """Calling bace c get_server_by_name in order to get
        respective publisher config

        :param name: Name of the server
        :type: string
        :return: Server class object
        :rtype : obj
        """
        try:
            bname = bytes(name, 'utf-8')
            self.server_cfg = cfgmgr_get_server_by_name(self.app_cfg, bname)
            if self.server_cfg == NULL:
                raise Exception("server_cfg is NULL in config_manager base c layer")

            # Create & return Server object
            return Server.create(self.app_cfg, self.server_cfg)
        except Exception as ex:
            raise ex


    def get_server_by_index(self, index):
        """Calling bace c get_server_by_index in order to get
        respective publisher config

        :param index: Index of the server
        :type: string
        :return: Server class object
        :rtype : obj
        """
        try:
            self.server_cfg = cfgmgr_get_server_by_index(self.app_cfg, index)
            if self.server_cfg == NULL:
                raise Exception("server_cfg is NULL in config_manager base c layer")

            # Create & return Server object
            return Server.create(self.app_cfg, self.server_cfg)
        except Exception as ex:
            raise ex

    def get_client_by_name(self, name):
        """Calling bace c get_server_by_name in order to get
        respective publisher config

        :param name: Name of the client
        :type: string
        :return: Client class object
        :rtype : obj
        """
        try:
            bname = bytes(name, 'utf-8')
            self.client_cfg = cfgmgr_get_client_by_name(self.app_cfg, bname)
            if self.client_cfg == NULL:
                raise Exception("client_cfg is NULL in config_manager base c layer")

            # Create & return Client object
            return Client.create(self.app_cfg, self.client_cfg)
        except Exception as ex:
            raise ex

    def get_client_by_index(self, index):
        """Calling bace c get_server_by_index in order to get
        respective publisher config

        :param index: Index of the client
        :type: int
        :return: Client class object
        :rtype : obj
        """
        try:
            self.client_cfg = cfgmgr_get_client_by_index(self.app_cfg, index)
            if self.client_cfg == NULL:
                raise Exception("client_cfg is NULL in config_manager base c layer")

            # Create & return Client object
            return Client.create(self.app_cfg, self.client_cfg)
        except Exception as ex:
            raise ex

    def get_num_publishers(self):
        """Calling bace c cfgmgr_get_num_elements_base in order to get
        number of publishers in interface

        :return: number of publishers in interface
        :rtype : int
        """
        result = cfgmgr_get_num_elements_base("Publishers", self.app_cfg.base_cfg)
        if result == -1:
            raise Exception("[Publisher] Failed to get number of elements from base c layer")
        return result

    def get_num_subscribers(self):
        """Calling bace c cfgmgr_get_num_elements_base in order to get
        number of subscribers in interface

        :return: number of subscribers in interface
        :rtype : int
        """
        result = cfgmgr_get_num_elements_base("Subscribers", self.app_cfg.base_cfg)
        if result == -1:
            raise Exception("[Subscriber] Failed to get number of elements from base c layer")
        return result

    def get_num_servers(self):
        """Calling bace c cfgmgr_get_num_elements_base in order to get
        number of servers in interface

        :return: number of servers in interface
        :rtype : int
        """
        result = cfgmgr_get_num_elements_base("Servers", self.app_cfg.base_cfg)
        if result == -1:
            raise Exception("[Server] Failed to get number of elements from base c layer")
        return result

    def get_num_clients(self):
        """Calling bace c cfgmgr_get_num_elements_base in order to get
        number of clients in interface

        :return: number of clients in interface
        :rtype : int
        """
        result = cfgmgr_get_num_elements_base("Clients", self.app_cfg.base_cfg)
        if result == -1:
            raise Exception("[Client] Failed to get number of elements from base c layer")
        return result
