#!/usr/bin/env python3
import cv2
import logging
from influxdb import InfluxDBClient
from DataIngestionLib.DataIngestionLib import DataIngestionLib as datain
from DataIngestionLib import settings

logging.basicConfig(level=logging.INFO)
log = logging.getLogger("DATA_IN_TEST")


def send_buffer_and_point_data():
    ret = True
    di = datain()
    capture = cv2.VideoCapture('bigbuckbunny.mp4')

    # Check if camera opened successfully
    if (capture.isOpened() is False):
        log.error("Error opening video stream or file")

    # Set the measurement name.
    di.set_measurement_name('big_buck_bunny_video')

    frame_num = 0

    # Read the first 10 frames of the video.
    while(capture.isOpened() and frame_num < 10):
        # Capture frame-by-frame
        ret, frame = capture.read()
        if ret is True:
            frame_num += 1
            log.info("Frame %d read successfully" % frame_num)
            # Added the buffer to datapoint after converting to byte stream.
            ret = di.add_fields("Frame"+str(frame_num), frame.tobytes())
            ret = di.add_fields("FrameC"+str(frame_num), frame.tobytes())
            if ret is False:
                log.warning("Adding of fields to data point failed.")
            else:
                # Adding Fields to datapoint.
                ret = di.add_fields("Part No", 12345)
                # Attached the tag video to the data point.
                di.attach_tags({'stream': 'video'})
                ret = di.save_data_point()
                if ret is False:
                    log.error("Data point saving failed")
                else:
                    log.info("Data point saved successfully")

        else:
            log.info("Capture Closed")
            break

    # Release the resources.
    capture.release()
    return ret


def send_point_data():
    di = datain()
    measurement = 'PointExample'

    # Set the measurement name.
    di.set_measurement_name(measurement)

    # Adding 4 Data points each with three fields and saving them.
    for idx in range(0, 4):
        ret = di.add_fields('X', 84 + idx)
        if ret is False:
            log.warning("Adding of field %s to data point failed." % 'X')
        ret = di.add_fields('Y', 84.234 + idx)
        if ret is False:
            log.warning("Adding of field %s to data point failed." % 'Y')
        ret = di.add_fields('Z', "Intel123_" + str(idx))
        if ret is False:
            log.warning("Adding of field %s to data point failed." % 'Z')

        ret = di.save_data_point()
        if ret is False:
            log.error("Data point saving failed")
        else:
            log.info("Data point saved successfully")


def retrieve_data_from_influx(measurement, tag):
    influx_c = InfluxDBClient(settings.value.url,
                              settings.value.port,
                              settings.value.username,
                              settings.value.password,
                              settings.value.db)
    result = influx_c.query('select ' + tag + ' from ' + measurement + ' ;')
    print("Result: {0}".format(result))


if __name__ == '__main__':
    # Sending a point data to influx.
    send_point_data()

    # Sending a buffer data to influx/imagestore.
    send_buffer_and_point_data()

    # Retrieve data from database.
    retrieve_data_from_influx('big_buck_bunny_video', '*')
