#include <curl/curl.h>
#include <archive.h>
#include <archive_entry.h>

#include <cstring>

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <stdexcept>

#include <iostream>

namespace
{

using curl_ptr = std::unique_ptr<CURL, void (*) (CURL*)>;

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
        throw std::runtime_error("failed to fetch: bad status: " + std::to_string(http_status));
    }

    return content.str();
}

std::string extract(std::string const data)
{
    archive * ar = archive_read_new();
    archive_read_set_format(ar, ARCHIVE_FORMAT_TAR);
    auto status = archive_read_open_memory(ar, data.data(), data.size());
    if (ARCHIVE_OK != status)
    {
        std::string const err = archive_error_string(ar);
        archive_free(ar);
        throw std::runtime_error("failed to open archive: " + err);
    }

    archive_entry * entry;
    status = archive_read_next_header(ar, &entry);
    if (ARCHIVE_OK != status)
    {
        std::string const err = archive_error_string(ar);
        archive_free(ar);
        throw std::runtime_error("failed to read archive entry: " + err);
    }

    auto size = archive_entry_size(entry);
    std::vector<char> buf(size);
    archive_read_data(ar, buf.data(), buf.size());
    archive_free(ar);

    return std::string(buf.data(), buf.size());
}

char hex_char(char c)
{
    char table[] = "0123456789abcdef";
    return table[ c & 0xf];
}

std::string url_encode(std::string const & text)
{
    std::stringstream out;
    for(auto const & c : text)
    {
        if ( (('0' <= c) && (c <= '9')) || (('a' <= c) && (c <= 'z')) || (('A' <= c) && (c <= 'Z')) || (c == '-') || (c == '_') || (c == '.') || (c == '~') )
        {
            out << c;
        }
        else
        {
            out << '%' << hex_char(c >> 4) << hex_char(c);
        }
    }

    return out.str();
}

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