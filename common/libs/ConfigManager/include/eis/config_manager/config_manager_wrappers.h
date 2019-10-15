#include<string.h>

class ConfigManager{
    public:
        ConfigManager(std::string storage_type, std::string ca_cert, std::string cert_file, std::string key_file);
        char * GetConfig(std::string key);
        void RegisterWatchKey(std::string key,void (*callback_fcn)(char* key, char* value));
        void RegisterWatchDir(std::string key,void (*callback_fcn)(char* key, char* value));
};
