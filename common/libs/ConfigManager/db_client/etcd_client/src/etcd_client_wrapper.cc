#include <cstdlib>
#include "db_client.h"
#include "etcd_client.h"

#ifdef __cplusplus
extern "C" {
#endif

void* etcd_init(void *etcd_client) {
    EtcdClient *etcd_cli = NULL;
    db_client_t *db_client = static_cast<db_client_t *>(etcd_client);
    etcd_config_t *etcd_config = static_cast<etcd_config_t *>(db_client->db_config);
    std::string host = etcd_config->hostname;
    std::string port = etcd_config->port;
    // std::string cert_file = etcd_config->cert_file;
    // std::string key_file = etcd_config->key_file ;
    // std::string ca_file = etcd_config->ca_cert_file;

    if(strcmp(etcd_config->cert_file, "") && strcmp(etcd_config->key_file, "") && strcmp(etcd_config->ca_cert_file, "")){
        std::cout << "Running in prod mode\n";
        etcd_cli = new EtcdClient(host, port,  etcd_config->cert_file, etcd_config->key_file, etcd_config->ca_cert_file);
    } else{
        std::cout << "Running in dev mode\n";
        etcd_cli = new EtcdClient(host, port);
    }

    std::cout << "Size of EtcdClient:" << sizeof(EtcdClient) << std::endl;
    return etcd_cli;
}

char* etcd_get(void *handle, char *key){
    std::cout << "C wrapper:In get....\n";
    std::string str_key = key;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    std::string str_val = cli->get(str_key);
    // char *val = const_cast<char*>(cli->get(str_key));
    char *value = const_cast<char*>(str_val.data());
    // char *val = &*str_val.begin();
    char* val = (char *)malloc(strlen(value));
    strcpy(val, value);
    // free(val);
    // std::cout << val << "\n val len:" << strlen(val) << std::endl;
    return val;
}

int etcd_put(void *handle, char *key, char *value){
    std::string str_key = key;
    std::string str_value = value;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    int status = cli->put(str_key, str_value);
    // std::cout << "put status:" << status << std::endl;
    return status;
}

void etcd_watch(void *handle, char *key, void (*user_cb)(char* watch_key, char* val, void *cb_user_data), void *user_data){
    std::string str_key = key;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    cli->watch(str_key, user_cb, user_data);
}

void etcd_watch_prefix(void *handle, char *key, void (*user_cb)(char* watch_key, char* val, void *cb_user_data), void *user_data){
    std::string str_key = key;
    EtcdClient *cli = static_cast<EtcdClient *>(handle);
    cli->watch_prefix(str_key, user_cb, user_data);
}

#ifdef __cplusplus
}
#endif
