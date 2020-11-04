/*
Copyright (c) 2020 Intel Corporation.

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

package eisconfigmgr

import "unsafe"

// ClientCfg context
type ClientCfg struct {
	clientCfg unsafe.Pointer
	appCfg unsafe.Pointer
	msgBusCfg unsafe.Pointer
} 

func (clientctx *ClientCfg) GetEndPoints() (string, error) {
	endPoint, err:=  clientctx.getEndPoints()
	if err != nil {
		return "", err
	}
	return endPoint, nil
}

func (clientctx *ClientCfg) GetMsgbusConfig() (map[string]interface{}, error) {
	conf, err:= clientctx.getMsgbusConfig()
	if err != nil {
		return nil, err
	}
	config, err :=  string_to_map_interface(conf)
	if err != nil {
		return nil, err
	}
	return config, nil
}

func (clientctx *ClientCfg) GetInterfaceValue(key string) (*ConfigValue, error) {
	interfaceVal, err:= clientctx.getInterfaceValue(key)
	if err != nil {
		return nil, err
	}
	return interfaceVal, nil
}

func (clientctx *ClientCfg) Destroy() {
	clientctx.destroyClient()
}
