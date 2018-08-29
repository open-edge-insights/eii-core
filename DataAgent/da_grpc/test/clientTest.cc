/*
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <stdio.h>
#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <fstream>
#include "../client/cpp/client.cc"

using namespace std;
using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;
using DataAgent::BlobReq;
using DataAgent::Chunk;
using DataAgent::da;

int main(int argc, char** argv) {
  BlobReq request;
  Chunk reply;
  GetBlobClient gclient(grpc::CreateChannel("localhost:50051",
                        grpc::InsecureChannelCredentials()));
  if(argc < 2)
  {
    cout << "Please provide imgHandle key and output file path as arguments." << endl;
    exit(1);
  }
  else if(argc == 2)
  {
    cout << "Please provide output file path as 2nd argument." << endl;
    exit(1);
  }
  std::cout << "-------------- Calling GetBlob --------------" << std::endl;
  std::string response = gclient.GetBlob(argv[1]);
  std::remove(argv[2]);
  std::ofstream out;
  out.open(argv[2],std::ios::app | std::ios::binary);
  out << response;
  out.close();
  return 0;
}