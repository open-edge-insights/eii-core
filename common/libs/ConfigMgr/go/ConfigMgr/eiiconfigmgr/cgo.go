/*
Copyright (c) 2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

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

package eiiconfigmgr

/*
#include <stdio.h>
#cgo CFLAGS: -g -Wall
#cgo LDFLAGS: -leiimsgbus -leiimsgenv -leiiutils -leiiconfigmanager -lsafestring


#include <stdio.h>
#include <stdlib.h>
#include <safe_lib.h>
#include <eii/utils/logger.h>
#include <eii/config_manager/cfgmgr.h>

// Declaring the Go callback to be called from C layer callback
extern void cfgmgrGoCallback(const char* key, config_t* config, void* user_data);
void cCallback(const char *key, config_t* value, void* cb_user_data) {
	cfgmgrGoCallback(key, value, cb_user_data);
}
*/
import "C"

import (
	"sync"
	"unsafe"

	"github.com/golang/glog"
)

var (
	mutex sync.RWMutex
	store = map[unsafe.Pointer]interface{}{}
)

// Save function to store interface{} for every unsafe.Pointer
func Save(v interface{}) unsafe.Pointer {
	if v == nil {
		glog.Errorf("Provided interface{} is nil")
		return nil
	}

	// This pointer will not store any data but will be used for indexing purposes
	// Since Go doest allow to cast dangling pointer to unsafe.Pointer, we need to allocate one byte
	var ptr unsafe.Pointer = C.malloc(C.size_t(1))
	if ptr == nil {
		glog.Errorf("Can't allocate memory for cgo unsafe.Ppointer")
		return nil
	}

	// We need indexing because Go doesn't allow C code to store pointers to Go data
	mutex.Lock()
	store[ptr] = v
	mutex.Unlock()

	return ptr
}

// Restore to fetch the interface associated with a particular unsafe.Pointer
func Restore(ptr unsafe.Pointer) (v interface{}) {
	if ptr == nil {
		glog.Errorf("Provided unsafe.Pointer is nil")
		return nil
	}

	mutex.RLock()
	v = store[ptr]
	mutex.RUnlock()
	return v
}

// Unref to free the unsafe.Pointer & the associated interface{}
func Unref(ptr unsafe.Pointer) {
	if ptr == nil {
		glog.Errorf("Can't free a nil pointer")
		return
	}

	mutex.Lock()
	delete(store, ptr)
	mutex.Unlock()

	C.free(ptr)
}
