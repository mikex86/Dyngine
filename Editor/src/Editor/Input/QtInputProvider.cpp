#include "Editor/Input/QtInputProvider.hpp"
#include "ErrorHandling/IllegalArgumentException.hpp"

namespace Editor {

    using namespace Dyngine;

    QtInputProvider::QtInputProvider() {
        QCoreApplication::instance()->installEventFilter(this);
    }

    MouseButton FromQtMouseButton(Qt::MouseButton button) {
        switch (button) {
            case Qt::MouseButton::LeftButton:
                return MouseButton::LEFT;
            case Qt::MouseButton::RightButton:
                return MouseButton::RIGHT;
            case Qt::MouseButton::MiddleButton:
                return MouseButton::MIDDLE;
            case Qt::MouseButton::XButton1:
                return MouseButton::X1;
            case Qt::MouseButton::XButton2:
                return MouseButton::X2;
            default:
                return MouseButton::UNKNOWN;
        }
    }

    KeyboardKey FromQtKeyboardKey(Qt::Key key) {
        switch (key) {
            case Qt::Key_Escape:
                return KeyboardKey::Escape;
            case Qt::Key_Tab:
                return KeyboardKey::Tab;
            case Qt::Key_Backtab:
                return KeyboardKey::BackSpace;
            case Qt::Key_Backspace:
                return KeyboardKey::BackSpace;
            case Qt::Key_Return:
                return KeyboardKey::Return;
            case Qt::Key_Enter:
                return KeyboardKey::Return;
            case Qt::Key_Insert:
                return KeyboardKey::Insert;
            case Qt::Key_Delete:
                return KeyboardKey::Delete;
            case Qt::Key_Pause:
                return KeyboardKey::Pause;
            case Qt::Key_Print:
                return KeyboardKey::Print;
            case Qt::Key_Clear:
                return KeyboardKey::Clear;
            case Qt::Key_Home:
                return KeyboardKey::Home;
            case Qt::Key_End:
                return KeyboardKey::End;
            case Qt::Key_Left:
                return KeyboardKey::Left;
            case Qt::Key_Up:
                return KeyboardKey::Up;
            case Qt::Key_Right:
                return KeyboardKey::Right;
            case Qt::Key_Down:
                return KeyboardKey::Down;
            case Qt::Key_PageUp:
                return KeyboardKey::PageUp;
            case Qt::Key_PageDown:
                return KeyboardKey::PageDown;
            case Qt::Key_Shift:
                return KeyboardKey::Shift;
            case Qt::Key_Control:
                return KeyboardKey::Control;
            case Qt::Key_Alt:
                return KeyboardKey::Menu;
            case Qt::Key_Meta:
                return KeyboardKey::Menu;
            case Qt::Key_AltGr:
                return KeyboardKey::Menu;
            case Qt::Key_F1:
                return KeyboardKey::F1;
            case Qt::Key_F2:
                return KeyboardKey::F2;
            case Qt::Key_F3:
                return KeyboardKey::F3;
            case Qt::Key_F4:
                return KeyboardKey::F4;
            case Qt::Key_F5:
                return KeyboardKey::F5;
            case Qt::Key_F6:
                return KeyboardKey::F6;
            case Qt::Key_F7:
                return KeyboardKey::F7;
            case Qt::Key_F8:
                return KeyboardKey::F8;
            case Qt::Key_F9:
                return KeyboardKey::F9;
            case Qt::Key_F10:
                return KeyboardKey::F10;
            case Qt::Key_F11:
                return KeyboardKey::F11;
            case Qt::Key_F12:
                return KeyboardKey::F12;
            case Qt::Key_F13:
                return KeyboardKey::F13;
            case Qt::Key_F14:
                return KeyboardKey::F14;
            case Qt::Key_F15:
                return KeyboardKey::F15;
            case Qt::Key_F16:
                return KeyboardKey::F16;
            case Qt::Key_F17:
                return KeyboardKey::F17;
            case Qt::Key_F18:
                return KeyboardKey::F18;
            case Qt::Key_F19:
                return KeyboardKey::F19;
            case Qt::Key_F20:
                return KeyboardKey::F20;
            case Qt::Key_F21:
                return KeyboardKey::F21;
            case Qt::Key_F22:
                return KeyboardKey::F22;
            case Qt::Key_F23:
                return KeyboardKey::F23;
            case Qt::Key_F24:
                return KeyboardKey::F24;
            case Qt::Key_Space:
                return KeyboardKey::Space;
            case Qt::Key_Asterisk:
                return KeyboardKey::KeypadMultiply;
            case Qt::Key_Plus:
                return KeyboardKey::KeypadPlus;
            case Qt::Key_Comma:
                return KeyboardKey::Comma;
            case Qt::Key_Minus:
                return KeyboardKey::KeypadMinus;
            case Qt::Key_Period:
                return KeyboardKey::Period;
            case Qt::Key_Slash:
                return KeyboardKey::KeypadDivide;
            case Qt::Key_0:
                return KeyboardKey::Keypad0;
            case Qt::Key_1:
                return KeyboardKey::Keypad1;
            case Qt::Key_2:
                return KeyboardKey::Keypad2;
            case Qt::Key_3:
                return KeyboardKey::Keypad3;
            case Qt::Key_4:
                return KeyboardKey::Keypad4;
            case Qt::Key_5:
                return KeyboardKey::Keypad5;
            case Qt::Key_6:
                return KeyboardKey::Keypad6;
            case Qt::Key_7:
                return KeyboardKey::Keypad7;
            case Qt::Key_8:
                return KeyboardKey::Keypad8;
            case Qt::Key_9:
                return KeyboardKey::Keypad9;
            case Qt::Key_A:
                return KeyboardKey::A;
            case Qt::Key_B:
                return KeyboardKey::B;
            case Qt::Key_C:
                return KeyboardKey::C;
            case Qt::Key_D:
                return KeyboardKey::D;
            case Qt::Key_E:
                return KeyboardKey::E;
            case Qt::Key_F:
                return KeyboardKey::F;
            case Qt::Key_G:
                return KeyboardKey::G;
            case Qt::Key_H:
                return KeyboardKey::H;
            case Qt::Key_I:
                return KeyboardKey::I;
            case Qt::Key_J:
                return KeyboardKey::J;
            case Qt::Key_K:
                return KeyboardKey::K;
            case Qt::Key_L:
                return KeyboardKey::L;
            case Qt::Key_M:
                return KeyboardKey::M;
            case Qt::Key_N:
                return KeyboardKey::N;
            case Qt::Key_O:
                return KeyboardKey::O;
            case Qt::Key_P:
                return KeyboardKey::P;
            case Qt::Key_Q:
                return KeyboardKey::Q;
            case Qt::Key_R:
                return KeyboardKey::R;
            case Qt::Key_S:
                return KeyboardKey::S;
            case Qt::Key_T:
                return KeyboardKey::T;
            case Qt::Key_U:
                return KeyboardKey::U;
            case Qt::Key_V:
                return KeyboardKey::V;
            case Qt::Key_W:
                return KeyboardKey::W;
            case Qt::Key_X:
                return KeyboardKey::X;
            case Qt::Key_Y:
                return KeyboardKey::Y;
            case Qt::Key_Z:
                return KeyboardKey::Z;
            case Qt::Key_NumLock:
                return KeyboardKey::NumLock;
            case Qt::Key_ScrollLock:
                return KeyboardKey::ScrollLock;
            default:
                return KeyboardKey::Unknown;
        }
    }

    bool QtInputProvider::eventFilter(QObject *watched, QEvent *event) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            MouseButtonPressEvent mouseButtonPressEvent{
                    .x = mouseEvent->x(),
                    .y = mouseEvent->y(),
                    .button = FromQtMouseButton(mouseEvent->button()),
            };
            fireMouseButtonPressEvent(mouseButtonPressEvent);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            MouseButtonReleaseEvent mouseButtonReleaseEvent{
                    .x = mouseEvent->x(),
                    .y = mouseEvent->y(),
                    .button = FromQtMouseButton(mouseEvent->button()),
            };
            fireMouseButtonReleaseEvent(mouseButtonReleaseEvent);
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            MouseMoveEvent mouseMoveEvent{
                    .x = mouseEvent->x(),
                    .y = mouseEvent->y(),
            };
            fireMouseMoveEvent(mouseMoveEvent);
        } else if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            KeyboardKeyPressEvent keyPressEvent{
                    .key = FromQtKeyboardKey(static_cast<Qt::Key>(keyEvent->key())),
                    .isRepeat = keyEvent->isAutoRepeat(),
            };
            fireKeyboardKeyPressEvent(keyPressEvent);
        } else if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            KeyboardKeyReleaseEvent keyReleaseEvent{
                    .key = FromQtKeyboardKey(static_cast<Qt::Key>(keyEvent->key()))
            };
            fireKeyboardKeyReleaseEvent(keyReleaseEvent);
        } else if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            MouseWheelEvent mouseWheelEvent{
                    .delta = wheelEvent->pixelDelta().y(),
            };
            fireMouseWheelEvent(mouseWheelEvent);
        }
        return false;
    }


}