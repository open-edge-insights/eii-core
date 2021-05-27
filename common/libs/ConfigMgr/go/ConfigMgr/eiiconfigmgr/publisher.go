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

import "unsafe"

// PublisherCfg context
type PublisherCfg struct {
	pubCfg unsafe.Pointer
}

// GetEndPoints for application to fetch Endpoint associated with message bus config
//
// Returns:
// 1. string
//    Endpoints value in string
// 2. error
//    Error on failure,  nil on success
func (pubctx *PublisherCfg) GetEndPoints() (string, error) {
	endPoint, err := pubctx.getEndPoints()
	if err != nil {
		return "", err
	}
	return endPoint, nil
}

// GetTopics gets topics from publisher interface config on which data will be published
//
// Returns:
// 1. topics : string array
//    array of topics
// 2. error
//    Error on failure,  nil on success
func (pubctx *PublisherCfg) GetTopics() ([]string, error) {
	topics, err := pubctx.getTopics()
	if err != nil {
		return []string{""}, err
	}
	return topics, nil
}

// GetAllowedClients gets the names of the clients allowed to get publishers data
//
// Returns:
// 1. allowed_clients : string array
//    array of allowed clients
// 2. error
//    Error on failure,  nil on success
func (pubctx *PublisherCfg) GetAllowedClients() ([]string, error) {
	allowedClients, err := pubctx.getAllowedClients()
	if err != nil {
		return []string{""}, err
	}
	return allowedClients, nil
}

// GetMsgbusConfig to fetch client msgbus config for application to communicate over EII message bus
//
// Returns:
// 1. map[string]interface{}
// 2. error
//    Error on failure,  nil on success
func (pubctx *PublisherCfg) GetMsgbusConfig() (map[string]interface{}, error) {
	conf, err := pubctx.getMsgbusConfig()
	if err != nil {
		return nil, err
	}
	config, err := string_to_map_interface(conf)
	if err != nil {
		return nil, err
	}

	return config, nil
}

// SetTopics sets new topic for publisher in publishers interface config
// Parameters:
// 1. topics : string array
//    array of topics that needs to be set
//
// Returns:
// 1. bool value : bool
//    true if success, false on failure
func (pubctx *PublisherCfg) SetTopics(topics []string) bool {
	return pubctx.setTopics(topics)
}

// GetInterfaceValue fetch interface value for application to communicate over EII message bus
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
func (pubctx *PublisherCfg) GetInterfaceValue(key string) (*ConfigValue, error) {
	interfaceVal, err := pubctx.getInterfaceValue(key)
	if err != nil {
		return nil, err
	}
	return interfaceVal, nil
}

// To delete Publisher context
func (pubctx *PublisherCfg) Destroy() {
	pubctx.destroyPublisher()
}
