#include "AssetRenderer.hpp"
#include <queue>

AssetRenderer::AssetRenderer(const DAsset::Asset &asset) : asset(asset) {
}

void AssetRenderer::render(LLGL::RenderSystem &renderSystem, LLGL::RenderContext &renderContext,
                           LLGL::CommandBuffer &mainRenderCmd) {
    if (hasChanged) {
        buildCommandBuffer(renderSystem, renderContext);
        hasChanged = false;
    }
    mainRenderCmd.Execute(*commandBuffer);
}

LLGL::CommandBuffer *
AssetRenderer::buildCommandBuffer(LLGL::RenderSystem &renderSystem, LLGL::RenderContext &renderContext) {
    LLGL::CommandBuffer *buffer = renderSystem.CreateCommandBuffer();

    // Create vertex buffers
    LLGL::Buffer **buffers = new LLGL::Buffer *[asset.bufferCollection.buffers.size()];
    size_t i = 0;
    for (auto &buffer: asset.bufferCollection.buffers) {
        auto bufferData = buffer->data;
        LLGL::BufferDescriptor bufferDesc{
                .size = bufferData.size(),
        };
        buffers[i++] = renderSystem.CreateBuffer(bufferDesc);
    }

    std::queue<DAsset::Node> nodes{};
    nodes.push(asset.rootNode);
    while (!nodes.empty()) {
        auto node = nodes.back();
        nodes.pop();
        // Render node
        {
            auto mesh = node.mesh;
            for (const auto &meshPart: mesh.meshParts) {
                // TODO:
            }
        }
        // Enqueue child nodes
        for (const auto &item: node.children) {
            nodes.push(item);
        }
    }
    return buffer;
}
