#include "Rendering/Scene/Asset/Node/Node.hpp"
#include "Rendering/Scene/Asset/Asset.hpp"

glm::mat4 Node::getCurrentModelMatrix() const {
    return parentAsset.modelMatrix * modelMatrix;
}

void Node::addMesh(std::unique_ptr<Mesh> &mesh) {
    meshes.push_back(std::move(mesh));
}

Node::Node(const glm::mat4 &modelMatrix, Asset &parentAsset) : modelMatrix(modelMatrix), parentAsset(parentAsset) {
}

const std::vector<std::shared_ptr<Mesh>> &Node::getMeshes() const {
    return meshes;
}
