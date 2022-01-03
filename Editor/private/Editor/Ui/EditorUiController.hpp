#pragma once

#include "ui_editorwindow.h"
#include "Editor/Project/ProjectState.hpp"
#include "ViewportController.hpp"

#include <Dyngine/Dyngine.hpp>
#include <QDirModel>
#include <QFileSystemWatcher>

namespace Editor {

    class EditorUiController : public QMainWindow {

    private:
        Ui::EditorWindow editorUi;

        QDirModel *explorerDirModel = nullptr;
        QFileSystemWatcher *fileWatcher = nullptr;

        std::unique_ptr<ProjectState> projectState{};

        std::unique_ptr<ViewportController> viewportController;

        std::shared_ptr<Dyngine::EngineInstance> engine;
    public:
        EditorUiController();

        void exec();

    private:
        // Listeners
        void onProjectOpenAction();
        void onFileChanged(const QString& path);
        void onDirectoryChanged(const QString& path);
        void onSelectedFileChanged(const QModelIndex &current, const QModelIndex &previous);
        void onFileClicked(const QModelIndex &current);

    private:
        void openProject(std::string dyProjectFilePath);

        void setCurrentlyViewedDirectory(QString directory);
    };

}