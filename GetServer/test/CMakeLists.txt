cmake_minimum_required(VERSION 3.10)
project(redis)

find_package(PkgConfig REQUIRED)

pkg_check_modules(HIREDIS REQUIRED hiredis)
include_directories(${HIREDIS_INCLUDE_DIRS})

add_executable(redis client.cpp)
target_link_libraries(redis PRIVATE ${HIREDIS_LIBRARIES})
