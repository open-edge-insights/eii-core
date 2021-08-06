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
"""EII ConfigManager Python mapping
"""

# Python imports
from .exc import *
import json
import logging
import os

# Cython imports
from .libeiiconfigmanager cimport *
from .publisher cimport Publisher
from .subscriber cimport Subscriber
from .app_config import AppCfg
from .app_config cimport Watch
from .server cimport Server
from .client cimport Client
from libc.stdlib cimport free


cdef class ConfigMgr:
    """EII ConfigManager context object
    """
    cdef cfgmgr_ctx_t* cfgmgr
    cdef cfgmgr_interface_t* cfgmgr_interface

    def __init__(self):
        """Constructor
        """
        # Initializing cdef variables
        cdef char* env_var

        # Initializing app_cfg object
        log = logging.getLogger('config_manager')
        try:
            self.cfgmgr = cfgmgr_initialize()
            if self.cfgmgr == NULL:
                raise Exception("cfgmgr initialization failed")
            # Setting /GlobalEnv/ env variables
            env_var = self.cfgmgr.env_var
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
        if self.cfgmgr != NULL:
            cfgmgr_destroy(self.cfgmgr)

    def get_app_config(self):
        """gets AppCfg object respective applications config

        :return: Return object of class AppCfg
        :rtype: obj
        """
        cdef config_t* conf
        cdef char* config
        try: 
            conf = cfgmgr_get_app_config(self.cfgmgr)
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


    def get_watch_obj(self):
        """Fetching the object to call watch APIs

        :return: Watch class object
        :rtype : obj
        """
        try:
            w = Watch.create(self.cfgmgr)
            return w
        except Exception as ex:
            raise Exception("[Watch] Failed to fetch watch object {}".format(ex))


    def is_dev_mode(self):
        """To check if application is running in dev or prod mode
        
        :return: Whether dev mode is set
        :rtype: bool
        """
        # Calling the base C API to fetch appname
        try:
            ret = cfgmgr_is_dev_mode(self.cfgmgr)
            if ret:
                return True
            return False
        except Exception as ex:
            raise ex


    def get_app_name(self):
        """Get the AppName for any application
        
        :return: App name
        :rtype: str
        """
        cdef config_value_t* appname
        cdef char* app_name
        try:
            # Calling the base C API to fetch appname
            appname = cfgmgr_get_appname(self.cfgmgr)
            if appname is NULL:
                raise Exception("Getting AppName from base c layer failed")

            if appname.body.string is NULL:
                raise Exception("Extraction of string from appname cvt failed")

            app_name = appname.body.string
            py_app_name = app_name.decode()

            # Returning python string of appname
            config_value_destroy(appname)
            return py_app_name
        except Exception as ex:
            raise ex


    def get_publisher_by_name(self, name):
        """To fetch a publisher interface using it's name

        :param name: Name of the publisher 
        :type: string
        :return: Publisher class object
        :rtype : obj
        """
        try:
            bname = bytes(name, 'utf-8')
            self.cfgmgr_interface = cfgmgr_get_publisher_by_name(self.cfgmgr, bname)
            if self.cfgmgr_interface == NULL:
                raise Exception("pub_cfg is NULL in config_manager base c layer")

            # Create & return Publisher object
            return Publisher.create(self.cfgmgr_interface)
        except Exception as ex:
            raise ex


    def get_publisher_by_index(self, index):
        """To fetch a publisher interface using it's index

        :param index: Index of the publisher 
        :type: int
        :return: Publisher class object
        :rtype : obj
        """
        try:
            self.cfgmgr_interface = cfgmgr_get_publisher_by_index(self.cfgmgr, index)
            if self.cfgmgr_interface == NULL:
                raise Exception("pub_cfg is NULL in config_manager base c layer")

            # Create & return Publisher object
            return Publisher.create(self.cfgmgr_interface)
        except Exception as ex:
            raise ex

    def get_subscriber_by_name(self, name):
        """To fetch a subscriber interface using it's name

        :param name: Name of the subscriber
        :type: string
        :return: Subscriber class object
        :rtype : obj
        """
        try:
            bname = bytes(name, 'utf-8')
            self.cfgmgr_interface = cfgmgr_get_subscriber_by_name(self.cfgmgr, bname)
            if self.cfgmgr_interface == NULL:
                raise Exception("sub_cfg is NULL in config_manager base c layer")

            # Create & return Subscriber object
            return Subscriber.create(self.cfgmgr_interface)
        except Exception as ex:
            raise ex
        

    def get_subscriber_by_index(self, index):
        """To fetch a subscriber interface using it's index

        :param index: Index of the subscriber 
        :type: int
        :return: Subscriber class object
        :rtype : obj
        """
        try:
            self.cfgmgr_interface = cfgmgr_get_subscriber_by_index(self.cfgmgr, index)
            if self.cfgmgr_interface == NULL:
                raise Exception("sub_cfg is NULL in config_manager in base c layer")

            # Create & return Subscriber object
            return Subscriber.create(self.cfgmgr_interface)
        except Exception as ex:
            raise ex


    def get_server_by_name(self, name):
        """To fetch a server interface using it's name

        :param name: Name of the server
        :type: string
        :return: Server class object
        :rtype : obj
        """
        try:
            bname = bytes(name, 'utf-8')
            self.cfgmgr_interface = cfgmgr_get_server_by_name(self.cfgmgr, bname)
            if self.cfgmgr_interface == NULL:
                raise Exception("server_cfg is NULL in config_manager base c layer")

            # Create & return Server object
            return Server.create(self.cfgmgr_interface)
        except Exception as ex:
            raise ex


    def get_server_by_index(self, index):
        """To fetch a server interface using it's index

        :param index: Index of the server
        :type: string
        :return: Server class object
        :rtype : obj
        """
        try:
            self.cfgmgr_interface = cfgmgr_get_server_by_index(self.cfgmgr, index)
            if self.cfgmgr_interface == NULL:
                raise Exception("server_cfg is NULL in config_manager base c layer")

            # Create & return Server object
            return Server.create(self.cfgmgr_interface)
        except Exception as ex:
            raise ex

    def get_client_by_name(self, name):
        """To fetch a client interface using it's name

        :param name: Name of the client
        :type: string
        :return: Client class object
        :rtype : obj
        """
        try:
            bname = bytes(name, 'utf-8')
            self.cfgmgr_interface = cfgmgr_get_client_by_name(self.cfgmgr, bname)
            if self.cfgmgr_interface == NULL:
                raise Exception("client_cfg is NULL in config_manager base c layer")

            # Create & return Client object
            return Client.create(self.cfgmgr_interface)
        except Exception as ex:
            raise ex

    def get_client_by_index(self, index):
        """To fetch a client interface using it's index

        :param index: Index of the client
        :type: int
        :return: Client class object
        :rtype : obj
        """
        try:
            self.cfgmgr_interface = cfgmgr_get_client_by_index(self.cfgmgr, index)
            if self.cfgmgr_interface == NULL:
                raise Exception("client_cfg is NULL in config_manager base c layer")

            # Create & return Client object
            return Client.create(self.cfgmgr_interface)
        except Exception as ex:
            raise ex

    def get_num_publishers(self):
        """Get total number of publishers from publisher's interface

        :return: number of publishers in interface
        :rtype : int
        """
        result = cfgmgr_get_num_publishers(self.cfgmgr)
        if result == -1:
            raise Exception("[Publisher] No publisher instances found")
        return result

    def get_num_subscribers(self):
        """Get total number of subscribers from subscriber's interface

        :return: number of subscribers in interface
        :rtype : int
        """
        result = cfgmgr_get_num_subscribers(self.cfgmgr)
        if result == -1:
            raise Exception("[Subscriber] No subscriber instances found")
        return result

    def get_num_servers(self):
        """Get total number of servers from server's interface

        :return: number of servers in interface
        :rtype : int
        """
        result = cfgmgr_get_num_servers(self.cfgmgr)
        if result == -1:
            raise Exception("[Server] No server instances found")
        return result

    def get_num_clients(self):
        """Get total number of clients from client's interface

        :return: number of clients in interface
        :rtype : int
        """
        result = cfgmgr_get_num_clients(self.cfgmgr)
        if result == -1:
            raise Exception("[Client] No client instances found")
        return result
