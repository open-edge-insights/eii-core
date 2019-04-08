
# CERT-TOOL for Generating the Asymmetric Key Generation of IEI:

Cert-Tool is a certificate generation tool created for IEI platform based on the openssl commands.It creates x509 
standard certificates for the server and client. Root Certificate Authority certificate and private is generated atfirst then all the IEI server and client components are signed by the RootCA.

## Prerequisites:
    i.  OPENSSL(v1.0 or later) 
    ii. Python3 

##  INSTALLATION:
*    Copy the cert-tool folder on the machine and run the following command for generating the certificates.

###  Generation of the certs:

1. Use **python3 main.py** command to generate all the certificates

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
                    "client_alt_name": ["grpc_external", "iei-simple-visualizer"]
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
    - grpc_external/  --> consists of the server (`IEI host ip`) and client certs/keys consumed by external clients calling into `ia_data_agent`                         exposed `Config()` and `Query()` interface. This is not consumed as of now
    - ImageStore /    --> consists of the server (consumed by external grpc server of `ia_imagestore`) and client certs/keys consumed by the                             internal and external clients calling into the exposed gRPC interfaces
    - kapacitor/      --> consists of the server certificate/key consumed by `ia_data_analytics` container to bring up kapacitor daemon
    - streammanager/  --> consists of the server certificate/key consumed by `ia_data_agent` stream manager module which is one of the influxdb 
                          subscribers
    - streamsublib/   --> consists of the server certificate/key consumed by `ia_factoryctrl_app` stream subscriber module which is one of the influxdb
                          subscribers
    - opcua/          --> consists of the server certificate/key consumed by `ia_data_agent` stream manager module to bring up the opcua server                          and the client certificate is consumed by opcua clients like `iei-simple-visualizer`

### CommandLine Options for Cert-Tool    

    -   Usage: python3 main.py [-h] [--dns DNS] [--clean] [--capath ROOTCA_PATH]

    -  Tool Used for Generating SSL Certificates

    -  optional arguments:
    -     -h, --help            show this help message and exit
    -     --dns DNS             Domain Names for the Server Certificates
    -     --clean               Clear All the Generated Certificates
    -     --capath ROOTCA_PATH  RootCA certificate Path, if given,cert-tool will re-use the existing certs.

   - (--dns DNS ) option is given, then server certificate will be generated with given DNS name along with the server_alt_name
   - (capath ROOTCA) option is given, then **cert-tool will not generate CA certificates** and  re-use the ROOTCA_PATH root-ca certificates.

  **Important Note:** 

  - If `--dns` switch is not provided, cert-tool will auto-generate the `Host Ip` and add it to the respective certs much like `--dns` switch. If     `--dns` switch is provided, then this value would override the auto-generated host ip.

   - cert-tool generates the RootCA certificates in the __rootca__ folder. If you would like to reuse the same rootCA certificates, then __rootca__ folder and all the generated rootca certificates and private keys must be present in the same directory.

   - If --clean option is used, then all the certificates, including __rootca__ also removed, hence we cannot re-use the rootCA certificates.So preserve a copy of __rootca__ if the rootca certificates are needed in future.

   - When the rootCA certificate is re-used, then the newly generated client & server certificate should be             having the new CN and server/client alter names.Otherwise, certificate cannot be signed by root-ca.**
 

###  Decoding the certificates generated into text format:

*  Use **openssl x509 -in <certificate.pem> -text** for pem certificates.
*  Use **openssl x509 -in <certificate.pem> -inform der -text** for der certificates
   
       
###  Verifying the certificates generated with rootCA:

*  Use **openssl verify  -verbose -CAfile <rootcafilepath> <generated_certificate_path>.

