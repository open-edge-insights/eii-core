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

"""Logging configuration module. Provides setup for the default Python
logging library.
"""
import os
import logging
import logging.handlers
from . import ConfigError


LOG_LEVELS = {
    'DEBUG': logging.DEBUG,
    'INFO': logging.INFO,
    'ERROR': logging.ERROR,
    'WARN': logging.WARN
}


def configure_logging(log_level, base_log_file, log_dir, 
        max_bytes=5.12e+8, file_count=2):
    """Configure logging to log to both stdout and to a rotating file. The
    rotating file names will use the base_log_file variable as the base name,
    where each rotating file will end with a number, except for the first log
    file.

    The log string will be formatted as follows:
        %(asctime)s %(levelname)s - %(name)s: %(message)s

    This function should only ever be called once in a the Python runtime.

    The max_bytes and file_count parameters should never both be 0. If this is
    the case, then the log files will never roll over. 
    
    Paramters
    ---------
    log_level : str
        Logging level to use, must be one of the following: DEBUG, INFO, ERROR,
        or WARN
    log_dir : str
        Directory to save rotating log files
    base_log_file : str
        Base log file name to use for the rotating log files
    max_bytes : int
        (Optional) Maximum number of bytes per log file, the default is 512MB
    file_count : int
        (Optional) Number of log files

    Exceptions
    ----------
    ConfigError
        If the given log level is unknown, or if max_bytes and file_count are
        both 0, or if the log file directory does not exist.
    """
    if log_level not in LOG_LEVELS:
        raise ConfigError('Unknown log level: {}'.format(log_level))
    elif max_bytes == 0 and file_count == 0:
        raise ConfigError('Max bytes and file count cannot both be 0')
    elif not os.path.exists(log_dir):
        raise ConfigError('Logging directory {} does not exist'.format(log_dir))

    #fmt_str = '%(asctime)s %(levelname)s - %(name)s: %(message)s'
    fmt_str=('%(asctime)s : %(levelname)s : %(name)s : [%(filename)s] :'+
            '%(funcName)s : in line : [%(lineno)d] : %(message)s')
    log_lvl = LOG_LEVELS[log_level]
    base_log = os.path.join(log_dir, base_log_file)

    # Do basic configuration of logging (just for stdout config)
    logging.basicConfig(format=fmt_str, level=log_lvl)

    # Add rotating file handler
    root = logging.getLogger()
    h = logging.handlers.RotatingFileHandler(
            base_log, maxBytes=max_bytes, backupCount=file_count)
    f = logging.Formatter(fmt_str)
    h.setFormatter(f)
    root.addHandler(h)

