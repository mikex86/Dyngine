#include <Asset/AssetRenderer.hpp>
#include <queue>
#include <ErrorHandling/IllegalArgumentException.hpp>

AssetRenderer::AssetRenderer(const DAsset::Asset &asset) : asset(asset) {
}

void AssetRenderer::render(LLGL::RenderSystem &renderSystem, LLGL::RenderContext &renderContext,
                           LLGL::CommandBuffer &mainRenderCmd) {
    if (commandBuffer == nullptr || hasChanged) {
        if (commandBuffer != nullptr) {
            renderSystem.Release(*commandBuffer);
        }
        commandBuffer = buildCommandBuffer(renderSystem, renderContext);
        hasChanged = false;
    }
    mainRenderCmd.Execute(*commandBuffer);
}

LLGL::Format GetFormat(DAsset::DataType type, DAsset::ComponentType componentType);

LLGL::CommandBuffer *
AssetRenderer::buildCommandBuffer(LLGL::RenderSystem &renderSystem, LLGL::RenderContext &renderContext) {
    LLGL::CommandBuffer *cmd;

    {
        LLGL::CommandBufferDescriptor cmdBufferDesc{
                .flags = LLGL::CommandBufferFlags::MultiSubmit | LLGL::CommandBufferFlags::DeferredSubmit
        };
        cmd = renderSystem.CreateCommandBuffer(cmdBufferDesc);
    }

    cmd->Begin();
    {

    }
    cmd->End();

    return cmd;
}


LLGL::Format GetFormat(DAsset::DataType type, DAsset::ComponentType componentType) {
    switch (type) {
        case DAsset::DataType::BYTE: {
            switch (componentType) {
                case DAsset::ComponentType::SCALAR:
                    return LLGL::Format::R8SInt;
                case DAsset::ComponentType::VEC2:
                    return LLGL::Format::RG8SInt;
                case DAsset::ComponentType::VEC3:
                    return LLGL::Format::RGB8SInt;
                case DAsset::ComponentType::VEC4:
                    return LLGL::Format::RGBA8SInt;
                default:
                    RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Invalid component type");
            }
        }
        case DAsset::DataType::UNSIGNED_BYTE: {
            switch (componentType) {
                case DAsset::ComponentType::SCALAR:
                    return LLGL::Format::R8UInt;
                case DAsset::ComponentType::VEC2:
                    return LLGL::Format::RG8UInt;
                case DAsset::ComponentType::VEC3:
                    return LLGL::Format::RGB8UInt;
                case DAsset::ComponentType::VEC4:
                    return LLGL::Format::RGBA8UInt;
                default:
                    RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Invalid component type");
            }
        }
        case DAsset::DataType::SHORT: {
            switch (componentType) {
                case DAsset::ComponentType::SCALAR:
                    return LLGL::Format::R16SInt;
                case DAsset::ComponentType::VEC2:
                    return LLGL::Format::RG16SInt;
                case DAsset::ComponentType::VEC3:
                    return LLGL::Format::RGB16SInt;
                case DAsset::ComponentType::VEC4:
                    return LLGL::Format::RGBA16SInt;
                default:
                    RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Invalid component type");
            }
        }
        case DAsset::DataType::UNSIGNED_SHORT: {
            switch (componentType) {
                case DAsset::ComponentType::SCALAR:
                    return LLGL::Format::R16UInt;
                case DAsset::ComponentType::VEC2:
                    return LLGL::Format::RG16UInt;
                case DAsset::ComponentType::VEC3:
                    return LLGL::Format::RGB16UInt;
                case DAsset::ComponentType::VEC4:
                    return LLGL::Format::RGBA16UInt;
                default:
                    RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Invalid component type");
            }
        }
        case DAsset::DataType::INT: {
            switch (componentType) {
                case DAsset::ComponentType::SCALAR:
                    return LLGL::Format::R32SInt;
                case DAsset::ComponentType::VEC2:
                    return LLGL::Format::RG32SInt;
                case DAsset::ComponentType::VEC3:
                    return LLGL::Format::RGB32SInt;
                case DAsset::ComponentType::VEC4:
                    return LLGL::Format::RGBA32SInt;
                default:
                    RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Invalid component type");
            }
        }
        case DAsset::DataType::UNSIGNED_INT: {
            switch (componentType) {
                case DAsset::ComponentType::SCALAR:
                    return LLGL::Format::R32UInt;
                case DAsset::ComponentType::VEC2:
                    return LLGL::Format::RG32UInt;
                case DAsset::ComponentType::VEC3:
                    return LLGL::Format::RGB32UInt;
                case DAsset::ComponentType::VEC4:
                    return LLGL::Format::RGBA32UInt;
                default:
                    RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Invalid component type");
            }
        }
        case DAsset::DataType::FLOAT: {
            switch (componentType) {
                case DAsset::ComponentType::SCALAR:
                    return LLGL::Format::R32Float;
                case DAsset::ComponentType::VEC2:
                    return LLGL::Format::RG32Float;
                case DAsset::ComponentType::VEC3:
                    return LLGL::Format::RGB32Float;
                case DAsset::ComponentType::VEC4:
                    return LLGL::Format::RGBA32Float;
                default:
                    RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Invalid component type");
            }
        }
        default:
            RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "Invalid data type");
    }
}