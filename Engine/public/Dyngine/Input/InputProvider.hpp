#pragma once

#include <vector>
#include <memory>

namespace Dyngine {

    enum MouseButton {
        LEFT, MIDDLE, RIGHT, X1, X2,

        UNKNOWN = -1
    };


    enum KeyboardKey {
        BackSpace,          //!< BACKSPACE key.
        Tab,                //!< TAB key.
        Clear,              //!< CLEAR key.
        Return,             //!< RETURN (or ENTER) key.
        Shift,              //!< SHIFT key.
        Control,            //!< CTRL key.

        Menu,               //!< ALT key.
        Pause,              //!< PAUSE key.
        Capital,            //!< CAPS LOCK key.

        Escape,             //!< Escape (ESC) key.
        Space,              //!< Space key.
        PageUp,             //!< Page up key.
        PageDown,           //!< Page down key.
        End,                //!< END key.
        Home,               //!< HOME (or POS1) key.

        Left,               //!< Left arrow key.
        Up,                 //!< Up arrow key.
        Right,              //!< Right arrow key.
        Down,               //!< Down arrow key.

        Select,             //!< Select key.
        Print,              //!< Print key.
        Exe,                //!< Execute key.
        Snapshot,           //!< Snapshot key.
        Insert,             //!< Insert key.
        Delete,             //!< Delete key.
        Help,               //!< Help key.

        D0,                 //!< Digit 0.
        D1,                 //!< Digit 1.
        D2,                 //!< Digit 2.
        D3,                 //!< Digit 3.
        D4,                 //!< Digit 4.
        D5,                 //!< Digit 5.
        D6,                 //!< Digit 6.
        D7,                 //!< Digit 7.
        D8,                 //!< Digit 8.
        D9,                 //!< Digit 9.

        A,                  //!< Letter A.
        B,                  //!< Letter B.
        C,                  //!< Letter C.
        D,                  //!< Letter D.
        E,                  //!< Letter E.
        F,                  //!< Letter F.
        G,                  //!< Letter G.
        H,                  //!< Letter H.
        I,                  //!< Letter I.
        J,                  //!< Letter J.
        K,                  //!< Letter K.
        L,                  //!< Letter L.
        M,                  //!< Letter M.
        N,                  //!< Letter N.
        O,                  //!< Letter O.
        P,                  //!< Letter P.
        Q,                  //!< Letter Q.
        R,                  //!< Letter R.
        S,                  //!< Letter S.
        T,                  //!< Letter T.
        U,                  //!< Letter U.
        V,                  //!< Letter V.
        W,                  //!< Letter W.
        X,                  //!< Letter X.
        Y,                  //!< Letter Y.
        Z,                  //!< Letter Z.

        LWin,               //!< Left Windows key.
        RWin,               //!< Right Windows key.
        Apps,               //!< Application key.
        Sleep,              //!< Sleep key.

        Keypad0,            //!< Keypad 0 key.
        Keypad1,            //!< Keypad 1 key.
        Keypad2,            //!< Keypad 2 key.
        Keypad3,            //!< Keypad 3 key.
        Keypad4,            //!< Keypad 4 key.
        Keypad5,            //!< Keypad 5 key.
        Keypad6,            //!< Keypad 6 key.
        Keypad7,            //!< Keypad 7 key.
        Keypad8,            //!< Keypad 8 key.
        Keypad9,            //!< Keypad 9 key.

        KeypadMultiply,     //!< Keypad multiply '*'.
        KeypadPlus,         //!< Keypad plus '+'.
        KeypadSeparator,    //!< Keypad separator.
        KeypadMinus,        //!< Keypad minus '-'.
        KeypadDecimal,      //!< Keypad decimal ',' or '.' (depends on language).
        KeypadDivide,       //!< Keypad divide '/'.

        F1,                 //!< F1 function key.
        F2,                 //!< F2 function key.
        F3,                 //!< F3 function key.
        F4,                 //!< F4 function key.
        F5,                 //!< F5 function key.
        F6,                 //!< F6 function key.
        F7,                 //!< F7 function key.
        F8,                 //!< F8 function key.
        F9,                 //!< F9 function key.
        F10,                //!< F10 function key.
        F11,                //!< F11 function key.
        F12,                //!< F12 function key.
        F13,                //!< F13 function key.
        F14,                //!< F14 function key.
        F15,                //!< F15 function key.
        F16,                //!< F16 function key.
        F17,                //!< F17 function key.
        F18,                //!< F18 function key.
        F19,                //!< F19 function key.
        F20,                //!< F20 function key.
        F21,                //!< F21 function key.
        F22,                //!< F22 function key.
        F23,                //!< F23 function key.
        F24,                //!< F24 function key.

        NumLock,            //!< Num lock key.
        ScrollLock,         //!< Scroll lock key.

        LShift,             //!< Left shift key.
        RShift,             //!< Right shift key.
        LControl,           //!< Left control (CTRL) key.
        RControl,           //!< Right control (CTRL) key.
        LMenu,              //!< Left menu key.
        RMenu,              //!< Right menu key.

        BrowserBack,
        BrowserForward,
        BrowserRefresh,
        BrowserStop,
        BrowserSearch,
        BrowserFavorits,
        BrowserHome,

        VolumeMute,
        VolumeDown,
        VolumeUp,

        MediaNextTrack,
        MediaPrevTrack,
        MediaStop,
        MediaPlayPause,

        LaunchMail,
        LaunchMediaSelect,
        LaunchApp1,
        LaunchApp2,

        Plus,               //!< '+'
        Comma,              //!< ','
        Minus,              //!< '-'
        Period,             //!< '.'

        Exponent,           //!< '^'

        Unknown = -1        //!< Unknown key.
    };

    class MouseButtonPressEvent {
    public:
        MouseButton button;
    };

    class MouseButtonReleaseEvent {
    public:
        MouseButton button;
    };

    class MouseDoubleClickEvent {
    public:
        int x;
        int y;
        MouseButton button;
    };

    class MouseMoveEvent {
    public:
        int x;
        int y;
    };

    class MouseWheelEvent {
    public:
        int x;
        int y;
        int delta;
    };

    class KeyboardKeyPressEvent {
    public:
        KeyboardKey key;
        bool isRepeat;
    };

    class KeyboardKeyReleaseEvent {
    public:
        KeyboardKey key;
    };

    class KeyboardCharacterEvent {
    public:
        char character;
    };

    class CursorDeltaEvent {
    public:
        int x;
        int y;
    };

    class InputListener;

    class InputProvider {

    private:
        int deltaX, deltaY;
        std::vector<std::shared_ptr<InputListener>> inputListeners;

    protected:
        virtual void fireMouseMoveEvent(const MouseMoveEvent &event);

        virtual void fireMouseButtonPressEvent(const MouseButtonPressEvent &event);

        virtual void fireMouseButtonReleaseEvent(const MouseButtonReleaseEvent &event);

        virtual void fireMouseDoubleClickEvent(const MouseDoubleClickEvent &event);

        virtual void fireMouseWheelEvent(const MouseWheelEvent &event);

        virtual void fireKeyboardKeyPressEvent(const KeyboardKeyPressEvent &event);

        virtual void fireKeyboardKeyReleaseEvent(const KeyboardKeyReleaseEvent &event);

        virtual void fireKeyboardCharacterEvent(const KeyboardCharacterEvent &event);

    public:
        void registerListener(const std::shared_ptr<InputListener> &listener);

        void unregisterListener(const std::shared_ptr<InputListener> &listener);

        virtual void setCursorGrabbed(bool grabbed) = 0;
    };

}