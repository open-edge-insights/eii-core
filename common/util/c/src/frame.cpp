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
 * @brief Implementation of @c Frame class
 * @author Kevin Midkiff (kevin.midkiff@intel.com)
 */

#include "eis/utils/frame.h"
#include "eis/utils/logger.h"

using namespace eis::utils;

Frame::Frame(void* frame, int width, int height, int channels, void* data,
             void (*free_frame)(void*)) :
    Serializable(), Deserializable(NULL), m_frame(frame),
    m_free_frame(free_frame), m_free_data(NULL), m_data(data), m_width(width),
    m_height(height), m_channels(channels), m_serialized(false)
{
    if(m_free_frame == NULL) {
        throw "The free_frame() method cannot be NULL";
    }

    msgbus_ret_t ret = MSG_SUCCESS;
    m_meta_data = msgbus_msg_envelope_new(CT_JSON);
    if(m_meta_data == NULL) {
        throw "Failed to initialize meta data envelope";
    }

    // Add frame details to meta data
    msg_envelope_elem_body_t* e_width = msgbus_msg_envelope_new_integer(
            width);
    if(e_width == NULL) {
        throw "Failed to initialize width meta-data";
    }
    ret = msgbus_msg_envelope_put(m_meta_data, "width", e_width);
    if(ret != MSG_SUCCESS) {
        throw "Failed to put width meta-data";
    }

    // Add height
    msg_envelope_elem_body_t* e_height = msgbus_msg_envelope_new_integer(
            height);
    if(e_height == NULL) {
        throw "Failed to initialize height meta-data";
    }
    ret = msgbus_msg_envelope_put(m_meta_data, "height", e_height);
    if(ret != MSG_SUCCESS) {
        msgbus_msg_envelope_elem_destroy(e_width);
        throw "Failed to put height meta-data";
    }

    // Add channels
    msg_envelope_elem_body_t* e_channels = msgbus_msg_envelope_new_integer(
            channels);
    if(e_channels == NULL) {
        throw "Failed to initialize channels meta-data";
    }
    ret = msgbus_msg_envelope_put(m_meta_data, "channels", e_channels);
    if(ret != MSG_SUCCESS) {
        msgbus_msg_envelope_elem_destroy(e_height);
        msgbus_msg_envelope_elem_destroy(e_width);
        throw "Failed to put channels meta-data";
    }
}

Frame::Frame(msg_envelope_t* msg) :
    Serializable(), Deserializable(msg), m_frame(NULL), m_free_frame(NULL),
    m_free_data(NULL), m_meta_data(msg), m_serialized(false)
{
    msgbus_ret_t ret = MSG_SUCCESS;

    // Get frame data
    msg_envelope_elem_body_t* blob = NULL;
    ret = msgbus_msg_envelope_get(msg, NULL, &blob);
    if(ret != MSG_SUCCESS) {
        throw "Failed to retrieve frame blob from  msg envelope";
    } else if(blob->type != MSG_ENV_DT_BLOB) {
        throw "Frame data was not blob data type";
    }
    m_data = (void*) blob->body.blob->data;

    // Get frame width
    msg_envelope_elem_body_t* width = NULL;
    ret = msgbus_msg_envelope_get(msg, "width", &width);
    if(ret != MSG_SUCCESS) {
        throw "Failed to retrieve width";
    } else if(width->type != MSG_ENV_DT_INT) {
        throw "Frame width must be an integer";
    }
    m_width = width->body.integer;

    // Get frame height
    msg_envelope_elem_body_t* height = NULL;
    ret = msgbus_msg_envelope_get(msg, "height", &height);
    if(ret != MSG_SUCCESS) {
        throw "Failed to retrieve height";
    } else if(height->type != MSG_ENV_DT_INT) {
        throw "Frame height must be an integer";
    }
    m_height = height->body.integer;

    // Get frame channels
    msg_envelope_elem_body_t* channels = NULL;
    ret = msgbus_msg_envelope_get(msg, "channels", &channels);
    if(ret != MSG_SUCCESS) {
        throw "Failed to retrieve channels";
    } else if(channels->type != MSG_ENV_DT_INT) {
        throw "Frame channels must be an integer";
    }
    m_channels = channels->body.integer;
}

Frame::~Frame() {
    // Free the underlying frame if the m_free_frame method is set
    if(m_free_frame != NULL) {
        this->m_free_frame(this->m_frame);
    }

    // Free the underlying pixel data if the m_free_data method is set
    if(m_free_data != NULL) {
        this->m_free_data(this->m_data);
    }

    // Destroy the meta data message envelope if the frame is not from a
    // deserialized message and if the frame itself has not been serialized
    if(m_msg == NULL && !m_serialized.load()) {
        msgbus_msg_envelope_destroy(m_meta_data);
    }

    // else: If m_msg is not NULL, then the Frame was deserialized from a
    // msg_envelope_t received over the msgbus. If this is the case, then
    // the Deserialized() destructure will delete the m_msg envelope struct
    // cleaning up all data used by this object.
}

int Frame::get_width() {
    return m_width;
}

int Frame::get_height() {
    return m_height;
}

int Frame::get_channels() {
    return m_channels;
}

void* Frame::get_data() {
    if(m_serialized.load()) {
        LOG_ERROR_0(
                "Writable data method called after frame serialization");
        return NULL;
    }
    return m_data;
}

msg_envelope_t* Frame::get_meta_data() {
    if(m_serialized.load()) {
        LOG_ERROR_0("Cannot get meta-data after frame serialization");
        return NULL;
    }
    return m_meta_data;
}

msg_envelope_t* Frame::serialize() {
    if(m_serialized.load()) {
        LOG_ERROR_0("Frame has already been serialized");
        return NULL;
    }

    if(m_msg != NULL) {
        // Message was deserialized
        // Set the frame as serialized
        m_serialized.store(true);

        // Get the data blob for the frame data from the envelope to set the
        // free function on the shared memory
        msg_envelope_elem_body_t* blob = NULL;
        msgbus_ret_t ret = msgbus_msg_envelope_get(m_msg, NULL, &blob);
        if(ret != MSG_SUCCESS) {
            throw "Failed to retrieve frame blob from  msg envelope";
        } else if(blob->type != MSG_ENV_DT_BLOB) {
            throw "Frame data was not blob data type";
        }

        // Set the m_free_data member, because the next lines after m_free_data
        // is set swap around the blob pointer and free functions so that the
        // frame object is correctly destroyed along with the actual blob bytes
        this->m_free_data = blob->body.blob->shared->free;

        // Resetting blob shared memory free pointers
        blob->body.blob->shared->ptr = this;
        blob->body.blob->shared->free = Frame::msg_free_frame;

        // Set m_msg to NULL so that the Deserializable destructor does not
        // call msgbus_msg_envelope_destroy() on the m_msg object. The reason
        // we do not want that called is because the message bus itself will
        // call the destroy method on the message envelope when the envelope
        // has been transmitted over the message bus.
        m_msg = NULL;

        return m_meta_data;
    } else {
        msg_envelope_elem_body_t* frame = NULL;
        size_t len = (size_t) (m_width * m_height * m_channels);
        frame = msgbus_msg_envelope_new_blob((char*) m_data, len);
        if(frame == NULL) {
            LOG_ERROR_0("Failed to initialize blob for frame data");
            return NULL;
        }

        // Set the blob free methods
        frame->body.blob->shared->ptr = (void*) this;
        frame->body.blob->shared->free = Frame::msg_free_frame;

        msgbus_ret_t ret = msgbus_msg_envelope_put(
                m_meta_data, NULL, frame);
        if(ret != MSG_SUCCESS) {
            LOG_ERROR_0("Failed to put frame data into envelope");

            // Set owned flag to false, so that the frame's data is not
            // freed yet...
            frame->body.blob->shared->owned = false;
            msgbus_msg_envelope_elem_destroy(frame);

            return NULL;
        }

        // Set the frame as serialized
        m_serialized.store(true);

        return m_meta_data;
    }
}
