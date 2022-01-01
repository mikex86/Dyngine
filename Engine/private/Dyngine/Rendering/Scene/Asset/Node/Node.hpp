#pragma once

#include <glm/glm.hpp>
#include "Dyngine/Rendering/Scene/Asset/Node/Mesh/Mesh.hpp"

class Asset;

class Node {
private:
    glm::mat4 modelMatrix;
    Asset &parentAsset;
    std::vector<std::shared_ptr<Mesh>> meshes{};
public:
    Node(const glm::mat4 &modelMatrix, Asset &parentAsset);

    void addMesh(std::unique_ptr<Mesh> &mesh);

    glm::mat4 getCurrentModelMatrix() const;

    const std::vector<std::shared_ptr<Mesh>> &getMeshes() const;
};