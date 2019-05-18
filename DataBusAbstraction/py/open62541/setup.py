from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

# By default, -Wformat -Wformat-security compile flags
# are used, so not including it in extra_compile_flags
compileArgs = ['-std=c99', '-g']
extensionName = "open62541W"
sources = ["open62541W.pyx",
           "../../c/open62541/src/open62541_wrappers.c",
           "../../c/open62541/src/open62541.c",
           "../../c/DataBus.c"]
includeDirs = ["../../c/open62541/include",
               "../../c"]
libraryDirs = ["."]
libraries = ["mbedtls", "mbedx509", "safestring",
             "mbedcrypto", "pthread"]

setup(
    name=extensionName,
    ext_modules=cythonize([Extension(extensionName,
                           sources,
                           include_dirs=includeDirs,
                           library_dirs=libraryDirs,
                           libraries=libraries,
                           language="c",
                           extra_compile_args=compileArgs)]),
)
