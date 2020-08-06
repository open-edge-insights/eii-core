#include <stdio.h>
#include "db_client.h"
#include <stdlib.h>

void watch_cb(char *key, char *value, void *user_data){
    printf("watch callback is called...\n");
    printf("key: %s and value: %s \n", key, value);
    char *data = user_data;
    printf("userdata: %s", data);
}

void watch_prefix_cb(char *key, char *value, void *user_data){
    printf("watch_prefix callback is called...\n");
    printf("key: %s and value: %s \n", key, value);
    char *data = user_data;
    printf("userdata: %s", data);
}

int main(){ 
    db_client_t* db_client = create_etcd_client("localhost", "2379", "","","");
    printf("Before init...\n");
    // void *something = db_client->db_config;
    void *handle = db_client->init(db_client);

    char* val = db_client->get(handle, "/GlobalEnv/");
    printf("In main..., value: %s and len:%ld \n", val, strlen(val));

    int status = db_client->put(handle, "/test", "C hello world.....");
    if(status == 0){
        printf("put is successful\n");
    }

    db_client->watch(handle, "/Global", watch_cb, NULL);
    db_client->watch_prefix(handle, "/Global", watch_prefix_cb, (void*)"userdatawatchprefix");
    sleep(20);

    db_client_destroy(db_client);
    return 0;
}

