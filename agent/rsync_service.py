"""ETR rsync service.
"""
import os
import json
import time
import tarfile
import logging
import requests
import traceback as tb
import threading as th
import subprocess as sub
import paho.mqtt.client as mqtt
from concurrent.futures import ThreadPoolExecutor

from agent.db import DatabaseAdapter
from agent.dpm.config import ConfigError
from agent.dpm.storage import LocalStorage
from agent.etr_utils import abspath, format_exc


# Globals for MQTT topic strings
MQTT_META_DATA_TOPIC = 'proj_1/gw_1/raw0'
MQTT_RESULTS_TOPIC = 'proj_1/gw_1/etr0'

# Refinement app REST API endpoint
REST_ENDPOINT = 'rest/v1/notify'


class RsyncServiceError(Exception):
    """Exception raised by the RsyncService when an error is encountered.
    """
    pass


class RsyncService:
    """Synchronization service for sending data to the edge server.
    """
    def __init__(self, config):
        """Constructor.

        Parameters
        ----------
        config : dpm.config.Configuration
            ETR configuration object
        """
        if config.rsync_service is None:
            raise ConfigError('Rsync service missing in configuration')
        
        self.log = logging.getLogger(__name__)
        self.db = DatabaseAdapter(config.machine_id, config.database)
        self.stop_ev = th.Event()
        self.mqtt_client = mqtt.Client()    
        self.meta_data = []
        self.storage = LocalStorage(config.storage)
        self.executor = ThreadPoolExecutor(max_workers=4)

        try:
            self.log.info('Initializing MQTT client')
            if 'mqtt_broker_host' not in config.classification and \
                    'mqtt_broker_port' not in config.classification:
                raise RsyncServiceError('Classification has MQTT disabled')

            self.remote_host = config.rsync_service['remote_host']
            self.remote_port = config.rsync_service['remote_port']
            self.url = f'http://{self.remote_host}:{self.remote_port}/{REST_ENDPOINT}'

            identity_file = abspath(config.rsync_service['identity_file'])

            if not os.path.exists(identity_file):
                raise RsyncServiceError(
                        f'SSH identity file "{identity_file}" does not exist')

            self.ssh = f'ssh -i {identity_file}'
            self.remote_location = '{0}@{1}:{2}'.format(
                    config.rsync_service['remote_user'],
                    self.remote_host,
                    config.rsync_service['remote_folder'])

            self.mqtt_client = mqtt.Client()
            self.mqtt_client.on_connect = self._on_connect
            self.mqtt_client.on_message = self._on_message
            self.mqtt_client.on_disconnect = self._on_disconnect

            self.log.debug('Connecting to MQTT broker')
            self.mqtt_client.connect(
                    config.classification['mqtt_broker_host'], 
                    config.classification['mqtt_broker_port'],
                    10)

            self.log.debug('Subscribing to %s topic', MQTT_META_DATA_TOPIC)
            self.mqtt_client.subscribe(MQTT_META_DATA_TOPIC, qos=1)

            self.log.debug('Subscribing to %s topic', MQTT_RESULTS_TOPIC)
            self.mqtt_client.subscribe(MQTT_RESULTS_TOPIC, qos=1)
        except ConnectionRefusedError:
            raise RsyncServiceError('Failed to connect to MQTT broker')
        except KeyError as e:
            raise RsyncServiceError(f'Rsync service config missing key: {e}')

    def run(self):
        """Run rsync service.
        """
        self.mqtt_client.loop_start()
        self.stop_ev.wait()
        self.mqtt_client.disconnect()
        self.mqtt_client.stop_loop()

    def stop(self):
        """Stop the rsync service.
        """
        self.stop_ev.set()
        self.db.close()

    def _on_message(self, client, userdata, msg):
        """MQTT client on_message() callback
        """
        if msg.topic == MQTT_META_DATA_TOPIC:
            self.log.debug('Received meta-data publication')
            self.meta_data.append(json.loads(msg.payload))
        elif msg.topic == MQTT_RESULTS_TOPIC:
            self.log.debug('Received summary publication')
            try:
                data = json.loads(msg.payload)
                fut = self.executor.submit(
                        self._synchronize, data['part_id'], list(self.meta_data))
                fut.add_done_callback(self._synchronization_done)
                self.meta_data = []  # Clear out old meta-data
            except json.JSONDecodeError:
                self.log.error('Malformed JSON')
            except KeyError as e:
                self.log.error('Summary publication missing key \'%s\'', str(e))
        else:
            self.log.error('Received data from unknown topic: %s', msg.topic)

    def _on_connect(self, client, userdata, flags, rc):
        """MQTT client on_connect() callback
        """
        if rc == 0:
            self.log.info('MQTT client connected')
        else:
            self.log.error('[%d] MQTT client failed to connect, reconnecting', rc) 
            self.mqtt_client.reconnect()

    def _on_disconnect(self, client, userdata, rc):
        """MQTT client on_disconnect() callback
        """
        if rc == 0:
            # Expected disconnect
            self.log.warn('MQTT client disconnected')
        else:
            self.log.error('[%d] Unexpected MQTT disconnect, reconnecting', rc)
            self.mqtt_client.reconnect()

    def _synchronize(self, part_id, meta_data):
        """Private method to synchronize the data for the specified part ID.
        """
        self.log.info('Starting synchronization')
        output = os.path.join('/tmp', part_id + '.tar.gz')

        try:
            self.log.info('Compressing images')
            with tarfile.open(output, 'w:gz') as tar:
                for md in meta_data:
                    image = self.db.find_image(md['image_id'])
                    assert image is not None, 'Image does not exist in database'
                    fn = self.storage.get_image_path(image)

                    # Wait for all files to be saved
                    self._wait_for_image(fn)

                    if not self.stop_ev.is_set():
                        tar.add(fn, arcname=os.path.basename(fn))
                    else:
                        return

            self.log.debug('Executing rsync of %s to %s', 
                    output, self.remote_location)
            out = sub.check_output(
                    f'rsync -v -e "{self.ssh}" {output} {self.remote_location}',
                    stderr=sub.STDOUT, shell=True)

            self._request(part_id, meta_data)

            self.log.info('Synchronization finished')
        except sub.CalledProcessError as e:
            self.log.error('rsync command failed:\n%s', e.output.decode('utf-8'))
            self.log.error('Synchronization failed')
        finally:
            try:
                self.log.info('Deleting %s', output)
                os.remove(output)
            except OSError:
                self.log.error('Failed to delete %s:\n%s', output, 
                        tb.format_exc())

    def _request(self, part_id, meta_data, wait=False):
        """Issue REST call to the edge server.
        """
        if wait:
            # Last request failed, wait the 5 seconds to issue next call
            self.log.debug('Waiting 5 seconds to issue request')
            time.sleep(5)

        self.log.info('Executing REST call')
        r = requests.post(self.url, json={
            'part_id': part_id, 
            'meta-data': meta_data
        })
        if r.status_code != 200:
            self.log.error('REST call failed: %d\n%s', r.status_code, r.text)
            # Submitting job to retry the request
            fut = self.executor.submit(self._request, part_id, meta_data, True)
            fut.add_done_callback(self._request_retry_done)
        else:
            self.log.info('REST call successful')
            self._clean_up(part_id, meta_data)


    def _clean_up(self, part_id, meta_data):
        """Delete the images associated with the file and remove it from the
        database.
        """
        self.log.info('Removing part \'%s\' from the gateway', part_id)

        for md in meta_data:
            image = self.db.find_image(md['image_id'])
            assert image is not None, 'Image does not exist in database'
            fn = self.storage.get_image_path(image)
            self.log.debug('Removing image \'%s\'', fn)
            os.remove(fn)

        self.log.debug('Removing part \'%s\' from the database', part_id)
        self.db.remove_part(part_id)

        self.log.info('Removal of part complete')

    def _synchronization_done(self, fut):
        """Synchronization future done callback.
        """
        exc = fut.exception()
        if exc is not None:
            self.log.error('Error during synchronization: %s', format_exc(exc)) 

    def _request_retry_done(self, fut):
        """Request retry future done callback.
        """
        exc = fut.exception()
        if exc is not None:
            self.log.error('Error during request retry: %s', format_exc(exc)) 

    def _wait_for_image(self, image_fn):
        """Helper method to wait for waiting for an image file to finished
        being saved by ETR.
        """
        size = 0 
        while True:
            if not os.path.exists(image_fn):
                time.sleep(0.1)
            else:
                curr_size = os.path.getsize(image_fn)
                if curr_size > size:
                    size = curr_size
                    time.sleep(0.1)
                else:
                    break

