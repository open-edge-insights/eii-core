#!/usr/bin/env python3

"""
Copyright (c) 2018 Intel Corporation.

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

import logging
from influxdb import InfluxDBClient
from DataAgent.da_grpc.client.py.client import GrpcClient
from ImageStore.py.imagestore import ImageStore
from Util.exception import DAException


class DataIngestionLib:
    ''' This class helps in ingesting the data to the imagestore and influx.
    If the data contains buffer, it stores the buffer into the image store
    and saves the handle in database.'''

    def __init__(self, storage_type='inmemory', log_level=logging.INFO):
        logging.basicConfig(level=log_level)
        self.log = logging.getLogger('data_ingestion')
        self.data_point = {'tags': {}, 'fields': {}}
        try:
            client = GrpcClient()
            self.config = client.GetConfigInt("InfluxDBCfg")
        except Exception as e:
            raise DAException("Seems to be some issue with gRPC server." +
                              "Exception: {0}".format(e))

        # TODO: plan a better approach to set this config later, not to be in
        # DataAgent.conf file as it's not intended to be known to user
        try:
            self.img_store = ImageStore()
            self.img_store.setStorageType(storage_type)
        except Exception as e:
            raise e
        self.log.debug("Instance created successfully.")

    def add_fields(self, name, value, time=None):
        ''' This method adds fields to the data point which will be
        stored in the database.
        Arguments:
            name: Specifies the name of the sensor/stream. Added as tag in LP.
            value: Value of the sensor or the buffer. Only int, float, str and
            buffer is accepted in byte stream format.
            time: Specifies the timestamp at which the value was captured. If
            not specified, storage time is used as timestamp.
        Returns: True, if field added to Data Point successfully.'''
        if isinstance(value, bytes) is True:
            try:
                return self._add_fields_buffer(name, value, time)
            except Exception as e:
                raise(e)
        elif (isinstance(value, int) or isinstance(value, float)
                or isinstance(value, str)):
            if name == 'ImgHandle' or name == 'ImgName':
                self.log.error("Name not supported.")
                return False
            else:
                self.data_point['fields'][name] = value
            if time is not None:
                self.data_point['time'] = time
        else:
            self.log.error("Value Type not supported.")
            return False
        return True

    def _add_fields_buffer(self, name, value, time):
        ''' This function sends the buffer to imagestore and saves the handle
        returned in the data point which is further send to database. If more
        than one buffer is present in a datapoint, the buffer and its name is
        stored in comma separated way in the field.'''
        # ToDo: Send the buffer to Image Store.
        try:
            handle = self.img_store.store(value)
        except Exception as e:
            raise(e)
        if handle is None:
            self.log.error("Error in saving the buffer into ImageStore.")
            return False
        self.log.info("Stored the buffer in Image Store .")

        # Save the handle of the image into Data Point.
        self.data_point['tags']['ImageStore'] = 1
        ImgHandle = 'ImgHandle'
        ImgName = 'ImgName'
        if ImgHandle in self.data_point['fields']:
            self.data_point['fields'][ImgHandle] =\
                self.data_point['fields'][ImgHandle] + ',' + str(handle)
        else:
            self.data_point['fields'][ImgHandle] = str(handle)

        if ImgName in self.data_point['fields']:
            self.data_point['fields'][ImgName] =\
                self.data_point['fields'][ImgName] + ',' + name
        else:
            self.data_point['fields'][ImgName] = name

        if time is not None:
            self.data_point['time'] = time
        self.log.debug("Added the buffer handle to Data Point.")
        return True

    def set_measurement_name(self, measurement_name):
        ''' Sets the measurement name. The measurement name is required by
        influx in its line protocol.
        Arguments:
            measurement_name: Required for constructing Line Protocol.'''
        self.data_point['measurement'] = measurement_name

    def attach_tags(self, tags):
        ''' Attaches the tags to the value in datapoint in line protocol.
        Arguments:
            tags: A dict of all the tags to be attached to the value '''
        for tag_name, tag in tags.items():
            if tag_name != 'ImageStore':
                self.data_point['tags'][tag_name] = tag
                self.log.debug("Tag " + tag_name + " added.")
            else:
                self.log.warning("Tag name ImageStore is reserved. Can't add")

    def remove_tags(self, tags):
        ''' Removes the tags attached to a value in line protocol.
        Arguments:
            tags: A dict containing tags to be removed from the value.'''

        for tag_name, tag in tags.items():
            if tag_name in self.data_point['tags']:
                del self.data_point['tags'][tag_name]
            else:
                self.log.info("%s Tag not present" % tag_name)

    def save_data_point(self):
        '''Saves the Data Point. Internally sends the data point to InfluxDB'''
        try:
            return self._send_data_to_influx(self.config["Host"],
                                             self.config["Port"],
                                             self.config["UserName"],
                                             self.config["Password"],
                                             self.config["DBName"])
        except Exception as e:
            raise DAException("Seems to be some issue with InfluxDB." +
                              "Exception: {0}".format(e))

    def _send_data_to_influx(self, url, port, username, password, db):
        '''Sends the data point to Influx.'''
        # Creates the influxDB client handle.
        influx_c = InfluxDBClient(url, port, username, password, db)
        json_body = [self.data_point]
        ret = influx_c.write_points(json_body)
        if ret is True:
            self.log.info("Data Point sent successfully to influx.")
        else:
            self.log.error("Data Point sending to influx failed.")

        # Removes the fields sections after sending successfully.
        self.data_point['fields'] = {}
        self.data_point['tags'] = {}
        if 'time' in self.data_point:
            del self.data_point['time']

        return ret
