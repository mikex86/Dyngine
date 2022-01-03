#pragma once

#include <Dyngine/Dyngine.hpp>
#include <Dyngine/EngineRenderContext.hpp>
#include <Dyngine/EngineRenderTarget.hpp>
#include <Editor/Input/QtInputProvider.hpp>

#include "ui_editorwindow.h"

namespace Editor {

    class ViewportController {

    private:
        QLabel *viewportLabel;

        uint8_t *pixelBuffer = nullptr;
        size_t pixelBufferSize = 0;
        QSize oldSize{0, 0};

        std::shared_ptr<Dyngine::EngineInstance> engineInstance;

        std::chrono::time_point<std::chrono::steady_clock> lastTime;
        std::shared_ptr<Dyngine::FrameBufferRenderTarget> frameBufferRenderTarget;

    public:
        ViewportController(const std::shared_ptr<Dyngine::EngineInstance> &engineInstance, const std::shared_ptr<Dyngine::FrameBufferRenderTarget> &frameBufferRenderTarget, QLabel *viewportLabel);

        void update();
    };

}