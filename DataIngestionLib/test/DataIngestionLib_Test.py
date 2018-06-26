#!/usr/bin/env python3
import cv2
import logging
import numpy as np
from influxdb import InfluxDBClient
from DataAgent.da_grpc.client.client import GrpcClient
from DataIngestionLib.DataIngestionLib import DataIngestionLib as datain
from ImageStore.py.imagestore import ImageStore

measurement_name = 'stream1'

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
    di.set_measurement_name(measurement_name)

    frame_num = 0

    # Loop to read the frames of the video.
    while(capture.isOpened() and frame_num < 50):
        # Capture frame-by-frame
        ret, frame = capture.read()
        if ret is True:
            height, width, channels = frame.shape
            frame_num += 1
            log.info("Frame %d read successfully" % frame_num)
            # Added the buffer to datapoint after converting to byte stream.
            ret = di.add_fields("Frame{0}".format(frame_num), frame.tobytes())
            if ret is False:
                log.warning("Adding fields to datapoint failed."
                            % "Frame{0}".format(frame_num))
                continue
            ret = di.add_fields("FrameC{0}".format(frame_num), frame.tobytes())
            if ret is False:
                log.warning("Adding fields to datapoint failed."
                            % "Frame{0}".format(frame_num))
                continue
            # Adding Fields to datapoint.
            if di.add_fields("Part No", 12345) is False:
                log.warning("Adding Field %s Failed" % "Part No")
                continue
            if di.add_fields("Width", width) is False:
                log.warning("Adding Field %s Failed" % "Width")
                continue
            if di.add_fields("Height", height) is False:
                log.warning("Adding Field %s Failed" % "Height")
                continue
            if di.add_fields("Channels", channels) is False:
                log.warning("Adding Field %s Failed" % "Channels")
                continue
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
    try:
        di = datain()
    except Exception as e:
        log.error(e)
        exit(1)
    di.set_measurement_name(measurement_name)
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
    config = GrpcClient.GetConfigInt("InfluxDBCfg")
    influx_c = InfluxDBClient(config["Host"],
                              config["Port"],
                              config["UserName"],
                              config["Password"],
                              config["DBName"])
    result = influx_c.query('select ' + tag + ' from ' + measurement + ' ;')
    # print("Result: {0}".format(result))
    return result


def retrieve_and_write_frames(data_points):
    # Initialize Image Store.
    img_store = ImageStore()
    # Loop over the data points to retrive the frames using frame handles and
    # write the frames.
    for elem in data_points:
        if elem['ImageStore'] == '1':
            img_handles = elem['ImgHandle'].split(',')
            img_names = elem['ImgName'].split(',')
            for idx in range(len(img_handles)):
                ret, frame = img_store.read(img_handles[idx])
                if ret is True:
                    # Convert the buffer into np array.
                    Frame = np.frombuffer(frame, dtype=np.uint8)
                    # Reshape the array.
                    img_height = elem['Height']
                    img_width = elem['Width']
                    img_channels = elem['Channels']
                    reshape_frame = np.reshape(Frame, (img_height, img_width,
                                                       img_channels))
                    cv2.imwrite(img_names[idx]+".jpg", reshape_frame)
                else:
                    log.error("Frame read unsuccessfull.")


if __name__ == '__main__':

    # Sending a point data to influx.
    try:
        send_point_data()
    except Exception as e:
        log.error(e)
        exit(1)
    # Sending a buffer data to influx/imagestore.
    try:
        send_buffer_and_point_data()
    except Exception as e:
        log.error(e)
        exit(1)
    # Retrieve data from database.
    data_points = retrieve_data_from_influx(measurement_name, '*')

    # Get the data points in a list.
    data_pts_list = list(data_points.get_points())

    # Retrive the frames from Imagestore and write a video file.
    retrieve_and_write_frames(data_pts_list)
