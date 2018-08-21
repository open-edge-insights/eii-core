"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. Software to be used for Made in China 2025 initiatives. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

"""Robotic arm trigger for the Yu Mei factory.
"""
import json
import traceback as tb
import logging
import threading as th
import paho.mqtt.client as mqtt
from . import BaseTrigger


MQTT_ROBOTIC_ARM_TOPIC = 'proj_1/gw_1/camera0'


class Trigger(BaseTrigger):
    """Yu Mei trigger based on signals received over MQTT from the robotic arm.
    """
    def __init__(self, mqtt_broker_host, mqtt_broker_port):
        """Constructor.
        """
        super(Trigger, self).__init__()
        self.log = logging.getLogger(__name__)
        self.mqtt_client = mqtt.Client()
        self.start_ev = th.Event()
        self.stop_ev = th.Event()

        self.stop_ev.set()

        self.log.debug('Connecting to MQTT broker %s:%d', 
                mqtt_broker_host, mqtt_broker_port)
        self.mqtt_client.on_message = self._on_message
        self.mqtt_client.on_connect = self._on_connect
        self.mqtt_client.on_disconnect = self._on_disconnect
        self.mqtt_client.connect(mqtt_broker_host, mqtt_broker_port)
        
        self.log.debug('Subscribing to robotic arm MQTT topic')
        self.mqtt_client.subscribe(MQTT_ROBOTIC_ARM_TOPIC)

        self.log.debug('Starting MQTT client loop')
        self.mqtt_client.loop_start()

    def get_supported_ingestors(self):
        """Returns supported ingestors which the trigger is interested in.
        """
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
        if self.start_ev.is_set():
            if self.is_triggered():
                self.send_data(data)
            else:
                self.log.debug('Sending start signal')
                self.send_start_signal(data)
        elif self.stop_ev.is_set() and self.is_triggered():
            self.log.debug('Sending stop signal')
            self.send_stop_signal()
    
    def stop(self):
        """Overloaded stop method
        """
        # Call parent stop method
        super(Trigger, self).stop()
        # Disconnect from MQTT broker
        self.mqtt_client.disconnect()
        # Stop MQTT lopp
        self.mqtt_client.loop_stop(force=True)

    def _on_message(self, client, userdata, msg):
        """MQTT client on_message() callback
        """
        try:
            self.log.debug('Received message over MQTT')
            datas = msg.payload.decode('utf-8')
            data = json.loads(datas)
            camera_on = data['camera_on']
            
            if camera_on == 0:
                # The classification should stop
                self.log.debug('Received stop signal from robotic arm')
                if self.stop_ev.is_set():
                    self.log.error('Received stop signal when already stopped')
                else:
                    self.stop_ev.set()
                    self.start_ev.clear()
            elif camera_on == 1:
                # The classification should start
                self.log.debug('Received start signal from robotic arm')
                if self.start_ev.is_set():
                    self.log.error('Received start signal when already started')
                else:
                    self.start_ev.set()
                    self.stop_ev.clear()
            else:
                self.log.error('Received unknown \'camera_on\' value: %d', 
                        camera_on)
        except:
            self.log.error('Exception while parsing MQTT payload:\n%s', 
                    tb.format_exc())

    def _on_connect(self, client, userdata, flags, rc):
        """MQTT client on_connect() callback
        """
        if rc == 0:
            self.log.info('MQTT client connected')
        else:
            self.log.error('[%d] MQTT failed to connect, reconnecting', rc)
            self.mqtt_client.reconnect()

    def _on_disconnect(self, client, userdata, rc):
        """MQTT client on_disconnect() callback
        """
        if rc == 0:
            # Expected disconnect
            self.log.info('MQTT client disconnected')
        else:
            self.log.error('[%d] Unexpected MQTT disconnect, reconnecting', rc)
            self.mqtt_client.reconnect()

