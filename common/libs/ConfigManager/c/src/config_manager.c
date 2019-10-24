#include "eis/config_manager/config_manager.h"
#include "eis/config_manager/go_config_manager.h"

char* init(char *storage_type, char *ca_cert, char *cert_file, char *key_file){
    char *err_msg = initialize(storage_type, ca_cert, cert_file, key_file);
    return err_msg;
}

char* get_config(char *key){
    return getConfig(key);
}

void register_watch_key(char *key, callback_fcn user_callback){
    registerWatchKey(key, user_callback);
}

void register_watch_dir(char *key, callback_fcn user_callback){
    registerWatchDir(key, user_callback);
}
