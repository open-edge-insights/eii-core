#include "eis/config_manager/config_mgr.h"

using namespace eis::config_manager;

int main(){

    setenv("DEV_MODE", "FALSE", 1);
    setenv("AppName","VideoIngestion", 1);
    ConfigMgr* g_ch = new ConfigMgr();    
    AppCfg* cfg = g_ch->getAppConfig();

    printf("========================================\n");
    config_value_t* app_config = cfg->getValue("max_workers");
    if (app_config->type != CVT_INTEGER){
        printf("Max_worker is not integer"); 
        exit(1); 
    }
    int max_workers = app_config->body.integer;
    printf("max_workers value is %d\n", max_workers);

    app_config = cfg->getValue("max_jobs");
    if (app_config->type != CVT_FLOATING){
        printf("max_jobs is not float"); 
        exit(1);     
    }
    float max_jobs = app_config->body.floating;
    printf("max_jobs value is %.2f\n", max_jobs);  

    app_config = cfg->getValue("string");
    if (app_config->type != CVT_STRING){
        printf("string type is is not string"); 
        exit(1);   
    } 
    char* string_value = app_config->body.string;
    printf("string value is %s\n", string_value); 

    app_config = cfg->getValue("loop_video");
    if (app_config->type != CVT_BOOLEAN){
        printf("loop_video type is is not boolean"); 
        exit(1);     
    }
    bool loop_video = app_config->body.boolean;
    (loop_video) ? printf("loop_video is true\n") : printf("loop_video is false\n");  

    app_config = cfg->getValue("ingestor");
    if (app_config->type != CVT_OBJECT){
        printf("ingestor type is is not object"); 
        exit(1);  
    }
    config_value_t* ingestor_type = config_value_object_get(app_config, "type");
    char* type = ingestor_type->body.string;
    printf("ingestor_type value is %s\n", type);  

    app_config = cfg->getValue("udfs");
    if (app_config->type != CVT_ARRAY){
        printf("udfs type is is not array"); 
        exit(1);      
    }
    config_value_t* udf = config_value_array_get(app_config, 0);
    config_value_t* udf_type = config_value_object_get(udf, "type");
    type = udf_type->body.string;
    printf("udf_type value is %s\n", type); 

    printf("========================================\n");

}