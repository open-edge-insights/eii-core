storage "file" {
  path = "/vault/file"
}

listener "tcp" {
 address     = "localhost:8200"
 tls_disable = 1
}

disable_mlock = 1
