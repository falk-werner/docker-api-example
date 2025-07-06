#include "url_encode.hpp"

#include <curl/curl.h>
#include <archive.h>
#include <archive_entry.h>

#include <fcntl.h>
#include <unistd.h>

#include <cstring>

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <stdexcept>

#include <fstream>
#include <iostream>

using docker_utils::url_encode;

namespace
{

using curl_ptr = std::unique_ptr<CURL, void (*) (CURL*)>;

static size_t curl_write_callback(void * data, size_t size, size_t count, void * user_data)
{
    auto * stream = reinterpret_cast<std::ofstream*>(user_data);
    size_t const bytes = size * count;
    *stream << std::string(reinterpret_cast<char*>(data), bytes);
    return count;
}

void fetch(std::string const & socket_path, std::string const & url, std::string path)
{
    std::ofstream content(path);

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
        throw std::runtime_error("failed to fetch: bad status: " + std::to_string(http_status));
    }
}

void extract(std::string const source_path, std::string const & target_path)
{
    int fd = creat(target_path.c_str(), 0644);
    if (fd < 0)
    {
        throw std::runtime_error("failed to create file");
    }

    archive * ar = archive_read_new();
    archive_read_set_format(ar, ARCHIVE_FORMAT_TAR);
    auto status = archive_read_open_filename(ar, source_path.c_str(), 10240);
    if (ARCHIVE_OK != status)
    {
        std::string const err = archive_error_string(ar);        
        archive_free(ar);
        close(fd);
        throw std::runtime_error("failed to open archive: " + err);
    }

    archive_entry * entry;
    status = archive_read_next_header(ar, &entry);
    if (ARCHIVE_OK != status)
    {
        std::string const err = archive_error_string(ar);
        archive_free(ar);
        close(fd);
        throw std::runtime_error("failed to read archive entry: " + err);
    }

    archive_read_data_into_fd(ar, fd);
    archive_free(ar);
    close(fd);
}


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
            fetch(ctx.socket_path, url, temp_path);
            extract(temp_path, path.local_path);
        }
        catch (std::exception const & ex)
        {
            std::cerr << "error: " << ex.what() << std::endl;
        }

        unlink(temp_path.c_str());
    }

    return EXIT_SUCCESS;
}