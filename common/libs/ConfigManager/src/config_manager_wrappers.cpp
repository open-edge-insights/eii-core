#include <string>
#include "config_manager.h"
#include "config_manager_wrappers.h"

ConfigManager::ConfigManager(std::string storage_type, std::string ca_cert, std::string cert_file, std::string key_file){
    char *cstorage_type = const_cast<char *>(storage_type.c_str());
    char *cca_cert = const_cast<char *>(ca_cert.c_str());
    char *ccert_file = const_cast<char *>(cert_file.c_str());
    char *ckey_file = const_cast<char *>(key_file.c_str());
    initialize(cstorage_type, cca_cert, ccert_file, ckey_file);
}

char * ConfigManager::GetConfig(std::string key){
    char *ckey = const_cast<char *>(key.c_str());
    char *value = (char *)getConfig(ckey);
    return value;
}

void ConfigManager::RegisterWatchKey(std::string key,void (*callback_fcn)(char* key, char* value)){
    char *ckey = const_cast<char *>(key.c_str());
    registerWatchKey(ckey, callback_fcn);
}

void ConfigManager::RegisterWatchDir(std::string key,void (*callback_fcn)(char* key, char* value)){
    char *ckey = const_cast<char *>(key.c_str());
    registerWatchDir(ckey, callback_fcn);
}

