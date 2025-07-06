#include "url_encode.hpp"
#include "fetch.hpp"
#include "extract.hpp"

#include <cstring>

#include <string>
#include <vector>
#include <stdexcept>

#include <iostream>

using docker_utils::url_encode;
using docker_utils::fetch;
using docker_utils::extract;

namespace
{

struct full_qualified_path
{
    std::string container;
    std::string path;
};

struct context
{
    std::string socket_path;
    std::vector<full_qualified_path> paths;
};

void scan_args(context & ctx, int argc, char* argv[])
{
    ctx.socket_path = "/run/docker.sock";
    bool parse_socket_path = false;
    for(size_t i = 1; i < argc; i++)
    {
        if (parse_socket_path == true)
        {
            ctx.socket_path = argv[i];
            parse_socket_path = false;
        }
        else if ((0 == strcmp("-s", argv[i])) || (0 == strcmp("--socket-path", argv[i])))
        {
            parse_socket_path = true;
        }
        else
        {
            full_qualified_path path = {"", argv[i]};
            auto const idx = path.path.find(':');
            if (idx != std::string::npos)
            {
                path.container = path.path.substr(0, idx);
                path.path = path.path.substr(idx + 1);
            }

            ctx.paths.push_back(path);
        }
    }
}


}

int main(int argc, char* argv[])
{
    if ((argc > 1) && ((0 == strcmp("-h", argv[1])) || (0 == strcmp("--help", argv[1]))))
    {
        std::cout << R"(docker-containers-cat
Print file contents to stdout.

Usage:
    docker-containers-cat [-s SOCKET_PATH] CONTAINER:PATH ...

Arguments:
    SOCKET_PATH - path to docker or podman socket (default: /run/docker.sock)
    CONTAINER   - ID or name of container to cat file of
    PATH        - path of the file inside the container

Note:
    Do not use for large files, since contents are kept in memory.
)";
        return EXIT_SUCCESS;
    }

    context ctx;
    scan_args(ctx, argc, argv);

    for(auto const & path: ctx.paths)
    {
        std::string const url = std::string("http://d/containers/") + path.container + "/archive" + "?path=" + url_encode(path.path);
        auto const data = fetch(ctx.socket_path, url);
        std::cout << extract(data);
    }

    return EXIT_SUCCESS;
}