storage "file" {
  path = "/vault/file"
}

listener "tcp" {
 address     = "0.0.0.0:8200"
 #tls_disable = 1
 tls_cert_file="/vault/config/vault_certs/server_certificate.pem"
 tls_key_file="/vault/config/vault_certs/server_key.pem"
}

disable_mlock = 1
