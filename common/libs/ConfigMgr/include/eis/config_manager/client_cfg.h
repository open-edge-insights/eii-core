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

#ifndef _EIS_CH_CLIENT_CFG_H
#define _EIS_CH_CLIENT_CFG_H

#include <string.h>
#include <cjson/cJSON.h>
#include <iostream>
#include <safe_lib.h>
#include <eis/utils/logger.h>
#include "eis/utils/json_config.h"
#include "eis/config_manager/kv_store_plugin.h"
#include "eis/config_manager/app_cfg.h"

#include "eis/config_manager/c_cfg_mgr.h"

namespace eis {
    namespace config_manager {

        class ClientCfg: public AppCfg {
            private:
                // client_cfg_t object
                client_cfg_t* m_cli_cfg;

                // app_cfg_t object
                app_cfg_t* m_app_cfg;
            public:
                /**
                * ClientCfg Constructor
                * @param client_config - The config associated with a client
                */
                explicit ClientCfg();

                /**
                 * Overridden base class method to fetch msgbus client configuration
                 * for application to communicate over EIS message bus
                 * @return config_t* - JSON msg bus server config of type config_t
                 */ 
                config_t* getMsgBusConfig() override;

                /**
                 * getEndpoint for application to fetch Endpoint associated with message bus config
                 * @return std::string - Endpoint of client config of type std::string
                 */
                std::string getEndpoint() override;

                /**
                * client_cfg_t getter to get private m_pub_cfg
                */
                client_cfg_t* getCliCfg();

                /**
                * client_cfg_t setter
                * @param cli_cfg - The pub_cfg to be set
                */
                void setCliCfg(client_cfg_t* cli_cfg);

                /**
                * app_cfg_t getter to get private m_app_cfg
                */
                app_cfg_t* getAppCfg();

                /**
                * app_cfg_t setter
                * @param app_cfg - The app_cfg to be set
                */
                void setAppCfg(app_cfg_t* app_cfg);

                /**
                * Destructor
                */
                ~ClientCfg();

        };
    }
}
#endif
