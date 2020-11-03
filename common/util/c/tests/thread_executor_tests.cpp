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
 * @brief ThreadExecutor unit tests
 */

#include "eis/utils/thread_executor.hpp"
#include "eis/utils/logger.h"
#include <gtest/gtest.h>
#include <stdlib.h>
#include <atomic>
#include <chrono>
#include <sstream>
#include <string>


void test_run(int tid, std::atomic<bool>& m_stop, void* vargp) {
    LOG_INFO("In thread: %d", tid);
    while (!m_stop.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void test_run_err(int tid, std::atomic<bool>& m_stop, void* vargp) {
    LOG_INFO("In thread: %d", tid);
    throw "test run error - this is meant to happen";
}

TEST(th_exec_tests, basic_run) {
    // Initialize ThreadExecutor
    eis::utils::ThreadExecutor* executor = new eis::utils::ThreadExecutor(
            4, test_run, NULL);

    // Sleep for 3 seconds
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Delete memory to make sure threads stop
    delete executor;
}

TEST(th_exec_tests, error_run) {
    // Initialize ThreadExecutor
    eis::utils::ThreadExecutor* executor = new eis::utils::ThreadExecutor(
            4, test_run_err, NULL);

    // Sleep for 3 seconds
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Delete memory to make sure threads stop
    delete executor;
}

/**
 * Overridden GTest main method
 */
GTEST_API_ int main(int argc, char** argv) {
    // Parse out gTest command line parameters
    ::testing::InitGoogleTest(&argc, argv);

    // Check if log level provided
    if (argc == 3) {
        if (strcmp(argv[1], "--log-level") == 0) {
            // LOG_INFO_0("Running msgbus tests over TCP");
            char* log_level = argv[2];

            if (strcmp(log_level, "INFO") == 0) {
                set_log_level(LOG_LVL_INFO);
            } else if (strcmp(log_level, "DEBUG") == 0) {
                set_log_level(LOG_LVL_DEBUG);
            } else if (strcmp(log_level, "ERROR") == 0) {
                set_log_level(LOG_LVL_ERROR);
            } else if (strcmp(log_level, "WARN") == 0) {
                set_log_level(LOG_LVL_WARN);
            } else {
                LOG_ERROR("Unknown log level: %s", log_level);
                return -1;
            }
        } else {
            LOG_ERROR("Unknown parameter: %s", argv[1]);
            return -1;
        }
    } else if (argc == 2) {
        LOG_ERROR_0("Incorrect number of arguments");
        return -1;
    }

    // Run the tests
    return RUN_ALL_TESTS();
}
