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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @brief dynlibload GTests unit tests
 */

#include <gtest/gtest.h>
#include <errno.h>
#include <limits.h>
#include <eii/utils/string.h>
#include "eii/dynlibload/dynlibload.h"

// Macros
#define ASSERT_NOT_NULL(val) { \
    if(val == NULL) FAIL() << dynlib_strerror(errno); \
}
#define ASSERT_NULL(val) { \
    if(val != NULL) FAIL() << "Value should be NULL"; \
}

// Defines
#define LIBRARY_NAME     "libtest.so"
#define FUNC_NAME        "test_function"
#define BAD_LIBRARY_NAME "libdoesnotexist.so"
#define BAD_FUNC_NAME    "func_does_not_exist"
#define LD_PATH_SET      "LD_LIBRARY_PATH="
#define LD_SEP           ":"

/**
 * Unit test class
 */
class DynlibTest : public ::testing::Test {
    private:
        char* m_ld_lib_path;

    protected:
        void SetUp() override {
            // Get LD_LIBRARY_PATH
            char* ld_library_path = getenv("LD_LIBRARY_PATH");
            size_t len = (ld_library_path != NULL) ? strlen(ld_library_path) : 0;

            // Get current working directory
           char cwd[PATH_MAX];
           if(getcwd(cwd, sizeof(cwd)) == NULL) {
               FAIL() << "Failed to get CWD";
           }

           size_t dest_len = strlen(LD_PATH_SET) + strlen(cwd) + len + 2;
           char* env_str = NULL;

           if(ld_library_path == NULL) {
               // Setting the environmental variable from scratch
               env_str = concat_s(dest_len, 3, LD_PATH_SET, LD_SEP, cwd);
           } else {
               // Setting the environmental variable with existing path
               env_str = concat_s(
                       dest_len, 4, LD_PATH_SET, ld_library_path, LD_SEP, cwd);
           }

           if(env_str == NULL) {
               FAIL() << "Failed to generate new LD_LIBRARY_PATH string";
           }

           // Put the new LD_LIBRARY_PATH into the environment
           putenv(env_str);

           // Set member variable to the env_str to be freed during the
           // TearDown() method
           m_ld_lib_path = env_str;
    }

    void TearDown() override {
        free(m_ld_lib_path);
    }
};

TEST_F(DynlibTest, simple_load) {
    dynlib_ctx_t* lib = NULL;
    void (*func)() = NULL;

    // Load the library
    int rc = dynlib_new(LIBRARY_NAME, &lib);
    ASSERT_EQ(rc, 0) << dynlib_strerror(rc);

    // Load the function symbol
    *(void**)(&func) = dynlib_load_sym(lib, FUNC_NAME);
    ASSERT_NOT_NULL(func);

    // Clean up
    dynlib_destroy(lib);
}

TEST_F(DynlibTest, no_library) {
    dynlib_ctx_t* lib = NULL;

    // Load the library
    int rc = dynlib_new(BAD_LIBRARY_NAME, &lib);
    ASSERT_EQ(rc, DYNLOAD_ERR_NO_LIB) << dynlib_strerror(rc);
}

TEST_F(DynlibTest, no_func) {
    dynlib_ctx_t* lib = NULL;
    void (*func)() = NULL;

    // Load the library
    int rc = dynlib_new(LIBRARY_NAME, &lib);
    ASSERT_EQ(rc, 0) << dynlib_strerror(rc);

    // Load the function symbol
    *(void**)(&func) = dynlib_load_sym(lib, BAD_FUNC_NAME);
    ASSERT_NULL(func);

    // Clean up
    dynlib_destroy(lib);
}
