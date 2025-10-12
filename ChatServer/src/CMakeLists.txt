
cmake_minimum_required(VERSION 3.10)
project(chatserver VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../include/message.proto")
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../include)
set(SRC_DIR ${CMAKE_SOURCE_DIR})

set(MYSQLCPPCONN_INCLUDE_DIR "/usr/include")
set(MYSQLCPPCONN_LIBRARY "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")

find_package(Boost 1.71.0 REQUIRED COMPONENTS filesystem system)
find_package(Boost REQUIRED COMPONENTS json)
find_package(jsoncpp REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

include_directories(${INCLUDE_DIR})
include_directories(${HIREDIS_INCLUDE_DIRS})
include_directories(${MYSQL_INCLUDE_DIR})
include_directories(${MYSQLCPPCONN_INCLUDE_DIR})

set(GENERATED_PB_SRC ${SRC_DIR}/message.pb.cc)
set(GENERATED_PB_HDR ${INCLUDE_DIR}/message.pb.h)
set(GENERATED_GRPC_SRC ${SRC_DIR}/message.grpc.pb.cc)
set(GENERATED_GRPC_HDR ${INCLUDE_DIR}/message.grpc.pb.h)

add_custom_command(
    OUTPUT ${GENERATED_PB_SRC} ${GENERATED_PB_HDR} ${GENERATED_GRPC_SRC} ${GENERATED_GRPC_HDR}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --proto_path=${INCLUDE_DIR}
         --cpp_out=${CMAKE_SOURCE_DIR}
         --grpc_out=${CMAKE_SOURCE_DIR}
         --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf and gRPC files from ${PROTO_FILE}"
)

set(GENERATED_SRCS ${GENERATED_PB_SRC} ${GENERATED_GRPC_SRC})
set(GENERATED_HDRS ${GENERATED_PB_HDR} ${GENERATED_GRPC_HDR})

file(GLOB SRC_LIST "*.cpp")

add_executable(chatserver ${SRC_LIST} ${GENERATED_SRCS})

target_link_libraries(chatserver PRIVATE Boost::boost Boost::json Boost::system Boost::filesystem)
target_link_libraries(chatserver PRIVATE jsoncpp_lib)
target_link_libraries(chatserver PRIVATE ${Protobuf_LIBRARIES})
target_link_libraries(chatserver PRIVATE ${HIREDIS_LIBRARIES})
target_link_libraries(chatserver PRIVATE ${MYSQLCPPCONN_LIBRARY} ${MYSQL_CLIENT_LIB})

target_link_libraries(chatserver PRIVATE
    gRPC::grpc++
    gRPC::grpc
    gRPC::grpc++_reflection
    ${Protobuf_LIBRARIES}
    pthread
    dl
    ssl
)
