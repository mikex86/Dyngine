#include "Dyngine/Rendering/Scene/Camera/Controller/FlyingPerspectiveCameraController.hpp"
#include <utility>
#include <iostream>

FlyingPerspectiveCameraController::FlyingPerspectiveCameraController(PerspectiveCamera &camera,
                                                                     std::unique_ptr<Dyngine::Input> &input)
        : camera(camera), input(std::move(input)) {
}

void FlyingPerspectiveCameraController::update(float deltaTime) {
    float moveForward = 0;
    float moveStrafing = 0;
    float moveUp = 0;
    float speedFactor = 0.5f;

    // keyboard and mouse controls
    {
        bool wPressed = input->isKeyboardKeyDown(Dyngine::KeyboardKey::W);
        bool sPressed = input->isKeyboardKeyDown(Dyngine::KeyboardKey::S);
        bool aPressed = input->isKeyboardKeyDown(Dyngine::KeyboardKey::A);
        bool dPressed = input->isKeyboardKeyDown(Dyngine::KeyboardKey::D);
        bool spacePressed = input->isKeyboardKeyDown(Dyngine::KeyboardKey::Space);
        bool shiftPressed = input->isKeyboardKeyDown(Dyngine::KeyboardKey::Shift);
        bool ctrlPressed = input->isKeyboardKeyDown(Dyngine::KeyboardKey::Control);
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
        if (ctrlPressed) {
            speedFactor = 4.0f;
        }

        {
            input->setCursorGrabbed(true);
            auto deltaX = input->getMouseDeltaX();
            auto deltaY = input->getMouseDeltaY();

            if (input->isMouseButtonDown(Dyngine::MouseButton::MIDDLE)) {
                camera.setRoll(camera.getRoll() + static_cast<float>(deltaX) * 0.1f);
            }
            if (deltaX != 0 || deltaY != 0) {
                camera.setRotation(camera.getYaw() + static_cast<float>(deltaX) * 0.1f,
                                   camera.getPitch() + static_cast<float>(deltaY) * -0.1f);
            }
        }
    }
//    else {
//        const LLGL::ControllerState &controllerState = input->GetController(0);
//
//        if (controllerState.isButtonDown(LLGL::ControllerButton::LEFT_BUMPER)) {
//            speedFactor = 4.0f;
//        }
//
//        moveStrafing = controllerState.getJoystickValue(LLGL::ControllerJoystick::LEFT_THUMB,
//                                                        LLGL::JoystickAxis::X) * -1;
//        moveForward = controllerState.getJoystickValue(LLGL::ControllerJoystick::LEFT_THUMB,
//                                                       LLGL::JoystickAxis::Y);
//
//        moveUp = controllerState.isButtonDown(LLGL::ControllerButton::DPAD_UP)
//                 ? 1.0f
//                 :
//                 controllerState.isButtonDown(LLGL::ControllerButton::DPAD_DOWN) ? -1.0f : 0.0f;
//
//        auto rollMove = controllerState.isButtonDown(LLGL::ControllerButton::DPAD_RIGHT)
//                        ? 1.0f
//                        :
//                        controllerState.isButtonDown(LLGL::ControllerButton::DPAD_LEFT) ? -1.0f : 0.0f;
//
//        auto yawMove = controllerState.getJoystickValue(LLGL::ControllerJoystick::RIGHT_THUMB,
//                                                        LLGL::JoystickAxis::X);
//        auto pitchMove = controllerState.getJoystickValue(LLGL::ControllerJoystick::RIGHT_THUMB,
//                                                          LLGL::JoystickAxis::Y) * -1.0f;
//
//        moveStrafing = applyDeadZone(moveStrafing, 0.1f);
//        moveForward = applyDeadZone(moveForward, 0.1f);
//        moveUp = applyDeadZone(moveUp, 0.1f);
//        rollMove = applyDeadZone(rollMove, 0.1f);
//        yawMove = applyDeadZone(yawMove, 0.1f);
//        pitchMove = applyDeadZone(pitchMove, 0.1f);
//
//        camera.setRotation(camera.getYaw() + yawMove * 0.01f,
//                           camera.getPitch() + pitchMove * -0.01f);
//        camera.setRoll(camera.getRoll() + rollMove * rollMove * 0.01f);
//    }
//    display->ShowCursor(!window->HasFocus());

    float targetFov = 70 + (speedFactor * 5 * (moveForward > 0.1) * moveForward);
    float currentFov = camera.getFov();
    float newFov = currentFov + (targetFov - currentFov) * 10.0f * deltaTime;
    camera.setFov(newFov);

    if (moveStrafing != 0 || moveForward != 0 || moveUp != 0) {
        auto direction = camera.getDirection() * moveForward + camera.getCameraRight() * moveStrafing +
                         camera.getCameraUp() * moveUp;
        auto speed = speedFactor * deltaTime;
        camera.setPosition(camera.getPosition() + direction * speed);
    }
}

float FlyingPerspectiveCameraController::applyDeadZone(float value, float deadZone) {
    return (abs(value) > deadZone) * value;
}
