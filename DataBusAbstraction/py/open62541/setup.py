from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

# By default, -Wformat -Wformat-security compile flags
# are used, so not including it in extra_compile_flags
compileArgs = ['-std=c99']
extensionName = "open62541W"
sources = ["open62541W.pyx",
           "../../c/open62541/src/open62541_wrappers.c",
           "../../c/open62541/src/open62541.c"]
includeDirs = ["../../c/open62541/include"]
libraries = ["mbedtls", "mbedx509",
             "mbedcrypto", "pthread"]

setup(
    name=extensionName,
    ext_modules=cythonize([Extension(extensionName,
                           sources,
                           include_dirs=includeDirs,
                           libraries=libraries,
                           language="c",
                           extra_compile_args=compileArgs)]),
)