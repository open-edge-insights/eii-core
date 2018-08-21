"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

"""Data Pipeline Manager
"""
import logging
import traceback as tb
from concurrent.futures import ThreadPoolExecutor

from agent.db import DatabaseAdapter
from agent.etr_utils import format_exc
from .ingestion.data_ingestion_manager import DataIngestionManager
from .triggers import load_trigger
from .storage import LocalStorage
from .classification.classifier_manager import ClassifierManager


class DataPipelineManagerError(Exception):
    """Exception raised by the DataPipelineManager.
    """
    pass


class DataPipelineManager:
    """Data Pipeline Manager object
    """
    def __init__(self, config):
        """Constructor

        Parameters
        ----------
        config : dpm.config.Configuration
            DPM configuration object
        """
        self.log = logging.getLogger(__name__)

        # self.log.info('Initializing the Database Adapter')
        # self.db = DatabaseAdapter(config.machine_id, config.database)

        self.log.info('Initializing Data Ingestion Manager')
        self.dim = DataIngestionManager(config.data_ingestion_manager)

        self.log.info('Loading triggers')
        self.trigger_ex = ThreadPoolExecutor(
                max_workers=config.trigger_threads)
        self.triggers = []

        self.config = config

        self.log.info('Initializing local storage')
        # self.storage = LocalStorage(config.storage)
        
        self.log.info('Initializing Classifier Manager')
        self.cm = ClassifierManager(
                config.machine_id, config.classification, None, None)

        # Load classifiers and connect data pipeline
        for (n, c) in config.classification['classifiers'].items():
            self.log.info('Setting up pipeline for %s classifier', n)
            triggers = c['trigger']
            if isinstance(triggers, list):
                # Load the first trigger, and register it to its supported
                # ingestors
                prev_trigger = self._init_trigger(triggers[0], register=True)
                prev_name = triggers[0]
                
                # Load the rest of the trigger pipeline
                for t in triggers[1:]:
                    # Initialize the trigger
                    trigger = self._init_trigger(t)

                    # Register it to receive data from the previous trigger in
                    # the pipeline
                    prev_trigger.register_trigger_callback(
                            lambda data: self._on_trigger_data(
                                trigger, prev_name, data, filtering=True))

                    # Set previous trigger
                    prev_trigger = trigger
                    prev_name = t

                # Register the classifier to the last loaded trigger
                self.cm.register_classifier(n, prev_name, prev_trigger)
            else:
                # Only loading a single trigger for a classifier
                trigger = self._init_trigger(triggers, register=True)
                self.cm.register_classifier(n, triggers, trigger)
       
    def run(self):
        """Run the data pipeline manager.
        """
        self.log.info('Starting the data ingestion')
        # self.storage.start()
        self.dim.start()
        self.dim.join()

    def stop(self):
        """Stop the data pipeline manager.
        """
        self.log.info('Stopping data pipeline manager')
        self.dim.stop()
        for t in self.triggers:
            t.stop()
        self.cm.stop()
        # self.storage.stop()
        self.trigger_ex.shutdown()
        self.log.info('Data pipeline manager stopped')

    def _init_trigger(self, name, register=False):
        """Initialize trigger
        """
        self.log.info('Loading trigger %s', name)

        if name not in self.config.triggers:
            raise DataPipelineManagerError(
                    ('Trigger \'{}\' is not specified in the ' 
                        'configuration').format(t))
        
        config = self.config.triggers[name]
        trigger = load_trigger(name, config)

        if register:
            registered = False
            ingestors = trigger.get_supported_ingestors()
            for i in ingestors:
                if self.dim.has_ingestor(i):
                    self.log.debug(
                            'Registering %s trigger to %s ingestor', name, i)
                    self.dim.register_interest(
                            i, lambda i,d: self._on_trigger_data(
                                    trigger, i, d))
                    registered = True

            if not registered:
                raise DataPipelineManagerError(
                    ('None of the supported ingestors are loaded for '
                     'trigger: {}').format(n))

        self.triggers.append(trigger)

        return trigger

    def _on_trigger_data(self, trigger, ingestor, data, filtering=False):
        """Private method to submit a worker to execute a trigger on ingestion
        data.
        """
        if filtering:
            fut = self.trigger_ex.submit(
                    self._on_filter_trigger_data, ingestor, trigger, data)
        else:
            fut = self.trigger_ex.submit(trigger.process_data, ingestor, data)
        fut.add_done_callback(self._on_trigger_done)

    def _on_trigger_done(self, fut):
        """Private method to log errors if they occur in the trigger method.
        """
        exc = fut.exception()
        if exc is not None:
            self.log.error('Error in trigger: \n%s', format_exc(exc))

    def _on_filter_trigger_data(self, ingestor, trigger, data):
        """Private method for passing data onto a trigger setup to filter data.
        """
        # TODO: Could probably optimize to not use entire executor thread for
        # this ingestor
        try:
            for i in data:
                if i is None:
                    break
                # Unpacking the data
                sample_num, user_data, video_data = i
                # Send the data through the trigger
                trigger.process_data(ingestor, video_data)
        except:
            self.log.error(
                    'Error while passing pipelined frames to trigger %s:\n%s', 
                    trigger, tb.format_exc())

            # Set error in the TriggerIter object to prevent build up of
            # unconsumed data
            data.set_error()

