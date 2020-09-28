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

/*
#cgo CFLAGS: -g -Wall
#cgo LDFLAGS: -leismsgbus -leismsgenv -leisutils -lneweisconfigmgr -leiskvstoreplugin


#include <stdio.h>
#include <stdlib.h>
#include <safe_lib.h>
#include <eis/utils/logger.h>
#include <eis/config_manager/cfg_mgr.h>

typedef struct {
	char** arr;
	int len;
}string_arr_t;

typedef struct {
	char* arr;
	int len;
}char_arr_t;


char** parse_config_value(config_value_t* config_value, int len) {
	char** char_config_value = (char**)malloc(len * sizeof(char*));
	config_value_t* config_val;
	for (int i = 0; i < len; i++) {
		config_val = config_value_array_get(config_value, i);
		if (config_val == NULL) {
			free(char_config_value);
			return NULL;
		}

		char_config_value[i] = (char*)malloc(strlen(config_val->body.string) + 1);
		strcpy(char_config_value[i], config_val->body.string);

		// Destroying config_val
		config_value_destroy(config_val);
	}
	return char_config_value;
}

static inline void destroy_string_arr(string_arr_t* str_arr) {
	if(str_arr != NULL) {
		for(int i = 0; i <  str_arr->len; i++){
			free(str_arr->arr[i]);
		}
		free(str_arr->arr);
		free(str_arr);
	}
}

static inline void destroy_char_arr(char_arr_t* char_arr) {
	if(char_arr != NULL) {
		free(char_arr->arr);
		free(char_arr);
	}
}

// ---------------CONFIG MANGER--------------------
static inline void* create_new_cfg_mr(){
	set_log_level(LOG_LVL_DEBUG);
	app_cfg_t* app_cfg = app_cfg_new();
	return app_cfg;
}

static inline char* get_env_var(void* app_cfg) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	return c_app_cfg->env_var;
}

static inline char* get_app_name(void* app_cfg) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	config_value_t* appname = cfgmgr_get_appname_base(c_app_cfg->base_cfg);
    if (appname->type != CVT_STRING) {
        LOG_ERROR_0("appname type is not string");
        return "";
    } else {
        if (appname->body.string == NULL) {
            LOG_ERROR_0("AppName is NULL");
            return "";
        }
    }
    return appname->body.string;
}

static inline int is_dev_mode(void* app_cfg) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	int result = cfgmgr_is_dev_mode_base(c_app_cfg->base_cfg);
	return result;
}

static inline int get_num_publihers(void* app_cfg) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	return cfgmgr_get_num_elements_base("Publishers", c_app_cfg->base_cfg);
}

static inline int get_num_subscribers(void* app_cfg) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	return cfgmgr_get_num_elements_base("Subscribers", c_app_cfg->base_cfg);
}

static inline int get_num_servers(void* app_cfg) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	return cfgmgr_get_num_elements_base("Servers", c_app_cfg->base_cfg);
}

static inline int get_num_clients(void* app_cfg) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	return cfgmgr_get_num_elements_base("Clients", c_app_cfg->base_cfg);
}

// ---------------APP CONFIG-----------------------
static inline char* get_apps_config(void* app_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	config_t* config = c_app_cfg->base_cfg->m_app_config;
	if(config == NULL) {
		return NULL;
	}

	char* c_config = configt_to_char(config);
	config_destroy(config);
	return c_config;
}

static inline char* get_apps_interface(void* app_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	config_t* app_interface = c_app_cfg->base_cfg->m_app_interface;
	char* c_app_interface = configt_to_char(app_interface);
	return c_app_interface;
}

// -----------------PUBLISHER----------------------
static inline void* get_publisher_by_name(void* ctx, char *name){
	pub_cfg_t* pub_cfg = NULL;
	app_cfg_t *app_cfg = (app_cfg_t *)ctx;
	pub_cfg = cfgmgr_get_publisher_by_name(app_cfg, name);
	return pub_cfg;
}

static inline void* get_publisher_by_index(void* ctx, int index){
	pub_cfg_t* pub_cfg = NULL;
	app_cfg_t *app_cfg = (app_cfg_t *)ctx;
	pub_cfg = cfgmgr_get_publisher_by_index(app_cfg, index);
	return pub_cfg;
}

static inline int set_pub_topics(void* app_cfg, void* pub_cfg, char **topics, int len) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	pub_cfg_t* c_pub_cfg = (pub_cfg_t *)pub_cfg;
	int ret = c_pub_cfg->cfgmgr_set_topics_pub(topics, len, c_app_cfg->base_cfg);
	return ret;
}

static inline char_arr_t* get_pub_end_points(void* app_cfg, void* pub_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	pub_cfg_t* c_pub_cfg = (pub_cfg_t *)pub_cfg;

	char_arr_t* char_arr = (char_arr_t*) malloc(sizeof(char_arr_t));
	if(char_arr == NULL)
		return NULL;

	config_value_t* endpoints = c_pub_cfg->cfgmgr_get_endpoint_pub(c_app_cfg->base_cfg);
	if (endpoints == NULL)
        return NULL;

	int len = strlen(endpoints->body.string);
	char_arr->arr = (char*)malloc(len + 1);
	if(char_arr->arr == NULL)
		return NULL;

	strcpy(char_arr->arr, endpoints->body.string);
	char_arr->len = len;

	// Destroying endpoints
	config_value_destroy(endpoints);
	return char_arr;
}

static inline char* get_pub_msgbus_config(void* app_cfg, void* pub_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	pub_cfg_t* c_pub_cfg = (pub_cfg_t *)pub_cfg;
	config_t* config = c_pub_cfg->cfgmgr_get_msgbus_config_pub(c_app_cfg->base_cfg);
	char* c_config = configt_to_char(config);
	config_destroy(config);
	return c_config;
}

static inline string_arr_t* get_pub_topics(void* app_cfg, void* pub_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	pub_cfg_t* c_pub_cfg = (pub_cfg_t *)pub_cfg;
	string_arr_t* str_arr = (string_arr_t*) malloc(sizeof(string_arr_t));
	if(str_arr == NULL)
		return NULL;

	config_value_t* topics = c_pub_cfg->cfgmgr_get_topics_pub(c_app_cfg->base_cfg);
	if (topics == NULL)
        return NULL;

	int topics_len = config_value_array_len(topics);
	char** arr_topics = parse_config_value(topics, topics_len);
	str_arr->arr = arr_topics;
	str_arr->len = topics_len;
	return str_arr;
}

static inline string_arr_t* get_pub_allowed_clients(void* app_cfg, void* pub_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	pub_cfg_t* c_pub_cfg = (pub_cfg_t *)pub_cfg;
	string_arr_t* str_arr = (string_arr_t*) malloc(sizeof(string_arr_t));
	if(str_arr == NULL)
		return NULL;

	config_value_t* allowed_clients = c_pub_cfg->cfgmgr_get_allowed_clients_pub(c_app_cfg->base_cfg);
	if (allowed_clients == NULL)
        return NULL;

	int clients_len = config_value_array_len(allowed_clients);
	char** arr_allowed_clients = parse_config_value(allowed_clients, clients_len);
	str_arr->arr = arr_allowed_clients;
	str_arr->len = 	clients_len;
	return str_arr;
}

//-----------------SUBSCRIBER----------------------

static inline void* get_subscriber_by_name(void* ctx, char *name){
	sub_cfg_t* sub_cfg = NULL;
	app_cfg_t *app_cfg = (app_cfg_t *)ctx;
	sub_cfg = cfgmgr_get_subscriber_by_name(app_cfg, name);
	return sub_cfg;
}

static inline void* get_subscriber_by_index(void* ctx, int index){
	sub_cfg_t* sub_cfg = NULL;
	app_cfg_t *app_cfg = (app_cfg_t *)ctx;
	sub_cfg = cfgmgr_get_subscriber_by_index(app_cfg, index);
	return sub_cfg;
}

static inline char_arr_t* get_sub_end_points(void* app_cfg, void* sub_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	sub_cfg_t* c_sub_cfg = (sub_cfg_t *)sub_cfg;

	char_arr_t* char_arr = (char_arr_t*) malloc(sizeof(char_arr_t));
	if(char_arr == NULL)
		return NULL;

	config_value_t* endpoints = c_sub_cfg->cfgmgr_get_endpoint_sub(c_app_cfg->base_cfg);
	if (endpoints == NULL)
		return NULL;

	int len = strlen(endpoints->body.string);
	char_arr->arr = (char*)malloc(len + 1);
	if(char_arr->arr == NULL)
		return NULL;

	strcpy(char_arr->arr, endpoints->body.string);
	char_arr->len = len;

	// Destroying endpoints
	config_value_destroy(endpoints);
	return char_arr;
}

static inline char* get_msgbus_config_subscriber(void* app_cfg, void* sub_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	sub_cfg_t* c_sub_cfg = (sub_cfg_t *)sub_cfg;
	config_t* config = c_sub_cfg->cfgmgr_get_msgbus_config_sub(c_app_cfg->base_cfg);
	char* c_config = configt_to_char(config);
	config_destroy(config);
	return c_config;
}

static inline string_arr_t* get_topics_subscriber(void* app_cfg, void* sub_cfg) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	sub_cfg_t* c_sub_cfg = (sub_cfg_t *)sub_cfg;
	string_arr_t* str_arr = (string_arr_t*) malloc(sizeof(string_arr_t));
	if(str_arr == NULL)
		return NULL;

	config_value_t* topics = c_sub_cfg->cfgmgr_get_topics_sub(c_app_cfg->base_cfg);
	if (topics == NULL)
        return NULL;

	int topics_len = config_value_array_len(topics);
	char** arr_topics = parse_config_value(topics, topics_len);
	str_arr->arr = arr_topics;
	str_arr->len = topics_len;
	return str_arr;
}

static inline int set_sub_topics(void* app_cfg, void* sub_cfg, char **topics, int len) {
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	sub_cfg_t* c_sub_cfg = (sub_cfg_t *)sub_cfg;
	int ret = c_sub_cfg->cfgmgr_set_topics_sub(topics, len, c_app_cfg->base_cfg);
	return ret;
}

//-----------------SERVER--------------------------

static inline void* get_server_by_name(void* ctx, char *name){
	server_cfg_t* server_cfg = NULL;
	app_cfg_t *app_cfg = (app_cfg_t *)ctx;
	server_cfg = cfgmgr_get_server_by_name(app_cfg, name);
	return server_cfg;
}

static inline void* get_server_by_index(void* ctx, int index){
	server_cfg_t* server_cfg = NULL;
	app_cfg_t *app_cfg = (app_cfg_t *)ctx;
	server_cfg = cfgmgr_get_server_by_index(app_cfg, index);
	return server_cfg;
}

static inline char_arr_t* get_server_end_points(void* app_cfg, void* server_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	server_cfg_t* c_server_cfg = (server_cfg_t *)server_cfg;

	char_arr_t* char_arr = (char_arr_t*) malloc(sizeof(char_arr_t));
	if(char_arr == NULL)
		return NULL;

	config_value_t* endpoints = c_server_cfg->cfgmgr_get_endpoint_server(c_app_cfg->base_cfg);
	if (endpoints == NULL)
		return NULL;

	int len = strlen(endpoints->body.string);
	char_arr->arr = (char*)malloc(len + 1);
	if(char_arr->arr == NULL)
		return NULL;

	strcpy(char_arr->arr, endpoints->body.string);
	char_arr->len = len;

	// Destroying endpoints
	config_value_destroy(endpoints);
	return char_arr;
}

static inline char* get_msgbus_config_server(void* app_cfg, void* server_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	server_cfg_t* c_server_cfg = (server_cfg_t *)server_cfg;
	config_t* config = c_server_cfg->cfgmgr_get_msgbus_config_server(c_app_cfg->base_cfg);
	char* c_config = configt_to_char(config);
	config_destroy(config);
	return c_config;
}

static inline string_arr_t* get_allowed_clients_server(void* app_cfg, void* server_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	server_cfg_t* c_server_cfg = (server_cfg_t *)server_cfg;
	string_arr_t* str_arr = (string_arr_t*) malloc(sizeof(string_arr_t));
	if(str_arr == NULL)
		return NULL;

	config_value_t* allowed_clients = c_server_cfg->cfgmgr_get_allowed_clients_server(c_app_cfg->base_cfg);
	if (allowed_clients == NULL)
        return NULL;

	int clients_len = config_value_array_len(allowed_clients);
	char** arr_allowed_clients = parse_config_value(allowed_clients, clients_len);
	str_arr->arr = arr_allowed_clients;
	str_arr->len = 	clients_len;
	return str_arr;
}

//-----------------CLIENT--------------------------
static inline void* get_client_by_name(void* ctx, char *name){
	client_cfg_t* client_cfg = NULL;
	app_cfg_t *app_cfg = (app_cfg_t *)ctx;
	client_cfg = cfgmgr_get_client_by_name(app_cfg, name);
	return client_cfg;
}

static inline void* get_client_by_index(void* ctx, int index){
	client_cfg_t* client_cfg = NULL;
	app_cfg_t *app_cfg = (app_cfg_t *)ctx;
	client_cfg = cfgmgr_get_client_by_index(app_cfg, index);
	return client_cfg;
}

static inline char_arr_t* get_client_end_points(void* app_cfg, void* client_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	client_cfg_t* c_client_cfg = (client_cfg_t *)client_cfg;

	char_arr_t* char_arr = (char_arr_t*) malloc(sizeof(char_arr_t));
	if(char_arr == NULL)
		return NULL;

	config_value_t* endpoints = c_client_cfg->cfgmgr_get_endpoint_client(c_app_cfg->base_cfg);
	if (endpoints == NULL)
		return NULL;

	int len = strlen(endpoints->body.string);
	char_arr->arr = (char*)malloc(len + 1);
	if(char_arr->arr == NULL)
		return NULL;

	strcpy(char_arr->arr, endpoints->body.string);
	char_arr->len = len;

	// Destroying endpoints
	config_value_destroy(endpoints);
	return char_arr;
}

static inline char* get_msgbus_config_client(void* app_cfg, void* client_cfg){
	app_cfg_t* c_app_cfg = (app_cfg_t *)app_cfg;
	client_cfg_t* c_client_cfg= (client_cfg_t *)client_cfg;
	config_t* config = c_client_cfg->cfgmgr_get_msgbus_config_client(c_app_cfg->base_cfg);
	char* c_config = configt_to_char(config);
	config_destroy(config);
	return c_config;
}

*/
import "C"

import (
	"bytes"
	"encoding/json"
	"fmt"
	"os"
	"unsafe"

	"github.com/golang/glog"
)

type ConfigMgrContext struct {
	cfgmgrCtx unsafe.Pointer
}

// ConfigMgr object
type ConfigMgr struct {
	ctx *ConfigMgrContext
}

// Convert C char** to Go string[]
func GoStrings(argc C.int, argv **C.char) []string {
	length := int(argc)
	tmpslice := (*[1 << 30]*C.char)(unsafe.Pointer(argv))[:length:length]
	gostrings := make([]string, length)
	for i, s := range tmpslice {
		gostrings[i] = C.GoString(s)
	}
	return gostrings
}

//  Convert string to map[string]interface{}
func string_to_map_interface(str string) (map[string]interface{}, error) {
	var out map[string]interface{}

	jsonBytes := []byte(str)

	// Initialize decoder
	decoder := json.NewDecoder(bytes.NewBuffer(jsonBytes))
	decoder.UseNumber()

	err := decoder.Decode(&out)
	if err != nil {
		return nil, err
	}

	return out, nil
}

// Initialize a new config manager context.
func ConfigManager() (*ConfigMgr, error) {
	// Fetching app_cfg object
	app_cfg := C.create_new_cfg_mr()
	// Fetching env_var from app_cfg object
	c_env_var := C.get_env_var(app_cfg)
	// Converting C string to Go string
	go_env_var := C.GoString(c_env_var)
	// Converting Go string to json
	json_env_var, err := string_to_map_interface(go_env_var)
	if err != nil {
		glog.Errorf("Error in fetching GlobalEnv json: %v", err)
		return nil, err
	}
	// Iterating through and setting env values
	// for all items in env_var
	for key, value := range json_env_var {
		os.Setenv(key, value.(string))
	}
	cfgmgrContext := new(ConfigMgrContext)
	cfgmgrContext.cfgmgrCtx = app_cfg
	config_mgr := new(ConfigMgr)
	config_mgr.ctx = cfgmgrContext
	return config_mgr, nil
}

func (ctx *ConfigMgr) GetAppConfig() (map[string]interface{}, error) {
	app_cfg := C.get_apps_config(ctx.ctx.cfgmgrCtx)
	json_app_config, err := string_to_map_interface(C.GoString(app_cfg))
	defer C.free(unsafe.Pointer(app_cfg))

	if err != nil {
		return nil, err
	}
	if json_app_config == nil {
		fmt.Println("value not found")
		return nil, nil
	}
	return json_app_config, nil
}

func (ctx *ConfigMgr) GetAppInterface() (map[string]interface{}, error) {
	app_interface := C.get_apps_interface(ctx.ctx.cfgmgrCtx)
	json_app_interface, err := string_to_map_interface(C.GoString(app_interface))

	defer C.free(unsafe.Pointer(app_interface))

	if err != nil {
		return nil, err
	}
	return json_app_interface, nil
}

func (ctx *ConfigMgr) GetAppName() (string, error) {
	// Fetching app_name
	c_app_name := C.get_app_name(ctx.ctx.cfgmgrCtx)
	// Converting c string to Go string
	go_app_name := C.GoString(c_app_name)
	defer C.free(unsafe.Pointer(c_app_name))
	return go_app_name, nil
}

func (ctx *ConfigMgr) IsDevMode() (bool, error) {
	// Fetching dev mode
	dev_mode := C.is_dev_mode(ctx.ctx.cfgmgrCtx)
	// Return true if dev_mode variable is 0
	if int(dev_mode) == 0 {
		return true, nil
	} else {
		return false, nil
	}
}

func (ctx *ConfigMgr) GetNumPublishers() (int, error) {
	// Fetching dev mode
	num_publihers := C.get_num_publihers(ctx.ctx.cfgmgrCtx)
	return int(num_publihers), nil
}

func (ctx *ConfigMgr) GetNumSubscribers() (int, error) {
	// Fetching dev mode
	num_subscribers := C.get_num_subscribers(ctx.ctx.cfgmgrCtx)
	return int(num_subscribers), nil
}

func (ctx *ConfigMgr) GetNumServers() (int, error) {
	// Fetching dev mode
	num_servers := C.get_num_servers(ctx.ctx.cfgmgrCtx)
	return int(num_servers), nil
}

func (ctx *ConfigMgr) GetNumClients() (int, error) {
	// Fetching dev mode
	num_clients := C.get_num_clients(ctx.ctx.cfgmgrCtx)
	return int(num_clients), nil
}

func (ctx *ConfigMgr) GetPublisherByName(name string) (*PublisherCfg, error) {
	pubCtx := new(PublisherCfg)
	pubCtx.appCfg = ctx.ctx.cfgmgrCtx
	pub_cfg := C.get_publisher_by_name(ctx.ctx.cfgmgrCtx, C.CString(name))
	pubCtx.pubCfg = pub_cfg
	return pubCtx, nil
}

func (ctx *ConfigMgr) GetPublisherByIndex(index int) (*PublisherCfg, error) {
	pubCtx := new(PublisherCfg)
	pubCtx.appCfg = ctx.ctx.cfgmgrCtx
	pub_cfg := C.get_publisher_by_index(ctx.ctx.cfgmgrCtx, C.int(index))
	pubCtx.pubCfg = pub_cfg
	return pubCtx, nil
}

func (ctx *ConfigMgr) GetSubscriberByName(name string) (*SubscriberCfg, error) {
	subCtx := new(SubscriberCfg)
	subCtx.appCfg = ctx.ctx.cfgmgrCtx
	sub_cfg := C.get_subscriber_by_name(ctx.ctx.cfgmgrCtx, C.CString(name))
	subCtx.subCfg = sub_cfg
	return subCtx, nil
}

func (ctx *ConfigMgr) GetSubscriberByIndex(index int) (*SubscriberCfg, error) {
	subCtx := new(SubscriberCfg)
	subCtx.appCfg = ctx.ctx.cfgmgrCtx
	sub_cfg := C.get_subscriber_by_index(ctx.ctx.cfgmgrCtx, C.int(index))
	subCtx.subCfg = sub_cfg
	return subCtx, nil
}

func (ctx *ConfigMgr) GetSeverByName(name string) (*ServerCfg, error) {
	serverCtx := new(ServerCfg)
	serverCtx.appCfg = ctx.ctx.cfgmgrCtx
	server_cfg := C.get_server_by_name(ctx.ctx.cfgmgrCtx, C.CString(name))
	serverCtx.serverCfg = server_cfg
	return serverCtx, nil
}

func (ctx *ConfigMgr) GetSeverByIndex(index int) (*ServerCfg, error) {
	serverCtx := new(ServerCfg)
	serverCtx.appCfg = ctx.ctx.cfgmgrCtx
	server_cfg := C.get_server_by_index(ctx.ctx.cfgmgrCtx, C.int(index))
	serverCtx.serverCfg = server_cfg
	return serverCtx, nil
}

func (ctx *ConfigMgr) GetClientByName(name string) (*ClientCfg, error) {
	clientCtx := new(ClientCfg)
	clientCtx.appCfg = ctx.ctx.cfgmgrCtx
	client_cfg := C.get_client_by_name(ctx.ctx.cfgmgrCtx, C.CString(name))
	clientCtx.clientCfg = client_cfg
	return clientCtx, nil
}

func (ctx *ConfigMgr) GetClientByIndex(index int) (*ClientCfg, error) {
	clientCtx := new(ClientCfg)
	clientCtx.appCfg = ctx.ctx.cfgmgrCtx
	client_cfg := C.get_client_by_index(ctx.ctx.cfgmgrCtx, C.int(index))
	clientCtx.clientCfg = client_cfg
	return clientCtx, nil
}

func (pubctx *PublisherCfg) getEndPoints() string {
	endpoints := C.get_pub_end_points(pubctx.appCfg, pubctx.pubCfg)
	defer C.destroy_char_arr(endpoints)
	return C.GoString(endpoints.arr)
}

func (pubctx *PublisherCfg) getTopics() []string {
	topics_arr := C.get_pub_topics(pubctx.appCfg, pubctx.pubCfg)
	defer C.destroy_string_arr(topics_arr)

	pubTopics := GoStrings(topics_arr.len, topics_arr.arr)
	return pubTopics
}

func (pubctx *PublisherCfg) getAllowedClients() []string {
	allowed_clients_arr := C.get_pub_allowed_clients(pubctx.appCfg, pubctx.pubCfg)
	defer C.destroy_string_arr(allowed_clients_arr)

	allowed_clients := GoStrings(allowed_clients_arr.len, allowed_clients_arr.arr)
	return allowed_clients
}

func (pubctx *PublisherCfg) getMsgbusConfig() string {
	conf := C.get_pub_msgbus_config(pubctx.appCfg, pubctx.pubCfg)
	return C.GoString(conf)
}

func (pubctx *PublisherCfg) setTopics(topics []string) bool {
	topicsList := make([]*C.char, len(topics))
	for i, s := range topics {
		fmt.Println(s)
		cs := C.CString(s)
		defer C.free(unsafe.Pointer(cs))
		topicsList[i] = cs
	}

	topicsSet := C.set_pub_topics(pubctx.appCfg, pubctx.pubCfg, &topicsList[0], C.int(len(topics)))
	if topicsSet == 0 {
		return true
	} else {
		return false
	}
}

func (subctx *SubscriberCfg) getEndPoints() string {
	endpoints := C.get_sub_end_points(subctx.appCfg, subctx.subCfg)
	defer C.destroy_char_arr(endpoints)
	return C.GoString(endpoints.arr)
}

func (subctx *SubscriberCfg) getMsgbusConfig() string {
	conf := C.get_msgbus_config_subscriber(subctx.appCfg, subctx.subCfg)
	return C.GoString(conf)
}

func (subctx *SubscriberCfg) getTopics() []string {
	topics_arr := C.get_topics_subscriber(subctx.appCfg, subctx.subCfg)
	defer C.destroy_string_arr(topics_arr)

	subTopics := GoStrings(topics_arr.len, topics_arr.arr)
	return subTopics
}

func (subctx *SubscriberCfg) setTopics(topics []string) bool {
	topicsList := make([]*C.char, len(topics))
	for i, s := range topics {
		fmt.Println(s)
		cs := C.CString(s)
		defer C.free(unsafe.Pointer(cs))
		topicsList[i] = cs
	}

	topicsSet := C.set_sub_topics(subctx.appCfg, subctx.subCfg, &topicsList[0], C.int(len(topics)))
	if topicsSet == 0 {
		return true
	} else {
		return false
	}
}

func (serverCtx *ServerCfg) getEndPoints() string {
	endpoints := C.get_server_end_points(serverCtx.appCfg, serverCtx.serverCfg)
	defer C.destroy_char_arr(endpoints)
	return C.GoString(endpoints.arr)
}

func (serverCtx *ServerCfg) getMsgbusConfig() string {
	conf := C.get_msgbus_config_server(serverCtx.appCfg, serverCtx.serverCfg)
	return C.GoString(conf)
}

func (serverCtx *ServerCfg) getAllowedClients() []string {
	allowed_clients_arr := C.get_allowed_clients_server(serverCtx.appCfg, serverCtx.serverCfg)
	defer C.destroy_string_arr(allowed_clients_arr)

	allowed_clients := GoStrings(allowed_clients_arr.len, allowed_clients_arr.arr)
	return allowed_clients
}

func (clientCtx *ClientCfg) getEndPoints() string {
	endpoints := C.get_client_end_points(clientCtx.appCfg, clientCtx.clientCfg)
	defer C.destroy_char_arr(endpoints)
	return C.GoString(endpoints.arr)

}

func (clientCtx *ClientCfg) getMsgbusConfig() string {
	conf := C.get_msgbus_config_client(clientCtx.appCfg, clientCtx.clientCfg)
	return C.GoString(conf)
}
