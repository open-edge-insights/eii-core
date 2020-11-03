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

cdef class Util:
    """EIS Message Bus Publisher object
    """

    def __init__(self):
        """Constructor
        """
        pass

    @staticmethod
    cdef get_cvt_data(config_value_t* cvt):
        """Helper method for extracting data from config_value_t struct.

        :param value: config_value_t struct
        :type: struct
        :return: value
        :rtype: integer/subscriber/float/boolean/dict/list based on the cvt data type
        """
        cdef char* c_value
        try:
            if(cvt.type == CVT_INTEGER):
                value = cvt.body.integer
            elif(cvt.type == CVT_FLOATING):
                value = cvt.body.floating
            elif(cvt.type == CVT_STRING):
                c_value = cvt.body.string
                if c_value is NULL:
                    raise Exception("Failed to get string value from cvt in util")
                value = c_value.decode('utf-8')
            elif(cvt.type == CVT_BOOLEAN):
                value = cvt.body.boolean
            elif(cvt.type == CVT_OBJECT or cvt.type == CVT_ARRAY):
                config = cvt_to_char(cvt);
                if config is NULL:
                    raise Exception("cvt to char failed in util")

                config_str = config.decode('utf-8')
                value = json.loads(config_str)
            else:
                value = None
                raise TypeError("Type mismatch of Interface value")
            
            return value
        except TypeError as type_ex:
            raise type_ex
        except Exception as ex:
            raise ex
