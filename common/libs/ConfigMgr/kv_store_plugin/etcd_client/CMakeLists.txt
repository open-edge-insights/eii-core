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


if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()

# Add cmake module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${EIS_COMMON_CMAKE}")

file(GLOB SOURCES ${SOURCES} "src/*.cc" "src/*.c")

# Finding libraries
find_package(Threads REQUIRED)
find_package(PROTOBUF REQUIRED)
find_package(GRPC REQUIRED)

include_directories(
    include/
    ${GRPC_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS})

add_library(eiskvstoreplugin SHARED
    ${SOURCES})

target_link_libraries(eiskvstoreplugin
    ${GRPC_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${EISUtils_LIBRARIES})

## Configure pkg-config file to be installed for the EIS Etcd Client
##
set(PKG_CONFIG_IN  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libkvstoreplugin.pc.in")
set(PKG_CONFIG_OUT "${CMAKE_CURRENT_BINARY_DIR}/libkvstoreplugin.pc")
set(DEST_DIR       "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS   "-lgrpc++ -lprotobuf")

configure_file(${PKG_CONFIG_IN} ${PKG_CONFIG_OUT} @ONLY)

##
## Add CMake configuration for installing the library including files for other
## projects finding the library using CMake
##

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/EISKVStorePlugin)

    
install(TARGETS eiskvstoreplugin
    EXPORT eiskvstoreplugin-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

set_target_properties(eiskvstoreplugin PROPERTIES EXPORT_NAME EISKVStorePlugin)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install pkg-config libeiskvstoreplugin.pc file
install(
    FILES
        ${PKG_CONFIG_OUT}
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Export targets to a script
install(EXPORT eiskvstoreplugin-targets
    FILE
        EISKVStorePlugin.cmake
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/EISKVStorePluginConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/EISKVStorePluginConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/EISKVStorePluginConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install the config, config version and custom find modules
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/EISKVStorePluginConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EISKVStorePluginConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT eiskvstoreplugin-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/EISKVStorePluginTargets.cmake)
  
# Register package in user's package registry
export(PACKAGE EISKVStorePlugin)

