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
 * @brief String utility function implementations
 */

#include <string.h>
#include <safe_lib.h>
#include "eii/utils/logger.h"
#include "eii/utils/string.h"

// helper function to fetch host & port from endpoint
char** get_host_port(const char* end_point) {
    char** host_port = NULL;
    char* data = NULL;
    host_port = (char **)calloc(strlen(end_point) + 1, sizeof(char*));
    if (host_port == NULL) {
        LOG_ERROR_0("Calloc failed for host_port");
        return NULL;
    }
    size_t data_len;
    int i = 0;
    char* host = NULL;
    char* port = NULL;
    int ret = 0;
    while ((data = strtok_r(end_point, ":", &end_point))) {
        data_len = strlen(data);
        if (host_port[i] == NULL) {
            host_port[i] = (char*) malloc(data_len + 1);
        }
        if (host_port[i] == NULL) {
            LOG_ERROR_0("Malloc failed for individual host_port");
            goto err;
        }
        ret = strncpy_s(host_port[i], data_len + 1, data, data_len);
        if (ret != 0) {
            LOG_ERROR("String copy failed (errno: %d) : Failed to copy data \" %s \" to host_port", ret, data);
            goto err;
        }
        i++;
    }
    return host_port;
    err:
        if (host_port != NULL) {
            free_mem(host_port);
        }
    return NULL;
}


// Helper function to trim strings
void trim(char* str_value) {
    int index;
    int i;

    // Trimming leading white spaces
    index = 0;
    while (str_value[index] == ' ' || str_value[index] == '\t' || str_value[index] == '\n') {
        index++;
    }

    i = 0;
    while (str_value[i + index] != '\0') {
        str_value[i] = str_value[i + index];
        i++;
    }
    str_value[i] = '\0'; // Terminate string with NULL

    // Trim trailing white spaces
    i = 0;
    index = -1;
    while (str_value[i] != '\0') {
        if (str_value[i] != ' ' && str_value[i] != '\t' && str_value[i] != '\n') {
            index = i;
        }
        i++;
    }
    // Mark the next character to last non white space character as NULL
    str_value[index + 1] = '\0';
}

// helper function to convert char* to uppercase
char* to_upper(char* string) {
    while (*string) {
        *string = toupper((unsigned char) *string);
        string++;
    }
    return string;
}

// helper function to convert char* to lowercase
char* to_lower(char* string) {
    while (*string) {
        *string = tolower((unsigned char) *string);
        string++;
    }
    return string;
}

// helper function to free char**
void free_mem(char** arr) {
    int k = 0;
    while (arr[k] != NULL) {
        free(arr[k]);
        k++;
    }
    free(arr);
}

// Helper function to concatenate strings
char* concat_s(size_t dst_len, int num_strs, ...) {
    char* dst = (char*) malloc(sizeof(char) * dst_len);
    if(dst == NULL) {
        LOG_ERROR_0("Failed to initialize dest for string concat");
        return NULL;
    }

    va_list ap;
    size_t curr_len = 0;
    int ret = 0;

    va_start(ap, num_strs);

    // First element must be copied into dest
    char* src = va_arg(ap, char*);
    size_t src_len = strlen(src);
    ret = strncpy_s(dst, dst_len, src, src_len);
    if(ret != 0) {
        LOG_ERROR("Concatincation failed (errno: %d)", ret);
        free(dst);
        va_end(ap);
        return NULL;
    }
    curr_len += src_len;

    for(int i = 1; i < num_strs; i++) {
        src = va_arg(ap, char*);
        src_len = strlen(src);
        LOG_DEBUG("%s", src);
        ret = strncat_s(dst + curr_len, dst_len, src, src_len);
        if(ret != 0) {
            LOG_ERROR("Concatincation failed (errno: %d)", ret);
            free(dst);
            dst = NULL;
            break;
        }
        curr_len += src_len;
        dst[curr_len] = '\0';
    }
    va_end(ap);

    if(dst == NULL)
        return NULL;
    else
        return dst;
}