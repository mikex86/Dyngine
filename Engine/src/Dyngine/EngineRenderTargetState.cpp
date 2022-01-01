#include "Dyngine/EngineRenderTargetState.hpp"
#include "Dyngine/EngineRenderContext.hpp"
#include "Dyngine/EngineRenderContextState.hpp"

Dyngine::FrameBufferRenderTargetState::FrameBufferRenderTargetState(std::unique_ptr<EngineRenderContext> &renderSystem,
                                                                    uint32_t width, uint32_t height)
        : renderSystem(renderSystem->getRenderContextState()->renderSystem) {
    auto renderContextState = renderSystem->getRenderContextState();

    // Create render pass
    {
        LLGL::RenderPassDescriptor renderPassDescriptor{
                .colorAttachments = {
                        {
                                LLGL::Format::RGBA8UNorm,
                                LLGL::AttachmentLoadOp::Clear,
                                LLGL::AttachmentStoreOp::Store,
                        }
                },
                .samples = 4
        };
        renderPass = this->renderSystem->CreateRenderPass(renderPassDescriptor);
    }

    // Create render context
    {
        LLGL::RenderContextDescriptor renderContextDescriptor{
            .samples = 4,
        };
        renderContextState->renderContext = this->renderSystem->CreateRenderContext(renderContextDescriptor);
    }

    // Create command buffer
    {
        LLGL::CommandBufferDescriptor commandBufferDescriptor{
                .flags =(LLGL::CommandBufferFlags::MultiSubmit)
        };
        renderContextState->commandBuffer = this->renderSystem->CreateCommandBuffer(commandBufferDescriptor);
    }
    // Create render target texture
    {
        LLGL::TextureDescriptor textureDescriptor{
                .type = LLGL::TextureType::Texture2D,
                .bindFlags = LLGL::BindFlags::ColorAttachment, // required for render pass
                .miscFlags = 0,
                .format = LLGL::Format::RGBA8UNorm,
                .extent = {width, height, 1},
        };
        renderTargetTexture = this->renderSystem->CreateTexture(textureDescriptor);
    }

    // Create render target
    {
        LLGL::RenderTargetDescriptor renderTargetDescriptor{
                .renderPass = renderPass,
                .resolution = {width, height},
                .attachments = {
                        {
                                LLGL::AttachmentType::Color,
                                renderTargetTexture
                        },
                        {
                                LLGL::AttachmentType::Depth
                        }
                }
        };
        renderContextState->renderTarget = this->renderSystem->CreateRenderTarget(renderTargetDescriptor);
    }
}

Dyngine::FrameBufferRenderTargetState::~FrameBufferRenderTargetState() {
    renderSystem->Release(*renderPass);
    renderSystem->Release(*renderTargetTexture);
}
