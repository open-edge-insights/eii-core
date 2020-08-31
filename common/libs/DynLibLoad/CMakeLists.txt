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

cmake_minimum_required(VERSION 3.12)
project(eis-dynlibload VERSION 1.0.0 LANGUAGES C CXX)

# Define CMake options
option(WITH_EXAMPLES "Compile with examples" OFF)
option(WITH_TESTS    "Compile with unit tests" OFF)

# Globals
set(EIS_COMMON_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

# Include directory for CMake modules
set(CMAKE_MODULE_PATH "${EIS_COMMON_CMAKE}")

# Set C++ version
set(CMAKE_CXX_STANDARD 14)

# Set C version
set(CMAKE_C_STANDARD 11)

# Set CFLAGS for C/C++
set(CMAKE_C_FLAGS "-fPIE -fPIC -O2 -Wall -fstack-protector-strong -D_FORTIFY_SOURCE=2")
set(CMAKE_CXX_FLAGS "-fPIE -fPIC -O2 -Wall -fstack-protector-strong -D_FORTIFY_SOURCE=2")

# Set LDFLAGS
set(CMAKE_SHARED_LINKER_FLAGS "-z noexecstack -z relro -z now")

# Find packages
find_package(IntelSafeString REQUIRED)
find_package(EISUtils REQUIRED)

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()

# Include header directories
include_directories(
    include/
    ${EISUtils_INCLUDE}
    ${IntelSafeString_INCLUDE})

# Get all source files
file(GLOB SOURCES "src/*.c")

# Add target
add_library(eisdynlibload SHARED ${SOURCES})
target_link_libraries(eisdynlibload
    PRIVATE
        ${EISUtils_LIBRARIES}
        ${IntelSafeString_LIBRARIES}
        ${CMAKE_DL_LIBS})

# Add examples if the option was selected
if(WITH_EXAMPLES)
    add_subdirectory(examples/)
endif()

# Add unit tests if the option was selected
if(WITH_TESTS)
    add_subdirectory(tests/)
endif()

##
## Add CMake configuration for installing the library including files for other
## projects finding the library using CMake
##

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/eisdynlibload)

install(TARGETS eisdynlibload
    EXPORT eisdynlibload-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

set_target_properties(eisdynlibload PROPERTIES EXPORT_NAME dynlibload)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets to a script
install(
    EXPORT eisdynlibload-targets
    FILE DynLibLoadTargets.cmake
    DESTINATION ${INSTALL_CONFIGDIR})

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/DynLibLoadConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/DynLibLoadConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DynLibLoadConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

# Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/DynLibLoadConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/DynLibLoadConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR})

export(EXPORT eisdynlibload-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/DynLibLoadTargets.cmake)

# Register package in user's package registry
export(PACKAGE eisdynlibload)
