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

// ServerCfg context
type ServerCfg struct {
	serverCfg unsafe.Pointer
	appCfg unsafe.Pointer
	msgBusCfg unsafe.Pointer
}

// GetEndPoints for application to fetch Endpoint associated with message bus config
//
// Returns:
// 1. string
//    Endpoints value in string
// 2. error
//    Error on failure,  nil on success
func (serverctx *ServerCfg) GetEndPoints() (string, error) {
	endPoint, err:=  serverctx.getEndPoints()
	if err != nil {
		return "", err
	}
	return endPoint, nil
}

// GetAllowedClients gets the names of the clients allowed to connect to server
//
// Returns:
// 1. allowed_clients : string array
//    array of allowed clients 
// 2. error
//    Error on failure,  nil on success
func (serverctx *ServerCfg) GetAllowedClients() ([]string, error) {
	allowedClients, err:=  serverctx.getAllowedClients()
	if err != nil {
		return []string{""}, err
	}
	return allowedClients, nil
}

// GetMsgbusConfig to fetch client msgbus config for application to communicate over EIS message bus
//
// Returns:
// 1. map[string]interface{}
// 2. error
//    Error on failure,  nil on success 
func (serverctx *ServerCfg) GetMsgbusConfig() (map[string]interface{}, error) {
	conf, err:= serverctx.getMsgbusConfig()
	if err != nil {
		return nil, err
	}
	config, err :=  string_to_map_interface(conf)
	if err != nil {
		return nil, err
	}
	return config, nil
}

// GetInterfaceValue fetch interface value for application to communicate over EIS message bus
//
// Parameters:
// 1. key: string
//    Key on which interface value is extracted
//
// Returns:
// 1. Config value : ConfigValue object
//    Interface value
// 2. error
//    Error on failure,  nil on success
func (serverctx *ServerCfg) GetInterfaceValue(key string) (*ConfigValue, error) {
	interfaceVal, err:= serverctx.getInterfaceValue(key)
	if err != nil {
		return nil, err
	}
	return interfaceVal, nil
}

// To delete server context
func (serverctx *ServerCfg) Destroy() {
	serverctx.destroyServer()
}
