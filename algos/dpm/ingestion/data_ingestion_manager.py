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

"""Data ingestion manager
"""
import threading
import logging
import importlib

from algos.dpm.config import ConfigError
from . import IngestorError


class IngestorWrapper:
    """Simple wrapper around an ingestor object for tracking the callbacks.
    """
    def __init__(self, name, ingestor):
        """Constructor

        Parameters
        ----------
        name : str
            Name of the ingestor
        ingestor : Ingestor
            Ingestor object
        """
        self.name = name
        self.ingestor = ingestor
        self.cbs = []

    def start(self):
        """Call underlying start method on the ingestor.
        """
        self.ingestor.start()

    def join(self):
        """Call underlying join method on the ingestor.
        """
        self.ingestor.join()

    def add_callback(self, cb):
        """Add a callback to the list of callbacks
        """
        self.cbs.append(cb)


class DataIngestionManager:
    """Ingestor responsible for ingesting data into the agent.

    This object initializes the ingestors specified in the configuration
    dictionary passed to the constructor. The dictionary structor of the
    configuration for this object is as follows:

        {
            "ingestors": {
                "<INGESTOR NAME>": <DICT: CONFIG FOR THE INGESTOR>,
                ...
            }
        }

    The, "ingestors", dictionary where the keys are ingestors to load and the 
    value is the dictionary object which is the configuration for the ingestor.

    See the submodules of the ingestion module for the available ingestors.
    """
    def __init__(self, config):
        """Constructor

        Parameters
        ----------
        config : dict
            Configuration dictionary for the ingestor

        Exceptions
        ----------
        KeyError 
            If missing a configuration key
        IngestorError
            If an error occurs loading the ingestors
        """
        self.log = logging.getLogger(__name__)
        self.ingestors = {}
        self.stop_ev = threading.Event()
        self.join_ev = threading.Event()

        ingestor_configs = config['ingestors']

        for (name, config) in ingestor_configs.items():
            self.log.info('Loading ingestor: %s', name)

            # An ingestor may only be specified once
            if name in self.ingestors:
                raise IngestorError('Ingestor listed twice: {}'.format(name))

            try:
                lib = importlib.import_module(
                        '.{}'.format(name), package='algos.dpm.ingestion')
                ingestor = lib.Ingestor(config, self.stop_ev, self._on_data)
                self.ingestors[name] = IngestorWrapper(name, ingestor)
            except ImportError:
                raise IngestorError(
                        'Failed to import "{}" ingestor'.format(name))
            except KeyError as e:
                raise IngestorError(
                        'Config for ingestor {0} missing key {1}'.format(
                            name, e.message))

    def register_interest(self, ingestor, cb):
        """Register an interest in an ingestor's data.


        Note: The callback has the following protoype fn(ingesor_name, data).

        Parameters
        ----------
        ingestor : str
            Name of the ingestor
        cb : function
            Function to call when new data is available

        Exceptions
        ----------
        IngestorError 
            If the ingestor does not exist
        """
        if ingestor not in self.ingestors:
            raise IngestorError('Ingestor "{}" is not loaded'.format(ingestor))

        self.ingestors[ingestor].add_callback(cb)

    def has_ingestor(self, ingestor):
        """Check whether or not a given ingestor is loaded.
        """
        return ingestor in self.ingestors

    def start(self):
        """Start the data ingestion.
        """
        self.log.info('Starting data ingestion')
        for ingestor in self.ingestors.values():
            ingestor.start()

    def stop(self):
        """Stop all ingestion activities. This will attempt to stop all threads
        and processes started to ingest data.
        """
        self.log.info('Stopping data ingestion')
        self.stop_ev.set()
        for ingestor in self.ingestors.values():
            ingestor.join()
        self.join_ev.set()
        self.log.info('Data ingestion manager stopped')

    def join(self):
        """Wait for the data ingestion to complete.
        """
        self.join_ev.wait()

    def _on_data(self, ingestor, data):
        """Callback given to ingestors for when data is available.

        Parameters
        ----------
        ingestor : str 
            String name of the ingestor
        data : Object
            Data that was made available
        """
        cbs = self.ingestors[ingestor].cbs
        for cb in cbs:
            cb(ingestor, data)

