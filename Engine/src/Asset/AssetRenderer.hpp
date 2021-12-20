#pragma once

#include <DAsset/Asset.hpp>
#include <LLGL/LLGL.h>

class AssetRenderer {

private:
    const DAsset::Asset &asset;

    bool hasChanged = true;

    LLGL::CommandBuffer *commandBuffer;

    LLGL::CommandBuffer *buildCommandBuffer(LLGL::RenderSystem &renderSystem, LLGL::RenderContext &renderContext);

public:
    AssetRenderer(const DAsset::Asset &asset);

    void
    render(LLGL::RenderSystem &renderSystem, LLGL::RenderContext &renderContext, LLGL::CommandBuffer &commandBuffer);
};
