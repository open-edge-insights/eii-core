
#ifndef ETCD_CLIENT_PLUGIN_H
#define ETCD_CLIENT_PLUGIN_H 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char *hostname;
	char *port;
	char *cert_file;
        char *key_file;
        char *ca_cert_file;
        char *storage_type;
} etcd_config_t;

void* etcd_init(void *etcd_client);
char* etcd_get(void *handle, char *key);
int etcd_put(void *handle, char *key, char *value);
void etcd_watch(void *handle, char *key_test, void (*user_cb)(char* watch_key, char* val, void *cb_user_data), void *user_data);
void etcd_watch_prefix(void *handle, char *key_test, void (*user_cb)(char *watch_key, char *val, void *cb_user_data), void *user_data);

#ifdef __cplusplus
}
#endif

#endif