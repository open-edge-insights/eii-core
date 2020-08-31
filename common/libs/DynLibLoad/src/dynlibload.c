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
 * @brief dynlibload implementation
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <safe_lib.h>
#include <eis/utils/string.h>

#include "eis/dynlibload/dynlibload.h"

#define DELIM ":"
#define SEP   "/"

int dynlib_new(const char* library_name, dynlib_ctx_t** library) {
    char* ld_library_path = getenv("LD_LIBRARY_PATH");
    if(ld_library_path == NULL) { return DYNLOAD_ERR_NO_LD_LIB; }

    char* next_token = NULL;
    char* abs_path = NULL;
    size_t library_name_len = strlen(library_name);
    size_t strmax = strlen(ld_library_path);

    // Tokenize the the LD_LIBRARY_PATH by the ":" delimiter
    char* pch = strtok_s(ld_library_path, &strmax, DELIM, &next_token);

    // Loop over tokens and look for the library
    while(pch != NULL) {
        // Adding 2, one for the SEP and the other for NULL termination
        size_t dest_len = library_name_len + strlen(pch) + 2;
        abs_path = concat_s(dest_len, 3, pch, SEP, library_name);
        if(access(abs_path, F_OK) != -1) {
            // Found the library! Break out of the while loop
            break;
        } else {
            free(abs_path);
            abs_path = NULL;
        }
        pch = strtok_s(NULL, &strmax, DELIM, &next_token);
    }

    // Check if the library was found
    if(abs_path == NULL) {
        return DYNLOAD_ERR_NO_LIB;
    }

    // Attempt to open the library
    void* handle = dlopen(abs_path, RTLD_LAZY);
    free(abs_path);
    if(handle == NULL) { return DYNLOAD_ERR_LIB_LOAD_FAILED; }

    dynlib_ctx_t* lib = (dynlib_ctx_t*) malloc(sizeof(dynlib_ctx_t));
    if(lib == NULL) {
        dlclose(handle);
        return DYNLOAD_ERR_OMEM;
    }

    // Assign struct values
    lib->handle = handle;

    // Assign output parameter
    *library = lib;

    return DYNLOAD_SUCCESS;
}

void dynlib_destroy(dynlib_ctx_t* library) {
    if(library != NULL) {
        dlclose(library->handle);
        free(library);
    }
}

void* dynlib_load_sym(dynlib_ctx_t* library, const char* fn_name) {
    void* func = dlsym(library->handle, fn_name);
    if(func == NULL) {
        errno = DYNLOAD_ERR_NO_SYM;
        return NULL;
    }
    return func;
}

const char* dynlib_strerror(int err) {
    switch(err) {
        case DYNLOAD_SUCCESS:
            return "Operation successful";
        case DYNLOAD_ERR_NO_LD_LIB:
            return "LD_LIBRARY_PATH env variable is not set";
        case DYNLOAD_ERR_NO_LIB:
            return "Library not found in the LD_LIBRARY_PATH";
        case DYNLOAD_ERR_LIB_LOAD_FAILED:
            return "dlopen() failed to open the library";
        case DYNLOAD_ERR_OMEM:
            return "Out of memory";
        case DYNLOAD_ERR_NO_SYM:
            return "Function symbol not found in the library";
        default:
            return "Unknown errno";
    }
}
