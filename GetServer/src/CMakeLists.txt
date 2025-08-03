
cmake_minimum_required(VERSION 3.10)
project(chat VERSION 1.0)



set(CMAKE_CXX_STANDARD 20)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Boost REQUIRED COMPONENTS json)
find_package(jsoncpp REQUIRED)

include_directories(.)
include_directories(../include)

aux_source_directory(. SRC_LIST)
add_executable(chat ${SRC_LIST})

target_link_libraries(chat PRIVATE Boost::boost Boost::json Boost::system)
target_link_libraries(chat PRIVATE jsoncpp_lib)
