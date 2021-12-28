#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vector>
#include "Rendering/Scene/Asset/Node/Mesh/Mesh.hpp"
#include "Rendering/Scene/Asset/Node/Node.hpp"

class Asset {
public:
    uint64_t id{};
    glm::mat4 modelMatrix = glm::identity<glm::mat4>();
    std::vector<std::unique_ptr<Node>> nodes{};
};