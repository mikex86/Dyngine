#include "Dyngine/EngineRenderTarget.hpp"
#include "Dyngine/EngineRenderContextState.hpp"
#include "Dyngine/EngineRenderTargetState.hpp"
#include "ErrorHandling/IllegalArgumentException.hpp"
#include <LLGL/LLGL.h>

namespace Dyngine {

    EngineRenderTarget::EngineRenderTarget(std::unique_ptr<EngineRenderContext> &renderContext) :
            renderContext(std::move(renderContext)) {
    }

    FrameBufferRenderTarget::FrameBufferRenderTarget(std::unique_ptr<EngineRenderContext> &engineRenderContext)
            : EngineRenderTarget(engineRenderContext) {
        renderTargetState = new FrameBufferRenderTargetState(this->renderContext, 1280, 720);
    }

    void FrameBufferRenderTarget::resizeFrameBuffer(int width, int height) {
        if (renderTargetState != nullptr) {
            delete renderTargetState;
        }
        renderTargetState = new FrameBufferRenderTargetState(this->renderContext, width, height);
    }

    void FrameBufferRenderTarget::getFrameBufferSize(int &width, int &height) {
        auto resolution = renderContext->getRenderContextState()->renderTarget->GetResolution();
        width = resolution.width;
        height = resolution.height;
    }

    void FrameBufferRenderTarget::dumpFrameBufferContentsRGBA(uint8_t *dstBuffer, size_t dstBufferLength) {
        auto resolution = renderContext->getRenderContextState()->renderTarget->GetResolution();
        if (dstBufferLength < resolution.width * resolution.height * 4) {
            RAISE_EXCEPTION(errorhandling::IllegalArgumentException, "dstBuffer too small");
        }
        // Dump render target texture data
        {
            LLGL::TextureRegion textureRegion{
                    LLGL::Offset3D{0, 0, 0},
                    LLGL::Extent3D{resolution.width, resolution.height, 1}
            };
            LLGL::DstImageDescriptor dstImageDescriptor{
                    LLGL::ImageFormat::RGBA,
                    LLGL::DataType::UInt8,
                    dstBuffer,
                    dstBufferLength
            };
            renderTargetState->renderSystem->ReadTexture(*renderTargetState->renderTargetTexture, textureRegion,
                                                         dstImageDescriptor);
        }
    }

    FrameBufferRenderTargetState *FrameBufferRenderTarget::getRenderTargetState() const {
        return renderTargetState;
    }

    EngineRenderTarget::~EngineRenderTarget() {
    }

    FrameBufferRenderTarget::~FrameBufferRenderTarget() {
        delete renderTargetState;
    }

    const std::unique_ptr<EngineRenderContext> &EngineRenderTarget::getRenderContext() const {
        return renderContext;
    }

}