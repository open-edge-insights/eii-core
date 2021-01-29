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
#include "eis/config_manager/kv_store_plugin/kv_store_plugin.h"
#include "eis/config_manager/app_cfg.hpp"
#include "eis/config_manager/cfgmgr.h"

namespace eis {
    namespace config_manager {

        class ClientCfg: public AppCfg {
            private:

                // cfgmgr_interface_t object
                cfgmgr_interface_t* m_cfgmgr_interface;
            public:
                /**
                * ClientCfg Constructor
                * @param client_config - The config associated with a client
                * @param app_cfg       - app_cfg_t pointer
                */
                explicit ClientCfg(cfgmgr_interface_t* cfgmgr_interface);

                /**
                 * Constructs message bus config for Client
                 * @return config_t* - On Success, returns JSON msg bus server config of type config_t
                 *                   - On Failure, returns NULL
                 */ 
                config_t* getMsgBusConfig() override;

                /**
                 * To fetch particular interface value from Client interface config
                 * @param key - Key on which interface value is extracted.
                 * @return config_value_t* - On Success, config_value_t object
                 *                         - On Failure, returns NULL
                 */ 
                config_value_t* getInterfaceValue(const char* key) override;

                /**
                 * To fetch Endpoint for particular client from its interface config
                 * @return std::string - Endpoint of client config of type std::string
                 */
                std::string getEndpoint() override;

                /**
                * cfgmgr_interface_t getter to get private m_pub_cfg
                */
                cfgmgr_interface_t* getCfg();

                /**
                * Destructor
                */
                ~ClientCfg();

        };
    }
}
#endif
