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
project(eii-configmanager VERSION 1.0.0 LANGUAGES C CXX)

# Define CMake options

option(WITH_EXAMPLES "Compile with examples" OFF)
option(WITH_GO       "Compile Go Bindings" OFF)
option(WITH_PYTHON   "Compile with Python bindings" OFF)
option(WITH_TESTS    "Compile with tests" OFF)

# Globals
set(EII_COMMON_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

# Set C standard to C11
set(CMAKE_CXX_STANDARD 11)

# Set CFLAGS
set(CMAKE_C_FLAGS "-fPIE -fPIC -O2 -Wall -pedantic -fstack-protector-strong -D_FORTIFY_SOURCE=2")

# Set LDFLAGS
set(CMAKE_SHARED_LINKER_FLAGS "-z noexecstack -z relro -z now")

# Add cmake module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${EII_COMMON_CMAKE}")

# Finding libraries
find_package(cJSON REQUIRED)
find_package(Threads REQUIRED)
find_package(PROTOBUF REQUIRED)
find_package(GRPC REQUIRED)
find_package(EIIUtils REQUIRED)
find_package(EIIMsgEnv REQUIRED)
find_package(IntelSafeString REQUIRED)

#set gRPC_FOUND to true
set(gRPC_FOUND TRUE)

# Install Go EnvConfig if specified to
if(WITH_GO)
    message("-- Configuring Go ConfigMgr installation")

    # Verify the $GOPATH environmental variable exists
    if("$ENV{GOPATH}" STREQUAL "")
        message(FATAL_ERROR "-- $GOPATH envronmental variable is not set")
    endif()

    # Configure install command to copy the Go ConfigMgr to the
    # user's $GOPATH
    install(
        DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/go/ConfigMgr
        DESTINATION
            $ENV{GOPATH}/src/
    )
endif()

# Include header directories
include_directories(
    include/
    ${GRPC_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${EIIMsgEnv_INCLUDE}
    ${EIIUtils_INCLUDE}
    ${IntelSafeString_INCLUDE})

# Get all source files
file(GLOB SOURCES "src/*.c" "cpp/*.cpp" "src/*/*.c" "src/*/etcd_client/*.c" "src/*/etcd_client/*.cpp" "src/*/etcd_client/*/*.cpp")
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)

add_library(eiiconfigmanager_static STATIC ${SOURCES})
add_library(eiiconfigmanager SHARED ${SOURCES})

target_link_libraries(eiiconfigmanager_static
    PUBLIC
        pthread
        ${GRPC_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${EIIMsgEnv_LIBRARIES}
        ${EIIUtils_LIBRARIES}
        ${IntelSafeString_LIBRARIES})

target_link_libraries(eiiconfigmanager
    PUBLIC
        pthread
        ${GRPC_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${EIIMsgEnv_LIBRARIES}
        ${EIIUtils_LIBRARIES}
        ${IntelSafeString_LIBRARIES})

# If compile in debug mode, set DEBUG flag for C code
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_definitions(eiiconfigmanager PRIVATE DEBUG=1)
endif()

if(WITH_EXAMPLES)
    add_subdirectory(examples/)
endif()

if(WITH_PYTHON)
    add_subdirectory(python/)
endif()

if(WITH_TESTS)
    add_subdirectory(tests/)
endif()

##
## Configure pkg-config file to be installed for the EII ConfigMgr
##
set(PKG_CONFIG_IN  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libeiiconfigmanager.pc.in")
set(PKG_CONFIG_OUT "${CMAKE_CURRENT_BINARY_DIR}/libeiiconfigmanager.pc")
set(DEST_DIR       "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS   "-lgrpc++ -lprotobuf")

configure_file(${PKG_CONFIG_IN} ${PKG_CONFIG_OUT} @ONLY)

##
## Add CMake configuration for installing the library including files for other
## projects finding the library using CMake
##

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/EIIConfigMgr)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/EIIConfigMgrStatic)

install(TARGETS eiiconfigmanager
    EXPORT eiiconfigmanager-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS eiiconfigmanager_static
    EXPORT eiiconfigmanager_static-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

set_target_properties(eiiconfigmanager PROPERTIES EXPORT_NAME EIIConfigMgr)
set_target_properties(eiiconfigmanager_static PROPERTIES EXPORT_NAME EIIConfigMgrStatic)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install pkg-config libeiiconfigmanager.pc file
install(
    FILES
        ${PKG_CONFIG_OUT}
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Export targets to a script
install(EXPORT eiiconfigmanager-targets
    FILE
        EIIConfigMgrTargets.cmake
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# Export targets to a script
install(EXPORT eiiconfigmanager_static-targets
    FILE
        EIIConfigMgrStaticTargets.cmake
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/EIIConfigMgrConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/EIIConfigMgrStaticConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT eiiconfigmanager-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrTargets.cmake)

export(EXPORT eiiconfigmanager_static-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticTargets.cmake)

# Register package in user's package registry
export(PACKAGE EIIConfigMgr)
export(PACKAGE EIIConfigMgrStatic)
