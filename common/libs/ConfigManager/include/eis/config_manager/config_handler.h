// Copyright (c) 2019 Intel Corporation.
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


/**
 * @file
 * @brief ConfigHandler interface
 */

#ifndef _EIS_CH_CONFIGHANDLER_H
#define _EIS_CH_CONFIGHANDLER_H

#include <thread>
#include <functional> 
#include <atomic>
#include <condition_variable>
#include <string.h>
#include <eis/utils/logger.h>
// #include <eis/config_manager/cfg_publisher.h>
#include "eis/utils/json_config.h"
#include "eis/config_manager/config_manager.h"
// #include <eis/utils/config.h>
// #include <eis/utils/thread_safe_queue.h>
// #include <eis/utils/json_config.h>
// #include <eis/config_manager/env_config.h>
// #include <eis/config_manager/config_manager.h>
// #include <eis/msgbus/msgbus.h>
// #include <eis/msgbus/msg_envelope.h>
// #include "eis/ech/env_ctx_base.h"

// using namespace eis::utils;

namespace eis {
    namespace config_manager {
        /**
         * ConfigHandler class
         */

        class CfgMgr;
        
        class ConfigHandler {
            private:

                // EtcdClient
                // EtcdClient* etcd_client;
                
                // App name
                std::string m_app_name;

                // config filled using getAppConfig
                // config_t* m_app_config;

                // config filled using getAppConfig
                // config_t* m_app_interface;
                
                
                // config filled using getAppConfig
                config_t* m_app_datastore;

                config_value_t* server_interface;
                config_value_t* client_interface;
                

            public:

                /**
                * Constructor
                */
                ConfigHandler();
             
                config_t* m_app_config;
                // void getAppData();

                config_t* m_app_interface;
                // config_value_t* server_cfg;
                

                // Publilsher* getPublisher();
                void getPublisher();

                // BaseCtx* getAppConfig();
                // void getAppConfig();
                CfgMgr* getAppConfig();

                CfgMgr* getServerByIndex(int index);

                CfgMgr* getClientByIndex(int index);

                void getSubscriber();

                // void getAppConfig();
                
                // Destructor
                ~ConfigHandler();

        };


        class CfgMgr : public ConfigHandler{
            public:
                CfgMgr();
                config_t* m_conf;
                config_t* m_intfc;

                
                config_value_t* get_value(char* key);
                config_t* get_server_msgbus_cfg();
                config_t* get_client_msgbus_cfg();
                // get_EndPoimt()
                // get_settopics();
                config_t* config;
                config_value_t* server_cfg;
                config_value_t* client_cfg;


        };
    }
}
#endif
