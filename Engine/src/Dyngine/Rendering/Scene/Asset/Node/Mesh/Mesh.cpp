#include "Dyngine/Rendering/Scene/Asset/Node/Mesh/Mesh.hpp"
#include "ErrorHandling/IllegalArgumentException.hpp"

Mesh::Mesh(MeshRenderMode meshRenderMode,
           std::shared_ptr<LLGL::RenderSystem> renderSystem,
           std::optional<uint32_t> numVertices,
           std::optional<uint32_t> numIndices,
           LLGL::Buffer *indexBuffer,
           const std::vector<LLGL::Buffer *> &buffers, LLGL::BufferArray *bufferArray,
           const std::vector<LLGL::VertexFormat> &vertexFormats,
           const std::shared_ptr<Material> &material) :
        meshRenderMode(meshRenderMode),
        renderSystem(renderSystem),
        numVertices(numVertices.has_value() ? numVertices.value() : 0),
        numIndices(numIndices.has_value() ? numIndices.value() : 0),
        indexBuffer(indexBuffer), buffers(buffers),
        bufferArray(bufferArray),
        vertexFormats(vertexFormats),
        material(material) {
    if (indexBuffer != nullptr && !numIndices.has_value()) {
        RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Index buffer given, but no number of indices given!");
    }
    if (numIndices.has_value() && numIndices.value() > 0 && indexBuffer == nullptr) {
        RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Number of indices given, but no index buffer given!");
    }
    if (indexBuffer == nullptr && !numVertices.has_value()) {
        RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Neither index buffer nor number of vertices given!");
    }
}

Mesh::~Mesh() {
    renderSystem->Release(*indexBuffer);
    for (auto &buffer: buffers) {
        renderSystem->Release(*buffer);
    }
    renderSystem->Release(*bufferArray);
}
