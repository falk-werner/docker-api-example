#ifndef DOCKER_UTILS_URL_ENCODE_HPP
#define DOCKER_UTILS_URL_ENCODE_HPP

#include <string>

namespace docker_utils
{

char hex_char(char c);

std::string url_encode(std::string const & text);

}

#endif
