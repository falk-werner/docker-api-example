#ifndef DOCKER_UTILS_HTTP_SERVER_HPP
#define DOCKER_UTILS_HTTP_SERVER_HPP

#include <string>
#include <functional>

namespace docker_utils
{

using http_callback = std::function<std::string (std::string const & url)>;

class http_server
{
public:
    http_server(std::string const & socket_path, http_callback callback);
    ~http_server();
private:
    class detail;
    detail * d;
};

}

#endif
