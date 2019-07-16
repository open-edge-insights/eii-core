# Using the ZMQ python temp library:

1. Generate certs required by PUB-SUB & REQ-REP

    ```sh
    python3.6 generate_certificates.py
    ```

2. Have etcd running as a service from docker image by running this command in a separate terminal

    ```sh
    docker run -p 2379:2379 --name etcd quay.io/coreos/etcd:v3.0.16 /usr/local/bin/etcd -advertise-client-urls http://0.0.0.0:2379 -listen-client-urls http://0.0.0.0:2379
    ```

3. Run publisher for PROD mode

    ```sh
    python3.6 publisher_test.py --public-key <Path to public keys directory> --private-key <Path to private keys directory> --save-to-disk <True/False whether images are to be saved to disk>
    ```

4. Run subscriber for PROD mode

    ```sh
    python3.6 subscriber_test.py --public-key <Path to public keys directory> --private-key <Path to private keys directory> --save-to-disk <True/False whether images are to be saved to disk>
    ```

5. Run publisher for DEV mode

    ```sh
    python3.6 publisher_test.py --save-to-disk <True/False whether images are to be saved to disk>
    ```

6. Run subscriber for DEV mode

    ```sh
    python3.6 subscriber_test.py --save-to-disk <True/False whether images are to be saved to disk>
    ```

7. Run responder for PROD mode

    ```sh
    python3.6 responder_test.py --public-key <Path to public keys directory> --private-key <Path to private keys directory> --save-to-disk <True/False whether images are to be saved to disk>
    ```

8. Run requester for PROD mode

    ```sh
    python3.6 requester_test.py --public-key <Path to public keys directory> --private-key <Path to private keys directory> --save-to-disk <True/False whether images are to be saved to disk>
    ```

9. Run responder for DEV mode

    ```sh
    python3.6 responder_test.py --save-to-disk <True/False whether images are to be saved to disk>
    ```

10. Run requester for DEV mode

    ```sh
    python3.6 requester_test.py --save-to-disk <True/False whether images are to be saved to disk>
    ```

> Note: Please have pcb_d2000.avi video file copied to current directory