#include "fetch.hpp"

#include <nlohmann/json.hpp>

#include <cstring>

#include <iostream>
#include <string>

using docker_utils::fetch;
using nlohmann::json;


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