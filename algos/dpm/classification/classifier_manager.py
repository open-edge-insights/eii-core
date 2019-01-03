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


"""Classifier Manager module
"""
import sys
import uuid
import json
import time
import logging
import traceback as tb
import threading as th
from concurrent.futures import ThreadPoolExecutor

from . import ClassifierConfigError, load_classifier


class ClassifierManager:
    """Classifier Manager

    The ClassifierManager is responsible for facilitating the connection
    between Trigger's and classification algorithms. It is also responsible
    for publishing both the meta-data from each frame ran through the
    classification and the summary results from the classification ran over
    the entire classification period.

    The configuration of the ClassifierManager must have the following
    structure:
        {
            "max_workers": <INT: (Optional) Number of threads for classifier execution>,
            "classifiers": {
                "<CLASSIFIER NAME>": {
                    "trigger": <STRING: Trigger Name>,
                    "config": <DICT: Classifier Configuration>
                }
                ...
            }
        }
    """
    def __init__(self, machine_id, config):
        """Constructor

        Parameters
        ----------
        machine_id : str
            ID of the machine the classification is running on
        config : dict
            Dictionary object with the configuration for the ClassifierManager
        triggers : dict
            Dictionary of loaded triggers, where the key is the name of the
            trigger, and the value is the Trigger object

        Exceptions
        ----------
        ClassifierConfigError
            If the configuration of the classifier is missing a value, or if
            the configuration asks to use a trigger which has not been loaded
            into the system.
        """
        self.log = logging.getLogger(__name__)
        self.machine_id = machine_id
        self.stopped = th.Event()
        self.meta_idx = 1
        self.summary_idx = 1
        self.samples = 0

        max_workers = config.get('max_workers', None)
        self.pool = ThreadPoolExecutor(max_workers=max_workers)
        self.classifiers = {}

        try:
            for (n, c) in config['classifiers'].items():
                self.log.debug('Loading classifier %s', n)
                self.classifiers[n] = load_classifier(n, c['config'])
        except KeyError as e:
            raise ClassifierConfigError('Config missing key {}'.format(e))

    def get_classifier(self, classifier_name):
        if classifier_name not in self.classifiers:
            raise ClassifierConfigError(
                    'Classifier \'{}\' is not loaded'.format(classifier_name))
        return self.classifiers[classifier_name]

    def register_classifier(self, classifier_name, trigger_name, trigger):
        """Register the specified classifier to the given triger.

        Parameters
        ----------
        TODO: Document

        Exceptions
        ----------
        ClassifierConfigError
            If the classifier was not in the configuration
        """
        if classifier_name not in self.classifiers:
            raise ClassifierConfigError(
                    'Classifier \'{}\' is not loaded'.format(classifier_name))
        classifier = self.classifiers[classifier_name]
        trigger.register_trigger_callback(
                lambda d: self._on_trigger(trigger_name, classifier, d))

    def stop(self):
        """Stop the classifier manager.
        """
        self.log.info('Stopping classification')
        self.stopped.set()
        self.pool.shutdown()
        self.log.info('Classification stopped')

    def _on_trigger(self, trigger, classifier, data):
        """Private method for handling the trigger start event, starting a
        thread to process all of the incoming data from the trigger.
        """
        self.log.info('Received start signal from trigger "%s"', trigger)
        fut = self.pool.submit(self._process_frames, classifier, data)
        fut.add_done_callback(self._on_process_frames_done)

    def _on_process_frames_done(self, fut):
        """Private method to log errors if they occur whilel processing frames.
        """
        exc = fut.exception()
        if exc is not None:
            self.log.error(
                    'Error while classifying frames:\n%s', format_exc(exc))

    def _process_frames(self, classifier, data):
        """Run method for the thread to process a stream of data coming from a
        trigger.
        """
        try:
            if self.stopped.is_set():
                return

            self.log.info('Classification started')
            frame_count = 0
            start = time.time()
            defects = {}
            msg = {}
            ret_point = []
            self.samples = self._incr_int(self.samples, 1)
            part_id = str(uuid.uuid4())

            for res in data:
                if res is None:
                    break

                sample_num, user_data, img_handle, (cam_sn, frame) = res
                ts = time.time()

                try:
                    self.log.debug('Classifying frame %d', frame_count)
                    results = classifier.classify(sample_num, frame, user_data)
                except:
                    self.log.error('Error in classifier:\n%s', tb.format_exc())
                    results = []  # Because of error, no results

                image_id = str(uuid.uuid4())

                defect_res = []
                for d in results:
                    if d.defect_class in defects:
                        defects[d.defect_class] += 1
                    else:
                        defects[d.defect_class] = 1
                    defect_res.append({
                        'type': d.defect_class,
                        'tl': d.tl,
                        'br': d.br
                    })

                msg = {
                        'idx': self.meta_idx,
                        'timestamp': ts,
                        'machine_id': self.machine_id,
                        'part_id': part_id,
                        'image_id': image_id,
                        'cam_sn': cam_sn,
                        'defects': defect_res
                }
                # Roll over the msg_idx value if we have reached the maximum
                # value that an int can store in Python
                self.meta_idx = self._incr_int(self.meta_idx, 1)

                msg['ImgHandle'] = img_handle
                ret_point.append(msg)
                
                frame_count += 1

            delta = time.time() - start
            fps = frame_count / delta
            self.log.info('Classification finished')
            self.log.debug('Total Time = %f, Frames = %d, FPS = %f',
                    delta, frame_count, fps)
        except:
            self.log.error('Error processing frames:\n%s', tb.format_exc())

            # Set error in the TriggerIter object to prevent build up of
            # unconsumed data
            data.set_error()
        finally:
            return ret_point

    def _incr_int(self, val, rollover=0):
        """Helper to safely increment an integer and roll over back to the
        given default value if the maximum integer value is reached.
        """
        if val == sys.maxsize:
            return rollover
        else:
            return val + 1
