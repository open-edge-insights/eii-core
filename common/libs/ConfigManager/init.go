/*
Copyright (c) 2019 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// Package init is a to initialize configmanager
package init

import (
	configmgr "IEdgeInsights/common/libs/ConfigManager/go"

	"github.com/golang/glog"
	"flag"
)

// ConfigManager interface
type ConfigManager interface {
	configmgr.ConfigMgr
}

//Init function to initialize config manager
func Init(storageType string, config map[string]string) configmgr.ConfigMgr {
	flag.Parse()
	flag.Lookup("logtostderr").Value.Set("true")
	defer glog.Flush()

	glog.Infof("initializing configuration manager...")
	return configmgr.GetConfigClient(storageType, config)
}
