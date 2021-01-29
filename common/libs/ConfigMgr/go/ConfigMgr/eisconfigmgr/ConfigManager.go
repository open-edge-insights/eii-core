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
#cgo LDFLAGS: -leismsgbus -leismsgenv -leisutils -leisconfigmanager -lsafestring


#include <stdio.h>
#include <stdlib.h>
#include <safe_lib.h>
#include <eis/utils/logger.h>
#include <eis/config_manager/cfgmgr.h>

typedef struct {
	char** arr;
	int len;
}string_arr_t;

typedef struct {
	char* arr;
	int len;
}char_arr_t;

typedef struct config_val {
	int config_type;
	int integer;
	double floating;
	char *str;
	bool boolean;
	char *obj;
} config_val_t;

static config_val_t* create_config_val_obj(config_value_t* value) {

	if(value == NULL){
		LOG_ERROR_0("Value in create_config_val_obj is NULL");
    	return NULL;
	}

	config_val_t* cv = NULL;
	cv = (config_val_t*) malloc(sizeof(config_val_t));
	int ret;

	if(cv == NULL) {
        LOG_ERROR_0("Out of memory creating config value");
        return NULL;
	}
	cv->str = NULL;
	cv->obj = NULL;

	if(value->type == CVT_INTEGER) {
	 	cv->config_type = 0;
		cv->integer = value->body.integer;
	} else if(value->type == CVT_FLOATING) {
	 	cv->config_type = 1;
		cv->floating = value->body.floating;
	} else if(value->type == CVT_STRING) {
		cv->config_type = 2;
		cv->str = (char*) malloc(strlen(value->body.string) + 1);
		if(cv->str == NULL) {
			LOG_ERROR_0("Malloc failed for creating cv str");
			return NULL;
		}
		int len = strlen(value->body.string);

		ret = strncpy_s(cv->str, len + 1, value->body.string, len);
		if (ret != 0) {
			free(cv->str);
			LOG_ERROR_0("String copy failed for config_val");
			return NULL;
		}
	} else if(value->type == CVT_BOOLEAN) {
	 	cv->config_type = 3;
		cv->boolean = value->body.boolean;
	}  else if(value->type == CVT_OBJECT) {
		cv->config_type = 4;
		char* cvchar = cvt_to_char(value);
		if(cvchar == NULL){
			LOG_ERROR_0("Conversion of cvt to char failed");
			return NULL;
		}
		cv->obj = (char*) malloc(strlen(cvchar) + 1);
		if(cv->obj == NULL) {
			LOG_ERROR_0("Out of memory creating cv obj");
			return NULL;
		}

		int len = strlen(cvchar);
		ret = strncpy_s(cv->obj, len + 1, cvchar, len);
		if (ret != 0) {
			free(cv->obj);
			LOG_ERROR_0("String copy failed for config_val");
			return NULL;
		}
	} else if(value->type == CVT_ARRAY) {
		cv->config_type = 5;
		char* cvchar = cvt_to_char(value);
		if(cvchar == NULL){
			LOG_ERROR_0("Conversion of cvt to char failed");
			return NULL;
		}

		cv->obj = (char*) malloc(strlen(cvchar) + 1);
		if(cv->obj == NULL) {
			LOG_ERROR_0("Out of memory creating cv obj");
			return NULL;
		}

		int len = strlen(cvchar);
		ret = strncpy_s(cv->obj, len + 1, cvchar, len);
		if (ret != 0) {
			free(cv->obj);
			LOG_ERROR_0("String copy failed for config_val");
			return NULL;
		}
	} else {
		LOG_ERROR_0("Type mismatch of Interface value");
		free(cv);
		return NULL;
	}
	return cv;
}

static inline void destroy_config_val(config_val_t* config_val) {
	if(config_val != NULL) {
		if(config_val->obj != NULL)
			free(config_val->obj);
		if(config_val->str != NULL)
			free(config_val->str);
		free(config_val);
	}
}

inline char** parse_config_value(config_value_t* config_value, int num_of_items) {
	char** char_config_value = NULL;
	char_config_value = (char**)malloc(num_of_items * sizeof(char*));
	if(char_config_value == NULL){
		LOG_ERROR_0("Malloc failed for char_config_value");
		return NULL;
	}
	config_value_t* config_val = NULL;
	for (int i = 0; i < num_of_items; i++) {
		config_val = config_value_array_get(config_value, i);
		if (config_val == NULL) {
			LOG_ERROR_0("Config value array get failed");
			free_mem(char_config_value);
			return NULL;
		}

		if (config_val->body.string == NULL) {
			LOG_ERROR_0("Config value string is NULL");
			free_mem(char_config_value);
			return NULL;
		}

		char_config_value[i] = (char*)malloc(strlen(config_val->body.string) + 1);
		if(char_config_value[i] == NULL){
			LOG_ERROR_0("Malloc failed for char_config_value");
			free_mem(char_config_value);
			return NULL;
		}

		int len = strlen(config_val->body.string);
		int ret;
		strcmp_s(config_val->body.string, strlen(config_val->body.string), "",&ret);

		if (ret == 0){
			// copying empty string and hence copying length of 1
			ret = strncpy_s(char_config_value[i], 1, "", 1);
			if (ret != 0) {
				free_mem(char_config_value);
				LOG_ERROR_0("String copy failed for config_val");
				return NULL;
			}
			return char_config_value;
		}
		ret = strncpy_s(char_config_value[i], len + 1, config_val->body.string, len);
		if (ret != 0) {
			free_mem(char_config_value);
			LOG_ERROR_0("String copy failed for config_val");
			return NULL;
		}

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
static inline void* create_new_cfg_mr() {

	cfgmgr_ctx_t* cfgmgr = cfgmgr_initialize();
	if (cfgmgr == NULL){
		LOG_ERROR_0("ConfigMgr initialization failed in base c layer");
		return NULL;
	}
	return cfgmgr;
}

static inline char* get_env_var(void* cfg_mgr) {
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	if (c_cfgmgr == NULL || c_cfgmgr->env_var == NULL){
		LOG_ERROR_0(" c_cfgmgr or Env_Var is NULL");
		return NULL;
	}
	return c_cfgmgr->env_var;
}

static inline char* get_app_name(void* cfg_mgr) {
	int ret;
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	if (c_cfgmgr == NULL){
		LOG_ERROR_0("App config is NULL");
		return NULL;
	}
	config_value_t* appname = cfgmgr_get_appname(c_cfgmgr);
	if (appname == NULL) {
		LOG_ERROR_0("Getting App name from base c layer failed");
		return NULL;
	}
    if (appname->type != CVT_STRING) {
        LOG_ERROR_0("appname type is not string");
        return NULL;
    } else {
        if (appname->body.string == NULL) {
            LOG_ERROR_0("AppName is NULL");
            return NULL;
    	}
    }
    int app_len = strlen(appname->body.string);
    char* c_appname = (char*)malloc(app_len + 1);
    if (c_appname == NULL) {
	LOG_ERROR_0("Failed to malloc for appname");
    }
    ret = strncpy_s(c_appname, app_len + 1, appname->body.string, app_len);
    if (ret != 0) {
	LOG_ERROR_0("String copy failed for appname");
	free(c_appname);
	return NULL;
    }
    config_value_destroy(appname);
    return c_appname;
}

static inline bool is_dev_mode(void* cfg_mgr) {
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	bool result = cfgmgr_is_dev_mode(c_cfgmgr);
	return result;
}

static inline int get_num_publihers(void* cfg_mgr) {
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	if (c_cfgmgr == NULL){
		LOG_ERROR_0("App config is NULL");
		return -1;
	}
	return cfgmgr_get_num_publishers(c_cfgmgr);
}

static inline int get_num_subscribers(void* cfg_mgr) {
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	if (c_cfgmgr == NULL){
		LOG_ERROR_0("App config is NULL");
		return -1;
	}
	return cfgmgr_get_num_subscribers(c_cfgmgr);
}

static inline int get_num_servers(void* cfg_mgr) {
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	if (c_cfgmgr == NULL){
		LOG_ERROR_0("App config is NULL");
		return -1;
	}
	return cfgmgr_get_num_servers( c_cfgmgr);
}

static inline int get_num_clients(void* cfg_mgr) {
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	if (c_cfgmgr == NULL){
		LOG_ERROR_0("App config is NULL");
		return -1;
	}
	return cfgmgr_get_num_clients(c_cfgmgr);
}

// ---------------APP CONFIG-----------------------
static inline void* get_apps_config(void* cfg_mgr){
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	if (c_cfgmgr == NULL){
		LOG_ERROR_0("App config is NULL");
		return NULL;
	}
	config_t* config = c_cfgmgr->app_config;
	if(config == NULL) {
		LOG_ERROR_0("Config value is NULL");
		return NULL;
	}
	return (void*)config;
}

static inline void destroy_cfg_mgr(void* cfg_mgr) {
	LOG_DEBUG_0("ConfigManager: destroy_cfg_mgr");
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;

	if(c_cfgmgr != NULL){
		LOG_DEBUG_0("ConfigManager: destroy cfgmgr");
		cfgmgr_destroy(c_cfgmgr);
	}
}

static inline void destroy_publisher(void* cfgmgr_ctx) {
	LOG_DEBUG_0("ConfigManager: destroy publisher context");
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	if (c_cfgmgr_ctx->interface != NULL) {
		LOG_DEBUG_0("ConfigManager: destroy publisher config");
		config_value_destroy(c_cfgmgr_ctx->interface);
		free(c_cfgmgr_ctx);
	}
}

static inline void destroy_subscriber(void* cfgmgr_ctx) {
	LOG_DEBUG_0("ConfigManager: destroy subscriber context");
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	if (c_cfgmgr_ctx->interface != NULL) {
		LOG_DEBUG_0("ConfigManager: destroy subscriber config");
		config_value_destroy(c_cfgmgr_ctx->interface);
		free(c_cfgmgr_ctx);
	}
}

static inline void destroy_client(void* cfgmgr_ctx) {
	LOG_DEBUG_0("ConfigManager: destroy client context");
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	if (c_cfgmgr_ctx->interface != NULL) {
		LOG_DEBUG_0("ConfigManager: destroy client config");
		config_value_destroy(c_cfgmgr_ctx->interface);
		free(c_cfgmgr_ctx);
	}
}

static inline void destroy_server(void* cfgmgr_ctx) {
	LOG_DEBUG_0("ConfigManager: destroy server context");
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	if (c_cfgmgr_ctx->interface != NULL) {
		LOG_DEBUG_0("ConfigManager: destroy server config");
		config_value_destroy(c_cfgmgr_ctx->interface);
		free(c_cfgmgr_ctx);
	}
}

// -----------------PUBLISHER----------------------
static inline void* get_publisher_by_name(void* ctx, char *name){
	cfgmgr_interface_t* cfgmgr_ctx = NULL;
	cfgmgr_ctx_t *cfg_mgr = (cfgmgr_ctx_t *)ctx;
	cfgmgr_ctx = cfgmgr_get_publisher_by_name(cfg_mgr, name);
	if(cfgmgr_ctx == NULL) {
		LOG_ERROR_0("[Publisher]: Get publisher by name from base C layer failed");
		return NULL;
	}
	return cfgmgr_ctx;
}

static inline void* get_publisher_by_index(void* ctx, int index){
	cfgmgr_interface_t* cfgmgr_ctx = NULL;
	cfgmgr_ctx_t *cfg_mgr = (cfgmgr_ctx_t *)ctx;
	cfgmgr_ctx = cfgmgr_get_publisher_by_index(cfg_mgr, index);
	if(cfgmgr_ctx == NULL) {
		LOG_ERROR_0("[Publisher]: Get publisher by index from base C layer failed");
		return NULL;
	}
	return cfgmgr_ctx;
}

static inline bool set_pub_topics(void* cfgmgr_ctx, char **topics, int len) {
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	bool ret = cfgmgr_set_topics(c_cfgmgr_ctx, topics, len);
	return ret;
}

static inline char_arr_t* get_pub_end_points(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	char_arr_t* char_arr = (char_arr_t*) malloc(sizeof(char_arr_t));
	if(char_arr == NULL){
		LOG_ERROR_0("[Publisher]: Malloc failed for char_arr");
		return NULL;
	}

	config_value_t* endpoints = cfgmgr_get_endpoint(c_cfgmgr_ctx);
	if (endpoints == NULL){
		destroy_char_arr(char_arr);
		LOG_ERROR_0("[Publisher]: Getting endpoint from base c layer failed");
		return NULL;
	}

	char* ep;
	if (endpoints->type == CVT_OBJECT){
		ep = cvt_to_char(endpoints);
		if(ep == NULL){
			LOG_ERROR_0("Conversion of cvt to char failed");
			destroy_char_arr(char_arr);
			return NULL;
		}
	} else if (endpoints->type == CVT_STRING) {
		ep = (endpoints->body.string);
	} else {
		LOG_ERROR_0("EndPoint type mismatch: It should be either string or json");
		destroy_char_arr(char_arr);
		return NULL;
	}

	int len = strlen(ep);
	char_arr->arr = (char*)malloc(len + 1);
	if(char_arr->arr == NULL){
		LOG_ERROR_0("[Publisher]: Malloc failed for char_arr's inner array");
		destroy_char_arr(char_arr);
		return NULL;
	}

	int ret;
	ret = strncpy_s(char_arr->arr, len + 1, ep, len);
	if (ret != 0) {
		destroy_char_arr(char_arr);
		LOG_ERROR_0("String copy failed for ep");
		return NULL;
	}
	char_arr->len = len;

	// Destroying endpoints
	config_value_destroy(endpoints);
	return char_arr;
}

static inline void* get_pub_msgbus_config(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	config_t* config = cfgmgr_get_msgbus_config(c_cfgmgr_ctx);
	if (config == NULL){
		LOG_ERROR_0("[Publisher] Getting message bus config failed");
		return NULL;
	}
	return (void*)config;
}

static inline string_arr_t* get_pub_topics(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	string_arr_t* str_arr = (string_arr_t*) malloc(sizeof(string_arr_t));
	if(str_arr == NULL){
		LOG_ERROR_0("[Publisher] Malloc failed for str_arr");
		return NULL;
	}

	config_value_t* topics = cfgmgr_get_topics(c_cfgmgr_ctx);
	if (topics == NULL) {
		destroy_string_arr(str_arr);
		LOG_ERROR_0("[Publisher]: Get topics from base c layer failed")
		return NULL;
	}

	int topics_len = config_value_array_len(topics);
	char** arr_topics = parse_config_value(topics, topics_len);
	config_value_destroy(topics);
	if (arr_topics == NULL) {
		destroy_string_arr(str_arr);
		LOG_ERROR_0("[Publisher] Parse config value failed");
		return NULL;
	}
	str_arr->arr = arr_topics;
	str_arr->len = topics_len;
	return str_arr;
}

static inline string_arr_t* get_pub_allowed_clients(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	string_arr_t* str_arr = (string_arr_t*) malloc(sizeof(string_arr_t));
	if(str_arr == NULL){
		LOG_ERROR_0("[Publisher]: Malloc failed for str_arr");
		return NULL;
	}

	config_value_t* allowed_clients = cfgmgr_get_allowed_clients(c_cfgmgr_ctx);
	if (allowed_clients == NULL){
		destroy_string_arr(str_arr);
		LOG_ERROR_0("[Publisher]: Getting Allowed clients from base c layer failed");
		return NULL;
	}

	int clients_len = config_value_array_len(allowed_clients);
	char** arr_allowed_clients = parse_config_value(allowed_clients, clients_len);
	if (arr_allowed_clients == NULL){
		destroy_string_arr(str_arr);
		LOG_ERROR_0("[Publisher] Parse config value failed");
		return NULL;
	}
	config_value_destroy(allowed_clients);
	str_arr->arr = arr_allowed_clients;
	str_arr->len = 	clients_len;
	return str_arr;
}

static inline config_val_t* get_pub_interface_value(void* cfgmgr_ctx, char *key) {
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	config_value_t* value = cfgmgr_get_interface_value(c_cfgmgr_ctx, key);
	if(value == NULL){
		LOG_ERROR_0("[Publisher]: Get interface value from base C layer failed");
		return NULL;
	}
	config_val_t* cv = create_config_val_obj(value);
	config_value_destroy(value);

	if(cv == NULL){
		LOG_ERROR_0("[Publisher]: creating config value obj failed ");
		return NULL;
	}
    return cv;
}

//-----------------SUBSCRIBER----------------------

static inline void* get_subscriber_by_name(void* ctx, char *name){
	cfgmgr_interface_t* cfgmgr_ctx = NULL;
	cfgmgr_ctx_t *cfg_mgr = (cfgmgr_ctx_t *)ctx;
	cfgmgr_ctx = cfgmgr_get_subscriber_by_name(cfg_mgr, name);
	if(cfgmgr_ctx == NULL){
		LOG_ERROR_0("[Subscriber]: Get subscriber by name from base C layer failed");
		return NULL;
	}
	return cfgmgr_ctx;
}

static inline void* get_subscriber_by_index(void* ctx, int index){
	cfgmgr_interface_t* cfgmgr_ctx = NULL;
	cfgmgr_ctx_t *cfg_mgr = (cfgmgr_ctx_t *)ctx;
	cfgmgr_ctx = cfgmgr_get_subscriber_by_index(cfg_mgr, index);
	if(cfgmgr_ctx == NULL) {
		LOG_ERROR_0("[Subscriber]: Get subscriber by index from base C layer failed");
		return NULL;
	}
	return cfgmgr_ctx;
}

static inline char_arr_t* get_sub_end_points(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	char_arr_t* char_arr = (char_arr_t*) malloc(sizeof(char_arr_t));
	if(char_arr == NULL){
		LOG_ERROR_0("[Subscriber]: Malloc failed for char_arr");
		return NULL;
	}

	config_value_t* endpoints = cfgmgr_get_endpoint(c_cfgmgr_ctx);
	if (endpoints == NULL){
		destroy_char_arr(char_arr);
		LOG_ERROR_0("[Subscriber]: Getting endpoint from base c layer failed");
		return NULL;
	}

	char* ep;
	if (endpoints->type == CVT_OBJECT){
		ep = cvt_to_char(endpoints);
		if(ep == NULL){
			LOG_ERROR_0("Conversion of cvt to char failed");
			destroy_char_arr(char_arr);
			return NULL;
		}
	} else if (endpoints->type == CVT_STRING) {
		ep = (endpoints->body.string);
	} else {
		LOG_ERROR_0("EndPoint type mismatch: It should be either string or json");
		destroy_char_arr(char_arr);
		return NULL;
	}

	int len = strlen(ep);
	char_arr->arr = (char*)malloc(len + 1);
	if(char_arr->arr == NULL){
		LOG_ERROR_0("[Subscriber]: Malloc failed for char_arr's inner array");
		destroy_char_arr(char_arr);
		return NULL;
	}

	int ret;
	ret = strncpy_s(char_arr->arr, len + 1, ep, len);
	if (ret != 0) {
		destroy_char_arr(char_arr);
		LOG_ERROR_0("String copy failed for ep");
		return NULL;
	}
	char_arr->len = len;

	// Destroying endpoints
	config_value_destroy(endpoints);
	return char_arr;
}

static inline void* get_msgbus_config_subscriber(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	config_t* config = cfgmgr_get_msgbus_config(c_cfgmgr_ctx);
	if (config == NULL){
		LOG_ERROR_0("[Subscriber] Getting message bus config failed");
		return NULL;
	}
	return (void*)config;
}

static inline string_arr_t* get_topics_subscriber(void* cfgmgr_ctx) {
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	string_arr_t* str_arr = (string_arr_t*) malloc(sizeof(string_arr_t));
	if(str_arr == NULL){
		LOG_ERROR_0("[Subscriber]: Malloc failed to str_arr");
		return NULL;
	}

	config_value_t* topics = cfgmgr_get_topics(c_cfgmgr_ctx);
	if (topics == NULL){
		destroy_string_arr(str_arr);
		LOG_ERROR_0("[Subscriber]: Get topics from base c layer failed");
		return NULL;
	}

	int topics_len = config_value_array_len(topics);
	char** arr_topics = parse_config_value(topics, topics_len);
	if (arr_topics == NULL){
		destroy_string_arr(str_arr);
		LOG_ERROR_0("[Subscriber] Parse config value failed");
		return NULL;
	}
	config_value_destroy(topics);
	str_arr->arr = arr_topics;
	str_arr->len = topics_len;
	return str_arr;
}

static inline bool set_sub_topics(void* cfgmgr_ctx, char **topics, int len) {
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	bool ret = cfgmgr_set_topics(c_cfgmgr_ctx, topics, len);
	return ret;
}

static inline config_val_t* get_sub_interface_value(void* cfgmgr_ctx, char *key) {
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	config_value_t* value = cfgmgr_get_interface_value(c_cfgmgr_ctx, key);
	if(value == NULL){
		LOG_ERROR_0("[Subscriber]: Get interface value from base C layer failed");
		return NULL;
	}
	config_val_t* cv = create_config_val_obj(value);
	config_value_destroy(value);
	if(cv == NULL){
		LOG_ERROR_0("[Subscriber]: creating config value obj failed ");
		return NULL;
	}
    return cv;
}

//-----------------SERVER--------------------------

static inline void* get_server_by_name(void* ctx, char *name){
	cfgmgr_interface_t* cfgmgr_ctx = NULL;
	cfgmgr_ctx_t *cfg_mgr = (cfgmgr_ctx_t *)ctx;
	cfgmgr_ctx = cfgmgr_get_server_by_name(cfg_mgr, name);
	if(cfgmgr_ctx == NULL) {
		LOG_ERROR_0("[Server]: Get publisher by name from base C layer failed");
		return NULL;
	}
	return cfgmgr_ctx;
}

static inline void* get_server_by_index(void* ctx, int index){
	cfgmgr_interface_t* cfgmgr_ctx = NULL;
	cfgmgr_ctx_t *cfg_mgr = (cfgmgr_ctx_t *)ctx;
	cfgmgr_ctx = cfgmgr_get_server_by_index(cfg_mgr, index);
	if(cfgmgr_ctx == NULL) {
		LOG_ERROR_0("[Server]: Get publisher by index from base C layer failed");
		return NULL;
	}
	return cfgmgr_ctx;
}

static inline char_arr_t* get_server_end_points(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	char_arr_t* char_arr = (char_arr_t*) malloc(sizeof(char_arr_t));
	if(char_arr == NULL){
		LOG_ERROR_0("[Server]: Malloc failed for char_arr");
		return NULL;
	}

	config_value_t* endpoints = cfgmgr_get_endpoint(c_cfgmgr_ctx);
	if (endpoints == NULL){
		destroy_char_arr(char_arr);
		LOG_ERROR_0("[Server]: Getting endpoint from base c layer failed");
		return NULL;
	}

	char* ep;
	if (endpoints->type == CVT_OBJECT){
		ep = cvt_to_char(endpoints);
		if(ep == NULL){
			LOG_ERROR_0("Conversion of cvt to char failed");
			destroy_char_arr(char_arr);
			return NULL;
		}
	} else if (endpoints->type == CVT_STRING) {
		ep = (endpoints->body.string);
	} else {
		LOG_ERROR_0("EndPoint type mismatch: It should be either string or json");
		destroy_char_arr(char_arr);
		return NULL;
	}

	int len = strlen(ep);
	char_arr->arr = (char*)malloc(len + 1);
	if(char_arr->arr == NULL){
		LOG_ERROR_0("[Server]: Malloc failed for char_arr's inner array");
		destroy_char_arr(char_arr);
		return NULL;
	}

	int ret;
	ret = strncpy_s(char_arr->arr, len + 1, ep, len);
	if (ret != 0) {
		destroy_char_arr(char_arr);
		LOG_ERROR_0("String copy failed for ep");
		return NULL;
	}
	char_arr->len = len;

	// Destroying endpoints
	config_value_destroy(endpoints);
	return char_arr;
}

static inline void* get_msgbus_config_server(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	config_t* config = cfgmgr_get_msgbus_config(c_cfgmgr_ctx);
	if (config == NULL){
		LOG_ERROR_0("[Server] Getting message bus config failed");
		return NULL;
	}
	return (void*)config;
}

static inline string_arr_t* get_allowed_clients_server(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	string_arr_t* str_arr = (string_arr_t*) malloc(sizeof(string_arr_t));
	if(str_arr == NULL){
		LOG_ERROR_0("[Server]: Malloc failed for str_arr");
		return NULL;
	}

	config_value_t* allowed_clients = cfgmgr_get_allowed_clients(c_cfgmgr_ctx);
	if (allowed_clients == NULL){
		destroy_string_arr(str_arr);
		LOG_ERROR_0("[Server]: Getting Allowed clients from base c layer failed");
		return NULL;
	}

	int clients_len = config_value_array_len(allowed_clients);
	char** arr_allowed_clients = parse_config_value(allowed_clients, clients_len);
	config_value_destroy(allowed_clients);

	if (arr_allowed_clients == NULL){
		destroy_string_arr(str_arr);
		LOG_ERROR_0("[Server] Parse config value failed");
		return NULL;
	}
	str_arr->arr = arr_allowed_clients;
	str_arr->len = 	clients_len;
	return str_arr;
}

static inline config_val_t* get_server_interface_value(void* cfgmgr_ctx, char *key) {
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	config_value_t* value = cfgmgr_get_interface_value(c_cfgmgr_ctx, key);
	if(value == NULL){
		LOG_ERROR_0("[Server]: Get interface value from base C layer failed");
		return NULL;
	}
	config_val_t* cv = create_config_val_obj(value);
	config_value_destroy(value);

	if(cv == NULL){
		LOG_ERROR_0("[Server]: creating config value obj failed ");
		return NULL;
	}
    return cv;
}

//-----------------CLIENT--------------------------
static inline void* get_client_by_name(void* ctx, char *name){
	cfgmgr_interface_t* cfgmgr_ctx = NULL;
	cfgmgr_ctx_t *cfg_mgr = (cfgmgr_ctx_t *)ctx;
	cfgmgr_ctx = cfgmgr_get_client_by_name(cfg_mgr, name);
	if(cfgmgr_ctx == NULL) {
		LOG_ERROR_0("[Client]: Get publisher by name from base C layer failed");
		return NULL;
	}
	return cfgmgr_ctx;
}

static inline void* get_client_by_index(void* ctx, int index){
	cfgmgr_interface_t* cfgmgr_ctx = NULL;
	cfgmgr_ctx_t *cfg_mgr = (cfgmgr_ctx_t *)ctx;
	cfgmgr_ctx = cfgmgr_get_client_by_index(cfg_mgr, index);
	if(cfgmgr_ctx == NULL) {
		LOG_ERROR_0("[Client]: Get publisher by index from base C layer failed");
		return NULL;
	}
	return cfgmgr_ctx;
}

static inline char_arr_t* get_client_end_points(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	char_arr_t* char_arr = (char_arr_t*) malloc(sizeof(char_arr_t));
	if(char_arr == NULL){
		LOG_ERROR_0("[Client]: Malloc failed for char_arr");
		return NULL;
	}

	config_value_t* endpoints = cfgmgr_get_endpoint(c_cfgmgr_ctx);
	if (endpoints == NULL){
		destroy_char_arr(char_arr);
		LOG_ERROR_0("[Client]: Getting endpoint from base c layer failed");
		return NULL;
	}

	char* ep;
	if (endpoints->type == CVT_OBJECT){
		ep = cvt_to_char(endpoints);
		if(ep == NULL){
			LOG_ERROR_0("Conversion of cvt to char failed");
			destroy_char_arr(char_arr);
			return NULL;
		}
	} else if (endpoints->type == CVT_STRING) {
		ep = (endpoints->body.string);
	} else {
		LOG_ERROR_0("EndPoint type mismatch: It should be either string or json");
		destroy_char_arr(char_arr);
		return NULL;
	}

	int len = strlen(ep);
	char_arr->arr = (char*)malloc(len + 1);
	if(char_arr->arr == NULL){
		LOG_ERROR_0("[Client]: Malloc failed for char_arr's inner array");
		destroy_char_arr(char_arr);
		return NULL;
	}

	int ret;
	ret = strncpy_s(char_arr->arr, len + 1, ep, len);
	if (ret != 0) {
		destroy_char_arr(char_arr);
		LOG_ERROR_0("String copy failed for ep");
		return NULL;
	}
	char_arr->len = len;

	// Destroying endpoints
	config_value_destroy(endpoints);
	return char_arr;
}

static inline void* get_msgbus_config_client(void* cfgmgr_ctx){
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;
	config_t* config = cfgmgr_get_msgbus_config(c_cfgmgr_ctx);
	if (config == NULL){
		LOG_ERROR_0("[Client] Getting message bus config failed");
		return NULL;
	}
	return (void*)config;
}

static inline config_val_t* get_client_interface_value(void* cfgmgr_ctx, char *key) {
	cfgmgr_interface_t* c_cfgmgr_ctx = (cfgmgr_interface_t *)cfgmgr_ctx;

	config_value_t* value = cfgmgr_get_interface_value(c_cfgmgr_ctx, key);
	if(value == NULL){
		LOG_ERROR_0("[Client]: Get interface value from base C layer failed");
		return NULL;
	}
	config_val_t* cv = create_config_val_obj(value);
	config_value_destroy(value);

	if(cv == NULL){
		LOG_ERROR_0("[Client]: creating config value obj failed ");
		return NULL;
	}
    return cv;
}

//-----------------WATCH--------------------------

typedef void (*cfgmgr_watch_callback_t)(const char *key, config_t* value, void* cb_user_data);
void cCallback(const char *key, config_t* value, void* cb_user_data);

static inline void watch(void* cfg_mgr, const char* key, cfgmgr_watch_callback_t callback_fn, void* user_data) {
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	cfgmgr_watch(c_cfgmgr, key, callback_fn, user_data);
}

static inline void watch_prefix(void* cfg_mgr, char* prefix, cfgmgr_watch_callback_t callback_fn, void* user_data) {
	cfgmgr_ctx_t* c_cfgmgr = (cfgmgr_ctx_t *)cfg_mgr;
	// Calling the base cfgmgr_watch_prefix C API
	cfgmgr_watch_prefix(c_cfgmgr, prefix, callback_fn, user_data);
}

*/
import "C"

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"unsafe"

	"github.com/golang/glog"
)

// ConfigMgr object
type ConfigMgr struct {
	cfgMgr unsafe.Pointer
}

// WatchObj context
type WatchObj struct {
	cfgMgr unsafe.Pointer
}

// goCallbacktype variable to define the go Callback Type
type goCallbacktype func(string, map[string]interface{}, interface{})

// cbFunc of type goCallbacktype to call the Go Callback from C
var cbFunc = func(key string, value map[string]interface{}, user_data interface{}) {}

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

// getConfigVal based on it's type
func getConfigVal(interfaceVal *C.config_val_t) (*ConfigValue, error) {
	if interfaceVal == nil {
		return nil, errors.New("Interface Value is not found")
	}

	configValue := new(ConfigValue)
	if interfaceVal.config_type == 0 {
		configValue.Type = Int
		configValue.Value = eval(integer(interfaceVal.integer))
	} else if interfaceVal.config_type == 1 {
		configValue.Type = Float32
		configValue.Value = eval(float(interfaceVal.floating))
	} else if interfaceVal.config_type == 2 {
		configValue.Type = String
		configValue.Value = eval(str(C.GoString(interfaceVal.str)))
	} else if interfaceVal.config_type == 3 {
		configValue.Type = Boolean
		configValue.Value = eval(boolean(interfaceVal.boolean))
	} else if interfaceVal.config_type == 4 {
		configValue.Type = Json
		str := C.GoString(interfaceVal.obj)
		mapInt, _ := string_to_map_interface(str)
		configValue.Value = eval(object(mapInt))
	} else if interfaceVal.config_type == 5 {
		configValue.Type = Array
		str := C.GoString(interfaceVal.obj)
		var arr []interface{}
		err := json.Unmarshal([]byte(str), &arr)
		if err != nil {
			glog.Errorf("Error in json unmarshal: %v", err)
			return nil, errors.New("json unmarshal error")
		}
		configValue.Value = eval(array(arr))
	} else {
		return nil, errors.New("Type mismatch of Interface value")
	}
	return configValue, nil
}

// Initialize a new config manager context.
//
// Returns:
// 1. ConfigMgr : ConfigMgr object
//    ConfigManager instance
// 2. error
//    Error on failure,  nil on success
func ConfigManager() (*ConfigMgr, error) {
	// Fetching cfg_mgr object
	cfgMgr := C.create_new_cfg_mr()
	if cfgMgr == nil {
		return nil, errors.New("Creating ConfigMgr instance failed in CGO\n")
	}
	// Fetching env_var from cfgMgr object
	cEnvVar := C.get_env_var(cfgMgr)
	if cEnvVar == nil {
		return nil, errors.New("Getting env var failed in CGO\n")
	}
	// Converting C string to Go string
	goEnvVar := C.GoString(cEnvVar)
	// Converting Go string to json
	jsonEnvVar, err := string_to_map_interface(goEnvVar)
	if err != nil {
		glog.Errorf("Error in fetching GlobalEnv json: %v", err)
		return nil, err
	}
	// Iterating through and setting env values
	// for all items in env_var
	for key, value := range jsonEnvVar {
		os.Setenv(key, value.(string))
	}
	config_mgr := new(ConfigMgr)
	config_mgr.cfgMgr = cfgMgr
	return config_mgr, nil
}

// Gets value from respective application's config
//
// Returns:
// 1. appConfig : map[string]interface{}
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetAppConfig() (map[string]interface{}, error) {
	cfgMgr := C.get_apps_config(ctx.cfgMgr)
	cAppConfig := (*C.config_t)(cfgMgr)
	cAppConf := C.configt_to_char(cAppConfig)
	if cAppConf == nil {
		return nil, errors.New("[AppConfig] Converting configt to char failed")
	}

	jsonAppConfig, err := string_to_map_interface(C.GoString(cAppConf))
	defer C.free(unsafe.Pointer(cAppConf))

	if err != nil {
		return nil, err
	}
	if jsonAppConfig == nil {
		fmt.Println("value not found")
		return nil, nil
	}
	return jsonAppConfig, nil
}

// Get Applications name.
//
// Returns:
// 1. App name : string
//    Applications name
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetAppName() (string, error) {
	// Fetching app_name
	cAppName := C.get_app_name(ctx.cfgMgr)
	if cAppName == nil {
		return "", errors.New("Getting app name failed in CGO\n")
	}
	// Converting c string to Go string
	goAppName := C.GoString(cAppName)
	defer C.free(unsafe.Pointer(cAppName))
	return goAppName, nil
}

// To check if application running in dev_mode or prod_mode
//
// Returns:
// 1. bool value : bool
//    true is dev_mode, false if prod_mode
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) IsDevMode() (bool, error) {
	// Fetching dev mode
	devMode := C.is_dev_mode(ctx.cfgMgr)
	// Return true if dev_mode variable is 0
	if devMode {
		return true, nil
	} else {
		return false, nil
	}
}

func (configMgr *ConfigMgr) GetWatchObj() (*WatchObj, error) {
	watchCtx := new(WatchObj)
	watchCtx.cfgMgr = configMgr.cfgMgr
	return watchCtx, nil
}

//export cfgmgrGoCallback
func cfgmgrGoCallback(key *C.char, value *C.config_t, user_data unsafe.Pointer) {
	cConfig := (*C.config_t)(value)
	// Converting C config_t to C String
	cConf := C.configt_to_char(cConfig)
	if cConf == nil {
		glog.Error("[cfgmgrGoCallback] Failed to convert configt to char")
		return
	}
	// Freeing the C config_t value
	defer C.free(unsafe.Pointer(cConf))
	// Converting C string to Go map[string]interface{}
	jsonAppConfig, err := string_to_map_interface(C.GoString(cConf))
	if err != nil {
		glog.Errorf("Error: %v", err)
		return
	}
	// Converting C string key to Go String
	cKey := C.GoString(key)
	// Freeing the C key string
	defer C.free(unsafe.Pointer(key))
	// Fetching the interface{} value associated with unsafe.Pointer user_data
	u := Restore(user_data)
	if u == nil {
		glog.Errorf("interface{} for user_data not found")
		return
	}
	// Freeing the unsafe.Pointer and it's associated interface{}
	defer Unref(user_data)
	// Calling the Go callback with the obtained values
	cbFunc(cKey, jsonAppConfig, u)
	return
}

func (watchCtx *WatchObj) Watch(key string, callbackFunc goCallbacktype, user_data interface{}) {
	cbFunc = callbackFunc
	// Storing the interface associated with an unsafe.Pointer
	// This is required since Go doesn't allow C code to store pointers to Go data
	ptr := Save(user_data)
	if ptr == nil {
		glog.Errorf("Unable to store interface{}")
		return
	}
	// Calling the C watch API
	C.watch(watchCtx.cfgMgr, C.CString(key), (C.cfgmgr_watch_callback_t)(unsafe.Pointer(C.cCallback)), ptr)
	return
}

func (watchCtx *WatchObj) WatchPrefix(prefix string, callbackFunc goCallbacktype, user_data interface{}) {
	cbFunc = callbackFunc
	// Storing the interface associated with an unsafe.Pointer
	// This is required since Go doesn't allow C code to store pointers to Go data
	ptr := Save(user_data)
	if ptr == nil {
		glog.Errorf("Unable to store interface{}")
		return
	}
	// Calling the C watch_prefix API
	C.watch_prefix(watchCtx.cfgMgr, C.CString(prefix), (C.cfgmgr_watch_callback_t)(unsafe.Pointer(C.cCallback)), ptr)
	return
}

func (watchCtx *WatchObj) WatchConfig(callbackFunc goCallbacktype, user_data interface{}) {
	cbFunc = callbackFunc
	// Storing the interface associated with an unsafe.Pointer
	// This is required since Go doesn't allow C code to store pointers to Go data
	ptr := Save(user_data)
	if ptr == nil {
		glog.Errorf("Unable to store interface{}")
		return
	}
	// Calling the C get_app_name API to fetch AppName
	cAppName := C.get_app_name(watchCtx.cfgMgr)
	if cAppName == nil {
		glog.Errorf("Failed to fetch appname")
		return
	}
	// Converting c string to Go string
	goAppName := C.GoString(cAppName)
	// Freeing the C AppName string
	defer C.free(unsafe.Pointer(cAppName))
	// Creating /<AppName>/config key
	key := "/" + goAppName + "/config"
	// Calling the C watch API
	C.watch(watchCtx.cfgMgr, C.CString(key), (C.cfgmgr_watch_callback_t)(unsafe.Pointer(C.cCallback)), ptr)
	return
}

func (watchCtx *WatchObj) WatchInterface(callbackFunc goCallbacktype, user_data interface{}) {
	cbFunc = callbackFunc
	// Storing the interface associated with an unsafe.Pointer
	// This is required since Go doesn't allow C code to store pointers to Go data
	ptr := Save(user_data)
	if ptr == nil {
		glog.Errorf("Unable to store interface{}")
		return
	}
	// Calling the C get_app_name API to fetch AppName
	cAppName := C.get_app_name(watchCtx.cfgMgr)
	if cAppName == nil {
		glog.Errorf("Failed to fetch appname")
		return
	}
	// Converting c string to Go string
	goAppName := C.GoString(cAppName)
	// Freeing the C AppName string
	defer C.free(unsafe.Pointer(cAppName))
	// Creating /<AppName>/interfaces key
	key := "/" + goAppName + "/interfaces"
	// Calling the C watch API
	C.watch(watchCtx.cfgMgr, C.CString(key), (C.cfgmgr_watch_callback_t)(unsafe.Pointer(C.cCallback)), ptr)
	return
}

// Get number of publishers in Publisher array in interface
//
// Returns:
// 1. num_of_publishers : int
//    Number of publishers
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetNumPublishers() (int, error) {
	// Fetching dev mode
	numPublihers := C.get_num_publihers(ctx.cfgMgr)
	return int(numPublihers), nil
}

// Get number of subscribers in Subscriber array in interface
//
// Returns:
// 1. num_of_subscribers : int
//    Number of subscribers
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetNumSubscribers() (int, error) {
	// Fetching dev mode
	numSubscribers := C.get_num_subscribers(ctx.cfgMgr)
	return int(numSubscribers), nil
}

// Get number of servers in Server array in interface
//
// Returns:
// 1. num_of_servers : int
//    Number of servers
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetNumServers() (int, error) {
	// Fetching dev mode
	numServers := C.get_num_servers(ctx.cfgMgr)
	return int(numServers), nil
}

// Get number of clients in Client array in interface
//
// Returns:
// 1. num_of_client : int
//    Number of client
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetNumClients() (int, error) {
	// Fetching dev mode
	num_clients := C.get_num_clients(ctx.cfgMgr)
	return int(num_clients), nil
}

// Get the respective publisher based on the name
//
// Returns:
// 1. PublisherCfg : PublisherCfg obj
//    PublisherCfg instance
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetPublisherByName(name string) (*PublisherCfg, error) {
	pubCtx := new(PublisherCfg)
	goPubCfg := C.get_publisher_by_name(ctx.cfgMgr, C.CString(name))
	if goPubCfg == nil {
		return nil, errors.New("[Publisher]: Failed to get publisher by name from CGO\n")
	}
	pubCtx.pubCfg = goPubCfg
	return pubCtx, nil
}

// Get the respective publisher based on the index
//
// Returns:
// 1. PublisherCfg : PublisherCfg obj
//    PublisherCfg instance
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetPublisherByIndex(index int) (*PublisherCfg, error) {
	pubCtx := new(PublisherCfg)
	goPubCfg := C.get_publisher_by_index(ctx.cfgMgr, C.int(index))
	if goPubCfg == nil {
		return nil, errors.New("[Publisher]: Failed to get publisher by index from CGO\n")
	}
	pubCtx.pubCfg = goPubCfg
	return pubCtx, nil
}

// Get the respective subscriber based on the name
//
// Returns:
// 1. SubscriberCfg : SubscriberCfg obj
//    SubscriberCfg instance
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetSubscriberByName(name string) (*SubscriberCfg, error) {
	subCtx := new(SubscriberCfg)
	goSubCfg := C.get_subscriber_by_name(ctx.cfgMgr, C.CString(name))
	if goSubCfg == nil {
		return nil, errors.New("[Subscriber]: Failed to get subscriber by name from CGO\n")
	}
	subCtx.subCfg = goSubCfg
	return subCtx, nil
}

// Get the respective subscriber based on the index
//
// Returns:
// 1. SubscriberCfg : SubscriberCfg obj
//    SubscriberCfg instance
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetSubscriberByIndex(index int) (*SubscriberCfg, error) {
	subCtx := new(SubscriberCfg)
	goSubCfg := C.get_subscriber_by_index(ctx.cfgMgr, C.int(index))
	if goSubCfg == nil {
		return nil, errors.New("[Subscriber]: Failed to get subscriber by index from CGO\n")
	}
	subCtx.subCfg = goSubCfg
	return subCtx, nil
}

// Get the respective server based on the name
//
// Returns:
// 1. ServerCfg : ServerCfg obj
//    ServerCfg instance
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetServerByName(name string) (*ServerCfg, error) {
	serverCtx := new(ServerCfg)
	goServerCfg := C.get_server_by_name(ctx.cfgMgr, C.CString(name))
	if goServerCfg == nil {
		return nil, errors.New("[Server]: Failed to get publisher by name from CGO\n")
	}
	serverCtx.serverCfg = goServerCfg
	return serverCtx, nil
}

// Get the respective server based on the index
//
// Returns:
// 1. ServerCfg : ServerCfg obj
//    ServerCfg instance
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetServerByIndex(index int) (*ServerCfg, error) {
	serverCtx := new(ServerCfg)
	goServerCfg := C.get_server_by_index(ctx.cfgMgr, C.int(index))
	if goServerCfg == nil {
		return nil, errors.New("[Server]: Failed to get publisher by index from CGO\n")
	}
	serverCtx.serverCfg = goServerCfg
	return serverCtx, nil
}

// Get the respective client based on the name
//
// Returns:
// 1. ClientCfg : ClientCfg obj
//    ClientCfg instance
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetClientByName(name string) (*ClientCfg, error) {
	clientCtx := new(ClientCfg)
	goClientCfg := C.get_client_by_name(ctx.cfgMgr, C.CString(name))
	clientCtx.clientCfg = goClientCfg
	return clientCtx, nil
}

// Get the respective client based on the index
//
// Returns:
// 1. ClientCfg : ClientCfg obj
//    ClientCfg instance
// 2. error
//    Error on failure,  nil on success
func (ctx *ConfigMgr) GetClientByIndex(index int) (*ClientCfg, error) {
	clientCtx := new(ClientCfg)
	goClientCfg := C.get_client_by_index(ctx.cfgMgr, C.int(index))
	clientCtx.clientCfg = goClientCfg
	return clientCtx, nil
}

func (ctx *ConfigMgr) Destroy() {
	C.destroy_cfg_mgr(ctx.cfgMgr)
}

func (pubctx *PublisherCfg) getEndPoints() (string, error) {
	endpoints := C.get_pub_end_points(pubctx.pubCfg)
	if endpoints == nil {
		return "", errors.New("[Publisher]: Failed to get endpoint from CGO\n")
	}
	defer C.destroy_char_arr(endpoints)
	return C.GoString(endpoints.arr), nil
}

func (pubctx *PublisherCfg) getTopics() ([]string, error) {
	topicsArr := C.get_pub_topics(pubctx.pubCfg)
	if topicsArr == nil {
		return []string{""}, errors.New("[Publisher]: Failed to get topics from CGO\n")
	}
	defer C.destroy_string_arr(topicsArr)

	pubTopics := GoStrings(topicsArr.len, topicsArr.arr)
	return pubTopics, nil
}

func (pubctx *PublisherCfg) getAllowedClients() ([]string, error) {
	allowedClientsArr := C.get_pub_allowed_clients(pubctx.pubCfg)
	if allowedClientsArr == nil {
		return []string{""}, errors.New("[Publisher]: Failed to get allowed clients from CGO\n")
	}
	defer C.destroy_string_arr(allowedClientsArr)
	allowedClients := GoStrings(allowedClientsArr.len, allowedClientsArr.arr)
	return allowedClients, nil
}

func (pubctx *PublisherCfg) getMsgbusConfig() (string, error) {
	conf := C.get_pub_msgbus_config(pubctx.pubCfg)
	if conf == nil {
		return "", errors.New("[Publisher]: Failed to get message bus config from CGO\n")
	}

	cConfig := (*C.config_t)(conf)
	cConf := C.configt_to_char(cConfig)
	if cConf == nil {
		return "", errors.New("[Publisher] Converting configt to char failed")
	}

	goStr := C.GoString(cConf)
	defer C.free(unsafe.Pointer(cConf))
	return goStr, nil
}

func (pubctx *PublisherCfg) setTopics(topics []string) bool {
	topicsList := make([]*C.char, len(topics))
	for i, s := range topics {
		fmt.Println(s)
		cs := C.CString(s)
		defer C.free(unsafe.Pointer(cs))
		topicsList[i] = cs
	}

	topicsSet := C.set_pub_topics(pubctx.pubCfg, &topicsList[0], C.int(len(topics)))
	if topicsSet {
		return true
	} else {
		return false
	}
}

func (pubctx *PublisherCfg) getInterfaceValue(key string) (*ConfigValue, error) {
	interfaceVal := C.get_pub_interface_value(pubctx.pubCfg, C.CString(key))
	if interfaceVal == nil {
		return nil, errors.New("[Publisher]: Failed to get interface value from CGO\n")
	}
	defer C.destroy_config_val(interfaceVal)

	val, err := getConfigVal(interfaceVal)
	if err != nil {
		glog.Errorf("get config val failed")
		return nil, err
	}
	return val, err
}

func (pubctx *PublisherCfg) destroyPublisher() {
	C.destroy_publisher(pubctx.pubCfg)
}

func (subctx *SubscriberCfg) getEndPoints() (string, error) {
	endpoints := C.get_sub_end_points(subctx.subCfg)
	if endpoints == nil {
		return "", errors.New("[Subscriber]: Failed to get endpoint from CGO\n")
	}
	defer C.destroy_char_arr(endpoints)
	return C.GoString(endpoints.arr), nil
}

func (subctx *SubscriberCfg) getMsgbusConfig() (string, error) {
	conf := C.get_msgbus_config_subscriber(subctx.subCfg)
	if conf == nil {
		return "", errors.New("[Subscriber]: Failed to get message bus config from CGO\n")
	}

	cConfig := (*C.config_t)(conf)
	cConf := C.configt_to_char(cConfig)
	if cConf == nil {
		return "", errors.New("[Subscriber] Converting configt to char failed")
	}

	goStr := C.GoString(cConf)
	defer C.free(unsafe.Pointer(cConf))
	return goStr, nil
}

func (subctx *SubscriberCfg) getTopics() ([]string, error) {
	topicsArr := C.get_topics_subscriber(subctx.subCfg)
	if topicsArr == nil {
		return []string{""}, errors.New("[Subscriber]: Failed to get topics from CGO\n")
	}
	defer C.destroy_string_arr(topicsArr)

	subTopics := GoStrings(topicsArr.len, topicsArr.arr)
	return subTopics, nil
}

func (subctx *SubscriberCfg) setTopics(topics []string) bool {
	topicsList := make([]*C.char, len(topics))
	for i, s := range topics {
		fmt.Println(s)
		cs := C.CString(s)
		defer C.free(unsafe.Pointer(cs))
		topicsList[i] = cs
	}

	topicsSet := C.set_sub_topics(subctx.subCfg, &topicsList[0], C.int(len(topics)))
	if topicsSet {
		return true
	} else {
		return false
	}
}

func (subctx *SubscriberCfg) getInterfaceValue(key string) (*ConfigValue, error) {
	interfaceVal := C.get_sub_interface_value(subctx.subCfg, C.CString(key))
	if interfaceVal == nil {
		return nil, errors.New("[Subscriber]: Failed to get interface value from CGO\n")
	}
	defer C.destroy_config_val(interfaceVal)

	val, err := getConfigVal(interfaceVal)
	if err != nil {
		glog.Errorf("get config val failed")
		return nil, err
	}
	return val, err
}

func (subctx *SubscriberCfg) destroySubscriber() {
	C.destroy_subscriber(subctx.subCfg)
}

func (serverCtx *ServerCfg) getEndPoints() (string, error) {
	endpoints := C.get_server_end_points(serverCtx.serverCfg)
	if endpoints == nil {
		return "", errors.New("[Server]: Failed to get endpoint from CGO\n")
	}
	defer C.destroy_char_arr(endpoints)
	return C.GoString(endpoints.arr), nil
}

func (serverCtx *ServerCfg) getMsgbusConfig() (string, error) {
	conf := C.get_msgbus_config_server(serverCtx.serverCfg)
	if conf == nil {
		return "", errors.New("[Server]: Failed to get message bus config from CGO\n")
	}

	cConfig := (*C.config_t)(conf)
	cConf := C.configt_to_char(cConfig)
	if cConf == nil {
		return "", errors.New("[Server] Converting configt to char failed")
	}

	goStr := C.GoString(cConf)
	defer C.free(unsafe.Pointer(cConf))
	return goStr, nil
}

func (serverCtx *ServerCfg) getAllowedClients() ([]string, error) {
	allowedClientsArr := C.get_allowed_clients_server(serverCtx.serverCfg)
	if allowedClientsArr == nil {
		return []string{""}, errors.New("[Server]: Failed to get allowed clients from CGO\n")
	}
	defer C.destroy_string_arr(allowedClientsArr)

	allowedClients := GoStrings(allowedClientsArr.len, allowedClientsArr.arr)
	return allowedClients, nil
}

func (serverCtx *ServerCfg) getInterfaceValue(key string) (*ConfigValue, error) {
	interfaceVal := C.get_server_interface_value(serverCtx.serverCfg, C.CString(key))
	if interfaceVal == nil {
		return nil, errors.New("[Server]: Failed to get interface value from CGO\n")
	}
	defer C.destroy_config_val(interfaceVal)

	val, err := getConfigVal(interfaceVal)
	if err != nil {
		glog.Errorf("get config val failed")
		return nil, err
	}
	return val, err
}

func (serverCtx *ServerCfg) destroyServer() {
	C.destroy_server(serverCtx.serverCfg)
}

func (clientCtx *ClientCfg) getEndPoints() (string, error) {
	endpoints := C.get_client_end_points(clientCtx.clientCfg)
	if endpoints == nil {
		return "", errors.New("[Client]: Failed to get endpoint from CGO\n")
	}
	defer C.destroy_char_arr(endpoints)
	return C.GoString(endpoints.arr), nil
}

func (clientCtx *ClientCfg) getMsgbusConfig() (string, error) {
	conf := C.get_msgbus_config_client(clientCtx.clientCfg)
	if conf == nil {
		return "", errors.New("[Client]: Failed to get message bus config from CGO\n")
	}
	cConfig := (*C.config_t)(conf)
	cConf := C.configt_to_char(cConfig)
	if cConf == nil {
		return "", errors.New("[Client] Converting configt to char failed")
	}

	goStr := C.GoString(cConf)
	defer C.free(unsafe.Pointer(cConf))
	return goStr, nil
}

func (clientCtx *ClientCfg) getInterfaceValue(key string) (*ConfigValue, error) {
	interfaceVal := C.get_client_interface_value(clientCtx.clientCfg, C.CString(key))
	if interfaceVal == nil {
		return nil, errors.New("[Publisher]: Failed to get interface value from CGO\n")
	}
	defer C.destroy_config_val(interfaceVal)

	val, err := getConfigVal(interfaceVal)
	if err != nil {
		glog.Errorf("get config val failed")
		return nil, err
	}
	return val, err
}

func (clientCtx *ClientCfg) destroyClient() {
	C.destroy_client(clientCtx.clientCfg)
}
