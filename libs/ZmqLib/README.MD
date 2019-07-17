# Using the ZMQ python temp library as a docker container :

1. Generate certs required by PUB-SUB & REQ-REP from [ZmqLib/py/test/generate_certificates.py](ZmqLib/py/test/generate_certificates.py)

    ```sh
    python3.6 generate_certificates.py
    ```

2. Build the PUB-SUB, REQ-REP & etcd containers using docker-compose

    ```sh
    docker-compose build
    ```

3. Have etcd running as a container by running this command in a separate terminal

    ```sh
    docker-compose run ia_etcd
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
