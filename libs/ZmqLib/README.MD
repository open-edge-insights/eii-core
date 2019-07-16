# Using the ZMQ python temp library as a docker container :

1. Generate certs required by PUB-SUB & REQ-REP from [ZmqLib/py/test/generate_certificates.py](ZmqLib/py/test/generate_certificates.py)

    ```sh
    python3.6 generate_certificates.py
    ```

2. Have etcd running as a service from docker image by running this command in a separate terminal

    ```sh
    docker run -p 2379:2379 --name etcd quay.io/coreos/etcd:v3.0.16 /usr/local/bin/etcd -advertise-client-urls http://0.0.0.0:2379 -listen-client-urls http://0.0.0.0:2379
    ```

3. Build both the PUB-SUB & REQ-REP containers using docker-compose

    ```sh
    docker-compose build
    ```

4. Run publisher

    ```sh
    docker-compose run ia_zmq_pub
    ```

5. Run subscriber

    ```sh
    docker-compose run ia_zmq_sub
    ```

6. Run responder

    ```sh
    docker-compose run ia_zmq_rep
    ```

7. Run requester

    ```sh
    docker-compose run ia_zmq_req
    ```
