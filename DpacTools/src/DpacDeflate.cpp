#include <iostream>
#include <filesystem>
#include <map>
#include <Dpac/Dpac.hpp>
#include <Utils/FileUtils.hpp>

#define DPAC_FILE_SEPARATOR '/'

static int run(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: dpac_deflate <directory> <outfile>" << std::endl;
        return 1;
    }

    std::string directoryPath = argv[1];
    std::string outFilePath = argv[2];

    dpac::WriteOnlyArchive archive = dpac::WriteOnlyArchive::Open(outFilePath);

    std::string rootDirectory = std::filesystem::absolute(directoryPath).u8string();
    std::replace(rootDirectory.begin(), rootDirectory.end(), '\\', DPAC_FILE_SEPARATOR);

    std::string currentDirectory;

    // Maps directory names to the files in them (absolute paths)
    std::vector<std::string> files{};
    {
        for (const auto &entry: std::filesystem::recursive_directory_iterator(directoryPath)) {
            std::string filePath = entry.path().generic_u8string();
            std::replace(filePath.begin(), filePath.end(), '\\', DPAC_FILE_SEPARATOR);
            if (entry.is_regular_file()) {
                files.push_back(filePath);
            }
        }
    }

    for (auto &filePath: files) {
        auto relativePath = filePath.substr(rootDirectory.length());
        if (relativePath.empty()) {
            continue;
        }
        uint64_t fileSize = FileUtils::GetFileSize(filePath);
        archive.createEntry(relativePath, fileSize);
    }
    archive.finalizeEntryTable();
    for (auto &filePath: files) {
        auto relativePath = filePath.substr(rootDirectory.length());
        if (relativePath.empty()) {
            continue;
        }
        std::ifstream streamIn;
        streamIn.open(filePath, std::ios::in | std::ios::binary);
        archive.createEntryContentDefinition(relativePath, streamIn);
        streamIn.close();
    }
    return 0;
}

int main(int argc, char **argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}