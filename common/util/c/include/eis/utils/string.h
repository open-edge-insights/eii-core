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
 * @brief String utility functions
 */

#ifndef _EIS_UTILS_STRING_H
#define _EIS_UTILS_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

/**
 * Secure helper function for concatinating a list of c-strings.
 *
 * @param dst_len  - Final length of the concatinated string
 * @param num_strs - Number of input strings to concatinate
 * @return Concatinated string
 */
char* concat_s(size_t dst_len, int num_strs, ...);

/**
 * concat_s function to concat multiple strings
 * @param string - input string
 * @return NULL for any errors occured or char* on success
 */
char* to_upper(char* string);

/**
 * concat_s function to concat multiple strings
 * @param string - input string
 * @return NULL for any errors occured or char* on success
 */
char* to_lower(char* string);

/**
 * trim function to trim a given string
 * @param str_value - string to be trimmed
 */
void trim(char* str_value);

/**
 * free_mem function is to de-allocate the memory of char**
 *
  @param arr - char** variable that needs memory deallocation
 */
void free_mem(char** arr);

/**
 * get_host_port function to get host & port from end_point
 * @param end_point - endpoint
 *  @return NULL for any errors occured or char** on success
 */
char** get_host_port(const char* end_point);

#ifdef __cplusplus
} // __cplusplus
#endif

#endif // _EIS_UTILS_STRING_H
