#pragma once

#include "EngineRenderContext.hpp"

namespace Dyngine {

    class EngineRenderTarget {
    protected:
        std::unique_ptr<EngineRenderContext> renderContext;

    public:
        EngineRenderTarget(std::unique_ptr<EngineRenderContext> &renderContext);

        const std::unique_ptr<EngineRenderContext> &getRenderContext() const;

        virtual ~EngineRenderTarget();
    };

    class FrameBufferRenderTargetState;

    class FrameBufferRenderTarget : public EngineRenderTarget {
    private:
        FrameBufferRenderTargetState *renderTargetState;

    public:
        FrameBufferRenderTarget(std::unique_ptr<EngineRenderContext> &renderContext);

        void resizeFrameBuffer(int width, int height);

        void getFrameBufferSize(int &width, int &height);

        void dumpFrameBufferContentsRGBA(uint8_t *dstBuffer, size_t dstBufferLength);

        FrameBufferRenderTargetState *getRenderTargetState() const;

        virtual ~FrameBufferRenderTarget();

    };
}