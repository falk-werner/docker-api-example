#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <cstring>

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <stdexcept>

using curl_ptr = std::unique_ptr<CURL, void (*) (CURL*)>;
using nlohmann::json;

namespace
{

static size_t curl_write_callback(void * data, size_t size, size_t count, void * user_data)
{
    auto * stream = reinterpret_cast<std::stringstream*>(user_data);
    size_t const bytes = size * count;
    *stream << std::string(reinterpret_cast<char*>(data), bytes);
    return count;
}

std::string fetch(std::string const & socket_path, std::string const & url)
{
    std::stringstream content;

    curl_ptr curl(curl_easy_init(), curl_easy_cleanup);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &curl_write_callback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, reinterpret_cast<void*>(&content));
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_UNIX_SOCKET_PATH, socket_path.c_str());

    auto const status = curl_easy_perform(curl.get());
    if (CURLE_OK != status)
    {
        throw std::runtime_error("failed to fetch");
    }

    long http_status = 500;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_status);
    if ((http_status < 200) || (300 <= http_status))
    {
        throw std::runtime_error("failed to fetch: bad status");
    }

    return content.str();
}

}

int main(int argc, char* argv[])
{
    int exit_code = EXIT_FAILURE;

    if ((argc > 1) && ((0 == strcmp("-h", argv[1])) || (0 == strcmp("--help", argv[1])))) 
    {
        std::cout << R"(docker-containers-list
List containers.

Usage:
    docker-containers-list [SOCKET_PATH]

Arguments:
    SOCKET_PATH - path to docker or podman socket (default: /run/docker.sock)
)";
        return EXIT_SUCCESS;
    }

    try
    {
        std::string const socket_path = (argc > 1) ? argv[1] : "/run/docker.sock";
        std::string const url = "http://d/containers/json";
        auto const text = fetch(socket_path, url);
        json containers = json::parse(text);
        for (auto const & container: containers)
        {
            std::string const id = container["Id"];
            std::string const name = container["Names"][0];
            std::string const image = container["Image"];
            std::string const status = container["Status"];

            std::cout << "[Container]" << std::endl;
            std::cout << "Id    : " << id << std::endl;
            std::cout << "Name  : " << name.substr(1) << std::endl;
            std::cout << "Image : " << image << std::endl;
            std::cout << "Status: " << status << std::endl;
            std::cout << std::endl;
        }


        exit_code = EXIT_SUCCESS;
    }
    catch (std::exception const & ex)
    {
        std::cerr << "error: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "fatal: unexpected error" << std::endl;
    }

    return exit_code;
}