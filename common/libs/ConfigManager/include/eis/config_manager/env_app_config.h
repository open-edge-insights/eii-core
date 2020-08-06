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
//  * @brief CtxPub BaseCtx interface
//  */

// #ifndef _EIS_CH_CtxPub_H
// #define _EIS_CH_CtxPub_H

// #include <eis/utils/thread_safe_queue.h>
// #include "eis/config_manager/env_ctx_base.h"


// namespace eis {
//     namespace config_manager {
//         /**
//          * CtxPub BaseCtx
//          */
//         class AppConfig : public ConfigHandler {
//             private:
                
//                 // App config
//                 config_t* m_config;

//                 // App interface
//                 // config_t* m_app_interface;


//             protected:
//                 // method to get value in a config
//                 // config_value_t* getValue(std::string key);

//             public:
//                 /**
//                  * Constructor
//                  */
//                 AppConfig();


//                 config_value_t* getValue(std::string key);

//                 /**
//                  * Destructor
//                  */
//                 ~AppConfig();
//         };

//     } // config_manager
// } // eis

// #endif // _EIS_CH_CtxPub_H
