#include <iostream>
#include <vector>
#include <string>
#include <Dpac/Dpac.hpp>

int run(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: dpac_get <dpac_file> <archive_file_name>" << std::endl;
        return 1;
    }
    std::string dpacFilePath = argv[1];
    std::string archiveFileName = argv[2];
    Dpac::ReadOnlyArchive archive = Dpac::ReadOnlyArchive::Open(dpacFilePath);
    buffer::FileDataReadStream stream = archive.getEntryStream(archiveFileName);
    uint64_t size = stream.getSize();
    for (uint64_t i = 0; i < size; i++) {
        char character = stream.readInt8();
        std::cout << character;
    }
    std::cout << std::endl;
    return 0;
}

int main(int argc, char **argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}