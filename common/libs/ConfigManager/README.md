# ConfigManager

ConfigManager provides go/py APIs for distributed key-value store like `etcd` to read EIS related config details.

# Python APIs

1. **get_config_client(storage_type, config)**
        
   ```
   Returns config manager client based on storage_type
   
   :param storage_type: Type of key-value storage, Eg. etcd
   :param config: config of type Dict with certFile, keyFile and trustFile
   ```

To refer Python examples on etcd client wrapper follow [python/test](python/test)

# Go APIs

1. **Init(storageType string, config map[string]string)**
        
    ```
    Initializes config manager and returns config manager client instance by calling 
    GetConfigClient API, With the client instance, respective storage_type apis can be called
    
    :param storage_type: Type of key-value storage, Eg. etcd
    :param config: config of type Dict with certFile, keyFile and trustFile
    :return config manager client if creation of client is successful, nil on failure
    ```

2. **GetConfigClient(storageType string, conf map[string]string)**
        
    ```
    Returns config manager client based on storage_type
    
    :param storage_type: Type of key-value storage, Eg. etcd
    :param config: config of type Dict with certFile, keyFile and trustFile
    ```
        
To refer GO examples on etcd client wrapper follow [go/README](go/README)

# C APIs

To refer C API documentation, follow [c/README.md](c/README.md)
