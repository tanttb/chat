#include <grpcpp/grpcpp.h>
#include "helloworld.grpc.pb.h"

int main() {
  auto channel = grpc::CreateChannel("localhost:50051", 
                                   grpc::InsecureChannelCredentials());
  std::unique_ptr<Greeter::Stub> stub = Greeter::NewStub(channel);
  
  HelloRequest request;
  request.set_name("World");
  
  HelloReply reply;
  grpc::ClientContext context;
  grpc::Status status = stub->SayHello(&context, request, &reply);
  
  if (status.ok()) {
    std::cout << reply.message() << std::endl;
  }
  return 0;
}