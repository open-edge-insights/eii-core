/*
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

package msgbus

// #cgo CFLAGS: -g -Wall
/*
#include <stdlib.h>
#include <eis/msgbus/msgbus.h>
#include <eis/msgbus/logger.h>

// The code below is for helping deal with the eccentricities of cgo

typedef struct {
	int num_parts;
	msg_envelope_serialized_part_t* parts;
} parts_wrapper_t;

parts_wrapper_t* get_parts(msg_envelope_t* msg) {
	msg_envelope_serialized_part_t* parts = NULL;
	int num_parts = msgbus_msg_envelope_serialize(msg, &parts);
	if(num_parts <= 0) {
		return NULL;
	} else {
		parts_wrapper_t* wrap = (parts_wrapper_t*) malloc(sizeof(parts_wrapper_t));
		wrap->num_parts = num_parts;
		wrap->parts = parts;
		return wrap;
	}
}

void free_parts_wrapper(parts_wrapper_t* wrap) {
	msgbus_msg_envelope_serialize_destroy(wrap->parts, wrap->num_parts);
	free(wrap);
}

char* get_part_bytes(parts_wrapper_t* wrap, int idx) {
	return (char*) wrap->parts[idx].bytes;
}

int get_part_len(parts_wrapper_t* wrap, int idx) {
	return (int) wrap->parts[idx].len;
}
*/
import "C"
import (
	types "EISMessageBus/pkg/types"
	"encoding/json"
	"errors"
	"reflect"
	"unsafe"
)

// Convert Go message envelope representation to C message envelope representation
func GoToMsgEnvelope(env *types.MsgEnvelope) (unsafe.Pointer, error) {
	if env.Data != nil {
		msg := C.msgbus_msg_envelope_new(C.CT_JSON)
		if msg == nil {
			return nil, errors.New("Failed to initialize message envelope")
		}

		var ret C.msgbus_ret_t
		for key, value := range env.Data {
			switch v := reflect.ValueOf(value); v.Kind() {
			case reflect.String:
				body := C.msgbus_msg_envelope_new_string(C.CString(value.(string)))
				ret = C.msgbus_msg_envelope_put(msg, C.CString(key), body)
				if ret != C.MSG_SUCCESS {
					C.msgbus_msg_envelope_elem_destroy(body)
					C.msgbus_msg_envelope_destroy(msg)
					return nil, errors.New("Error adding msg envelope element")
				}
			case reflect.Int, reflect.Int8, reflect.Int16, reflect.Int32, reflect.Int64:
				body := C.msgbus_msg_envelope_new_integer(C.long(value.(int)))
				ret = C.msgbus_msg_envelope_put(msg, C.CString(key), body)
				if ret != C.MSG_SUCCESS {
					C.msgbus_msg_envelope_elem_destroy(body)
					C.msgbus_msg_envelope_destroy(msg)
					return nil, errors.New("Error adding msg envelope element")
				}
			case reflect.Float32, reflect.Float64:
				body := C.msgbus_msg_envelope_new_floating(C.double(value.(float64)))
				ret = C.msgbus_msg_envelope_put(msg, C.CString(key), body)
				if ret != C.MSG_SUCCESS {
					C.msgbus_msg_envelope_elem_destroy(body)
					C.msgbus_msg_envelope_destroy(msg)
					return nil, errors.New("Error adding msg envelope element")
				}
			case reflect.Bool:
				body := C.msgbus_msg_envelope_new_bool(C.bool(value.(bool)))
				ret = C.msgbus_msg_envelope_put(msg, C.CString(key), body)
				if ret != C.MSG_SUCCESS {
					C.msgbus_msg_envelope_elem_destroy(body)
					C.msgbus_msg_envelope_destroy(msg)
					return nil, errors.New("Error adding msg envelope element")
				}
			default:
				C.msgbus_msg_envelope_destroy(msg)
				return nil, errors.New("Unknown type in data map")
			}
		}

		if env.Blob != nil {
			err := addBytesToMsgEnvelope(env.Blob, unsafe.Pointer(msg))
			if err != nil {
				C.msgbus_msg_envelope_destroy(msg)
				return nil, errors.New("Failed to add blob to JSON message")
			}
		}
		return unsafe.Pointer(msg), nil
	} else {
		msg := C.msgbus_msg_envelope_new(C.CT_BLOB)
		if msg == nil {
			return nil, errors.New("Failed to initialize message envelope")
		}

		err := addBytesToMsgEnvelope(env.Blob, unsafe.Pointer(msg))
		if err != nil {
			C.msgbus_msg_envelope_destroy(msg)
			return nil, err
		}

		return unsafe.Pointer(msg), nil
	}
}

func MsgEnvelopeDestroy(msg unsafe.Pointer) {
	if msg != nil {
		C.msgbus_msg_envelope_destroy((*C.msg_envelope_t)(msg))
	}
}

func addBytesToMsgEnvelope(data []byte, msg unsafe.Pointer) error {
	env := (*C.msg_envelope_t)(msg)

	elem := C.msgbus_msg_envelope_new_blob(C.CString(string(data)), C.size_t(len(data)))
	if elem == nil {
		return errors.New("Failed to initialize blob")
	}

	ret := C.msgbus_msg_envelope_put(env, nil, elem)
	if ret != C.MSG_SUCCESS {
		C.msgbus_msg_envelope_elem_destroy(elem)
		return errors.New("Failed to add blob element")
	}

	return nil
}

func MsgEnvelopeToGo(msg unsafe.Pointer) (*types.MsgEnvelope, error) {
	res := new(types.MsgEnvelope)
	env := (*C.msg_envelope_t)(msg)
	parts := C.get_parts(env)
	if parts == nil {
		return nil, errors.New("Failed to serialize message envelope parts")
	}

	defer C.free_parts_wrapper(parts)

	if env.content_type == C.CT_BLOB {
		res.Blob = C.GoBytes(unsafe.Pointer(C.get_part_bytes(parts, 0)), C.get_part_len(parts, 0))
		res.Data = nil
	} else {
		n := C.get_part_len(parts, 0)
		jsonBytes := C.GoBytes(unsafe.Pointer(C.get_part_bytes(parts, 0)), n)

		if env.blob != nil {
			res.Blob = C.GoBytes(unsafe.Pointer(C.get_part_bytes(parts, 1)), C.get_part_len(parts, 1))
		} else {
			res.Blob = nil
		}

		// Need to do (n - 1) to remove the terminating 0 in C-string
		err := json.Unmarshal(jsonBytes, &res.Data)
		if err != nil {
			return nil, err
		}
	}

	return res, nil
}
