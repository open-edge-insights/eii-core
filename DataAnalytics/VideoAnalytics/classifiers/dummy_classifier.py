# Copyright (c) 2019 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


import logging
import threading
from libs.base_classifier import BaseClassifier

class Classifier(BaseClassifier):
    """Dummy classifier which never returns any defects. This is meant to aid
    pure data collection without running any classification on ingested video
    frames.
    """
    def __init__(self, classifier_config, input_queue, output_queue):
        """Constructor to initialize classifier object

        Parameters
        ----------
        classifier_config : dict
            Configuration object for the classifier
        input_queue : Queue
            input queue for classifier
        output_queue : Queue
            output queue of classifier
       
        Returns
        -------
            Classification object
        """
        super().__init__(classifier_config, input_queue, output_queue)
        self.log = logging.getLogger('DUMMY_CLASSIFIER')

    def classify(self):
        """Classify the given image.
        """
        thread_id = threading.get_ident()
        self.log.info("Classifier thread ID: {} started...".format(thread_id))
        
        while True:
            data = self.input_queue.get()
            defects_dict = {
                "defects": [],
                "display_info": []
            }
            self.send_data(data, defects_dict)        

        self.log.info("Classifier thread ID: {} stopped...".format(thread_id))

