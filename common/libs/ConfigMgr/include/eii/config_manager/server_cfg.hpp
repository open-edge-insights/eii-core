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

#ifndef _EII_CH_SERVER_CFG_H
#define _EII_CH_SERVER_CFG_H

#include <string.h>
#include <cjson/cJSON.h>
#include <iostream>
#include <safe_lib.h>
#include <eii/utils/logger.h>
#include "eii/utils/json_config.h"
#include "eii/config_manager/kv_store_plugin/kv_store_plugin.h"
#include "eii/config_manager/app_cfg.hpp"
#include "eii/config_manager/cfgmgr.h"


namespace eii {
    namespace config_manager {

        class ServerCfg : public AppCfg {
            private:

                // cfgmgr_interface_t object
                cfgmgr_interface_t* m_cfgmgr_interface;

                /**
                 * Private @c ServerCfg copy constructor.
                 */
                ServerCfg(const ServerCfg& src);

                /**
                 * Private @c SubscriberCfg assignment operator.
                 */
                ServerCfg& operator=(const ServerCfg& src);
            public:
                /**
                * ServerCfg Constructor
                * This constructor is not to be directly called since it is only used
                * internally by the ConfigMgr
                * @param cfgmgr_interface - The interface associated with a server
                */
                explicit ServerCfg(cfgmgr_interface_t* cfgmgr_interface);

                /**
                 * Constructs message bus config for Server
                 * @return config_t* - On Success, JSON msg bus server config of type config_t
                 *                   - On Failure, returns NULL
                 */
                config_t* getMsgBusConfig() override;

                /**
                 * To get particular interface value from Server interface config
                 * @param key - Key on which interface value is extracted.
                 * @return config_value_t* - On Success, config_value_t object
                 *                         - On Failure, returns NULL
                 */
                config_value_t* getInterfaceValue(const char* key) override;

                /**
                 * To get endpoint for particular server from its interface config
                 * @return std::string - On Success returns Endpoint of server config
                 *                     - On Failure returns empty string
                 */
                std::string getEndpoint() override;

                /**
                 * To get the names of the clients allowed to connect to server
                 * @return vector<string> - On Success, returns Allowed client of server config
                 *                        - On Failure, returns empty vector
                 */
                std::vector<std::string> getAllowedClients() override;

                /**
                * cfgmgr_interface_t getter to get server interface
                */
                cfgmgr_interface_t* getServCfg();

                /**
                * Destructor
                */
                ~ServerCfg();

        };
    }
}
#endif
