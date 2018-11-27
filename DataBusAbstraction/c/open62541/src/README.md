# Compilation steps (current directory: <repo>/DataBusAbstraction/c/src/test):

```sh
make build
```

# Start server, publish and destroy

```sh
make server
```

# Start client, subscribe and destroy

1. Passing the same ca_cert.der that was used to sign the server certificate used for
   started the server above

```sh
make client
```

2. Passing the different ca_cert.der that was `not` used to sign the server certificate used for
   started the server above

```sh
make invalid_client
```

# Remove all binaries/object files

```sh
make clean
```

