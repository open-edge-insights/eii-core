# Compilation steps (current directory: <repo>/DataBusAbstraction/c):

```sh
make build
```

# Start publisher, publish and destroy

```sh
make pub
```

# Start subscriber, subscribe and destroy

1. Passing the same ca_cert.der that was used to sign the server certificate used for
   started the publisher above

```sh
make sub
```

# Remove all binaries/object files

```sh
make clean
```

# Compilation steps for running DBA tests in dist_libs:

# Start publisher, publish and destroy
```sh
make -f Makefile.dist_libs dist_libs_pub
```

# Start subscriber, subscribe and destroy
```sh
make -f Makefile.dist_libs dist_libs_sub
```
