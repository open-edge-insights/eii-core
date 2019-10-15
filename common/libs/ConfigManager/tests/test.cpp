#include <iostream>
#include <string>
#include<unistd.h>
#include "config_manager_wrappers.h"

void c_callback(char* key, char * val){
    std::cout << "user callback is called" << std::endl;
    std::cout << "key: "<< key << "value: " << val << std::endl;
}

int main() {
    ConfigManager obj = ConfigManager("etcd","", "", "");
    std::cout << "Cpp GetConfig is called" << obj.GetConfig("/GlobalEnv/") << std::endl;
    std::cout << "Watch call on key /GlobalEnv/" << std::endl;
    obj.RegisterWatchKey((char *)"/GlobalEnv/", c_callback);
    sleep(15);
    std::cout << "Watch call on prefix /Global" << std::endl;
    obj.RegisterWatchDir((char *)"/Global", c_callback);
    sleep(15);

    return 0;
}
