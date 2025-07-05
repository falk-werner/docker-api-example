cmake_minimum_required(VERSION 3.22)
project(docker-api-example)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(LibArchive REQUIRED)

add_executable(docker-containers-list
    src/containers_list.cpp)

target_link_libraries(docker-containers-list PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json)


add_executable(docker-containers-cat
src/containers_cat.cpp)

target_link_libraries(docker-containers-cat PRIVATE
    CURL::libcurl
    LibArchive::LibArchive)


add_executable(docker-containers-cp
src/containers_cp.cpp)

target_link_libraries(docker-containers-cp PRIVATE
    CURL::libcurl
    LibArchive::LibArchive)
