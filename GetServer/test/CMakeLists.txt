cmake_minimum_required(VERSION 3.10)
project(grpc_example)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

add_executable(server server.cpp helloworld.pb.cc helloworld.grpc.pb.cc)
target_link_libraries(server PRIVATE gRPC::grpc++ protobuf::libprotobuf)

add_executable(client client.cpp helloworld.pb.cc helloworld.grpc.pb.cc)
target_link_libraries(client PRIVATE gRPC::grpc++ protobuf::libprotobuf)