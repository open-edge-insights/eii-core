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

#ifndef _EIS_CH_CONFIGHANDLER_H
#define _EIS_CH_CONFIGHANDLER_H

#include <string.h>
#include <cjson/cJSON.h>
#include <iostream>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <safe_lib.h>
#include <eis/utils/logger.h>
#include "eis/utils/json_config.h"
#include "db_client.h"

namespace eis {
    namespace config_manager {

        /**
         * AppCfg class
         */  
        class AppCfg {
            private:

                // App's config
                config_t* m_conf;

                // App's interface
                config_t* m_intfc;

                // Server/Client/Publish/Subscribr config
                config_t* config;

            protected:

                /**
                 * Helper base class function to split string based on delimiter
                 * @param str - string to be split
                 * @param delim - delimiter
                 * @return std::vector<std::string> - vector of split strings
                 */
                std::vector<std::string> tokenizer(const char* str,
                                                   const char* delim);

                // Bool to determine whether DEV or PROD mode
                bool m_dev_mode;

            public:

                // db_client handle to fetch private & public keys
                db_client_t* m_db_client_handle;

                // App name of caller
                std::string m_app_name;

                // Interface Cfg for any publisher/subscriber/server/client
                config_value_t* m_interface_cfg;

                /**
                * AppCfg Constructor
                * @param app_config - The config associated with a service
                * @param app_interface - The interface associated with a service
                * @param dev_mode - bool whether dev mode is set
                */
                AppCfg(config_t* app_config, config_t* app_interface, bool dev_mode);

                /**
                 * Gets value from Config
                 * @param key - Key for which value is needed
                 * @return config_value_t* - config_value_t object
                 */
                config_value_t* getValue(char* key);

                /**
                 * Get msgbus configuration for application to communicate over EIS message bus
                 * @return config_t* - JSON msg bus server config of type config_t
                 */ 
                virtual config_t* getMsgBusConfig();

                /**
                 * virtual getEndpoint function implemented by child classes to fetch Endpoint
                 * @return std::string - Endpoint of associated config of type std::string
                 */
                virtual std::string getEndpoint();

                /**
                 * virtual getTopics function implemented by child classes to fetch topics
                 * @return std::string - Endpoint of associated config of type std::string
                 */
                virtual std::vector<std::string> getTopics();

                /**
                 * virtual setTopics function implemented by child classes to set topics
                 * @param topics_list - vector of strings containing topics
                 * @return bool - Boolean whether topics were set
                 */
                virtual bool setTopics(std::vector<std::string> topics_list);

                /**
                 * virtual getTopics function implemented by child classes to fetch topics
                 * @return std::string - Endpoint of associated config of type std::string
                 */
                virtual std::vector<std::string> getAllowedClients();

                // Destructor
                virtual ~AppCfg();
        };
    }
}
#endif
