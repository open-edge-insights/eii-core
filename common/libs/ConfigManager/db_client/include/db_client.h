#include "etcd_client_plugin.h"

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        void *db_config;
        void* (*init)(void *db_config);
        char* (*get) (void *handle, char *key);
        int (*put) (void *handle, char *key, char *value);
        void (*watch) (void *handle, char *key, void (*callback)(char *key, char *value, void *cb_user_data), void *user_data);
        void (*watch_prefix) (void *handle, char *key, void (*callback)(char *key, char *value, void *cb_user_data), void *user_data);
} db_client_t;

// void* init(void *db_config);
void db_client_destroy(db_client_t *config_mgr_config);

// Add the datastore plugins
db_client_t* create_etcd_client(char *hostname, char *port, char *cert_file, char *key_file, char *ca_cert_file);

#ifdef __cplusplus
}
#endif

#endif
