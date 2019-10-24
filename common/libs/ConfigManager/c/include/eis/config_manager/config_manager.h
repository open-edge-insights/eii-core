#ifdef __cplusplus
extern "C" {
#endif
typedef void (*callback_fcn)(char *key, char *value);

char* init(char *storage_type, char *ca_cert, char *cert_file, char *key_file);
char* get_config(char *key);
void register_watch_dir(char *key, callback_fcn user_callback);
void register_watch_key(char *key, callback_fcn user_callback);

#ifdef __cplusplus
} //end extern "C"
#endif
