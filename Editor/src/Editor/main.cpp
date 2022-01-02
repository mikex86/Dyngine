#include <QApplication>
#include <QPushButton>
#include "ui_editorwindow.h"

#include <Dyngine/Dyngine.hpp>
#include <Dyngine/EngineRenderContext.hpp>
#include <Dyngine/EngineRenderTarget.hpp>

#include "Editor/Input/QtInputProvider.hpp"

int main(int argc, char **argv) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    QMainWindow *window = new QMainWindow();
    Ui::EditorWindow ui;
    ui.setupUi(window);

    auto renderContext = std::make_unique<Dyngine::EngineRenderContext>();
    auto frameBufferRenderTarget = std::make_shared<Dyngine::FrameBufferRenderTarget>(renderContext);

    auto qtInputProvider = std::make_shared<Editor::QtInputProvider>(ui.frameBuffer);

    Dyngine::EngineInstance *engineInstance = new Dyngine::EngineInstance(
            frameBufferRenderTarget,
            qtInputProvider
    );
    engineInstance->startEngine();
    window->show();

    uint8_t *pixelBuffer = nullptr;
    size_t pixelBufferSize = 0;
    QSize oldSize{0, 0};

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (window->isVisible()) {

        auto currentTime = std::chrono::high_resolution_clock::now();
        auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
        lastTime = currentTime;
        int fps = deltaTime > 0 ? 1000 / deltaTime : 0;

        auto frameBufferSize = ui.frameBuffer->size();

        if (frameBufferSize != oldSize) {
            oldSize = frameBufferSize;
            frameBufferRenderTarget->resizeFrameBuffer(frameBufferSize.width(), frameBufferSize.height());
        }

        app.processEvents();
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
        ui.frameBuffer->setPixmap(QPixmap::fromImage(image.mirrored(false, true)));
        ui.fpsLabel->setText(QString("FPS: %1").arg(fps));

        // TODO: Implement cursor grab api
        // TODO: Implement controller support
    }

    delete window;
    delete engineInstance;
    return 0;
}