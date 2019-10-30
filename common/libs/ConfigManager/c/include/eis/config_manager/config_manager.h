#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* status;

    /**
     * get_config function gets the value of a key from config manager
     *
     * @param key       - key to be queried on from config manager
     * @return char*    - values returned from config manager based on key
     */
    char* (*get_config)(char *key);

    /**
     * register_watch_key function registers to a callback and keeps a watch on a specified key
     * 
     * @param key                                                 - key to keep a watch on
     * @param (*register_watch_dir_cb)(char* key, char* value)    - user callback to be called on watch event 
     *                                                              with updated value on the respective key
     */
    void (*register_watch_key)(
            char *key, 
            void (*register_watch_dir_cb)(char *key, char *value));

    /**
     * register_watch_dir function registers to a callback and keeps a watch on the prefix of a specified key
     *
     * @param key                                                 - prefix of a key to keep a watch on
     * @param (*register_watch_dir_cb)(char* key, char* value)    - user callback to be called on watch event
     *                                                              with updated value on the respective key
     */
    void (*register_watch_dir)(
            char *key, 
            void (*register_watch_dir_cb)(char *key, char *value) );
    
    /**
     * free_config function to free memory
     */
    void (*free_config)(void *config);
} config_mgr_t;

typedef struct {
    char* storage_type;
    char* ca_cert;
    char* cert_file;
    char* key_file;
} config_mgr_config_t;

/**
 * config_mgr_new function to creates a new config manager client
 *  @param config_mgr_config_t:
 *       storage_type      - Type of key-value storage, Eg. etcd
 *       ca_cert           - config manager client ca cert
 *       cert_file         - config manager client cert file
 *       key_file          - config manager client key file
 *  @return char*          - on success of initialization "0", on failure "-1"
 */
config_mgr_t* config_mgr_new(config_mgr_config_t *config_mgr_config);

#ifdef __cplusplus
} //end extern "C"
#endif
