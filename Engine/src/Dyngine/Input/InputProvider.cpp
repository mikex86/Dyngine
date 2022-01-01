#include "Dyngine/Input/InputProvider.hpp"
#include "Dyngine/Input/InputListener.hpp"

namespace Dyngine {

#define INVOKE_LISTENERS(listenerMethod, event) \
    for (auto &listener : inputListeners) { \
        listener->listenerMethod((event)); \
    }

    void InputProvider::fireMouseMoveEvent(const MouseMoveEvent &event) {
        INVOKE_LISTENERS(onMouseMoveEvent, event);
    }

    void InputProvider::fireMouseButtonPressEvent(const MouseButtonPressEvent &event) {
        INVOKE_LISTENERS(onMousePressEvent, event);
    }

    void InputProvider::fireMouseButtonReleaseEvent(const MouseButtonReleaseEvent &event) {
        INVOKE_LISTENERS(onMouseReleaseEvent, event);
    }

    void InputProvider::fireMouseDoubleClickEvent(const MouseDoubleClickEvent &event) {
        INVOKE_LISTENERS(onMouseDoubleClickEvent, event);
    }

    void InputProvider::fireMouseWheelEvent(const MouseWheelEvent &event) {
        INVOKE_LISTENERS(onMouseWheelEvent, event);
    }

    void InputProvider::fireKeyboardKeyPressEvent(const KeyboardKeyPressEvent &event) {
        INVOKE_LISTENERS(onKeyboardKeyPressEvent, event);
    }

    void InputProvider::fireKeyboardKeyReleaseEvent(const KeyboardKeyReleaseEvent &event) {
        INVOKE_LISTENERS(onKeyboardKeyReleaseEvent, event);
    }

    void InputProvider::fireKeyboardCharacterEvent(const KeyboardCharacterEvent &event) {
        INVOKE_LISTENERS(onKeyboardCharacterEvent, event);
    }

    void InputProvider::registerListener(const std::shared_ptr<InputListener> &listener) {
        inputListeners.push_back(listener);
    }

    void InputProvider::unregisterListener(const std::shared_ptr<InputListener> &listener) {
        inputListeners.erase(std::remove(inputListeners.begin(), inputListeners.end(), listener), inputListeners.end());
    }

}