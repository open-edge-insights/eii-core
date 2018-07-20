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
