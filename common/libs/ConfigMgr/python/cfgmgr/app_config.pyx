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
"""EIS Message Bus Publisher wrapper object
"""

from .libneweisconfigmgr cimport *


cdef void watch_callback_fn(const char* key, config_t* value, void* func) with gil:
    """C callback def which internally calls
       the Py callback function
    """
    # TODO: Registering multiple callbacks to the same key
    # is not supported in cython yet
    py_value = configt_to_char(value)
    if py_value is NULL:
        raise Exception("[Watch] Failed to convert value to char")
    # Destroying config_t* obj after converting to py obj
    config_destroy(value)
    (<object>func)(key.decode(), py_value.decode())

class AppCfg:
    """EIS Message Bus Publisher object
    """
  
    def __init__(self, cfg):
        """Constructor
        """
        self.cfg = cfg

    def __getitem__(self, key):
        """ iterator of the class

        :param: key: Key on which value is retrived
        :type: string
        :return: value of the key
        :rtype: string
        """
        return self.cfg[key]

    def get_dict(self):
        """ Get JSON object from the class object

        :return: JSON object of AppCfg
        :rtype: JSON
        """
        return self.cfg

cdef class Watch:
    """EIS Message Bus Watch class
    """

    def __init__(self):
        """Constructor
        """
        pass

    def __cinit__(self, *args, **kwargs):
        """Cython base constructor
        """
        self.app_cfg = NULL

    @staticmethod
    cdef create(app_cfg_t* app_cfg):
        """Helper method for initializing the client object.

        :param app_cfg: Applications config struct
        :type: struct
        :return: Watch class object
        :rtype: obj
        """
        w = Watch()
        w.app_cfg = app_cfg
        return w

    def __dealloc__(self):
        """Cython destructor
        """
        self.destroy()

    def destroy(self):
        """Destroy the publisher.
        """
        pass

    def watch(self, key, pyFunc):
        """Method to watch over a given key
           Calls the base C cfgmgr_watch() API

        :param key: key to watch on
        :type: str
        :param pyFunc: python function
        :type: object
        """
        try:
            cfgmgr_watch(self.app_cfg.base_cfg, bytes(key, 'utf-8'), watch_callback_fn, <void *> pyFunc)
            return
        except Exception as ex:
            raise Exception("[Watch] Failed to register watch callback {}".format(ex))

    def watch_prefix(self, prefix, pyFunc):
        """Method to watch over a given prefix
           Calls the base C cfgmgr_watch_prefix() API

        :param prefix: prefix to watch on
        :type: str
        :param pyFunc: python function
        :type: object
        """
        try:
            cfgmgr_watch_prefix(self.app_cfg.base_cfg, bytes(prefix, 'utf-8'), watch_callback_fn, <void *> pyFunc)
            return
        except Exception as ex:
            raise Exception("[Watch] Failed to register watch_prefix callback {}".format(ex))

    def watch_config(self, pyFunc):
        """Method to watch over an application's config
           Calls the base C cfgmgr_watch() API

        :param pyFunc: python function
        :type: object
        """
        app_name = self.app_cfg.base_cfg.app_name.decode()
        config_key = "/" + app_name + "/config"
        try:
            cfgmgr_watch(self.app_cfg.base_cfg, bytes(config_key, 'utf-8'), watch_callback_fn, <void *> pyFunc)
            return
        except Exception as ex:
            raise Exception("[Watch] Failed to register watch config callback {}".format(ex))

    def watch_interface(self, pyFunc):
        """Method to watch over an application's interfaces
           Calls the base C cfgmgr_watch() API

        :param pyFunc: python function
        :type: object
        """
        app_name = self.app_cfg.base_cfg.app_name.decode()
        interface_key = "/" + app_name + "/interfaces"
        try:
            cfgmgr_watch(self.app_cfg.base_cfg, bytes(interface_key, 'utf-8'), watch_callback_fn, <void *> pyFunc)
            return
        except Exception as ex:
            raise Exception("[Watch] Failed to register watch interface callback {}".format(ex))
