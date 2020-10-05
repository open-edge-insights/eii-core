# Copyright (c) 2020 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import os
import json
import queue
from distutils.util import strtobool
import cv2
import numpy as np
import eis.msgbus as mb


class Visualizer:
    """Object for the databus callback to wrap needed state variables for the
    callback in to EIS.
    """

    def __init__(self, topic_queue_dict, logger, draw_results,
                 good_color=(0, 255, 0), bad_color=(0, 0, 255), dir_name=None,
                 save_image="False", labels=None):
        """Constructor

        :param topic_queue_dict: Dictionary to maintain multiple queues.
        :type: dict
        :param labels: (Optional) Label mapping for text to draw on the frame
        :type: dict
        :param good_color: (Optional) Tuple for RGB color to use for outlining
            a good image
        :type: tuple
        :param bad_color: (Optional) Tuple for RGB color to use for outlining a
            bad image
        :type: tuple
        :param draw_results: For enabling bounding box in visualizer
        :type: string

        """
        self.topic_queue_dict = topic_queue_dict
        self.logger = logger
        self.good_color = good_color
        self.bad_color = bad_color
        self.dir_name = dir_name
        self.save_image = bool(strtobool(save_image))
        self.labels = labels
        self.msg_frame_queue = queue.Queue(maxsize=15)
        self.draw_results = bool(strtobool(draw_results))

    def queue_publish(self, topic, frame):
        """queue_publish called after defects bounding box is drawn
        on the image. These images are published over the queue.

        :param topic: Topic the message was published on
        :type: str
        :param frame: Images with the bounding box
        :type: numpy.ndarray
        """
        for key in self.topic_queue_dict:
            if key == topic:
                if not self.topic_queue_dict[key].full():
                    self.topic_queue_dict[key].put_nowait(frame)
                    del frame
                else:
                    self.logger.debug("Dropping frames as the queue is full")

    def decode_frame(self, results, blob):
        """Identify the defects on the frames

        :param results: Metadata of frame received from message bus.
        :type: dict
        :param blob: Actual frame received from message bus.
        :type: bytes
        :return: Return classified results(metadata and frame)
        :rtype: dict and numpy array
        """
        height = int(results['height'])
        width = int(results['width'])
        channels = int(results['channels'])
        encoding = None

        if 'encoding_type' and 'encoding_level' in results:
            encoding = {"type": results['encoding_type'],
                        "level": results['encoding_level']}
        # Convert to Numpy array and reshape to frame
        self.logger.debug('Preparing frame for visualization')
        frame = np.frombuffer(blob, dtype=np.uint8)
        if encoding is not None:
            frame = np.reshape(frame, (frame.shape))
            try:
                frame = cv2.imdecode(frame, 1)
            except cv2.error as ex:
                self.logger.error("frame: {}, exception: {}".format(frame, ex))
        else:
            self.logger.debug("Encoding not enabled...")
            frame = np.reshape(frame, (height, width, channels))

        return frame

    def draw_defect(self, results, frame, stream_label=None):
        """Draw boxes on the frames

        :param results: Metadata of frame received from message bus.
        :type: dict
        :param frame: Classified frame.
        :type: numpy
        :param stream_label: Message received on the given topic (JSON blob)
        :type: str
        """
        # Draw defects for Gva
        if 'gva_meta' in results:
            count = 0
            for defect in results['gva_meta']:
                x_1 = defect['x']
                y_1 = defect['y']
                x_2 = x_1 + defect['width']
                y_2 = y_1 + defect['height']

                top_left = tuple([x_1, y_1])
                bottom_right = tuple([x_2, y_2])

                # Draw bounding box
                cv2.rectangle(frame, top_left,
                              bottom_right, self.bad_color, 2)

                # Draw labels
                for label_list in defect['tensor']:
                    if label_list['label_id'] is not None:
                        pos = (x_1, y_1 - count)
                        count += 10
                        if stream_label is not None and \
                           str(label_list['label_id']) in stream_label:
                            label = stream_label[str(label_list['label_id'])]
                            cv2.putText(frame, label, pos,
                                        cv2.FONT_HERSHEY_DUPLEX,
                                        0.5, self.bad_color, 2,
                                        cv2.LINE_AA)
                        else:
                            self.logger.error("Label id:{}\
                                              not found".format(
                                label_list['label_id']))

        # Draw defects
        if 'defects' in results:
            for defect in results['defects']:
                defect['tl'][0] = int(defect['tl'][0])
                defect['tl'][1] = int(defect['tl'][1])
                defect['br'][0] = int(defect['br'][0])
                defect['br'][1] = int(defect['br'][1])

                # Get tuples for top-left and bottom-right coordinates
                top_left = tuple(defect['tl'])
                bottom_right = tuple(defect['br'])

                # Draw bounding box
                cv2.rectangle(frame, top_left,
                              bottom_right, self.bad_color, 2)

                # Draw labels for defects if given the mapping
                if stream_label is not None:
                    # Position of the text below the bounding box
                    pos = (top_left[0], bottom_right[1] + 20)

                    # The label is the "type" key of the defect, which
                    #  is converted to a string for getting from the labels
                    if str(defect['type']) in stream_label:
                        label = stream_label[str(defect['type'])]
                        cv2.putText(frame, label, pos,
                                    cv2.FONT_HERSHEY_DUPLEX,
                                    0.5, self.bad_color, 2, cv2.LINE_AA)
                    else:
                        cv2.putText(frame, str(defect['type']), pos,
                                    cv2.FONT_HERSHEY_DUPLEX,
                                    0.5, self.bad_color, 2, cv2.LINE_AA)

            # Draw border around frame if has defects or no defects
            if results['defects']:
                outline_color = self.bad_color
            else:
                outline_color = self.good_color

            frame = cv2.copyMakeBorder(frame, 5, 5, 5, 5, cv2.BORDER_CONSTANT,
                                       value=outline_color)

        # Display information about frame FPS
        x_cord = 20
        y_cord = 20
        for res in results:
            if "Fps" in res:
                fps_str = "{} : {}".format(str(res), str(results[res]))
                self.logger.info(fps_str)
                cv2.putText(frame, fps_str, (x_cord, y_cord),
                            cv2.FONT_HERSHEY_DUPLEX, 0.5,
                            self.good_color, 1, cv2.LINE_AA)
                y_cord = y_cord + 20

        # Display information about frame
        (d_x, d_y) = (20, 50)
        if 'display_info' in results:
            for d_i in results['display_info']:
                # Get priority
                priority = d_i['priority']
                info = d_i['info']
                d_y = d_y + 10

                #  LOW
                if priority == 0:
                    cv2.putText(frame, info, (d_x, d_y),
                                cv2.FONT_HERSHEY_DUPLEX,
                                0.5, (0, 255, 0), 1, cv2.LINE_AA)
                #  MEDIUM
                if priority == 1:
                    cv2.putText(frame, info, (d_x, d_y),
                                cv2.FONT_HERSHEY_DUPLEX,
                                0.5, (0, 150, 170), 1, cv2.LINE_AA)
                #  HIGH
                if priority == 2:
                    cv2.putText(frame, info, (d_x, d_y),
                                cv2.FONT_HERSHEY_DUPLEX,
                                0.5, (0, 0, 255), 1, cv2.LINE_AA)

    def save_images(self, msg, frame):
        """Save_images save the image to a directory based
           on good or bad images.

        :param msg: metadata of the frame
        :type: str
        :param frame: Images with the bounding box
        :type: numpy.ndarray
        """
        img_handle = msg['img_handle']
        tag = ''
        if 'defects' in msg:
            if msg['defects']:
                tag = 'bad_'
            else:
                tag = 'good_'
        imgname = tag + img_handle + ".png"
        cv2.imwrite(os.path.join(self.dir_name, imgname),
                    frame,
                    [cv2.IMWRITE_PNG_COMPRESSION, 3])

    def callback(self, msgbus_cfg, topic):
        """Callback called when the databus has a new message.

        :param msgbus_cfg: config for the context creation in EISMessagebus
        :type: str
        :param topic: Topic the message was published on
        :type: str
        """
        self.logger.debug('Initializing message bus context')

        msgbus = mb.MsgbusContext(msgbus_cfg)

        self.logger.debug(f'Initializing subscriber for topic \'{topic}\'')
        subscriber = msgbus.new_subscriber(topic)

        stream_label = None

        for key in self.labels:
            if key == topic:
                stream_label = self.labels[key]
                break

        while True:
            metadata, blob = subscriber.recv()

            if metadata is not None and blob is not None:
                frame = self.decode_frame(metadata, blob)

                self.logger.debug(f'Metadata is : {metadata}')

                if(self.draw_results):
                    self.draw_defect(metadata, frame, stream_label)

                if self.save_image:
                    self.save_images(results, frame)

                self.queue_publish(topic, frame)
            else:
                self.logger.debug(f'Non Image Data Subscription\
                                 : Classifier_results: {metadata}')
