# Copyright (c) 2021 Intel Corporation.
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

##
## Configure pkg-config file to be installed for the EII ConfigMgr
##
set(PKG_CONFIG_IN  "${CMAKE_SOURCE_DIR}/cmake/libeiiconfigmanager.pc.in")
set(PKG_CONFIG_OUT "${CMAKE_BINARY_DIR}/libeiiconfigmanager.pc")
set(PKG_CONFIG_STATIC_IN  "${CMAKE_SOURCE_DIR}/cmake/libeiiconfigmanager_static.pc.in")
set(PKG_CONFIG_STATIC_OUT "${CMAKE_BINARY_DIR}/libeiiconfigmanager_static.pc")
set(DEST_DIR       "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS   "-lgrpc++ -lprotobuf")

configure_file(${PKG_CONFIG_IN} ${PKG_CONFIG_OUT} @ONLY)
configure_file(${PKG_CONFIG_STATIC_IN} ${PKG_CONFIG_STATIC_OUT} @ONLY)

##
## Add CMake configuration for installing the library including files for other
## projects finding the library using CMake
##

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/EIIConfigMgr)
set(INSTALL_CONFIGDIR_STATIC ${CMAKE_INSTALL_LIBDIR}/cmake/EIIConfigMgrStatic)

install(TARGETS eiiconfigmanager
    EXPORT eiiconfigmanager-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# gRPC is dynamically linked to the static config manager target.
# Exporting a statically linked target means the person linking to the
# static library needs to know how to link its dynamically linked dependencies,
# so all the target info for grpc must be exposed to whoever's linking the
# static library
install(TARGETS
    libprotobuf libprotoc
    zlibstatic absl_algorithm absl_inlined_vector_internal  absl_log_severity
    absl_atomic_hook absl_base absl_meta absl_bits absl_str_format
    absl_compressed_tuple absl_span  absl_dynamic_annotations
    absl_spinlock_wait absl_str_format_internal absl_errno_saver
    absl_base_internal absl_time_zone absl_throw_delegate
    absl_bad_optional_access absl_utility absl_inlined_vector absl_civil_time
    absl_config absl_int128 absl_raw_logging_internal absl_strings_internal
    absl_type_traits absl_endian absl_core_headers absl_time absl_memory ssl
    crypto c-ares absl_optional absl_strings address_sorting gpr grpc
    grpc_unsecure grpc++ grpc++_alts grpc++_error_details grpc++_reflection
    grpc++_unsecure grpc_plugin_support grpcpp_channelz upb grpc_cpp_plugin
    grpc_csharp_plugin grpc_node_plugin grpc_objective_c_plugin grpc_php_plugin
    grpc_python_plugin grpc_ruby_plugin eiiconfigmanager_static
    EXPORT eiiconfigmanager_static-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR})

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

install(
    FILES
        ${PKG_CONFIG_STATIC_OUT}
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
        ${INSTALL_CONFIGDIR_STATIC}
)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/EIIConfigMgrConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/EIIConfigMgrConfig.cmake.in
    ${CMAKE_BINARY_DIR}/EIIConfigMgrConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/EIIConfigMgrStaticConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR_STATIC}
)

# Install the config, configversion and custom find modules
install(
    FILES
        ${CMAKE_BINARY_DIR}/EIIConfigMgrConfigVersion.cmake
        ${CMAKE_BINARY_DIR}/EIIConfigMgrConfig.cmake
    DESTINATION
        ${INSTALL_CONFIGDIR}
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR_STATIC}
)

export(EXPORT eiiconfigmanager-targets
       FILE ${CMAKE_BINARY_DIR}/EIIConfigMgrTargets.cmake)

export(EXPORT eiiconfigmanager_static-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/EIIConfigMgrStaticTargets.cmake)

# Register package in user's package registry
export(PACKAGE EIIConfigMgr)
export(PACKAGE EIIConfigMgrStatic)
