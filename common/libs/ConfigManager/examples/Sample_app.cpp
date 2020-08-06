#include "eis/config_manager/config_handler.h"

#include <stdio.h>
using namespace eis::config_manager;

int main(){

    ConfigHandler* g_ch = new ConfigHandler();
    
    
    // CfgMgr* m_ctx = g_ch->getAppConfig();

    // config_value_t* value = m_ctx->get_value("ingestor");

    // if (value->type == CVT_STRING){
    //     fprintf(stderr, "value is %s\n ", value->body.string);
    // }else if (value->type == CVT_INTEGER){
    //     fprintf(stderr, "value is %d\n ", value->body.integer);
    // }else if (value->type == CVT_FLOATING){
    //     fprintf(stderr, "value is %.2f\n ", value->body.floating);
    // }else if (value->type == CVT_BOOLEAN){
    //     fprintf(stderr, "its bool \n ");
    // }else if (value->type == CVT_OBJECT){
        

 
    //     config_value_t* queue_size = config_value_object_get(value,
    //                                                                 "queue_size");

    //     int queue_size_value = queue_size->body.integer;
    //     fprintf(stderr,"QS is %d\n", queue_size_value); 




         CfgMgr* server_ctx = g_ch->getServerByIndex(0);
         config_t* server_msgbus_config = server_ctx->get_server_msgbus_cfg();

         CfgMgr* client_ctx = g_ch->getClientByIndex(0);
         config_t* client_msgbus_config = server_ctx->get_client_msgbus_cfg();


    }
    
