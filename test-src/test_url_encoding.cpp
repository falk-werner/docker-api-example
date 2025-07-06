#include "url_encode.hpp"
#include <gtest/gtest.h>

using docker_utils::url_encode;

TEST(url_encode, dont_encode_number_characters)
{
    std::string unencoded = "0123456789";
    std::string encoded = url_encode(unencoded);

    ASSERT_EQ(unencoded, encoded);
}

TEST(url_encode, dont_encode_alphabetic_characters_lowercase)
{
    std::string unencoded = "abcdefghijklmnopqrstuvwxyz";
    std::string encoded = url_encode(unencoded);

    ASSERT_EQ(unencoded, encoded);
}

TEST(url_encode, dont_encode_alphabetic_characters_uppercase)
{
    std::string unencoded = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string encoded = url_encode(unencoded);

    ASSERT_EQ(unencoded, encoded);
}

TEST(url_encode, dont_encode_special_characters)
{
    std::string unencoded = "-._~";
    std::string encoded = url_encode(unencoded);

    ASSERT_EQ(unencoded, encoded);
}

TEST(url_encode, encode_other_characters_nonexhaustive)
{
    ASSERT_EQ("%20", url_encode(" "));
    ASSERT_EQ("%3a", url_encode(":"));
    ASSERT_EQ("%2f", url_encode("/"));
    ASSERT_EQ("%3f", url_encode("?"));
    ASSERT_EQ("%23", url_encode("#"));
}

