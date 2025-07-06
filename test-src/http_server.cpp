#include "http_server.hpp"
#include <microhttpd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <stdexcept>

namespace docker_utils
{

namespace
{

MHD_Result handle_request(
    void * cls,
    struct MHD_Connection * connection,
    const char * url,
    const char * method,
    const char * version,
    const char * upload_data,
    size_t * upload_data_size,
    void ** req_cls)
{
    if (0 == strcmp(method, "GET"))
    {
        *req_cls = nullptr;
        MHD_Response * response;
        unsigned int status = MHD_HTTP_OK;

        try 
        {
            http_callback * callback = reinterpret_cast<http_callback*>(cls);
            std::string const response_text = (*callback)(url);
            response = MHD_create_response_from_buffer_copy(response_text.size(), response_text.c_str());
        }
        catch (...)
        {
            status = MHD_HTTP_INTERNAL_SERVER_ERROR;
            std::string const response_text = "Interal error";
            response = MHD_create_response_from_buffer_copy(response_text.size(), response_text.c_str());
        }

        enum MHD_Result result = MHD_queue_response(connection, status, response);
        MHD_destroy_response(response);
        return result;
    }

    return MHD_NO;
}

}

struct http_server::detail
{
    detail(std::string const & socket_path, http_callback callback)
    : socket_path_(socket_path)
    , callback_(std::move(callback))
    {
        unlink(socket_path.c_str());

        fd = socket(AF_LOCAL, SOCK_STREAM, 0);
        if (fd < 0)
        {
            throw std::runtime_error("failed to create socket");
        }

        sockaddr_un address;
        memset(&address, 0, sizeof(address));
        address.sun_family = AF_LOCAL;
        strncpy(address.sun_path, socket_path.c_str(), sizeof(address.sun_path) - 1);

        int rc = bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        if (rc != 0)
        {
            close(fd);
            throw std::runtime_error("failed to bind socket");
        }

        constexpr int const backlog = 5;
        rc = listen(fd, backlog);
        if (rc != 0)
        {
            close(fd);
            throw std::runtime_error("failed to listen");
        }

        daemon = MHD_start_daemon(
            MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
            0,
            nullptr, nullptr,
            &handle_request, reinterpret_cast<void*>(&callback_),
            MHD_OPTION_LISTEN_SOCKET, fd,
            MHD_OPTION_END
        );
    }

    ~detail()
    {
        MHD_stop_daemon(daemon);
        close(fd);
        unlink(socket_path_.c_str());
    }

    std::string socket_path_;
    http_callback callback_;
    int fd;
    MHD_Daemon * daemon;
};

http_server::http_server(std::string const & socket_path, http_callback callback)
: d(new detail(socket_path, std::move(callback)))
{

}

http_server::~http_server()
{
    delete d;
}
 
}