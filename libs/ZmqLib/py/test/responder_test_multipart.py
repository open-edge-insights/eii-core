import cv2
import os
import argparse
import json
import etcd3
from distutils.util import strtobool
from libs.ZmqLib.py.rep.responder import ZMQ_RESPONDER


def callback_a(request):
    print("Request from client {}".format(request))
    args = parse_args()
    save_to_disk = bool(strtobool(args.save_to_disk))
    etcd = etcd3.client()

    # Initiate the OpenCV VideoCapture
    if not os.path.exists("./pcb_d2000.avi"):
        print("Video file does not exist...")
    cap = cv2.VideoCapture("./pcb_d2000.avi")
    if not cap.isOpened():
        print("Failed to open video file...")

    # Read frames from video and send meta-data and numpy frame
    ret, frame = cap.read()
    if ret:
        metaData = dict(
            dtype=str(frame.dtype),
            shape=frame.shape,
            name="{0}".format('Frame')
        )
        # Sample etcd tests
        result = json.dumps(metaData)
        etcd.put('metaData', result)
        etcd_metaData = etcd.get('metaData')
        etcd.delete('metaData')

        # Send the meta-data and numpy frame
        list_multipart = [etcd_metaData[0], frame]
        # Write sent image frame to disk
        if save_to_disk:
            outputFilePath = "./{0}".format(metaData['name'])
            with open(outputFilePath, "wb") as outfile:
                outfile.write(frame)
        cap.release()
        return list_multipart
    print("Failed to read video file...")


def main():
    """main method"""
    args = parse_args()

    # Initiate the responder
    responder = ZMQ_RESPONDER("tcp://*:5563",
                              public_keys_dir=args.public_keys_dir,
                              secret_keys_dir=args.private_keys_dir)

    while True:
        responder.send_multipart(callback=callback_a)

    # Close socket
    responder.close_socket()


def parse_args():

    parser = argparse.ArgumentParser()

    parser.add_argument('--public-key', dest='public_keys_dir',
                        default="",
                        help='Public keys directory')

    parser.add_argument('--private-key', dest='private_keys_dir',
                        default="",
                        help='Public keys directory')

    parser.add_argument('--save-to-disk', dest='save_to_disk',
                        default="false",
                        help='Save to disk option')

    return parser.parse_args()


if __name__ == "__main__":
    main()
