// Copyright (c) 2020 Intel Corporation.
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
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @brief ConfigManager getvalue usage example
 */

#include "eis/config_manager/config_mgr.h"

using namespace eis::config_manager;

int main(){

    setenv("KVStore", "etcd", 1);

    // In a dockerized environment,
    // these variables are set in environment
    setenv("DEV_MODE", "FALSE", 1);
    // Replace 2nd parameter with path to certs
    setenv("CONFIGMGR_CERT", "", 1);
    setenv("CONFIGMGR_KEY", "", 1);
    setenv("CONFIGMGR_CACERT", "", 1);
    
    // Uncomment below lines to test DEV mode
    // setenv("DEV_MODE", "TRUE", 1);
    // setenv("CONFIGMGR_CERT", "", 1);
    // setenv("CONFIGMGR_KEY", "", 1);
    // setenv("CONFIGMGR_CACERT", "", 1);

    setenv("AppName","VideoIngestion", 1);
    ConfigMgr* ch = new ConfigMgr();
    AppCfg* cfg = ch->getAppConfig();

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