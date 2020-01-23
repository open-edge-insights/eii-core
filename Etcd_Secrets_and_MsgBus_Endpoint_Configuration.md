# `EIS Etcd Secrets and MsgBus Endpoints Configuration`

## `Etcd Secrets Configuration`

The below are the ENV's that should be present in the environment section
of every service in [docker-compose.yml](build/docker-compose.yml):

```
  [service_name]:
    ...
    environment:
        AppName: "app_name" # required if a unique first level key `/[AppName]` is needed in etcd
    secrets:  # This section needs to be commented out when run in DEV mode. Only needed for PROD mode
        - etcd_ca_certificate
        - app_public_certificate_for_etcd
        - app_private_key_for_etcd
```

Etcd secrets(certs) for all the services of docker compose file will be generated automatically during provisioning.
Post provisioning this secrets should be mentioned in following two places for each service.

1) in the `secrets` section at the bottom of [docker-compose.yml](build/docker-compose.yml):
    ```
    secrets:
      etcd_ca_certificate:
        file: [path_to_pem_file]
      app_public_certificate_for_etcd:
        file: [path_to_pem_file]
      app_private_key_for_etcd:
        file: [path_to_pem_file]
    ```
2) In the `secrets` section in for each service.

> **NOTE**: The secret name must follow proper naming convention and file path as below
   ```

    etcd_<AppName>_cert:
      file: provision/Certificates/<AppName>/<AppName>_client_certificate.pem
    etcd_<AppName>_key:
      file: provision/Certificates/<AppName>/<AppName>_client_key.pem
   
   ```
  For example, if the AppName is `VideoIngestion1`, secrets must be mentioned as below

  ```
  etcd_VideoIngestion1_cert:
    file: provision/Certificates/VideoIngestion1/VideoIngestion1_client_certificate.pem
  etcd_VideoIngestion1_key:
    file: provision/Certificates/VideoIngestion1/VideoIngestion1_client_key.pem

  ```

  ---
  **NOTE**:
  * `Telegraf` service must use `InfluxDBConnector` secrets as it needs access to InfluxDBConnector certs and configs. To enable this please mention InfluxDBConnector AppName as `InfluxDBAppName` in environment section of Telegraf app. Also use secrets of InfluxDBConnector app for Telegraf Service. A seperate certificates for Telegraf app is also generated during provisioning and can be used if needed for storing Telegraf service related configurations.

  * `socket_dir_name` where unix socket files are created. If `<protocol>` is
    `zmq/tcp`, then `<endpoint>` has to be the combination of `<hostname>:<port>`.
  
  ---



## `Messagebus Endpoints Configuration`

The below are the ENV's that should be present in the environment section
of every service in [docker-compose.yml](build/docker-compose.yml). All services needs
to have all or section of the below ENV's based on if the service is a
publisher/subscriber/server/client type.

```
  [service_name]:
    ...
    environment:
      # list of apps which act as messagebus clients or subscribers
      # Based on this, this app/service will get the messagebus public key of the receiving app
      # This enables access control and only these clients will be able to subscribe
      # to streams published by this app
      Clients: "AppName1,AppName2,..."
      
      # Only required in PROD mode, ignored in DEV mode. zmq is required to generate zmq public keys
      CertType: "zmq"
      
      # Needed if the app/service acts as a publisher that creates messagebus PUB socket
      # Single or list of comma separated streams based on app/service capability
      PubTopics: "pub_stream"
      # Each stream mentioned above should have its own `stream_name_cfg` ENV
      pub_stream_cfg: "<protocol>,<endpoint>"

      # Needed if the app/service acts as a subcriber that creates messagebus SUB socket
      # Single or list of comma separated streams based on app/service capability
      # The `AppName` in SubTopics stream would help in fetching the messagebus
      # public key of the publishing app
      SubTopics: "[AppName1]/sub_stream1,[AppName2]/sub_stream2..."
      # Each stream mentioned above should have its own `stream_name_cfg` ENV
      sub_stream1_cfg: "<protocol>,<endpoint>"
      sub_stream2_cfg: "<protocol>,<endpoint>"

      # Needed if the app/service acts as a server(request-response model) that
      # creates messagebus REP socket to serve the data based on the request
      # Eg: InfluxDBConnector and ImageStore apps use this to provide historical data access
      Server: "<protocol>,<endpoint>"
```

> **NOTE**: If `<protocol>` is `zmq/ipc`, then `<endpoint>` has to be the
> `socket_dir_name` where unix socket files are created. If `<protocol>` is
> `zmq/tcp`, then `<endpoint>` has to be the combination of `<hostname>:<port>`.


## `Other X509 Certificates`
If any of the Services needs to generate other X509 Certificates(pem or der format), please use below option and provisioining will generate certificates and put them in ETCD key for the App.

```
  [service_name]:
    ...
    environment:
      
      # Only required in PROD mode
      CertType: "pem" or "der"
  
  ```

