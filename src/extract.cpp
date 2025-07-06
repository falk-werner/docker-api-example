#include "extract.hpp"

#include <archive.h>
#include <archive_entry.h>

#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <stdexcept>

namespace docker_utils
{

std::string extract(std::string const & data)
{
    archive * ar = archive_read_new();
    archive_read_set_format(ar, ARCHIVE_FORMAT_TAR);
    auto status = archive_read_open_memory(ar, data.data(), data.size());
    if (ARCHIVE_OK != status)
    {
        std::string const err = archive_error_string(ar);
        archive_read_close(ar);
        archive_free(ar);
        throw std::runtime_error("failed to open archive: " + err);
    }

    archive_entry * entry;
    status = archive_read_next_header(ar, &entry);
    if (ARCHIVE_OK != status)
    {
        std::string const err = archive_error_string(ar);
        archive_read_close(ar);
        archive_free(ar);
        throw std::runtime_error("failed to read archive entry: " + err);
    }

    auto size = archive_entry_size(entry);
    std::vector<char> buf(size);
    archive_read_data(ar, buf.data(), buf.size());
    archive_read_close(ar);
    archive_free(ar);

    return std::string(buf.data(), buf.size());
}


void extract_file(std::string const & source_path, std::string const & target_path)
{
    int fd = creat(target_path.c_str(), 0644);
    if (fd < 0)
    {
        throw std::runtime_error("failed to create file");
    }

    archive * ar = archive_read_new();
    archive_read_set_format(ar, ARCHIVE_FORMAT_TAR);
    auto status = archive_read_open_filename(ar, source_path.c_str(), 10240);
    if (ARCHIVE_OK != status)
    {
        std::string const err = archive_error_string(ar);        
        archive_free(ar);
        close(fd);
        throw std::runtime_error("failed to open archive: " + err);
    }

    archive_entry * entry;
    status = archive_read_next_header(ar, &entry);
    if (ARCHIVE_OK != status)
    {
        std::string const err = archive_error_string(ar);
        archive_free(ar);
        archive_read_close(ar);
        close(fd);
        throw std::runtime_error("failed to read archive entry: " + err);
    }

    archive_read_data_into_fd(ar, fd);
    archive_read_close(ar);
    archive_free(ar);
    close(fd);
}

}