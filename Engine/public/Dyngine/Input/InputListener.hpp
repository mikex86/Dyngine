#pragma once

namespace Dyngine {

    class MouseMoveEvent;
    class MouseButtonPressEvent;
    class MouseButtonReleaseEvent;
    class MouseDoubleClickEvent;
    class MouseWheelEvent;
    class KeyboardKeyPressEvent;
    class KeyboardKeyReleaseEvent;
    class KeyboardCharacterEvent;

    class InputListener {

    public:
        virtual void onMouseMoveEvent(const MouseMoveEvent &event) {};

        virtual void onMousePressEvent(const MouseButtonPressEvent &event) {};

        virtual void onMouseReleaseEvent(const MouseButtonReleaseEvent &event) {};

        virtual void onMouseDoubleClickEvent(const MouseDoubleClickEvent &event) {};

        virtual void onMouseWheelEvent(const MouseWheelEvent &event) {};

        virtual void onKeyboardKeyPressEvent(const KeyboardKeyPressEvent &event) {};

        virtual void onKeyboardKeyReleaseEvent(const KeyboardKeyReleaseEvent &event) {};

        virtual void onKeyboardCharacterEvent(const KeyboardCharacterEvent &event) {};

    };

}