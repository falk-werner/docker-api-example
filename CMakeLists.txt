cmake_minimum_required(VERSION 3.22)
project(docker-api-example)

option(WITHOUT_TEST "Disable unit test" OFF)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -fprofile-arcs -ftest-coverage")
link_libraries(debug gcov)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(LibArchive REQUIRED)

add_library(docker-utils STATIC
    src/url_encode.cpp
    src/fetch.cpp
    src/extract.cpp)
target_include_directories(docker-utils PUBLIC src)
target_link_libraries(docker-utils PUBLIC 
    CURL::libcurl
    nlohmann_json::nlohmann_json
    LibArchive::LibArchive)

add_executable(docker-containers-list
    src/app/containers_list.cpp)
target_link_libraries(docker-containers-list PRIVATE docker-utils)

add_executable(docker-containers-cat
    src/app/containers_cat.cpp)
target_link_libraries(docker-containers-cat PRIVATE docker-utils)

add_executable(docker-containers-cp
    src/app/containers_cp.cpp)
target_link_libraries(docker-containers-cp PRIVATE docker-utils)


# Testing

if(NOT(WITHOUT_TEST))

enable_testing()
include(CTest)

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(alltests
    test-src/test_url_encoding.cpp
    test-src/test_hex_char.cpp
    test-src/test_extract.cpp)

target_link_libraries(alltests PRIVATE docker-utils GTest::gtest GTest::gtest_main)
gtest_discover_tests(alltests)

endif()
