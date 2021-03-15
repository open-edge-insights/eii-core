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
#include "eii/config_manager/util_cfg.h"
#include "eii/config_manager/cfg_mgr.h"


namespace eii {
    namespace config_manager {

        class ServerCfg : public AppCfg {
            private:
                // server_cfg_t object
                server_cfg_t* m_serv_cfg;

                // app_cfg_t object
                app_cfg_t* m_app_cfg;
            public:
                /**
                * ServerCfg Constructor
                * @param server_config - The config associated with a server
                * @param app_cfg       - app_cfg_t pointer
                */
                explicit ServerCfg(server_cfg_t* serv_cfg, app_cfg_t* app_cfg);

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
                * server_cfg_t getter to get private m_serv_cfg
                */
                server_cfg_t* getServCfg();

                /**
                * app_cfg_t getter to get private m_app_cfg
                */
                app_cfg_t* getAppCfg();

                /**
                * Destructor
                */
                ~ServerCfg();

        };
    }
}
#endif
