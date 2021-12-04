#include <Utils/FileUtils.hpp>
#include <ErrorHandling/FileExcept.hpp>

uint64_t FileUtils::GetFileSize(const std::string &filePath) {
    struct stat stat_buf{};
    int status = stat(filePath.c_str(), &stat_buf);
    if (status != 0) {
        RAISE_EXCEPTION(errorhandling::FileException, "Failed to retrieve file size for file \"" + filePath + "\"");
    }
    return stat_buf.st_size;
}
