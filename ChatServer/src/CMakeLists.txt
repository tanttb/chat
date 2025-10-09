
cmake_minimum_required(VERSION 3.10)
project(chatserver VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../../GetServer/include/message.proto")
set(HOME_PATH "${CMAKE_CURRENT_SOURCE_DIR}/..")

set(MYSQLCPPCONN_INCLUDE_DIR "/usr/include")
set(MYSQLCPPCONN_LIBRARY "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")

find_package(Boost 1.71.0 REQUIRED COMPONENTS filesystem system)
find_package(Boost REQUIRED COMPONENTS json)
find_package(jsoncpp REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

include_directories(${HOME_PATH}/include/)
include_directories(${HIREDIS_INCLUDE_DIRS})
include_directories(${MYSQL_INCLUDE_DIR})
include_directories(${MYSQLCPPCONN_INCLUDE_DIR})
# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

file(GLOB SRC_LIST "*.cpp")
set(PROTO 
    "message.pb.cc"
    "message.grpc.pb.cc"
)
# message("${HOME_PATH}/include/" ${SRC_LIST})

# aux_source_directory(. SRC_LIST)
add_executable(chatserver ${SRC_LIST} ${PROTO})

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
