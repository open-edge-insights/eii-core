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
 * @file
 * @brief ConfigHandler Implementation
 */

#include <iostream>
#include <safe_lib.h>
#include "eis/config_manager/config_handler.h"
#include "eis/config_manager/env_ctx_base.h"
#include "db_client.h"
#include <cjson/cJSON.h>

// using namespace eis::utils;
using namespace eis::config_manager;



ConfigHandler::ConfigHandler() {
        LOG_INFO_0("I am in ConfigHandler constructor");
        fprintf(stderr,"I am in ConfigHandler constructor \n"); 
        // Contact Etcd & update all vars

    //     // m_app_config;
    //       static char* m_app_config = NULL;
    // char* appname = getenv("AppName");
    
    // // appmame will be used to get the config
    db_client_t* db_client = create_etcd_client("localhost", "2379", "", "", "");
    // void *handle = init(db_client->db_config);

    // m_app_config = db_client->get(handle, "/VideoIngestion/config");

    // // return m_app_config;


    void *handle = db_client->init(db_client);

    char* interface = db_client->get(handle, "/GlobalEnv/");
    printf("In main..., value: %s and len:%ld \n", interface, strlen(interface));
    
    char* value = db_client->get(handle, "/Visualizer/config");
    printf("In main..., value: %s and len:%ld \n", value, strlen(value));
    
    

    // todo: integrate with New etcd plugins
    //todt: based on the appname, fetch from the etcd
    // config_mgr_t* config_mgr_client = NULL;
  
    
    // config_mgr_client = config_mgr_new("etcd", "", "", "");

    // char* value = config_mgr_client->get_config("/VideoIngestion/config");
    // char* interface = config_mgr_client->get_config("/VideoIngestion/interface");

    // fprintf(stderr, "VI interface is : %s\n", interface);
    m_app_config = json_config_new_from_buffer(value);
    m_app_interface = json_config_new_from_buffer(interface);


}

CfgMgr::CfgMgr():m_conf(m_app_config),m_intfc(m_app_interface){
    fprintf(stderr,"in mainctx class \n"); 
}

// BaseCtx* ConfigHandler::getPublisher() {
void ConfigHandler::getPublisher() {
    LOG_INFO_0("ConfigHandler getPublisher  method");
    fprintf(stderr,"ConfigHandler getPublisher  method\n"); 
    // BaseCtx* app_context = this->getAppConfig("interface");
    // // BaseCtx* basectx = new BaseCtx(app_context->m_interface);
    // BaseCtx* pubCtx = new CtxPublisher(app_context->m_config);
    // basectx->get_BaseCtx("Publisher");
    
    // return pubCtx;

}


// BaseCtx* ConfigHandler::getAppConfig(char* type) {
CfgMgr* ConfigHandler::getAppConfig() {
    LOG_INFO_0("ConfigHandler getAppConfig  method");
    fprintf(stderr,"ConfigHandler getAppConfig  method\n"); 

    
    CfgMgr* val = new CfgMgr();
    // // basectx1->get_BaseCtx("AppConfig");
    // basectx1->m_config = (implementaion to get data);
    // // Contacting ETCD & return config/interface
    return val;

}



CfgMgr* ConfigHandler::getServerByIndex(int index) {
    LOG_INFO_0("ConfigHandler getAppConfig  method");
    fprintf(stderr,"ConfigHandler get server by index  method\n"); 

    
    CfgMgr* val = new CfgMgr();
     server_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Server");

    val->server_cfg = config_value_array_get(server_interface, index);

    return val;

}

CfgMgr* ConfigHandler::getClientByIndex(int index) {
    LOG_INFO_0("ConfigHandler getAppConfig  method");
    fprintf(stderr,"ConfigHandler get client by index  method\n"); 

    
    CfgMgr* val = new CfgMgr();
     client_interface = m_app_interface->get_config_value(m_app_interface->cfg, "Client");

    val->client_cfg = config_value_array_get(client_interface, index);

    return val;

}


void ConfigHandler::getSubscriber() {
    LOG_INFO_0("ConfigHandler getSubscriber  method");
}


config_value_t* CfgMgr::get_value(char* key){
    fprintf(stderr,"in Get value\n");

    config_value_t* value = m_conf->get_config_value(m_conf->cfg,key);
    return value;

}



config_t* CfgMgr::get_server_msgbus_cfg(){
    fprintf(stderr,"get_server_msgbus_cfg\n");

    

    config_value_t* server_type = config_value_object_get(server_cfg, "Type");
    char* type = server_type->body.string;
    cJSON* json = cJSON_CreateObject(); 
    cJSON_AddStringToObject(json, "type", type);
    config_value_t* server_json_endpoint = config_value_object_get(server_cfg, "EndPoint");
    char* EndPoint = server_json_endpoint->body.string;

    if(!strcmp(type, "zmq_tcp")){

        config_value_t* allowed_clients = config_value_object_get(server_cfg, "AllowedClients");

        cJSON* server_topic = cJSON_CreateObject();
        cJSON* all_clients = cJSON_CreateArray();

        config_value_t* array_value; 

        for (int i =0; i < config_value_array_len(allowed_clients); i++){
            array_value = config_value_array_get(allowed_clients, i);
            cJSON_AddItemToArray(all_clients, cJSON_CreateString(array_value->body.string));
        }

        cJSON_AddItemToObject(json, "allowed_clients",  all_clients);

        config_value_t* name = config_value_object_get(server_cfg, "Name");
        char* ser_name = name->body.string;
        cJSON_AddStringToObject(server_topic, "host", EndPoint);
        cJSON_AddNumberToObject(server_topic, "port", 123);
        cJSON_AddStringToObject(server_topic, "server_secret_key", "Appname_secret_key");
        cJSON_AddItemToObject(json, ser_name,  server_topic);

    }

        char* config_value = cJSON_Print(json);
        fprintf(stderr,"Env publish Config is : %s \n", config_value);

            config = config_new(
            (void*) json, free_json, get_config_value);
            if (config == NULL) {
                LOG_ERROR_0("Failed to initialize configuration object");
                return NULL;
            }

    return config;
}


config_t* CfgMgr::get_client_msgbus_cfg(){

    config_value_t* clientappname = config_value_object_get(client_cfg, "AppName");
     fprintf(stderr,"1 \n");
    char* appname_ = clientappname->body.string;

    config_value_t* client_json_type = config_value_object_get(client_cfg, "Type");

    char* type = client_json_type->body.string;
    cJSON* json_1 = cJSON_CreateObject(); 
    cJSON_AddStringToObject(json_1, "type", type);

    config_value_t* client_json_endpoint = config_value_object_get(client_cfg, "EndPoint");
    char* EndPoint = client_json_endpoint->body.string;

    if(!strcmp(type, "zmq_tcp")){

        cJSON* client_app = cJSON_CreateObject();
        cJSON_AddStringToObject(client_app, "host", EndPoint);
        cJSON_AddNumberToObject(client_app, "host", 1234);
        cJSON_AddStringToObject(client_app, "server_secret_key", "appname_ privatekey");
        cJSON_AddStringToObject(client_app, "client_secret_key", "client app privatekey");
        cJSON_AddStringToObject(client_app, "client_public_key", "client app publickey");
        cJSON_AddItemToObject(json_1, appname_, client_app);
        
    }

  char* config_value_1 = cJSON_Print(json_1);
    fprintf(stderr,"Env Config client is : %s \n", config_value_1);

}







ConfigHandler::~ConfigHandler() {
    // if(m_app_name) {
    //     delete m_app_name;
    // }
    // Stop the thread (if it is running)
    LOG_INFO_0("ConfigHandler destructor");
}
