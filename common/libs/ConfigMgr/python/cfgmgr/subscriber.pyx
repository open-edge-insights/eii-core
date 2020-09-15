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
"""EIS Message Bus Subscriber wrapper object
"""

import json

from .libneweisconfigmgr cimport *
from libc.stdlib cimport malloc


cdef class Subscriber:
    """EIS Message Bus Subscriber object
    """

    def __init__(self):
        """Constructor
        """
        pass

    def __cinit__(self, *args, **kwargs):
        """Cython base constructor
        """
        self.app_cfg = NULL
        self.sub_cfg = NULL

    @staticmethod
    cdef create(app_cfg_t* app_cfg, sub_cfg_t* sub_cfg):
        """Helper method for initializing the client object.

        :param app_cfg: Applications config struct
        :type: struct
        :param sub_cfg: Subscriber config struct
        :type: struct
        :return: Subscriber class object
        :rtype: obj
        """
        s = Subscriber()
        s.app_cfg = app_cfg
        s.sub_cfg = sub_cfg
        return s

    def __dealloc__(self):
        """Cython destructor
        """
        self.destroy()

    def destroy(self):
        """Close the subscriber.
        """
        if self.app_cfg != NULL:
            self.app_cfg = NULL
        if self.sub_cfg != NULL:
            self.sub_cfg = NULL

    def get_msgbus_config(self):
        """Calling the base C get_msgbus_config() API

        :return: Messagebus config
        :rtype: dict
        """
        cdef char* config
        new_config_new = self.sub_cfg.get_msgbus_config_sub(self.app_cfg.base_cfg)
        config = configt_to_char(new_config_new)
        config_str = config.decode('utf-8')
        return json.loads(config_str)

    def get_endpoint(self):
        """Calling the base C get_endpoint() API

        :return: Endpoint config
        :rtype: string
        """
        ep = self.sub_cfg.get_endpoint_sub(self.app_cfg.base_cfg)
        return ep.decode('utf-8')
    
    def get_topics(self):
        """Calling the base C get_topics() API
        
        :return: List of topics
        :rtype: List
        """
        topics = self.sub_cfg.get_topics_sub(self.app_cfg.base_cfg)
        topics_list = []
        i = 0
        while topics[i] != NULL:
            topics_list.append(topics[i].decode('utf-8'))
            i += 1
        return topics_list

    # TODO: This API is segfaulting for unknown reasons, should be fixed soon
    def set_topics(self, topics_list):
        cdef int topics_set
        cdef char** topics_to_be_set
        # Allocate memory
        topics_to_be_set = <char**>malloc(len(topics_list) * sizeof(char*))
        # Check if allocation went fine
        if topics_to_be_set is NULL:
            print("Out of memory")
        # Convert str to char* and store it into our char**
        for i in range(len(topics_list)):
            topics_list[i] = topics_list[i].encode()
            topics_to_be_set[i] = topics_list[i]
        # Calling the base C set_topics_sub() API
        topics_set = self.sub_cfg.set_topics_sub(topics_to_be_set, self.app_cfg.base_cfg)
        return 0