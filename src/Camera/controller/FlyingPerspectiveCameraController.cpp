#include "FlyingPerspectiveCameraController.hpp"
#include <utility>

FlyingPerspectiveCameraController::FlyingPerspectiveCameraController(PerspectiveCamera &camera,
                                                                     std::shared_ptr<LLGL::Input> input,
                                                                     std::shared_ptr<LLGL::Display> display,
                                                                     std::shared_ptr<LLGL::Window> window)
        : camera(camera), input(std::move(input)), display(std::move(display)), window(std::move(window)) {
}

void FlyingPerspectiveCameraController::update(float deltaTime) {
    float moveForward = 0;
    float moveStrafing = 0;
    float moveUp = 0;
    {
        bool wPressed = input->KeyPressed(LLGL::Key::W);
        bool sPressed = input->KeyPressed(LLGL::Key::S);
        bool aPressed = input->KeyPressed(LLGL::Key::A);
        bool dPressed = input->KeyPressed(LLGL::Key::D);
        bool spacePressed = input->KeyPressed(LLGL::Key::Space);
        bool shiftPressed = input->KeyPressed(LLGL::Key::Shift);
        if (wPressed ^ sPressed) {
            if (wPressed)
                moveForward = 1;
            if (sPressed)
                moveForward = -1;
        }
        if (aPressed ^ dPressed) {
            if (aPressed)
                moveStrafing = 1;
            if (dPressed)
                moveStrafing = -1;
        }
        if (spacePressed ^ shiftPressed) {
            if (spacePressed)
                moveUp = 1;
            if (shiftPressed)
                moveUp = -1;
        }
    }
    auto windowSize = window->GetSize();
    auto windowPosition = window->GetPosition();

    if (window->HasFocus()) {
        auto mouseMotion = input->GetMouseMotion();
        if (input->KeyDown(LLGL::Key::Q)) {
            camera.setRoll(camera.getRoll() + static_cast<float>(mouseMotion.x) * 0.1f);
        } else {
            display->ShowCursor(false);
            display->SetCursorPosition(static_cast<int>(windowPosition.x + windowSize.width / 2),
                                       static_cast<int>(windowPosition.y + windowSize.height / 2));
            camera.setRotation(camera.getYaw() + static_cast<float>(mouseMotion.x) * 0.1f,
                               camera.getPitch() + static_cast<float>(mouseMotion.y) * -0.1f);
        }
    } else {
        display->ShowCursor(true);
    }

    if (moveStrafing != 0 || moveForward != 0 || moveUp != 0) {
        auto direction = camera.getDirection() * moveForward + camera.getCameraRight() * moveStrafing +
                         camera.getCameraUp() * moveUp;
        auto speed = 2.0f * deltaTime;
        camera.setPosition(camera.getPosition() + direction * speed);
    }

}