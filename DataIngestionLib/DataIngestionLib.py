#!/usr/bin/env python3
import logging
from influxdb import InfluxDBClient
import DataIngestionLib.settings as settings
from ImageStore.py.imagestore import ImageStore


class DataIngestionLib:
    ''' This class helps in ingesting the data to the imagestore and influx.
    If the data contains buffer, it stores the buffer into the image store
    and saves the handle in database.'''

    def __init__(self, storage_type='inmemory', log_level=logging.INFO):
        logging.basicConfig(level=log_level)
        self.log = logging.getLogger('data_ingestion')
        self.data_point = {'tags': {}, 'fields': {}}
        self.img_store = ImageStore()
        self.img_store.setStorageType('inmemory')
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
            return self._add_fields_buffer(name, value, time)
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
        ret, handle = self.img_store.store(value)
        if ret is not True:
            self.log.error("Error in saving the buffer into ImageStore.")
            return ret
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
        '''Saves the Data Point. Internally sends the data point to InfluxDB.'''
        return self._send_data_to_influx(settings.value.url,
                                         settings.value.port,
                                         settings.value.username,
                                         settings.value.password,
                                         settings.value.db)

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
