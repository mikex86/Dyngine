#pragma once

#include "Dyngine/Input/InputProvider.hpp"
#include <QCoreApplication>
#include <QApplication>
#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMainWindow>

namespace Editor {

    class QtInputProvider : public Dyngine::InputProvider, public QObject {

    private:
        QWidget *captureInputFrom;

        bool shouldGrabCursor = false;
        bool cursorGrabbed = false;
        QPoint cursorGrabVirtualPos = QPoint(0, 0);
        int cursorMovementsSinceWindowHasGainedFocus = 0;
    public:
        QtInputProvider(QWidget *captureInputFrom);

    protected:
    public:
        bool eventFilter(QObject *watched, QEvent *event) override;

        void setCursorGrabbed(bool cursorGrabbed) override;


    protected:
        void fireMouseMoveEvent(const Dyngine::MouseMoveEvent &event) override;
    };

}