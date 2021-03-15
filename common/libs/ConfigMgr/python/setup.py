# Copyright (c) 2020 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
"""Python distutils installer for the Python EII Message Bus library
"""

import os
from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

# Python install script for config manager is added
# for v2.4
eii_configmgr_version = "2.4"
eii_version = os.getenv("EII_VERSION")
if eii_version is not None and eii_version != "":
    eii_configmgr_version = eii_version

# Main package setup
setup(
    name='eii-configmgr',
    version=eii_configmgr_version,
    description='EII ConfigMgr Python wrapper',
    keywords='eii configmgr',
    url='',
    package_dir={'': '.'},
    packages=['cfgmgr'],
    ext_modules = cythonize([
            Extension(
                '*',
                ['./cfgmgr/*.pyx'],
                libraries=['eiiconfigmanager'])],
        build_dir='./build/cython',
        compiler_directives={'language_level': 3}
    )
)
