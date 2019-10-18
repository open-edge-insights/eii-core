#include<stdio.h>
#include<unistd.h>
#include "eis/config_manager/config_manager_wrappers.h"

void callback(char* key, char * val){
   printf("callback is called\n");
   printf("key: %s and value: %s\n", key, val);
}

void test_callback(char* key, char * val){
   printf("test_callback callback is called\n");
   printf("key: %s and value: %s\n", key, val);
}

int main() {
   char *err_status = init("etcd","", "", "");
   if(!strcmp(err_status, "-1")) {
      printf("Config manager initializtaion failed: %s\n", err_status);
      return -1;
   }

   printf("get_config is called, value is: %s \n", get_config("/GlobalEnv/"));
   register_watch_key("/Kapacitor", callback);
   register_watch_dir("/Kapacitor/", test_callback);
   sleep(35);

   return 0;
}
