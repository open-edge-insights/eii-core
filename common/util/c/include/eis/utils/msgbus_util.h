// Copyright (c) 2019 Intel Corporation.
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

#ifndef _EIS_MSGBUS_UTIL_H
#define _EIS_MSGBUS_UTIL_H

#include <iostream>
#include <string>
#include <bits/stdc++.h>
#include <cjson/cJSON.h>
#include <eis/config_manager/config_manager.h>
#include "eis/utils/json_config.h"


namespace eis {
    namespace utils {
        class MsgBusUtil {
            private:
                /** Get the tokenized values from a string based on a delimiter set
                 *
                 * @param tokenizable_data: data that needs to be tokenized
                 * @param tokenized_data: tokenized data in std::vector
                 * @param delimeter: character based on which data needs to be tokenized
                 */
                void tokenize(const std::string& tokenizable_data,
                            std::vector<std::string>& tozenized_data,
                            const char delimeter);
                std::string ltrim(const std::string& value);
                std::string rtrim(const std::string& value);
                std::string trim(const std::string& value);

                // True for dev mode and false for prod mode
                bool m_dev_mode;

                // App Name
                std::string m_app_name;

                std::string whitespace = " \n\t";
                // ConfigManager client
                config_mgr_t* m_config_mgr_client;

                // ConfigManager config
                config_mgr_config_t* m_config_mgr_config;

            public:

                /** Constructor
                 */
                MsgBusUtil();

                /** Destructor
                 */
                ~MsgBusUtil();

                /**
                 * Returns a list of all topics the module needs to subscribe or publish
                 *
                 * @param topic_type: type of the topic(pub/sub)
                 * @return std::vector of topics in PubTopics/SubTopics, empty if an
                 *         error occurs
                 */
                std::vector<std::string> get_topics_from_env(const std::string& topic_type);

                /** Returns the config associated with the corresponding topic
                 *
                 * @param topic: name of the topic
                 * @param topic_type: type of the topic(pub/sub)
                 *
                 * @return: config_t or NULL if an error occurs
                 */
                config_t* get_messagebus_config(std::string& topic,
                                                std::string& topic_type);

                /** Returns config manager client
                 */
                config_mgr_t* get_config_mgr_client();

        };
    }
}
#endif
