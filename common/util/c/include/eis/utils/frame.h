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
 * @file
 * @brief High level video frame abstraction.
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#ifndef _EIS_UTILS_FRAME_H
#define _EIS_UTILS_FRAME_H

namespace eis {
namespace utils {

/**
 * Wrapper around a frame object
 */
class Frame {
private:
    // Underlying frame object
    void* m_frame;

    // Underlying free method for the frame
    void (*m_free_frame)(void*);

    // Check if the underlying frame is writable.
    bool (*m_is_writable)(void*);

    // Underlying pointer to the frame data
    void* m_data;

public:
    // Public attributes
    const int width;
    const int height;
    const int channels;

    /**
     * Constructor
     *
     * @param frame             - Underlying frame object
     * @param width             - Frame width
     * @param height            - Frame height
     * @param data              - Constant pointer to the underlying frame data
     * @param free_frame        - Function to free the underlying frame
     * @param is_frame_writable - Function to check if a frame is writable
     */
    Frame(void* frame, int width, int height, int channels, void* data,
            void (*free_frame)(void*), bool (*is_frame_writable)(void*)) :
        m_frame(frame), m_free_frame(free_frame),
        m_is_writable(is_frame_writable), m_data(data), width(width),
        height(height), channels(channels)
    {};

    /**
     * Destructor
     */
    ~Frame() {
        this->m_free_frame(this->m_frame);
    };

    /**
     * Check whether or not the underlying frame data is writable.
     *
     * @return bool
     */
    bool is_writable() {
        return this->m_is_writable(this->m_frame);
    };

    /**
     * Get a constant pointer to the underlying frame data.
     *
     * @return void*
     */
    const void* get_data() const {
        return m_data;
    };

    /**
     * Get the underlying frame data. The frame must be writeable in order for
     * this method to return any frame data. NULL will be returned if it is not
     * writeable.
     *
     * @return void*
     */
    void* get_data() {
        if(this->is_writable()) {
            return m_data;
        } else {
            return NULL;
        }
    };

};

} // utils
} // eis

#endif // _EIS_UDF_FRAME_H
