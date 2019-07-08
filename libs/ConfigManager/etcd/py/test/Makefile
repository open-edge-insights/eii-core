HOST = localhost
PORT = 2379
CLIENT_CERTS = /etc/ssl/etcd
CA_CERTS = /etc/ssl/ca

etcdclient:
	python3.6 main.py -endpoint $(HOST):$(PORT) \
	-certFile $(CLIENT_CERTS)/etcd-client.pem \
	-privateFile $(CLIENT_CERTS)/etcd-client-key.pem \
	-trustFile $(CA_CERTS)/ca.pem \
	-key $(key) \
	-action $(action)

etcdclient_insecure:
	python3.6 main.py -endpoint $(HOST):$(PORT) \
	-certFile "" -privateFile "" -trustFile "" \
	-key $(key) \
	-action $(action)
