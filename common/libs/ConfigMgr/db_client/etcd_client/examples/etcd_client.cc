
// Copyright (c) 2020 Intel Corporation.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "etcd_client.h"
#include <unistd.h>

using namespace eis::etcdcli;

void watch_prefix_cb(char *key, char *value, void *user_data){
    std::cout << "watch_prefix callback is called..." << std::endl;
    std::cout << "PREFIX:::\n";
    std::cout << "key:" << key << " value:" << value << std::endl;
    char *data = (char *) user_data;
    printf("userdata: %s\n", data);
}


void watch_cb(char *key, char *value, void *user_data){
    std::cout << "watch callback is called..." << std::endl;
    std::cout << "key:" << key << " value:" << value << std::endl;
}

int main(int argc, char** argv) {
    // etcd certs to run in prod mode
    std::string cert_file = "";
	std::string key_file = "";
	std::string root_file = "";

    // Uncomment below line to run in prod mode
    // EtcdClient etcd_cli("localhost", "2379", cert_file, key_file, root_file);

    // To run in dev mode
    EtcdClient etcd_cli("localhost", "2379");

    std::string get_key = "/VideoIngestion/config";
    std::string get_value = etcd_cli.get(get_key);
    std::cout << "Get result:" << get_value << std::endl;

    std::string key = "/VideoIngestion/datastore";
    std::string put_value = "hello world....";
    
    int put_config_status = etcd_cli.put(key, put_value);
    if(put_config_status == 0) {
        std::cout << "put_config() is success" << std::endl;
    }
    
    std::cout << etcd_cli.get(key) << std::endl;
    std::string key2 = "/Global";

    etcd_cli.watch_prefix(key2, watch_prefix_cb, (void*)"userdatawatch");
    etcd_cli.watch(key, watch_cb, NULL);

    sleep(20);
    return 0;
}
