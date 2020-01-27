#include "eis/msgbus/msgbus.h"
#include <eis/config_manager/env_config.h>

static config_mgr_t* g_config_mgr = NULL;

config_mgr_t* get_config_mgr(){
    std::string pub_cert_file = "";
    std::string pri_key_file = "";
    std::string trust_file = "";
    std::string m_app_name = getenv("AppName");
    bool dev_mode;

    std::string dev_mode_str=getenv("DEV_MODE");
    if (dev_mode_str == "false") {
        dev_mode = false;
    } else if (dev_mode_str == "true") {
        dev_mode = true;
    }

    if(!dev_mode) {
        pub_cert_file = "/run/secrets/etcd_" + m_app_name + "_cert";
        pri_key_file = "/run/secrets/etcd_" + m_app_name + "_key";
        trust_file = "/run/secrets/ca_etcd";
    }

    g_config_mgr = config_mgr_new("etcd",
                                 (char*)pub_cert_file.c_str(),
                                 (char*) pri_key_file.c_str(),
                                 (char*) trust_file.c_str());
    return g_config_mgr;
}
