
# DataAgentClient

DataAgentClient python program is used for demonstrating GetBlob(imgHandle) gRPC interface.

* Pre-requisite :
  * Install gRPC, gRPC tools by running below commands. More details @ [python grpc quick start](https://grpc.io/docs/quickstart/python.html)
    * `sudo -H pip3 install grpcio`
    * `sudo -H pip3 install grpcio-tools`
  * If the IEI is running on a node behind a coporate network/proxy server, please set IP address      of the node in the no_proxy/NO_PROXY env variable  on the system where you are executing   
    the grpc clients so that the communication doesn't go via the proxy server.
    Eg. `export no_proxy=$no_proxy,<IEI node IP address>`
    If this is not set, one would into gRPC errors like `StatusCode.UNAVIALABLE`      

* Start python gRPC client: `python3.6 grpc_example.py --img_Handle [imgHandle key] --output_file [output_image_file_path]`

    **Note**: To use this client file outside the current workspace, just make sure one copy the `client/py/client.py` file along with `protobuff/py/da_pb2.py` and `protobuff/py/da_pb2_grpc.py` and take care of imports accordingly

Here, `--img_Handle` coresponds to the ImageStore key which has been obtained from the ImageStore.store() python API. Using `GetBlob(imgHandle)` gRPC interface, the byte array corresponding to that `imgHandle` is received from the ImageStore and `--output_file` is created out of that. `md5sum` of the output file can be checked manually and cross-verified with the md5sum of img frame that was stored in the ImageStore.
