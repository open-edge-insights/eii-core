
# CERT-TOOL for Generating the Asymmetric Key Generation of ETA:

Cert-Tool is a certificate generation tool created for ETA platform based on the openssl commands.It creates x509 
standard certificates for the server and client. Root Certificate Authority certificate and private is generated atfirst then all the ETA server and client components are signed by the RootCA.

## Prerequisites:
    i.  OPENSSL(v1.0 or later) 
    ii. Python3 

##  INSTALLATION:
*    Copy the cert-tool folder on the machine and run the following command for generating the certificates.

###  Generation of the certs:

1. Use **python3 main.py** command to generate certificates

2. Certificates will be generated in the "Certificates" Folder inside the cert-tool directory.
3. All the certificates will be generated based on the config.json details provided by the user.If new certificates are needed, new entry can be   
   added in the config.json. 
       
4. The format of the `config.json` is as follows :
    ```sh        
    {
        "certificates": [
            {
                "influxdb":
                {
                    "server_alt_name":"influxdb",
                    "client_alt_name": ["influxdb"]
                }
            },
            
                {"imagestore" :
                {
                    "server_alt_name":"imagestore",
                    "client_alt_name": ["imagestore"]
                }
            },
            {
                "grpc_external" :
                {
                    "server_alt_name":"grpc_external",
                    "client_alt_name": ["grpc_external", "visualhmi"]
                }
            }
        ]
    }
    ```

    The current [config.json](config.json) file generates the below certificates under `cert-tool/Certificates` folder:
    - ca/             --> consists of the ca certs/keys in .pem and .der format. 
    - influxdb/       --> consists of the influxdb server cert/key consumed by `ia_influxdb` container
    - grpc_internal/  --> consists of the server (grpc server of `ia_data_agent`) and client certs/keys consumed by containers making 
                          `GetConfigInt()` internal gRPC call
                         exposed by `ia_data_agent` container
    - grpc_external/  --> consists of the server (`ETA host ip`) and client certs/keys consumed by external clients calling into `ia_data_agent`                         exposed `Config()` and `Query()` interface. This is not consumed as of now
    - ImageStore /    --> consists of the server (consumed by external grpc server of `ia_imagestore`) and client certs/keys consumed by the                             internal and external clients calling into the exposed gRPC interfaces
    - kapacitor/      --> consists of the server certificate/key consumed by `ia_data_analytics` container to bring up kapacitor daemon
    - streammanager/  --> consists of the server certificate/key consumed by `ia_data_agent` stream manager module which is one of the influxdb 
                          subscribers
    - streamsublib/   --> consists of the server certificate/key consumed by `ia_yumei_app` stream subscriber module which is one of the influxdb 
                          subscribers
    - opcua/          --> consists of the server certificate/key consumed by `ia_data_agent` stream manager module to bring up the opcua server                          and the client certificate is consumed by opcua clients like visual hmi
 
**Below Some of the limitations of the tool as of now (will be addressed in the upcoming releases):**
* cert-tool on every run would re-generate all certs freshly (ca, server and client certificates/keys) as per config.json. Currently, 
  there is no provision to re-use the ca cert/key and generate the server and client certificates/keys separately
* We would like to have `server_alt_name` also to take in the array for the alternate domain names
* Keep the `client_alt_name` in the config.json optional, so that we can not generate client certs/keys when not needed.

###  Removing the certificates:
           
Use `python3 main.py clean` to clear all the generated certificates.

###  Decoding the certificates generated into text format:

*  Use **openssl x509 -in <certificate.pem> -text** for pem certificates.
*  Use **openssl x509 -in <certificate.pem> -inform der -text** for der certificates
   
       
