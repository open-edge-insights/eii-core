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
#include <iostream>
#include <safe_lib.h>
#include <eis/utils/logger.h>
#include "eis/utils/json_config.h"
#include "eis/config_manager/kv_store_plugin.h"
#include "eis/config_manager/app_cfg.hpp"
#include "eis/config_manager/publisher_cfg.hpp"
#include "eis/config_manager/subscriber_cfg.hpp"
#include "eis/config_manager/server_cfg.hpp"
#include "eis/config_manager/client_cfg.hpp"


namespace eis {
    namespace config_manager {

        /**
         * ConfigMgr class
         */        
        class ConfigMgr {
            private:
                // AppCfg handler
                AppCfg* m_app_cfg_handler;

                // app_cfg_t object
                app_cfg_t* m_app_cfg;

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
                 * @param index - These servers are in array for which index is sent to get the respective server config.
                 * @return ServerCfg* - ServerCfg class object
                 */
                ServerCfg* getServerByIndex(int index);

                /**
                 * Get server interface from ConfigManager
                 * @param name - These servers are in array for which name is sent to get the respective server config.
                 * @return ServerCfg* - ServerCfg class object
                 */
                ServerCfg* getServerByName(const char* name);

                /**
                 * Get client interface from ConfigManager
                 * @param index - These clients are in array for which index is sent to get the respective client config.
                 * @return ClientCfg* - ClientCfg class object
                 */
                ClientCfg* getClientByIndex(int index);

                /**
                 * Get client interface from ConfigManager
                 * @param name - These clients are in array for which name is sent to get the respective client config.
                 * @return ClientCfg* - ClientCfg class object
                 */
                ClientCfg* getClientByName(const char* name);

                /**
                 * Get publisher interface from ConfigManager
                 * @param index - These publishers are in array for which index is sent to get the respective publisher config.
                 * @return PublisherCfg* - PublisherCfg class object
                 */
                PublisherCfg* getPublisherByIndex(int index);

                /**
                 * Get publisher interface from ConfigManager
                 * @param name - These publishers are in array for which name is sent to get the respective publisher config.
                 * @return PublisherCfg* - PublisherCfg class object
                 */
                PublisherCfg* getPublisherByName(const char* name);

                /**
                 * Get subscriber interface from ConfigManager
                 * @param index - These subscribers are in array for which name is sent to get the respective subscriber config.
                 * @return SubscriberCfg* - SubscriberCfg class object
                 */
                SubscriberCfg* getSubscriberByIndex(int index);

                /**
                 * Get subscriber interface from ConfigManager
                 * @param name - These subscribers are in array for which name is sent to get the respective subscriber config.
                 * @return SubscriberCfg* - SubscriberCfg class object
                 */
                SubscriberCfg* getSubscriberByName(const char* name);
                
                /**
                * Destructor
                */
                ~ConfigMgr();

        };
    }
}
#endif
