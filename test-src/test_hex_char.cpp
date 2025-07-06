#include "url_encode.hpp"
#include <gtest/gtest.h>
#include <limits>

using docker_utils::hex_char;

TEST(hex_char, encode_0_to_15)
{
    ASSERT_EQ('0', hex_char(static_cast<char>(0)));
    ASSERT_EQ('1', hex_char(static_cast<char>(1)));
    ASSERT_EQ('2', hex_char(static_cast<char>(2)));
    ASSERT_EQ('3', hex_char(static_cast<char>(3)));
    ASSERT_EQ('4', hex_char(static_cast<char>(4)));
    ASSERT_EQ('5', hex_char(static_cast<char>(5)));
    ASSERT_EQ('6', hex_char(static_cast<char>(6)));
    ASSERT_EQ('7', hex_char(static_cast<char>(7)));
    ASSERT_EQ('8', hex_char(static_cast<char>(8)));
    ASSERT_EQ('9', hex_char(static_cast<char>(9)));
    ASSERT_EQ('a', hex_char(static_cast<char>(10)));
    ASSERT_EQ('b', hex_char(static_cast<char>(11)));
    ASSERT_EQ('c', hex_char(static_cast<char>(12)));
    ASSERT_EQ('d', hex_char(static_cast<char>(13)));
    ASSERT_EQ('e', hex_char(static_cast<char>(14)));
    ASSERT_EQ('f', hex_char(static_cast<char>(15)));
}

TEST(hex_char, dont_fail_on_other_valued)
{
    std::string const allowed_encodings = "0123456789abcdef";
    for(char c = std::numeric_limits<char>::min(); c < std::numeric_limits<char>::max(); c++)
    {
        char const encoded = hex_char(c);
        ASSERT_NE(std::string::npos, allowed_encodings.find(encoded));
    }

    char const encoded = hex_char(std::numeric_limits<char>::max());
    ASSERT_NE(std::string::npos, allowed_encodings.find(encoded));
}