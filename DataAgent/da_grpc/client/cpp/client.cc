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
    if (status.ok()) {
      return response;
    } else {
      std::cout << status.error_code() << "Transfer failed." << status.error_message() << std::endl;
      return response;
    }
}
private:
  std::unique_ptr<da::Stub> stub_;
};