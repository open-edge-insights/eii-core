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
 * @brief Thread Executor Utility
 */

#ifndef EII_UTILS_INCLUDE_THREAD_EXECUTOR_H_
#define EII_UTILS_INCLUDE_THREAD_EXECUTOR_H_

#include <functional>
#include <thread>
#include <vector>
#include <atomic>

namespace eii {
namespace utils {

typedef std::function<void(int, std::atomic<bool>&, void*)> ExecutorRunMethod;

/**
 * Thread executor utility for spinning up multiple threads with the same
 * main run method.
 *
 * The @c ThreadExecutor will take run methods with the prototype defined
 * below:
 *
 *  void run_method(int tid, std::atmoic<bool>& stop, void* varg)
 *
 * The tid argument indicates the Thread's internal ID (i.e. index) in the
 * @c ThreadExecutor.
 *
 * The stop atomic boolean value provides a flag for when the thread should
 * exit. Note that it is up to the thread to incorporate this into its
 * run loop. If it does not, then the @c ThreadExecutor will block indefinitely
 * when it attempts to exit.
 *
 * The varg parameter is a user provided argument to provide the function with
 * its application state. Note that all of the run methods receive the same
 * state variable, so it is important that whatever the state is to be
 * thread-safe.
 */
class ThreadExecutor {
 private:
    // Flag for signaling when the threads should stop
    std::atomic<bool> m_stop;

    // List of threads handles
    std::vector<std::thread*> m_threads;

    /**
     * Wrapper function for running threads. Helps with detecting whether
     * or not errors have occurred in the threads.
     */
    void run_wrapper(
            int tid, ExecutorRunMethod fn, void* varg);

 public:
    /**
     * Constructor
     *
     * @param num_threads - Number of threads to launch
     * @param fn          - Run method
     * @param varg        - Run method state variable (can be NULL)
     */
    ThreadExecutor(int num_threads, ExecutorRunMethod fn, void* varg);

    /**
     * Destructor
     */
    ~ThreadExecutor();

    /**
     * Signal all threads to stop.
     *
     * \note This is also called in the destructor, so this can be accomplished
     *      by freeing the @c ThreadExecutor's memory.
     */
    void stop();
};


}  // namespace utils
}  // namespace eii

#endif  // EII_UTILS_INCLUDE_THREAD_EXECUTOR_H_
