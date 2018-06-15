"""Common utilities used through out ETR.
"""
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

