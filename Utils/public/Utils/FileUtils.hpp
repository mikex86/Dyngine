#pragma once

#include <cstdint>
#include <string>

namespace FileUtils {

    /**
     * Retrieves the file size of the specified file
     * @param filePath the path of the file to get the file size of
     * @return the file size
     * @throws errorhandling__FileException if retrieval of the file size fails. eg. if the file does not exist
     */
    uint64_t GetFileSize(const std::string &filePath);

}