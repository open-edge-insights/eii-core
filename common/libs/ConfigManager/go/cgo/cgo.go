/*
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package main

/*
typedef void (*callback_fcn)(char* key, char* value);
callback_fcn userCallback;
static inline void cgoCallBack(char *key, char *value){
	userCallback(key, value);
}
*/
import "C"

import (
	configmgr "IEdgeInsights/common/libs/ConfigManager"

	"github.com/golang/glog"
)

var configMgr configmgr.ConfigManager

func callback(CKey string, CValue string) {
	key := C.CString(CKey)
	value := C.CString(CValue)
	C.cgoCallBack(key, value)
}

//export initialize
func initialize(CStorageType *C.char, CCertFile *C.char, CKeyFile *C.char, CTrustFile *C.char) {

	storageType := C.GoString(CStorageType)
	config := map[string]string{
		"certFile":  C.GoString(CCertFile),
		"keyFile":   C.GoString(CKeyFile),
		"trustFile": C.GoString(CTrustFile),
	}
	configMgr = configmgr.Init(storageType, config)
}

//export getConfig
func getConfig(keyy *C.char) *C.char {
	key := C.GoString(keyy)
	value, err := configMgr.GetConfig(key)
	if err != nil {
		glog.Fatal(err)
	}
	glog.V(1).Infof("GetConfig is called and the value of the key %s is: %s", key, value)
	return C.CString(value)
}

//export registerWatchKey
func registerWatchKey(CKey *C.char, userCallback C.callback_fcn) {
	C.userCallback = userCallback
	key := C.GoString(CKey)
	glog.V(1).Infof("Watching on the key %s", key)
	configMgr.RegisterKeyWatch(key, callback)
}

//export registerWatchDir
func registerWatchDir(keyy *C.char, userCallback C.callback_fcn) {
	C.userCallback = userCallback
	key := C.GoString(keyy)
	glog.V(1).Infof("Watching on the dir %s", key)
	configMgr.RegisterDirWatch(key, callback)
}

func main() {

}
