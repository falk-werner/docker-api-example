#include "fetch.hpp"
#include "http_server.hpp"

#include <gtest/gtest.h>

#include <filesystem>

using docker_utils::fetch;
using docker_utils::fetch_file;
using docker_utils::http_server;

namespace
{

std::string get_socket_path()
{
    return std::string("fetch_") + ::testing::UnitTest::GetInstance()->current_test_info()->name() + ".sock";
}

}

TEST(fetch, get)
{
    auto const socket_path = get_socket_path();

    http_server server(socket_path, [](auto const & url){ 
        return "[]";
    });

    auto const resp = fetch(socket_path, "http://d/containers/json");
    ASSERT_EQ("[]", resp);
}

TEST(fetch, fail_invalid_socket)
{
    auto const socket_path = "invalid.sock";

    ASSERT_ANY_THROW({
        fetch(socket_path, "http://d/containers/json");
    });
}

TEST(fetch, fail_invalid_response)
{
    auto const socket_path = get_socket_path();

    http_server server(socket_path, [](auto const & url) -> std::string { 
        throw std::runtime_error("fail");
    });

    ASSERT_ANY_THROW({
        fetch(socket_path, "http://d/containers/json");
    });
}

TEST(fetch_file, get_file)
{
    auto const socket_path = get_socket_path();
    std::string const file_name = "fetch_get_files.bin";

    http_server server(socket_path, [](auto const & url){ 
        return "[]";
    });

    fetch_file(socket_path, "http://d/containers/json", file_name);
    ASSERT_TRUE(std::filesystem::exists(file_name));
    unlink(file_name.c_str());
}

TEST(fetch_file, fail_invalid_socket)
{
    auto const socket_path = get_socket_path();
    std::string const file_name = "fetch_fail_invalid_socket.bin";

    EXPECT_ANY_THROW({
        fetch_file(socket_path, "http://d/containers/json", file_name);
    });
    EXPECT_FALSE(std::filesystem::exists(file_name));
    unlink(file_name.c_str());
}

TEST(fetch_file, fail_invalid_response_file)
{
    auto const socket_path = get_socket_path();
    std::string const file_name = "fetch_fail_invalid_response_file.bin";

    http_server server(socket_path, [](auto const & url) -> std::string { 
        throw std::runtime_error("fail");
    });

    EXPECT_ANY_THROW({
        fetch_file(socket_path, "http://d/containers/json", file_name);
    });
    EXPECT_FALSE(std::filesystem::exists(file_name));
    unlink(file_name.c_str());
}
