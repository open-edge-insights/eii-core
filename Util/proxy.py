"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

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
