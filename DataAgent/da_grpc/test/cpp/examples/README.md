
# DataAgentClient

DataAgentClient program is used for demonstrating GetBlob(imgHandle) gRPC interface.

Pre-requisites:
* **Setting up C++ dev env**
    * Run the [setup_ubuntu_dev_env_cpp.sh](setup_ubuntu_dev_env_cpp.sh) script file after copying it to the directory where you need grpc to be installed. Give necessary permissions required.
    * In case of any issues running the above script file, use the following guide
        (https://github.com/grpc/grpc/blob/master/BUILDING.md)
    * To verify successfull installation, try running gRPC C++ HelloWorld example:
        * cd grpc/examples/cpp/helloworld
        * make
        * ./greeter_server
        * ./greeter_client (In a separate terminal)
        * Terminal displaying Greeter received: Hello world on correct installation.
    * Refer DataAgent README for further instructions on how to run gRPC C++ client.

* Start C++ gRPC client: `sudo ./clientTest [imgHandle] [output_image_file_path]`
  Since you need to compile the test files before running them, follow the below given steps:
  * Replace all occurences of PROTOPATH & PROTOCPPPATH to PROTODPATH & PROTODCPPPATH respectively from lines 17-34 in the Makefile.
  * Run the [Makefile](Makefile) present in test/cpp/examples folder using the command:
    * sudo make
  * If make file is run successfully, a clientTest file should be generated within your examples folder.
    * Pre-requisite: Run python gRPC client and get imgHandle of the Image frame which was given
      as input.
    * Run the clientTest using the following command:
      * ./clientTest [imgHandle] [output_image_file_path]
    * Verify the result by making sure md5sum of both the generated output image file and input
      file are the same.

      **Note**: To use this client file outside the current workspace, just make sure you copy the `client/cpp/client.cc` file along with all the files present in `protobuff/cpp/` folder and take care of imports accordingly.
