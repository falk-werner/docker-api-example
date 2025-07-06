#ifndef DOCKER_UTILS_EXTRACT_HPP
#define DOCKER_UTILS_EXTRACT_HPP

#include <string>

namespace docker_utils
{

/// @brief Extract the first entry of a tar archive in memory.
/// @param data Data of the tar archive.
/// @return extracted data.
std::string extract(std::string const & data);

/// @brief Extract the first entry of a tar archive to a file.
/// @param source_path path of the tar archive
/// @param target_path path where the first entry should be extracted to
void extract_file(std::string const & source_path, std::string const & target_path);

}

#endif
