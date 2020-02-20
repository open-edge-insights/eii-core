//get_app_config gets the app config and returns the sleep value from etcd_pre_load.json

#include "eis/msgbus/msgbus.h"
#include <eis/config_manager/env_config.h>

int get_app_config(char* cpp_app_config){
    config_t* config = json_config_new_from_buffer(cpp_app_config);
    if(config == NULL) {
        const char* err = "Failed to initialize configuration object";
        LOG_ERROR("%s", err);
        throw(err);
    }
    config_value_t* loop_interval = config->get_config_value(config->cfg,
                                                            "loop_interval");
    if(loop_interval == NULL) {
        const char* err = "\"loop_interval\" key is missing";
        LOG_ERROR("%s", err);
        throw(err);
    }
    if(loop_interval->type != CVT_STRING) {
        const char* err = "\"type\" value has to be of string type";
        LOG_ERROR("%s", err);
        config_destroy(config);
        config_value_destroy(loop_interval);
        throw(err);
    }
    char* loop_val = loop_interval->body.string;
    int sleep_val = atoi (loop_val);
    return sleep_val;
}
