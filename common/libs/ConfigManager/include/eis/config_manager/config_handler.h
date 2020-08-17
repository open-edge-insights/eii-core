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
#include <functional> 
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
         * ConfigHandler class
         */  
        class ConfigHandler {
            private:

                // App's config
                config_t* m_conf;

                // App's interface
                config_t* m_intfc;

                // Server/Client/Publish/Subscribr config
                config_t* config;

            protected:
                std::vector<std::string> tokenizer(const char* str,
                                                   const char* delim);

                bool m_dev_mode;
                
                db_client_t* db_client_handle;

            public:

                /**
                * ConfigHandler Constructor
                */
                ConfigHandler(config_t* app_config, config_t* app_interface, bool dev_mode);

                // Interface Cfg for any publisher/subscriber/server/client
                config_value_t* interface_cfg;

                /**
                 * Gets value from Config
                 * @param key - Key for which value is needed
                 * @return config_value_t* - config_value_t object
                 */
                config_value_t* get_value(char* key);

                /**
                 * Get msgbus configuration for application to communicate over EIS message bus
                 * @return config_t* - JSON msg bus server config of type config_t
                 */ 
                virtual config_t* getMsgBusConfig();

                // Destructor
                ~ConfigHandler();
        };
    }
}
#endif
