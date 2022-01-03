#include <QApplication>

#include <Dyngine/Dyngine.hpp>

#include "Editor/Ui/EditorUiController.hpp"
#include "Editor/Input/QtInputProvider.hpp"

int main(int argc, char **argv) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);

    // TODO: use Qt's resource system for icons
    Editor::EditorUiController controller;
    controller.exec();

    return 0;
}