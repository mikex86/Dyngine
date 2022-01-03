#pragma once

#include <string>
#include <filesystem>

class ProjectState {

private:
    std::filesystem::path projectDirPath;

public:
    ProjectState(std::filesystem::path projectDirPath);
};