"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

from influxdb import InfluxDBClient
import datetime
import time


def send_data_to_influx(url, port, username, password, db, data):
    '''Sends the data point to Influx.'''
    # Creates the influxDB client handle.
    influx_c = InfluxDBClient(url, port, username, password, db)
    json_body = [data]
    ret = influx_c.write_points(json_body)
    if ret is True:
        print("Data Point sent successfully to influx.")
    else:
        print("Data Point sending to influx failed.")
    # Removes the fields sections after sending successfully.

    return ret


if __name__ == "__main__":
    data_point = {"tags": {'ImageStore': 1}, "fields": {}, "measurement": "classifier_results"}
    while True:
        data_point["fields"]["val"] = "Timestamp: {:%Y-%m-%d %H:%M:%S}".format(datetime.datetime.now())
        print("Data -> influx: ", data_point)
        send_data_to_influx("localhost", "8086", "root", "root", "datain", data_point)
        time.sleep(5)
