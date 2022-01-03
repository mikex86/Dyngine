#include "Editor/Ui/ViewportController.hpp"

Editor::ViewportController::ViewportController(const std::shared_ptr<Dyngine::EngineInstance> &engineInstance,
                                               const std::shared_ptr<Dyngine::FrameBufferRenderTarget> &frameBufferRenderTarget,
                                               QLabel *viewportLabel)
        : engineInstance(engineInstance),
          frameBufferRenderTarget(frameBufferRenderTarget),
          viewportLabel(viewportLabel) {
}

void Editor::ViewportController::update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
    lastTime = currentTime;
    int fps = deltaTime > 0 ? 1000 / deltaTime : 0;

    auto frameBufferSize = viewportLabel->size();

    if (frameBufferSize != oldSize) {
        oldSize = frameBufferSize;
        frameBufferRenderTarget->resizeFrameBuffer(frameBufferSize.width(), frameBufferSize.height());
    }
    engineInstance->renderFrame();

    int width{}, height{};
    frameBufferRenderTarget->getFrameBufferSize(width, height);

    size_t requiredBufferSize = width * height * 4;
    if (requiredBufferSize != pixelBufferSize) {
        pixelBufferSize = requiredBufferSize;
        delete[] pixelBuffer;
        pixelBuffer = new uint8_t[requiredBufferSize];
    }
    frameBufferRenderTarget->dumpFrameBufferContentsRGBA(pixelBuffer, pixelBufferSize);

    QImage image(pixelBuffer, width, height, QImage::Format_RGBA8888);
    viewportLabel->setPixmap(QPixmap::fromImage(image.mirrored(false, true)));
//        ui.fpsLabel->setText(QString("FPS: %1").arg(fps));
}
