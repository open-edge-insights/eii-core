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

#ifndef _EIS_CH_CONFIGSERVER_H
#define _EIS_CH_CONFIGSERVER_H

#include <string.h>
#include <cjson/cJSON.h>
#include <functional> 
#include <iostream>
#include <safe_lib.h>
#include <eis/utils/logger.h>
#include "eis/utils/json_config.h"
#include "db_client.h"
#include "eis/config_manager/config_handler.h"


namespace eis {
    namespace config_manager {

        class ServerCfg : public ConfigHandler {
            public:

                // App config
                config_t* config;

                // Subscriber config
                config_value_t* server_cfg;

                /**
                * ServerCfg Constructor
                */
                ServerCfg(config_value_t* server_config);

                /**
                 * Overridden base class method to fetch msgbus server configuration
                 * for application to communicate over EIS message bus
                 * @return config_t* - JSON msg bus server config of type config_t
                 */
                config_t* getMsgBusConfig() override;

                /**
                 * getEndpoint for application to fetch Endpoint associated with message bus config
                 * @return std::string - Endpoint of server config of type std::string
                 */
                std::string getEndpoint() override;

                /**
                 * getAllowedClients for application to list of allowed clients associated with message bus config
                 * @return vector<string> - Allowed client of server config
                 */
                std::vector<std::string> getAllowedClients() override;

                // Destructor
                ~ServerCfg();

        };
    }
}
#endif
