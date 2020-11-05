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

#ifndef _EIS_CH_PUBLISHER_CFG_H
#define _EIS_CH_PUBLISHER_CFG_H

#include <string.h>
#include <cjson/cJSON.h>
#include <iostream>
#include <vector>
#include <safe_lib.h>
#include <eis/utils/logger.h>
#include "eis/utils/json_config.h"
#include "eis/config_manager/kv_store_plugin/kv_store_plugin.h"
#include "eis/config_manager/app_cfg.hpp"
#include "eis/config_manager/pub_cfg.h"
#include "eis/config_manager/util_cfg.h"

#include "eis/config_manager/cfg_mgr.h"

namespace eis {
    namespace config_manager {

        class PublisherCfg : public AppCfg {
            private:
                // pub_cfg_t object
                pub_cfg_t* m_pub_cfg;

                // app_cfg_t object
                app_cfg_t* m_app_cfg;

            public:

                /**
                * PublisherCfg Constructor
                * @param pub_config - The config associated with a client
                */
                explicit PublisherCfg(pub_cfg_t* pub_cfg, app_cfg_t* app_cfg);

                /**
                 * Overridden base class method to fetch msgbus publisher configuration
                 * for application to communicate over EIS message bus
                 * @return config_t* - On Success, JSON msg bus publisher config of type config_t
                 *                   - On failure, On success, returns NULL
                 */
                config_t* getMsgBusConfig() override;

                /**
                 * getEndpoint for application to fetch Endpoint associated with message bus config
                 * @return std::string - On Success, returns Endpoint of server config
                 *                     - On Failure, returns empty string
                 */
                std::string getEndpoint() override;

                /**
                 * Overridden base class method to fetch interface value
                 * for application to communicate over EIS message bus
                 * @param key - Key on which interface value is extracted.
                 * @return config_value_t* - On Success, returns config_value_t object
                 *                         - On Failure, returns NULL
                 */
                config_value_t* getInterfaceValue(const char* key) override;

                /**
                 * getTopics for application to fetch the topics associated with message bus config
                 * @return vector<string> - On Success, returns Topics of publisher config
                 *                        - On Failure, returns empty vector
                 */
                std::vector<std::string> getTopics() override;

                /**
                 * setTopics for application to set topics associated with message bus config
                 * @param topics_list - List of topics to be set
                 * @return bool - Boolean whether topics were set
                 */
                bool setTopics(std::vector<std::string> topics_list) override;

                /**
                 * getAllowedClients for application to list of allowed clients associated with message bus config
                 * @return vector<string> - On Success, Allowed client of publisher config
                 *                        - On Failure, returns empty vector
                 */
                std::vector<std::string> getAllowedClients() override;

                /**
                * pub_cfg_t getter to get private m_pub_cfg
                */
                pub_cfg_t* getPubCfg();

                /**
                * app_cfg_t getter to get private m_app_cfg
                */
                app_cfg_t* getAppCfg();

                /**
                * Destructor
                */
                ~PublisherCfg();

        };
    }
}
#endif
