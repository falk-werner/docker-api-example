#include "url_encode.hpp"
#include "fetch.hpp"
#include "extract.hpp"

#include <unistd.h>

#include <cstring>

#include <string>
#include <vector>
#include <iostream>

using docker_utils::url_encode;
using docker_utils::fetch_file;
using docker_utils::extract_file;

namespace
{

struct full_qualified_path
{
    std::string container;
    std::string path;
    std::string local_path;
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
            std::string entry = argv[1];
            auto idx = entry.find(':');            
            if (idx == std::string::npos)
            {
                throw std::runtime_error("invalid entry");
            }

            std::string const container =  entry.substr(0, idx);
            entry = entry.substr(idx + 1);
            idx = entry.find(':');
            if (idx == std::string::npos)
            {
                throw std::runtime_error("invalid entry");
            }

            std::string const path = entry.substr(0, idx);
            std::string const local_path = entry.substr(idx + 1);

            ctx.paths.push_back({container, path, local_path});
        }
    }
}


}

int main(int argc, char* argv[])
{
    if ((argc > 1) && ((0 == strcmp("-h", argv[1])) || (0 == strcmp("--help", argv[1]))))
    {
        std::cout << R"(docker-containers-cp
Copy a file from container to host.

Usage:
    docker-containers-cat [-s SOCKET_PATH] CONTAINER:PATH:LOCAL_PATH ...

Arguments:
    SOCKET_PATH - path to docker or podman socket (default: /run/docker.sock)
    CONTAINER   - ID or name of container to cat file of
    PATH        - path of the file inside the container
    LOCAL_PATH  - path of the file on host

Note:
    Do not use for large files, since contents are kept in memory.
)";
        return EXIT_SUCCESS;
    }

    context ctx;
    scan_args(ctx, argc, argv);

    for(auto const & path: ctx.paths)
    {
        std::string const temp_path = path.local_path + ".tmp";
        try
        {
            std::string const url = std::string("http://d/containers/") + path.container + "/archive" + "?path=" + url_encode(path.path);
            fetch_file(ctx.socket_path, url, temp_path);
            extract_file(temp_path, path.local_path);
        }
        catch (std::exception const & ex)
        {
            std::cerr << "error: " << ex.what() << std::endl;
        }

        unlink(temp_path.c_str());
    }

    return EXIT_SUCCESS;
}