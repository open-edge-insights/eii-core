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


/**
 * @file
 * @brief ConfigMgr interface
 */

#ifndef _EIS_CH_CONFIGMAGR_H
#define _EIS_CH_CONFIGMAGR_H

#include <string.h>
#include <cjson/cJSON.h>
#include <functional> 
#include <iostream>
#include <safe_lib.h>
#include <eis/utils/logger.h>
#include "eis/utils/json_config.h"
#include "db_client.h"
#include "eis/config_manager/config_app.h"
#include "eis/config_manager/config_publisher.h"
#include "eis/config_manager/config_subscriber.h"
#include "eis/config_manager/config_server.h"
#include "eis/config_manager/config_client.h"


namespace eis {
    namespace config_manager {

        /**
         * ConfigMgr class
         */        
        class ConfigMgr {
            private:
                
                // App name
                std::string m_app_name;

                // config filled using getAppConfig
                config_t* m_app_config;

                // config filled using getAppConfig
                config_t* m_app_interface;
                                
                // datastore filled using getAppConfig
                config_t* m_app_datastore;

                // ETCD config handler
                AppCfg* m_etcd_handler;
            
            public:

                /**
                * Constructor
                */
                ConfigMgr();
             
                /**
                 * gets app related configs from ConfigManager
                 * @return AppCfg* - AppCfg class object
                 */
                AppCfg* getAppConfig();

                /**
                 * Get server interface from ConfigManager
                 * @param index - These servers are in array for which index is sent to get the respective subscriber config.
                 * @return ServerCfg* - ServerCfg class object
                 */ 
                ServerCfg* getServerByIndex(int index);

                /**
                 * Get client interface from ConfigManager
                 * @param index - These clients are in array for which index is sent to get the respective subscriber config.
                 * @return ClientCfg* - ClientCfg class object
                 */ 
                ClientCfg* getClientByIndex(int index);

                /**
                 * Get publisher interface from ConfigManager
                 * @param index - These publishers are in array for which index is sent to get the respective subscriber config.
                 * @return PublisherCfg* - PublisherCfg class object
                 */ 
                PublisherCfg* getPublisherByIndex(int index);


                /**
                 * Get subscriber interface from ConfigManager
                 * @param index - These subscribers are in array for which index is sent to get the respective subscriber config.
                 * @return SubscriberCfg* - SubscriberCfg class object
                 */ 
                SubscriberCfg* getSubscriberByIndex(int index);
                
                // Destructor
                ~ConfigMgr();

        };
    }
}
#endif
