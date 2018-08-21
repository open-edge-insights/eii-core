"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

"""Common utilities used through out ETR.
"""
import os
import traceback as tb


class ConfigError(Exception):
    """Exception raise when a configuration error occurs.
    """
    pass


def format_exc(exc):
    """Simple function to take an Exception object and give a Python-style
    formatted exception statement.

    Parameters
    ----------
    exc : Exception
        Input exception
    
    Returns
    -------
    String
    """
    return '{0}{1}: {2}'.format(''.join(tb.format_tb(exc.__traceback__)), 
            type(exc).__name__, str(exc))


def abspath(path):
    """Get the absolute path with expanded variables.

    Note that this method does not check if the given path exists.

    Parameters
    ----------
    path : str
        Path to expand
    
    Returns
    -------
    String
    """
    return os.path.abspath(os.path.expanduser(os.path.expandvars(path)))
