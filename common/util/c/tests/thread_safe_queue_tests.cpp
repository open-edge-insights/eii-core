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

/**
 * @brief EIS thread safe queue unit tests
 */

#include <stdlib.h>
#include <cstring>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>
#include "eis/utils/thread_safe_queue.h"

using namespace eis::utils;

QueueRetCode push_char_arr(
        ThreadSafeQueue<char*>* q, const char* value, bool wait=false) {
    QueueRetCode ret = QueueRetCode::SUCCESS;
    size_t len = strlen(value);
    char* val = (char*) malloc(sizeof(char) * (len + 1));
    memcpy(val, value, len);
    val[len] = '\0';
    if (wait) {
        ret = q->push_wait(val);
    } else {
        ret = q->push(val);
    }
    if (ret != QueueRetCode::SUCCESS) {
        delete val;
    }
    return ret;
}

TEST(tsp_tests, push_pop) {
    const char* val = "hello, world!";

    ThreadSafeQueue<char*> queue(-1);
    ASSERT_EQ(queue.empty(), true);
    ASSERT_EQ(queue.size(), 0);

    QueueRetCode ret = push_char_arr(&queue, val);;
    ASSERT_EQ(ret, QueueRetCode::SUCCESS);
    ASSERT_EQ(queue.empty(), false);
    ASSERT_EQ(queue.size(), 1);

    char* val2 = queue.front();
    EXPECT_TRUE(std::string(val2) == std::string(val));
    delete val2;

    queue.pop();
    ASSERT_EQ(queue.empty(), true);
    ASSERT_EQ(queue.size(), 0);
}

TEST(tsp_tests, wait_no_thread) {
    const char* val = "hello, world!";

    ThreadSafeQueue<char*> queue(-1);
    ASSERT_EQ(queue.empty(), true);
    ASSERT_EQ(queue.size(), 0);

    QueueRetCode ret = push_char_arr(&queue, val);
    ASSERT_EQ(ret, QueueRetCode::SUCCESS);
    ASSERT_EQ(queue.empty(), false);
    ASSERT_EQ(queue.size(), 1);

    // This should return immediately
    auto duration = std::chrono::milliseconds(100);
    ASSERT_EQ(queue.wait_for(duration), true);

    char* val2 = queue.front();
    EXPECT_TRUE(std::string(val2) == std::string(val));
    delete val2;

    queue.pop();
    ASSERT_EQ(queue.empty(), true);
    ASSERT_EQ(queue.size(), 0);
}

void run(ThreadSafeQueue<char*>* queue) {
    const char* val = "hello, world!";
    push_char_arr(queue, val);
}

TEST(tsp_tests, wait_thread) {
    const char* val = "hello, world!";

    ThreadSafeQueue<char*> queue(-1);
    ASSERT_EQ(queue.empty(), true);
    ASSERT_EQ(queue.size(), 0);

    std::thread th(&run, &queue);

    // Wait for the queue to have an item
    queue.wait();

    char* val2 = queue.front();
    EXPECT_TRUE(std::string(val2) == std::string(val));
    delete val2;

    queue.pop();
    ASSERT_EQ(queue.empty(), true);
    ASSERT_EQ(queue.size(), 0);

    th.join();
}

TEST(tsp_tests, wait_timeout) {
    ThreadSafeQueue<char*> queue(-1);
    ASSERT_EQ(queue.empty(), true);
    ASSERT_EQ(queue.size(), 0);

    // This should return immediately
    auto duration = std::chrono::milliseconds(100);
    ASSERT_EQ(queue.wait_for(duration), false);
}

TEST(tsp_tests, max_reached) {
    const char* val = "hello, world!";

    ThreadSafeQueue<char*> queue(1);
    ASSERT_EQ(queue.empty(), true);
    ASSERT_EQ(queue.size(), 0);

    QueueRetCode ret = push_char_arr(&queue, val);
    ASSERT_EQ(ret, QueueRetCode::SUCCESS);
    ASSERT_EQ(queue.empty(), false);
    ASSERT_EQ(queue.size(), 1);

    ret = push_char_arr(&queue, val);
    ASSERT_NE(ret, QueueRetCode::SUCCESS);
    ASSERT_EQ(queue.size(), 1);
}

void wait_pop_run(ThreadSafeQueue<char*>* queue) {
    auto duration = std::chrono::milliseconds(500);
    std::this_thread::sleep_for(duration);
    char* val = queue->front();
    queue->pop();
    delete val;
}

TEST(tsp_tests, queue_full_push_wait) {
    const char* val = "hello, world!";

    ThreadSafeQueue<char*> queue(5);
    QueueRetCode ret = QueueRetCode::SUCCESS;

    for(int i = 0; i < 4; i++) {
        ret = push_char_arr(&queue, val);
        ASSERT_EQ(ret, QueueRetCode::SUCCESS);
    }

    ret = push_char_arr(&queue, val);
    ASSERT_EQ(ret, QueueRetCode::QUEUE_FULL);

    std::thread th(&wait_pop_run, &queue);

    ret = push_char_arr(&queue, val, true);
    ASSERT_EQ(ret, QueueRetCode::SUCCESS);

    th.join();
}
