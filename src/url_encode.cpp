#include "url_encode.hpp"

#include <sstream>

namespace docker_utils
{

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

}