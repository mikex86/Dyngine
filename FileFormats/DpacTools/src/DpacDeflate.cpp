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

    Dpac::WriteOnlyArchive archive = Dpac::WriteOnlyArchive::Open(outFilePath);

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

    archive.reserveNEntries(files.size());
    archive.finalizeEntryTable();

    for (size_t entryIndex = 0; entryIndex < files.size(); ++entryIndex) {
        std::string filePath = files[entryIndex];
        auto relativePath = filePath.substr(rootDirectory.length());
        if (relativePath.empty()) {
            continue;
        }
        std::shared_ptr<Stream::FileDataReadStream> fileStream = Stream::FileDataReadStream::Open(filePath);
        archive.defineEntryFromUncompressedStream(entryIndex, relativePath, fileStream);
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