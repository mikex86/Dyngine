#pragma once

#include <memory>
#include <map>
#include "Dyngine/Input/InputListener.hpp"
#include "Dyngine/Input/InputProvider.hpp"

namespace Dyngine {

    class Input;

    class InputListenerProxy : public InputListener {
    private:
        Input &input;
    public:
        InputListenerProxy(Input &input);

        void onMouseMoveEvent(const MouseMoveEvent &event) override;

        void onMousePressEvent(const MouseButtonPressEvent &event) override;

        void onMouseReleaseEvent(const MouseButtonReleaseEvent &event) override;

        void onMouseDoubleClickEvent(const MouseDoubleClickEvent &event) override;

        void onMouseWheelEvent(const MouseWheelEvent &event) override;

        void onKeyboardKeyPressEvent(const KeyboardKeyPressEvent &event) override;

        void onKeyboardKeyReleaseEvent(const KeyboardKeyReleaseEvent &event) override;

        void onKeyboardCharacterEvent(const KeyboardCharacterEvent &event) override;

    };

    class Input : public InputListener {

    private:
        std::shared_ptr<InputListenerProxy> listenerProxy;
        std::shared_ptr<InputProvider> inputProvider;

        int lastMouseX{}, lastMouseY{};
        int mouseX{}, mouseY{};

        int lastMouseWheelPosition = 0;
        int mouseWheelPosition = 0;

        std::map<MouseButton, bool> mouseButtonDownStates;

        std::map<KeyboardKey, bool> keyboardKeyDownStates;

    public:
        explicit Input(const std::shared_ptr<InputProvider> &inputProvider);

        void onMouseMoveEvent(const MouseMoveEvent &event) override;

        void onMousePressEvent(const MouseButtonPressEvent &event) override;

        void onMouseReleaseEvent(const MouseButtonReleaseEvent &event) override;

        void onMouseWheelEvent(const MouseWheelEvent &event) override;

        void onKeyboardKeyPressEvent(const KeyboardKeyPressEvent &event) override;

        void onKeyboardKeyReleaseEvent(const KeyboardKeyReleaseEvent &event) override;

        void registerInputListener(const std::shared_ptr<InputListener> &inputListener);

        void unregisterInputListener(const std::shared_ptr<InputListener> &inputListener);

        int getMouseX();

        int getMouseY();

        int getMouseDeltaX();

        int getMouseDeltaY();

        int getMouseWheelPosition();

        int getMouseWheelDelta();

        bool isMouseButtonDown(MouseButton button);

        bool isKeyboardKeyDown(KeyboardKey key);

        void computeDeltas();

        virtual ~Input();
    };

}