#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Rendering/Mesh/Mesh.hpp"

class Node {
public:
    glm::mat4 modelMatrix;
    std::vector<std::unique_ptr<Mesh>> meshes{};
};

class Asset {
public:
    std::vector<std::unique_ptr<Node>> nodes{};
};