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

#ifndef _EII_CH_CONFIGMAGR_H
#define _EII_CH_CONFIGMAGR_H

#include <string.h>
#include <cjson/cJSON.h>
#include <iostream>
#include <safe_lib.h>
#include <eii/utils/logger.h>
#include "eii/utils/json_config.h"
#include "eii/config_manager/kv_store_plugin/kv_store_plugin.h"
#include "eii/config_manager/app_cfg.hpp"
#include "eii/config_manager/publisher_cfg.hpp"
#include "eii/config_manager/subscriber_cfg.hpp"
#include "eii/config_manager/server_cfg.hpp"
#include "eii/config_manager/client_cfg.hpp"


namespace eii {
    namespace config_manager {

        /**
         * ConfigMgr class
         */        
        class ConfigMgr {
            private:
                // AppCfg handler
                AppCfg* m_app_cfg_handler;

                // cfgmgr_ctx_t object
                cfgmgr_ctx_t* m_cfgmgr;

            public:

                /**
                * Constructor
                * To instantiate main ConfigMgr object
                */
                ConfigMgr();
             
                /**
                 * gets app related configs from ConfigManager
                 * @return AppCfg* - AppCfg class object
                 */
                AppCfg* getAppConfig();

                /**
                 * Get total number of publishers in an interface
                 * @return int - number of publisher interfaces
                 */
                int getNumPublishers();

                /**
                 * Get total number of subscribers in an interface
                 * @return int - number of subscriber interfaces
                 */
                int getNumSubscribers();

                /**
                 * Get total number of servers in an interface
                 * @return int - number of server interfaces
                 */
                int getNumServers();

                /**
                 * Get total number of clients in an interface
                 * @return int - number of client interfaces
                 */
                int getNumClients();

                /**
                 * To check if application is running in dev or prod mode
                 * @return bool - True if dev mode & false if prod mode
                 */
                bool isDevMode();

                /**
                 * Get the AppName for any service
                 * @return std::string - AppName string
                 */
                std::string getAppName();

                /**
                 * Get server interface using it's index
                 * @param index - These servers are in array for which index is sent to get the respective server config.
                 * @return ServerCfg* - ServerCfg class object
                 */
                ServerCfg* getServerByIndex(int index);

                /**
                 * Get server interface using it's name
                 * @param name - These servers are in array for which name is sent to get the respective server config.
                 * @return ServerCfg* - ServerCfg class object
                 */
                ServerCfg* getServerByName(const char* name);

                /**
                 * Get client interface using it's index
                 * @param index - These clients are in array for which index is sent to get the respective client config.
                 * @return ClientCfg* - ClientCfg class object
                 */
                ClientCfg* getClientByIndex(int index);

                /**
                 * Get client interface using it's name
                 * @param name - These clients are in array for which name is sent to get the respective client config.
                 * @return ClientCfg* - ClientCfg class object
                 */
                ClientCfg* getClientByName(const char* name);

                /**
                 * Get publisher interface using it's index
                 * @param index - These publishers are in array for which index is sent to get the respective publisher config.
                 * @return PublisherCfg* - PublisherCfg class object
                 */
                PublisherCfg* getPublisherByIndex(int index);

                /**
                 * Get publisher interface using it's name
                 * @param name - These publishers are in array for which name is sent to get the respective publisher config.
                 * @return PublisherCfg* - PublisherCfg class object
                 */
                PublisherCfg* getPublisherByName(const char* name);

                /**
                 * Get subscriber interface using it's index
                 * @param index - These subscribers are in array for which name is sent to get the respective subscriber config.
                 * @return SubscriberCfg* - SubscriberCfg class object
                 */
                SubscriberCfg* getSubscriberByIndex(int index);

                /**
                 * Get subscriber interface using it's name
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
