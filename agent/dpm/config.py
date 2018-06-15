"""Data pipeline manager configuration module
"""
import logging
import json


class ConfigError(Exception):
    """Exception raised when there is a configuration error
    """
    pass


class Configuration:
    """Factory agent configuration object
    """
    def __init__(self, configfile):
        """Constructor

        Parameters
        ----------
            configfile - Configuration file to read

        Exceptions
        ----------
        OSError 
            If the configuration file does not exist
        KeyError
            If the configuration is missing keys.
        """
        self.log = logging.getLogger(__name__)
        self.log.info('Loading configuration file %s', configfile)
        with open(configfile, 'r') as f:
            config = json.load(f)
        
        # ID of the gateway
        self.machine_id = config['machine_id']
        self.trigger_threads = config.get('trigger_threads', None)
        self.log_file_size = config.get('log_file_size', None)

        # Reading classifier configuration
        self.classification = config['classification']
        self.data_ingestion_manager = config['data_ingestion_manager']
        self.storage = config['storage']
        self.triggers = config['triggers']
        self.database = config['database']

