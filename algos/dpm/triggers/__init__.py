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

import logging
import importlib
import inspect
import queue
import threading

from algos.dpm.config import ConfigError

"""Trigger subsystem for the agent.
"""


class TriggerLoadError(Exception):
    """Exception raised if there is an error loading the trigger module.
    """
    pass


class TriggerConfigError(Exception):
    """Exception raised if the trigger config is incorrect.
    """
    pass


class TriggerIter:
    """Trigger iterator object passed to the trigger callback when the trigger
    signal is sent. Receiving this object serves as the start signal, and the
    iterator ending serves as the stop signal.
    """
    def __init__(self, init_data):
        """Constructor

        Parameters
        ----------
        init_data : Object
            Initial data received by the trigger indicating the start of
            the classification which the trigger is signalling
        """
        self.queue = queue.Queue()
        self.stop_ev = threading.Event()
        self.full_stop_ev = threading.Event()
        self.err_flag = threading.Event()
        self.queue.put(init_data)

    def __iter__(self):
        """Iterator overload
        """
        return self

    def __next__(self):
        """Next element overload.
        """
        return self.next()

    def next(self):
        """Get the next element in the iterator.
        """
        if (self.queue.empty() and self.stop_ev.is_set()) or \
           self.full_stop_ev.is_set():
            raise StopIteration()
        else:
            return self.queue.get()

    def size(self):
        """Get the size of the queue.
        """
        return self.queue.qsize()

    def put(self, data):
        """Put an item into the trigger iterator's queue.

        Parameters
        ----------
        data : Object
            Object received by the Trigger that is to be passed on to a
            classifier.
        """
        # Only enqueue data while no errors have occurred with the consumer.
        # This prevents memory being "leaked", i.e. always having a reference,
        # in Python
        if not self.err_flag.is_set():
            self.queue.put(data)

    def stop(self, full_stop=False):
        """Send the stop signal to signal the end of the iterator.
        """
        self.stop_ev.set()
        if full_stop:
            # Putting None in the queue to signal the end, and setting the
            # full stop event so that if there are many items in the queue the
            # iterator still fully stops
            self.put(None)
            self.full_stop_ev.set()

    def set_error(self):
        """Mark that the consumer of the iterator encountered an error, so that
        the data in the internal queue can be dequeued and further data will
        not be added.
        """
        # Only set the flag if no error has already occurred (no need to double
        # clear the underlying queue)
        if self.err_flag.is_set():
            return

        self.err_flag.set()

        # Clearing all current data in the queue to remove all references to
        # data frames
        try:
            while not self.queue.empty():
                self.queue.get_nowait()
                self.queue.task_done()
        except queue.Empty:
            pass


class BaseTrigger:
    """Base trigger object. Handles all start/stop signaling, including full
    stopping the trigger iterator if it exists when the agent needs to fully
    shutdown.
    """
    def __init__(self):
        """Constructor. This must be called by subclasses, otherwise errors
        will occur when sending the start/stop signals.
        """
        self.trigger_iter = None
        self.cb = None
        self.lck = threading.Lock()
        self.stopped = threading.Event()
        self.sample_count = 0
        self.triggered_ev = threading.Event()

    def register_trigger_callback(self, cb):
        """Register callback to be called when the classifier should be
        triggered.

        Parameters
        ----------
        cb : function
            Callback to register with the trigger

        Exceptions
        ----------
        AssertionError
            If the trigger already has a registered callback
        """
        assert self.cb is None, 'Trigger already has registered callback'
        self.cb = cb

    def is_triggered(self):
        """Check if the trigger has sent the start signal without a stop yet.
        """
        return self.triggered_ev.is_set()

    def get_supported_ingestors(self):
        """Get the names of the trigger's supported ingestors.
        """
        raise NotImplemented('Trigger MUST implement get_ingestor_name()')

    def send_start_signal(self, data, user_data=None):
        """Send the start signal to the registered callback (if there is one).

        Parameters
        ----------
        data : Object
            Initial data the set off the trigger
        user_data : Object
            (Optional) Any object to pass forward as user data from the trigger

        Exceptions
        ----------
        AssertionError
            If the trigger has already been started
        """
        if self.cb is not None:
            self.lck.acquire()
            if self.is_triggered():
                # If the start signal was sent while waiting for the lock,
                # then just send this data
                self.lck.release()
                self.send_data(data, user_data=user_data)
            else:
                # Clear the previous trigger of images
                self.triggered_ev.set()
                self.trigger_iter = TriggerIter((0, user_data, data))
                self.lck.release()
                self.cb(self.trigger_iter)

    def send_data(self, data, user_data=None):
        """Send data to the trigger iterator.

        Parameters
        ----------
        data : Object
            Initial data the set off the trigger
        user_data : Object
            (Optional) Any object to pass forward as user data from the trigger

        Exceptions
        ----------
        AssertionError
            If the trigger has not been started
        """
        assert self.trigger_iter is not None, 'Trigger has not sent start \
                                              signal'
        self.lck.acquire()
        if not self.is_triggered():
            # If the tirgger iterator was stopped prior to this being set
            self.lck.release()
            return
        self.trigger_iter.put((self.sample_count, user_data, data))
        self.sample_count += 1
        self.lck.release()

    def send_stop_signal(self):
        """Send the stop signal to the trigger iterator if it has been started.
        """
        if self.trigger_iter is not None:
            self.lck.acquire()
            self.trigger_iter.stop()
            self.trigger_iter.put(None)
            self.sample_count = 0
            self.triggered_ev.clear()
            self.lck.release()

    def stop(self):
        """Full stop the iterator, cutting short the actual iterator even if
        there is currently data in the queue.
        """
        self.stopped.set()
        if self.is_triggered():
            self.trigger_iter.stop(full_stop=True)

    def process_data(self, ingestor, data):
        if not self.stopped.is_set():
            self.on_data(ingestor, data)


def load_trigger(trigger, config):
    """Load the given trigger with the specified configuration.

    Parameters
    ----------
    trigger : str
        String name of the trigger
    config : dict
        Dictionary configuration for the trigger

    Returns
    -------
        Trigger object for the specified trigger

    Exceptions
    ----------
    TriggerLoadError
        If an issue arises while loading the Python module for the trigger
    TriggerConfigError
        If the configuration for the trigger is incorrect
    """
    try:
        lib = importlib.import_module(
                '.{}'.format(trigger), package='algos.dpm.triggers')

        arg_names = inspect.getargspec(lib.Trigger.__init__).args
        if len(arg_names) > 0:
            # Skipping the first argument since it is the self argument
            args = [config[arg] for arg in arg_names[1:]]
        else:
            args = []

        trigger = lib.Trigger(*args)

        return trigger
    except AttributeError:
        raise TriggerConfigError(
                '"{}" module is missing the Trigger class'.format(trigger))
    except ImportError:
        raise TriggerConfigError('Failed to load trigger: {}'.format(trigger))
    except KeyError as e:
        raise TriggerConfigError('Trigger config missing key: {}'.format(e))
