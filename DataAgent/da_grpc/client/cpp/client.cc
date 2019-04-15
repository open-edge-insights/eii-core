/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <cstdlib>
#include "../../protobuff/cpp/da.grpc.pb.h"

using namespace std;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using DataAgent::BlobReq;
using DataAgent::Chunk;
using DataAgent::da;

class GetBlobClient{
public:
GetBlobClient(std::shared_ptr<Channel> channel)
      : stub_(da::NewStub(channel)) {}

std::string GetBlob(const std::string& imgHandle)
{
    /*
            GetBlob is a wrapper around gRPC C++ client implementation
            for GetBlob gRPC interface.
            Arguments:
            imgHandle(string): key for ImageStore
            Returns:
            The consolidated string(value from ImageStore) associated with
            that imgHandle
    */
    BlobReq request;
    request.set_imghandle(imgHandle);
    Chunk reply;
    ClientContext context;
    std::cout << imgHandle << std::endl;
    std::unique_ptr<grpc::ClientReader<Chunk> > reader(stub_->GetBlob(&context, request));
    std::string response = "";
    while (reader->Read(&reply)) {
      response = response + reply.chunk();
    }
    Status status = reader->Finish();
    if ((status.ok()) != 1 ) {
      std::cout << "Transfer failed." << std::endl;
      return "";
    }
    return response;
}
private:
  std::unique_ptr<da::Stub> stub_;
};