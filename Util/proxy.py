import os


class ProxyHelper(object):
    """
        Helper class providing apis to unset and reset http/https proxy envvars
    """
    def __init__(self):
        self.http_proxy = None
        self.https_proxy = None
        self.HTTP_PROXY = None
        self.HTTPS_PROXY = None
        if 'http_proxy' in os.environ:
            self.http_proxy = os.environ['http_proxy']
        if 'https_proxy' in os.environ:
            self.https_proxy = os.environ['https_proxy']
        if 'HTTP_PROXY' in os.environ:
            self.HTTP_PROXY = os.environ['HTTP_PROXY']
        if 'HTTPS_PROXY' in os.environ:
            self.HTTPS_PROXY = os.environ['HTTPS_PROXY']

    def unsetProxies(self):
        """
            Unsets the http/https proxy env vars
        """
        if self.http_proxy is not None:
            del os.environ['http_proxy']

        if self.https_proxy is not None:
            del os.environ['https_proxy']

        if self.HTTP_PROXY is not None:
            del os.environ['HTTP_PROXY']

        if self.HTTPS_PROXY is not None:
            del os.environ['HTTPS_PROXY']

    def resetProxies(self):
        """
            Resets the http/https proxy env vars
        """
        if self.http_proxy is not None:
            os.environ['http_proxy'] = self.http_proxy

        if self.https_proxy is not None:
            os.environ['https_proxy'] = self.https_proxy

        if self.HTTP_PROXY is not None:
            os.environ['HTTP_PROXY'] = self.HTTP_PROXY

        if self.HTTPS_PROXY is not None:
            os.environ['HTTPS_PROXY'] = self.HTTPS_PROXY
