# Using the ZMQ go temp library:

1. Generate certs required by PUB-SUB & REQ-REP

    ```sh
    go run generateCertificates.go
    ```

2. Run publisher for PROD mode

   ```sh
   go run publisherTest.go --public_keys <Path to public keys directory> --private_keys <Path to private keys directory>
   ```

3. Run subscriber for PROD mode

   ```sh
   go run subscriberTest.go --public_keys <Path to public keys directory> --private_keys <Path to private keys directory>
   ```

4. Run publisher for DEV mode

   ```sh
   go run publisherTest.go
   ```

5. Run requester for DEV mode

   ```sh
   go run requesterTest.go
   ```

2. Run responder for PROD mode

   ```sh
   go run responderTest.go --public_keys <Path to public keys directory> --private_keys <Path to private keys directory>
   ```

3. Run subscriber for PROD mode

   ```sh
   go run subscriberTest.go --public_keys <Path to public keys directory> --private_keys <Path to private keys directory>
   ```

4. Run responder for DEV mode

   ```sh
   go run responderTest.go
   ```

5. Run requester for DEV mode

   ```sh
   go run requesterTest.go
   ```

> Note: Please have a sample image named pic2.jpg copied to current directory.