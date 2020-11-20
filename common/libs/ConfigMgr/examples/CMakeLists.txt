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

# Add config manager example
add_executable(pub "sample_pub.cpp")
add_executable(sub "sample_sub.cpp")
add_executable(server "sample_server.cpp")
add_executable(client "sample_client.cpp")
add_executable(sample_app "sample_getvalue.cpp")
add_executable(kv_store_etcd "kv_store_etcd.c")
target_link_libraries(pub eismsgbus eismsgenv eisutils eisconfigmanager)
target_link_libraries(sub eismsgbus eismsgenv eisutils eisconfigmanager)
target_link_libraries(server eismsgbus eismsgenv eisutils eisconfigmanager)
target_link_libraries(client eismsgbus eismsgenv eisutils eisconfigmanager)
target_link_libraries(sample_app eismsgbus eismsgenv eisutils eisconfigmanager)
target_link_libraries(kv_store_etcd eisconfigmanager eisutils)

# Copy JSON configuration to run examples
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/configs/etcd_kv_store_config.json"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")