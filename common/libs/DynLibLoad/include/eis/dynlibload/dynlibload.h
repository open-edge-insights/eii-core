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
 * @file
 * @brief Library for dynamically loading shared libraries.
 */

#ifndef _EIS_DYNLIBLOAD_H
#define _EIS_DYNLIBLOAD_H

#ifdef __cplusplus
extern "C" {
#endif

// Return / error values
#define DYNLOAD_SUCCESS             0
#define DYNLOAD_ERR_NO_LD_LIB       1
#define DYNLOAD_ERR_NO_LIB          2
#define DYNLOAD_ERR_LIB_LOAD_FAILED 3
#define DYNLOAD_ERR_OMEM            4
#define DYNLOAD_ERR_NO_SYM          5

/**
 * Dynamically loaded library context.
 */
typedef struct {
    void* handle;
} dynlib_ctx_t;

/**
 * Attempt to load the given library.
 *
 * This function will seach the system's LD_LIBRARY_PATH directories for the
 * given library.
 *
 * Zero will be returned if the library is loaded successfully.
 *
 * \note This function searches / parses the LD_LIBRARY_PATH every call. This
 *      method should be called as minimally as possible.
 *
 * @param[in]  library_name The library to load
 * @param[out] library      Pointer to the loaded library
 * @return int
 */
int dynlib_new(const char* library_name, dynlib_ctx_t** library);

/**
 * Destroy a dynamically loaded library.
 *
 * @param library Dynamically loaded library to destroy
 */
void dynlib_destroy(dynlib_ctx_t* library);

/**
 * Load the symbol for a function from the given dynamic library.
 *
 * \note errno is set if an error occurs.
 *
 * \note Function symbols do not need to be freed, since they will be freed
 *      when the @c dynload_ctx_t is destroyed.
 *
 * @param library Dynamically loaded library to get the function symbol from
 * @param fn_name Name of the symbol to load
 * @return void*
 */
void* dynlib_load_sym(dynlib_ctx_t* library, const char* fn_name);

/**
 * Get a string definition of the error which occurred.
 *
 * @param errno Error number
 * @return const char*
 */
const char* dynlib_strerror(int err);

#ifdef __cplusplus
} // __cplusplus
#endif

#endif // _EIS_DYNLIBLOAD_H
