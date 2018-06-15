"""Test trigger for doing a trigger over a socket connection.
"""
import socket
import logging

from . import BaseTrigger, TriggerIter


class Trigger(BaseTrigger):
    """Test trigger object.
    """

    def __init__(self, host, port):
        """Constructor.
        """
        super(Trigger, self).__init__()
        self.log = logging.getLogger(__name__)
        self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.conn.settimeout(0.5)
        self.log.info('Connecting to (%s, %d)', host, port)
        self.conn.connect((host, port))

    def get_supported_ingestors(self):
        return ['video']

    def on_data(self, ingestor, data):
        """Process video frames as they are received and call the callback
        registered by the `register_trigger_callback()` method if the frame
        should trigger the execution of the classifier.

        Parameters
        ----------
        ingestor : str
            String name of the ingestor which received the data
        data : tuple
            Tuple of (camera serial number, camera frame)
        """
        try:
            self.conn.recv(1024)
            if self.is_triggered():
                self.log.debug('Stopping iteration')
                self.send_stop_signal()
            else:
                self.send_start_signal(data)
        except socket.timeout:
            if self.is_triggered():
                self.send_data(data)

