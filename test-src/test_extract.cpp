#include "extract.hpp"

#include <archive.h>
#include <archive_entry.h>

#include <gtest/gtest.h>

#include <vector>

using docker_utils::extract;

TEST(extract, fail_invalid_archive)
{
    EXPECT_ANY_THROW({
        extract("invalid");
    });
}

TEST(extract, fail_empty_archive)
{
    std::vector<char> buffer(10240);
    size_t used = 0;

    archive * ar = archive_write_new();
    archive_write_set_format(ar, ARCHIVE_FORMAT_TAR);
    archive_write_open_memory(ar, buffer.data(), buffer.size(), &used);

    archive_entry * entry = archive_entry_new();
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_pathname(entry, "foo");
    archive_entry_set_size(entry, 2);
    archive_write_header(ar, entry);
    archive_write_data(ar, "42", 2);
    archive_write_finish_entry(ar);
    archive_entry_free(entry);    

    archive_write_close(ar);
    archive_free(ar);

    std::string data(buffer.data(), used);

    auto const extracted = extract(data);
    ASSERT_EQ("42", extracted);
}