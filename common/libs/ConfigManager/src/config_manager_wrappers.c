#include "eis/config_manager/config_manager.h"
#include "eis/config_manager/config_manager_wrappers.h"

char* init(const char *storage_type, const char *ca_cert, const char *cert_file, const char *key_file){
    char *err_msg = initialize(storage_type, ca_cert, cert_file, key_file);
    return err_msg;
}

char* get_config(const char *key){
    return getConfig(key);
}

void register_watch_key(const char *key, callback_fcn user_callback){
    registerWatchKey(key, user_callback);
}

void register_watch_dir(const char *key, callback_fcn user_callback){
    registerWatchDir(key, user_callback);
}
