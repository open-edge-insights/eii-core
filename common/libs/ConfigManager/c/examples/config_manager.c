#include<stdio.h>
#include <string.h>
#include<unistd.h>
#include "eis/config_manager/config_manager.h"

void callback(char* key, char * val){
   printf("callback is called\n");
   printf("key: %s and value: %s\n", key, val);
}

void test_callback(char* key, char * val){
   printf("test_callback callback is called\n");
   printf("key: %s and value: %s\n", key, val);
}

int main() {
   config_mgr_config_t *config_mgr_config = (config_mgr_config_t *)malloc(sizeof(config_mgr_config_t));
   config_mgr_config->storage_type = "etcd";
   config_mgr_config->ca_cert = "";
   config_mgr_config->cert_file = "";
   config_mgr_config->key_file = "";

   config_mgr_t *config_mgr_client = config_mgr_new(config_mgr_config);
   if (!strcmp(config_mgr_client->status, "-1")){
      printf("Config manager client creation failed\n");
      return;
   }
   char *value = config_mgr_client->get_config("/GlobalEnv/");
   printf("get_config is called, value is: %s \n", value);
   config_mgr_client->register_watch_key("/GlobalEnv/", callback);
   config_mgr_client->register_watch_dir("/Kapacitor/", test_callback);
   config_mgr_client->free_config(config_mgr_client);
   config_mgr_client->free_config(config_mgr_config);
   sleep(35);
   return 0;
}
