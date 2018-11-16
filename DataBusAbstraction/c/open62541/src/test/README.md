# Compilation steps (current directory: <repo>/DataBusAbstraction/c/src):

```sh
gcc -std=c99 -I../include test/open62541_test.c open62541_wrappers.c open62541.c -lmbedtls -lmbedx509 -lmbedcrypto -pthread -o open62541_test
```

# Start server, publish and destroy

```sh
./open62541_test server ../../../../Certificates/server/server_cert.der ../../../../Certificates/server/server_key.der ../../../../Certificates/ca/ca_cert.der
```

# Start client, subscribe and destroy

1. Passing the same ca_cert.der that was used to sign the server certificate used for
   started the server above

```sh
./open62541_test client ../../../../Certificates/client/client0_cert.der ../../../../Certificates/client/client0_key.der ../../../../Certificates/ca/ca_cert.der
```

2. Passing the different ca_cert.der that was `not` used to sign the server certificate used for
   started the server above

```sh
./open62541_test client ../../../../Certificates/client/client0_cert.der ../../../../Certificates/client/client0_key.der test/invalid_certs/ca/ca_cert.der
```