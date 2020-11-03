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
 * @brief Thread Executor Utility implementation
 */

#include "eis/utils/thread_executor.hpp"
#include "eis/utils/logger.h"

namespace eis {
namespace utils {

ThreadExecutor::ThreadExecutor(
        int num_threads, ExecutorRunMethod fn, void* varg) :
    m_stop(false) {
    // Launch thread executor threads
    for (int i = 0; i < num_threads; i++) {
        LOG_DEBUG("Starting thread %d", i);
        std::thread* th = new std::thread(
                &ThreadExecutor::run_wrapper, this, i, fn, varg);
        m_threads.push_back(th);
    }
}

ThreadExecutor::~ThreadExecutor() {
    LOG_DEBUG_0("ThreadExecutor destructor");
    this->stop();
}

void ThreadExecutor::stop() {
    // Do not allow for duplicate stops
    if (m_stop.load()) { return; }

    LOG_DEBUG_0("Stopping thread executor threads");

    // Flag all threads to stop
    m_stop.store(true);

    // Wait for threads to join, and clean up memory
    size_t num_threads = m_threads.size();
    for (int i = 0; i < num_threads; i++) {
        LOG_DEBUG("Waiting for thread %d to join", i);
        std::thread* th = m_threads.at(i);
        th->join();
        LOG_DEBUG("Thread %d joined, deleting thread", i);
        delete th;
    }
}

void ThreadExecutor::run_wrapper(
        int tid, ExecutorRunMethod fn, void* varg) {
    LOG_DEBUG("Thread %d started", tid);

    try {
        // Call the user provided function pointer
        fn(tid, m_stop, varg);
    } catch (const char* ex) {
        LOG_ERROR("[tid: %d] Thread Error: %s", tid, ex);
    }

    LOG_DEBUG("Thread %d stopped", tid);
}

}  // namespace utils
}  // namespace eis
