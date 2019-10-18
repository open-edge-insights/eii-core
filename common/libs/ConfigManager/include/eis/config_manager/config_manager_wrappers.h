#ifdef __cplusplus
extern "C" {
#endif
typedef void (*callback_fcn)(char *key, char *value);

char* init(const char *storage_type, const char *ca_cert, const char *cert_file, const char *key_file);
char* get_config(const char *key);
void register_watch_dir(const char *key, callback_fcn user_callback);
void register_watch_key(const char *key, callback_fcn user_callback);

#ifdef __cplusplus
} //end extern "C"
#endif
