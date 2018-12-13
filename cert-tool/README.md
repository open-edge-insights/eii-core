
# CERT-TOOL for Generating the Asymmetric Key Generation of ETA:

Cert-Tool is a certificate generation tool created for ETA platform based on the openssl commands.It creates x509 
standard certificates for the server and client. Root Certificate Authority certificate and private is generated atfirst then all the ETA server and client components are signed by the RootCA.

## Prerequisites:
    i.  OPENSSL(v1.0 or later) 
    ii. Python3 

##  INSTALLATION:
*    Copy the cert-tool folder on the machine and run the following command for generating the certificates.

###  Generation of the certs:
>  1. Use **python3 main.py** command to generate certificates

>  2. Certificates will be generated in the "Certificates" Folder inside the cert-tool directory.
>  3. All the certificates will be generated based on the config.json details provided by the user.If new certificates are needed, new entry can be added in the config.json. 
       
>  4. The format of the config.json is as follows:
            
           {
            "certificates": [
                {"influxdb":
                    {
                     "server_alt_name":"influxdb",
                     "client_alt_name":"influxdb"
                    }
                },
                {"imagestore" :
                    {
                     "server_alt_name":"imagestore",
                     "client_alt_name":"imagestore"
                    }
                },
                {"grpc_external" :
                    {
                     "server_alt_name":"grpc_external",
                     "client_alt_name":"grpc_external"
                    }
                }
           ]
        }
###  Removing the certificates:
           
          Use **python3 main.py clean** to clear all the generated certificates.
###  Decoding the certificates generated into  text format:
         *  Use **openssl x509 -in <certificate.pem> -text** for pem certificates.
         *  Use **openssl x509 -in <certificate.pem> -inform der -text** for der certificates
   
       
