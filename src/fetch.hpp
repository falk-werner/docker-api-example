#ifndef DOCKER_UTILS_FETCH_HPP
#define DOCKER_UTILS_FETCH_HPP

#include <string>

namespace docker_utils
{

/// @brief Downloads a resource from a given url and returns it as string.
///
/// @param socket_path path to the unix domain socket.
/// @param url url to download
/// @return contents of the downloaded resource
/// @throws std:runtime_error download fails or http status is not good
std::string fetch(std::string const & socket_path, std::string const & url);

/// @brief Downloads a resource from a given url into a file
/// @param socket_path path to the unix domain socket.
/// @param url url to download
/// @param path path to the file where to store the resource's content
/// @throws std:runtime_error download fails or http status is not good
void fetch_file(std::string const & socket_path, std::string const & url, std::string const & path);

}

#endif
