#include "Dyngine/Input/Input.hpp"

namespace Dyngine {
    
    Input::Input(const std::shared_ptr<InputProvider> &inputProvider) : inputProvider(inputProvider), listenerProxy(std::make_shared<InputListenerProxy>(*this)) {
        inputProvider->registerListener(listenerProxy);
    }

    void Input::registerInputListener(const std::shared_ptr<InputListener> &inputListener) {
        inputProvider->registerListener(inputListener);
    }

    void Input::unregisterInputListener(const std::shared_ptr<InputListener> &inputListener) {
        inputProvider->unregisterListener(inputListener);
    }

    void Input::onMouseMoveEvent(const MouseMoveEvent &event) {
        mouseX = event.x;
        mouseY = event.y;
    }

    void Input::onMousePressEvent(const MouseButtonPressEvent &event) {
        mouseButtonDownStates[event.button] = true;
    }

    void Input::onMouseReleaseEvent(const MouseButtonReleaseEvent &event) {
        mouseButtonDownStates[event.button] = false;
    }

    void Input::onMouseWheelEvent(const MouseWheelEvent &event) {
        mouseWheelPosition += event.delta;
    }

    void Input::onKeyboardKeyPressEvent(const KeyboardKeyPressEvent &event) {
        keyboardKeyDownStates[event.key] = true;
    }

    void Input::onKeyboardKeyReleaseEvent(const KeyboardKeyReleaseEvent &event) {
        keyboardKeyDownStates[event.key] = false;
    }

    int Input::getMouseX() {
        return mouseX;
    }

    int Input::getMouseY() {
        return mouseY;
    }

    int Input::getMouseDeltaX() {
        return mouseX - lastMouseX;
    }

    int Input::getMouseDeltaY() {
        return mouseY - lastMouseY;
    }

    int Input::getMouseWheelPosition() {
        return mouseWheelPosition;
    }

    int Input::getMouseWheelDelta() {
        return mouseWheelPosition - lastMouseWheelPosition;
    }

    bool Input::isMouseButtonDown(MouseButton button) {
        auto it = mouseButtonDownStates.find(button);
        if (it != mouseButtonDownStates.end()) {
            return it->second;
        }
        return false;
    }

    bool Input::isKeyboardKeyDown(KeyboardKey key) {
        auto it = keyboardKeyDownStates.find(key);
        if (it != keyboardKeyDownStates.end()) {
            return it->second;
        }
        return false;
    }

    void Input::computeDeltas() {
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        lastMouseWheelPosition = mouseWheelPosition;
    }

    Input::~Input() {
        inputProvider->unregisterListener(listenerProxy);
        listenerProxy = nullptr;
    }

    InputListenerProxy::InputListenerProxy(Input &input) : input(input) {
    }

    void InputListenerProxy::onMouseMoveEvent(const MouseMoveEvent &event) {
        input.onMouseMoveEvent(event);
    }

    void InputListenerProxy::onMousePressEvent(const MouseButtonPressEvent &event) {
        input.onMousePressEvent(event);
    }

    void InputListenerProxy::onMouseReleaseEvent(const MouseButtonReleaseEvent &event) {
        input.onMouseReleaseEvent(event);
    }

    void InputListenerProxy::onMouseDoubleClickEvent(const MouseDoubleClickEvent &event) {
        input.onMouseDoubleClickEvent(event);
    }

    void InputListenerProxy::onMouseWheelEvent(const MouseWheelEvent &event) {
        input.onMouseWheelEvent(event);
    }

    void InputListenerProxy::onKeyboardKeyPressEvent(const KeyboardKeyPressEvent &event) {
        input.onKeyboardKeyPressEvent(event);
    }

    void InputListenerProxy::onKeyboardKeyReleaseEvent(const KeyboardKeyReleaseEvent &event) {
        input.onKeyboardKeyReleaseEvent(event);
    }

    void InputListenerProxy::onKeyboardCharacterEvent(const KeyboardCharacterEvent &event) {
        input.onKeyboardCharacterEvent(event);
    }

}