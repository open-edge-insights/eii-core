# Copyright (c) 2020 Intel Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

cmake_minimum_required(VERSION 3.11)

project(eis-dbclient VERSION 1.0.0 LANGUAGES C CXX)

# Define CMake options
option(WITH_EXAMPLES "Compile with examples" OFF)
option(WITH_TESTS "Compile with tests" OFF)

# Globals
set(EIS_COMMON_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")

# Set C standard to C11
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_FLAGS "-fPIE -fPIC -O2 -Wall -pedantic -fstack-protector-strong -D_FORTIFY_SOURCE=2")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

# Add cmake module path
set(CMAKE_MODULE_PATH "${EIS_COMMON_CMAKE}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# EtcdClient headers
set(EtcdClientHearders "etcd_client/include/")

file(GLOB SOURCES ${SOURCES} "src/*.cpp" "src/*.c")

# Include header directories
include_directories(
    include/
    ${EtcdClientHearders}
    ${EISUtils_INCLUDE}
    ${IntelSafeString_INCLUDE})

if(WITH_EXAMPLES)
    add_subdirectory(examples/)
endif()

if(WITH_TESTS)
    add_subdirectory(tests/)
endif()

add_subdirectory(etcd_client/)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
