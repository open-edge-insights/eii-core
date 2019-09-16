# `EIS Etcd and MsgBus Endpoints Configuration`

## `Etcd Configuration`

The below are the ENV's that should be present in the environment section
of every service in [docker-compose.yml](docker_setup/docker-compose.yml):

```
  [service_name]:
    ...
    environment:
        AppName: "app_name" # required if a unique first level key `/[AppName]` is needed in etcd
        CertType: "[cert_type]" # Only required in PROD mode, ignored in DEV mode
                                # list of certs to be created: "zmq,der,pem".
                                # zmq is required to generate zmq public keys
                                # der is required for apps like OpcuaExport which needs pub/pri cert in binary format
                                # pem is required for apps like InfluxDBConnector, DataAnalytics, Telegraf etc.,
    secrets:  # This section needs to be commented out when run in DEV mode. Only needed for PROD mode
        - etcd_ca_certificate
        - app_public_certificate_for_etcd
        - app_private_key_for_etcd
```

All the etcd secrets for every service should be mentioned in the `secrets`
section in [docker-compose.yml](docker_setup/docker-compose.yml):
```
 secrets:
   etcd_ca_certificate:
     file: [path_to_pem_file]
   app_public_certificate_for_etcd:
     file: [path_to_pem_file]
   app_private_key_for_etcd:
     file: [path_to_pem_file]
```

## `Messagebus Endpoints Configuration`

The below are the ENV's that should be present in the environment section
of every service in [docker-compose.yml](docker_setup/docker-compose.yml). All services needs
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


