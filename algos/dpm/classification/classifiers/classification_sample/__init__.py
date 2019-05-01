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

import os
import logging
import cv2
import numpy as np
import json
from time import time

from algos.dpm.defect import Defect
from algos.dpm.display_info import DisplayInfo
from openvino.inference_engine import IENetwork, IEPlugin

"""Sample classification algorithm
"""


class Classifier:
    """Classifier object
    """

    def __init__(self, model_xml, model_bin, labels, device):
        """Constructor
        Parameters
        ----------
        model_xml      : Model xml file
        model_bin      : Model bin file
        labels         : Labels mapping file
        device         : Run time device [CPU/GPU/MYRIAD/HDDL]
            Classifier configuration
        """

        self.log = logging.getLogger('CLASSIFICATION SAMPLE')

        # Assert all input parameters exist
        assert os.path.exists(model_xml), \
            'Classification model xml file missing: {}'.format(model_xml)
        assert os.path.exists(model_bin), \
            'Classification model bin file missing: {}'.format(model_bin)
        assert os.path.exists(labels), \
            'Labels mapping file missing: {}'.format(labels)

        # Load labels file associated with the model
        with open(labels, 'r') as f:
            self.labels_map = [x.split(sep=' ', maxsplit=1)[-1].strip() for x
                               in f]

        # Load OpenVINO model
        self.plugin = IEPlugin(device=device.upper(), plugin_dirs="")
        self.log.debug("Loading network files:\n\t{}\n\t{}".format(model_xml,
                                                                   model_bin))
        self.net = IENetwork.from_ir(model=model_xml, weights=model_bin)
        if device.upper() == "CPU":
            supported_layers = self.plugin.get_supported_layers(self.net)
            not_supported_layers = [l for l in self.net.layers.keys() if l not
                                    in supported_layers]
            if len(not_supported_layers) != 0:
                self.log.debug('ERROR: Following layers are not supported by \
                                the plugin for specified device {}:\n \
                                {}'.format(self.plugin.device,
                                           ', '.join(not_supported_layers)))

        assert len(self.net.inputs.keys()) == 1, \
            'Sample supports only single input topologies'
        assert len(self.net.outputs) == 1, \
            'Sample supports only single output topologies'

        self.input_blob = next(iter(self.net.inputs))
        self.output_blob = next(iter(self.net.outputs))
        self.net.batch_size = 1  # change to enable batch loading
        self.exec_net = self.plugin.load(network=self.net)

    # Main classification algorithm
    def classify(self, frame_num, img, user_data):
        """Sample classification algorithm to classify input images.

        Parameters
        ----------
        frame_num : int
            Frame count since the start signal was received from the trigger
        img : NumPy Array
            Image to classify
        user_data : tuple
            Any user information of frame

        Returns
        -------
            List of defects on the part in the frame, empty array if no defects
            exist on the part.


        This sample algorithm classifies input images, hence no defect
        information in generated. The trigger algorithm associated with
        with this classifier is "bypass_trigger" which selects each input
        image for classification.
        """

        index = user_data   # user data holds user information of frame

        # The trigger start frame (user data holds -1) indicates the start of
        # the current part/factory item/unit under inspection.
        # The start and stop signal combines all frames captured per part
        # as belonging to single unit.
        # The classification sample algoithm is associated with the
        # bypass trigger that send every frame as a separate unit.
        if(index == -1):
            defects = []
            self.log.debug('Received trigger frame')
            return defects

        self.log.debug('Classifying image')

        # Read and preprocess input images
        n, c, h, w = self.net.inputs[self.input_blob].shape
        images = np.ndarray(shape=(n, c, h, w))
        for i in range(n):
            if img.shape[:-1] != (h, w):
                self.log.debug('Image is resized from {} to {}'.format(
                                img.shape[:-1], (w, h)))
                img = cv2.resize(img, (w, h))
            # Change layout from HWC to CHW
            img = img.transpose((2, 0, 1))
            images[i] = img
        self.log.debug('Batch size is {}'.format(n))

        # Start sync inference
        infer_time = []
        t0 = time()
        res = self.exec_net.infer(inputs={self.input_blob: images})
        infer_time.append((time() - t0)*1000)
        self.log.info('Average running time of one iteration: {} ms'.format(
                       np.average(np.asarray(infer_time))))

        # No defects in classification algorithm
        defects = []

        # Display information for visualizer
        d_info = []

        # Processing output blob
        self.log.debug('Processing output blob')
        res = res[self.output_blob]
        self.log.info("Top 5 results :")

        for i, probs in enumerate(res):
            probs = np.squeeze(probs)
            top_ind = np.argsort(probs)[-5:][::-1]
            for id in top_ind:
                det_label = self.labels_map[id] \
                            if self.labels_map else '#{}'.format(id)
                self.log.info('prob: {:.7f}, label: {}'.format(probs[id],
                                                               det_label))
                # LOW priority information string to be displayed with frame
                disp_info = DisplayInfo('prob: {:.7f}, label: {} \
                                        '.format(probs[id], det_label), 0)
                d_info.append(disp_info)

        return d_info, defects
