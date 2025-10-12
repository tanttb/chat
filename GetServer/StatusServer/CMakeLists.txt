
cmake_minimum_required(VERSION 3.10)
project(stutsServer VERSION 1.0)



set(CMAKE_CXX_STANDARD 20)
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../include/message.proto")

# set(MYSQLCPPCONN_INCLUDE_DIR "/usr/include")
# set(MYSQLCPPCONN_LIBRARY "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")

find_package(Boost 1.71.0 REQUIRED COMPONENTS filesystem system)
find_package(Boost REQUIRED COMPONENTS json)
find_package(jsoncpp REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

include_directories(../include)
include_directories(${HIREDIS_INCLUDE_DIRS})

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(SRC_LIST
    "${SOURCE_DIR}/message.pb.cc"
    "${SOURCE_DIR}/message.grpc.pb.cc"
    "${SOURCE_DIR}/ConfigMgr.cpp"
    "${SOURCE_DIR}/redisMgr.cpp"
    "StatusServer.cpp"
)

# protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

add_executable(stutsServer ${SRC_LIST} ) #${PROTO_SRCS}

target_link_libraries(stutsServer PRIVATE Boost::boost Boost::json Boost::system Boost::filesystem)
target_link_libraries(stutsServer PRIVATE jsoncpp_lib)
target_link_libraries(stutsServer PRIVATE ${Protobuf_LIBRARIES})
target_link_libraries(stutsServer PRIVATE ${HIREDIS_LIBRARIES})
# target_link_libraries(stutsServer PRIVATE ${MYSQLCPPCONN_LIBRARY} ${MYSQL_CLIENT_LIB})

target_link_libraries(stutsServer PRIVATE
    gRPC::grpc++
    gRPC::grpc
    gRPC::grpc++_reflection
    ${Protobuf_LIBRARIES}
    pthread
    dl
    ssl
)
