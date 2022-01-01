#pragma once

#include "Dyngine/Input/InputProvider.hpp"
#include <QCoreApplication>
#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

namespace Editor {

    class QtInputProvider : public Dyngine::InputProvider, public QObject {

    public:
        QtInputProvider();

    protected:
    public:
        bool eventFilter(QObject *watched, QEvent *event) override;

    };

}