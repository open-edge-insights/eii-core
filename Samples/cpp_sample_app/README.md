## Sample CPP APP for EIS platform ##

# 'Short Description of an App containers'
   The sample CPP application demonstrates the usage of EIS core libraries like EISMessageBus and ConfigManager.
   In this APP, there is a publisher,subscriber,client and server, subscriber-client and publisher-server are running, inside independent
   containers (container names are ia_cpp_publisher, ia_cpp_subscriber).

   **NOTE**: ia_cpp_publisher container comprises of both publisher and server functionality.
             ia_cpp_subscriber container comprises of both subscriber and client functionality.


The high level logical flow of Sample App is as below:
   
   1. Publisher contacts to ETCD service, using the [env_config](../../common/libs/ConfigManager/c/src/env_config.c)
      library. It fetches it's private key and allowed client's public keys. Then it creates
      the publisher object and starts publishing the sample data at given topic
      (topic name : publish_test).

   2. Server contacts to ETCD service, and fetches it's private key and allowed client's public keys.
      Then it creates a server object according to given service name and waits for the client's request.
      
      As mentioned in **NOTE** the publisher and server are running inside ia_cpp_publisher container.

   3. Subscriber contacts to ETCD service using the [env_config](../../common/libs/ConfigManager/c/src/env_config.c)
      library. It fetches it's private and public key and public key of publisher. It then creates
      the subscriber object (which subscribes to the same topic : publish_test) and start receiving
      the data and prints the same onto a console.

   4. Client contacts to ETCD service, and fetches its private key and public key of server. Then
      it creates a client object and sends a request to the server and gets the response back.

     As mentioned in **NOTE** the subscriber and client are running inside ia_cpp_publisher container.

## `Configuration`
1. Please replace the

   [docker-compose.yml](../../docker-compose.yml) with
   [docker-compose.yml](./docker-compose.yml)

   And

   [etcd_pre_load.json](../../provision/config/etcd_pre_load.json) with
   [etcd_pre_load.json](./config/etcd_pre_load.json)

2. Do provisioning and start containers.

   ```sh
   $ cd build/provision
   $ sudo ./provision_eis.sh ../docker-compose.yml
   $ cd ..
   ```

   ```sh
   $ docker-compose build
   $ docker-compose up -d
   $ docker logs -f ia_cpp_publisher
   $ docker logs -f ia_cpp_subscriber
   ```


