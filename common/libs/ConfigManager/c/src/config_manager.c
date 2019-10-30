#include "eis/config_manager/config_manager.h"
#include "eis/config_manager/go_config_manager.h"

static char* get_config(char *key){
    return getConfig(key);
}

static void register_watch_key(char *key, callback_fcn user_callback){
    registerWatchKey(key, user_callback);
}

static void register_watch_dir(char *key, callback_fcn user_callback){
    registerWatchDir(key, user_callback);
}

static void free_config(void *config){
    free(config);
}

config_mgr_t* config_mgr_new(config_mgr_config_t *config_mgr_config){
    config_mgr_t *config_mgr = (config_mgr_t *)malloc(sizeof(config_mgr_t));

    config_mgr->status = initialize(config_mgr_config->storage_type,
                                    config_mgr_config->ca_cert,
                                    config_mgr_config->cert_file,
                                    config_mgr_config->key_file);

    config_mgr->get_config = get_config;
    config_mgr->register_watch_key = register_watch_key;
    config_mgr->register_watch_dir = register_watch_dir;
    config_mgr->free_config = free_config;
    return config_mgr;
}
