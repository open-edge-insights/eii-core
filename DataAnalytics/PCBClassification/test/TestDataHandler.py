#!/usr/bin/env python3.6

"""
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import os
import unittest
import datetime
import json

from Util.log import configure_logging
from DataAnalytics.PCBClassification.DataHandler import DataHandler


class DataHandlerTest(unittest.TestCase):
    """Test cases for DataHandler module"""

    @classmethod
    def setUpClass(cls):
        current_date_time = str(datetime.datetime.now())
        list_date_time = current_date_time.split(" ")
        current_date_time = "_".join(list_date_time)

        log_dir = 'testPCBDemo'
        if not os.path.exists(log_dir):
            os.mkdir(log_dir)

        log_file_name = 'test_datahandler_' + current_date_time + '.log'
        cls.log = configure_logging('DEBUG', log_file_name, log_dir, __name__)
        cls.config = '../../docker_setup/config/factory.json'
        cls.dh = DataHandler(cls.config, cls.log)

    def test_init(self):
        self.dh.init()
        self.assertNotEqual(self.dh.classifier, None)
        self.assertNotEqual(self.dh.img_store, None)
        self.assertNotEqual(self.dh.di, None)

    def test_handle_point_data(self):
        self.dh.init()

        # Read stream1 measurement
        influx_client = self.dh.di.influx_c
        result_set = influx_client.query(
            'select * from stream1 order by time desc limit 2;')
        data_points = result_set.get_points()

        # Test the handle_point_data method
        for data_point in data_points:
            result = self.dh.handle_point_data(json.dumps(data_point))
            user_data = data_point['user_data']
            if user_data == -1:
                self.assertEqual(result, None)
            else:
                self.assertEqual(result, True)

        # -ve scenario when invalid img_handle passed
        point_data_json = '{"time": "2019-03-12T17:09:14.839885818Z",' \
                          ' "Cam_Sn": "pcb_d2000.avi", "Channels": 3,' \
                          ' "Height": 1200, "ImageStore": "1",' \
                          ' "ImgHandle": "inmem_invalid,persist_invalid", ' \
                          '"ImgName": "vid-fr-inmem,vid-fr-persist",' \
                          ' "Sample_num": 0, "Width": 1920, "user_data": 1}'

        self.assertRaises(
            Exception, self.dh.handle_point_data(point_data_json))

        # imageHandle not exists
        point_data_json = '{"time": "2019-03-12T17:09:14.839885818Z",' \
                          ' "Cam_Sn": "pcb_d2000.avi", "Channels": 3,' \
                          ' "Height": 1200, "ImageStore": "1",' \
                          ' "ImgHandle": "", ' \
                          '"ImgName": "vid-fr-inmem,vid-fr-persist",' \
                          ' "Sample_num": 0, "Width": 1920, "user_data": 1}'
        self.assertEqual(self.dh.handle_point_data(point_data_json), None)


if __name__ == '__main__':
    unittest.main()

# To run the test, use following command
# /DataAnalytics/PCBClassification$ python3.6 test/TestDataHandler.py
