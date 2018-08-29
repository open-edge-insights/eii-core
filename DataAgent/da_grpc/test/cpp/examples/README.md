
# DataAgentClient

DataAgentClient program is used for demonstrating GetBlob(imgHandle) gRPC interface.

* Start C++ gRPC client: `sudo ./clientTest [imgHandle] [output_image_file_path]`
  Since you need to compile the test files before running them, follow the below given steps:
  * Change to parent directory of cpp client directory and run these following commands one by one:
    * cd protobuff/
    * sudo g++ -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o cpp/da.pb.o cpp/da.pb.cc
    * sudo g++ -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o cpp/da.grpc.pb.o cpp/da.grpc.pb.cc
    * cd ../test/cpp/examples/
    * sudo g++ -std=c++11 `pkg-config --cflags protobuf grpc`  -c -o clientTest.o clientTest.cc
    * sudo g++ ../../../protobuff/cpp/da.pb.o ../../../protobuff/cpp/da.grpc.pb.o clientTest.o -L/usr/local/lib `pkg-config --libs protobuf grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -ldl -o clientTest
  * If all of these steps are run successfully, a clientTest file should be generated within your    test folder.
    * Pre-requisite: Run python gRPC client and get imgHandle of the Image frame which was given
      as input.
    * Run the clientTest using the following command:
      * ./clientTest [imgHandle] [output_image_file_path]
    * Verify the result by making sure md5sum of both the generated output image file and input
      file are the same.

      **Note**: To use this client file outside the current workspace, just make sure you copy the `client/cpp/client.cc` file along with all the files present in `protobuff/cpp/` folder and take care of imports accordingly.