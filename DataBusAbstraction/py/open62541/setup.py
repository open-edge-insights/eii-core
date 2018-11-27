from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

setup(
    name="open62541W",
    ext_modules=cythonize([Extension("open62541W",
                           ["open62541W.pyx",
                            "../../c/open62541/src/open62541_wrappers.c",
                            "../../c/open62541/src/open62541.c"],
                           include_dirs=["../../c/open62541/include"],
                           libraries=["mbedtls", "mbedx509",
                                      "mbedcrypto", "pthread"],
                           language="c",
                           extra_compile_args=['-std=c99'])]),
)
