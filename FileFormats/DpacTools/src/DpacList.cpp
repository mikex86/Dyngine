#include <iostream>
#include <vector>
#include <string>
#include <Dpac/Dpac.hpp>

int run(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: dpac_list <dpac_file>" << std::endl;
        return 1;
    }
    std::string dpacFilePath = argv[1];
    Dpac::ReadOnlyArchive archive = Dpac::ReadOnlyArchive::Open(dpacFilePath);
    auto fileTable = archive.getFileContentOffsetTable();
    for (auto &entry: fileTable) {
        std::cout << entry.first << " at " << entry.second << std::endl;
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