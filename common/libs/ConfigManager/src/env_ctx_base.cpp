// // Copyright (c) 2019 Intel Corporation.
// //
// // Permission is hereby granted, free of charge, to any person obtaining a copy
// // of this software and associated documentation files (the "Software"), to
// // deal in the Software without restriction, including without limitation the
// // rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// // sell copies of the Software, and to permit persons to whom the Software is
// // furnished to do so, subject to the following conditions:
// //
// // The above copyright notice and this permission notice shall be included in
// // all copies or substantial portions of the Software.
// //
// // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// // AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// // LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// // FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// // IN THE SOFTWARE.

// /**
//  * @file
//  * @brief BaseCtx Base Implementation
//  */

// #include <unistd.h>
// #include <sstream>
// #include <random>
// #include <string>
// #include <string.h>
// #include <algorithm>
// #include <eis/utils/logger.h>
// #include "eis/config_manager/env_ctx_base.h"
// #include "eis/config_manager/env_ctx_publisher.h"
// // #include "eis/vi/env_ctx_subscriber.h"

// using namespace eis::utils;
// using namespace eis::config_manager;

// BaseCtx::BaseCtx(config_t* config) :
//     m_config(config) {

// }

// BaseCtx::~BaseCtx() {
//     LOG_DEBUG_0("BaseCtx destructor");
// }


// // BaseCtx* getAppConfig(char* key) {

// // }

// BaseCtx* get_BaseCtx(const char* type) {

//     BaseCtx* BaseCtx = NULL;

//     if(!strcmp(type, "Publisher")) {
//         BaseCtx = new Publisher(m_config);
//     } else if(!strcmp(type, "Subscriber")) {
//         // BaseCtx = new EnvCtxSubscriber();
//         throw("Unknown Subscriber");
//     } else {
//         throw("Unknown BaseCtx");
//     }
//     return BaseCtx;
// }


// char* BaseCtx::