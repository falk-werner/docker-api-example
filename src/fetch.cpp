#include "fetch.hpp"

#include <curl/curl.h>

#include <memory>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace
{

size_t fetch_write_callback(void * data, size_t size, size_t count, void * user_data)
{
    auto * stream = reinterpret_cast<std::stringstream*>(user_data);
    size_t const bytes = size * count;
    *stream << std::string(reinterpret_cast<char*>(data), bytes);
    return count;
}

size_t fetch_file_write_callback(void * data, size_t size, size_t count, void * user_data)
{
    auto * stream = reinterpret_cast<std::ofstream*>(user_data);
    size_t const bytes = size * count;
    *stream << std::string(reinterpret_cast<char*>(data), bytes);
    return count;
}

}

namespace docker_utils
{

using curl_ptr = std::unique_ptr<CURL, void (*) (CURL*)>;


std::string fetch(std::string const & socket_path, std::string const & url)
{
    std::stringstream content;

    curl_ptr curl(curl_easy_init(), curl_easy_cleanup);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &fetch_write_callback);
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

void fetch_file(std::string const & socket_path, std::string const & url, std::string const & path)
{
    curl_ptr curl(curl_easy_init(), curl_easy_cleanup);

    CURLcode status;
    {
        std::ofstream content(path);

        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &fetch_file_write_callback);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, reinterpret_cast<void*>(&content));
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_UNIX_SOCKET_PATH, socket_path.c_str());

        status = curl_easy_perform(curl.get());
    }

    if (CURLE_OK != status)
    {
        std::filesystem::remove(path);
        throw std::runtime_error("failed to fetch");
    }

    long http_status = 500;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_status);
    if ((http_status < 200) || (300 <= http_status))
    {
        std::filesystem::remove(path);
        throw std::runtime_error("failed to fetch: bad status: " + std::to_string(http_status));
    }
}


}