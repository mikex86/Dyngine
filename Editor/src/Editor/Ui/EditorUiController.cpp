#include "Editor/Ui/EditorUiController.hpp"
#include "Editor/Input/QtInputProvider.hpp"

#include <Dyngine/EngineRenderContext.hpp>
#include <Dyngine/EngineRenderTarget.hpp>

#include <QFileDialog>
#include <QDirIterator>

Editor::EditorUiController::EditorUiController() : editorUi(Ui::EditorWindow()) {
    editorUi.setupUi(this);

    // Create file model for file browsers
    explorerDirModel = new QDirModel();
    explorerDirModel->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);
    explorerDirModel->setResolveSymlinks(false);

    editorUi.treeFileExplorer->setModel(nullptr);
    editorUi.listFileExplorer->setModel(nullptr);

    // Open Project Action Listener
    QObject::connect(editorUi.actionOpen, &QAction::triggered, this, &EditorUiController::onProjectOpenAction);


    auto renderContext = std::make_unique<Dyngine::EngineRenderContext>();
    auto frameBufferRenderTarget = std::make_shared<Dyngine::FrameBufferRenderTarget>(renderContext);
    auto qtInputProvider = std::make_shared<Editor::QtInputProvider>(editorUi.frameBuffer);

    engine = std::make_shared<Dyngine::EngineInstance>(frameBufferRenderTarget, qtInputProvider);
    viewportController = std::make_unique<ViewportController>(engine, frameBufferRenderTarget, editorUi.frameBuffer);
}

void Editor::EditorUiController::exec() {
    engine->startEngine();
    setWindowState(Qt::WindowMaximized);
    show();

    while (isVisible()) {
        QApplication::processEvents();
        viewportController->update();
    }
}

void Editor::EditorUiController::onProjectOpenAction() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("Project Files (*.dyproject)"));
    if (fileName.isEmpty()) {
        return;
    }
    openProject(fileName.toStdString());
}


void Editor::EditorUiController::openProject(std::string dyProjectFilePathStr) {
    // get projectDirectory
    std::filesystem::path projectDirectory;
    {
        std::filesystem::path dyProjectFilePath(dyProjectFilePathStr);
        projectDirectory = dyProjectFilePath.parent_path();
    }
    projectState = std::make_unique<ProjectState>(projectDirectory);

    std::u16string projectFilePathU16Str = projectDirectory.u16string();
    QString projectFilePathQStr = QString::fromStdU16String(projectFilePathU16Str);

    // Setup file model for file browsers
    editorUi.treeFileExplorer->setModel(explorerDirModel);
    editorUi.treeFileExplorer->setColumnHidden(2, true); // hide file type
    editorUi.treeFileExplorer->setColumnHidden(3, true); // hide date modified
    editorUi.listFileExplorer->setModel(explorerDirModel);

    // Selected file changed listener
    QObject::connect(editorUi.treeFileExplorer->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                     &EditorUiController::onSelectedFileChanged);
    // File double clicked listener
    QObject::connect(editorUi.listFileExplorer, &QListView::doubleClicked, this, &EditorUiController::onFileClicked);

    // update tree project explorer
    {
        // Set root path to project directory
        {
            QModelIndex rootIndex = explorerDirModel->index(projectFilePathQStr);
            editorUi.treeFileExplorer->setRootIndex(rootIndex);
        }

        // Update project directory file watcher
        if (fileWatcher != nullptr) {
            delete fileWatcher;
        }
        fileWatcher = new QFileSystemWatcher();
        // Add project directory and all sub folders to watch list
        {
            fileWatcher->addPath(projectFilePathQStr);

            QDirIterator directoryIterator(projectFilePathQStr, QStringList(), QDir::Dirs,
                                           QDirIterator::Subdirectories);
            while (directoryIterator.hasNext()) {
                fileWatcher->addPath(directoryIterator.next());
            }
        }
        QObject::connect(fileWatcher, &QFileSystemWatcher::fileChanged, this,
                         &EditorUiController::onFileChanged);
        QObject::connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this,
                         &EditorUiController::onDirectoryChanged);
    }

    // Update content browser
    {
        setCurrentlyViewedDirectory(projectFilePathQStr);
    }
}

void Editor::EditorUiController::onFileChanged(const QString &path) {
    if (explorerDirModel != nullptr) {
        explorerDirModel->refresh(explorerDirModel->index(path));
    }
}

void Editor::EditorUiController::onDirectoryChanged(const QString &path) {
    if (fileWatcher != nullptr) {
        // Remove path and sub-paths from fileWatcher
        // This is to ensure that the file watcher is not watching the same file twice, as there is no way
        // to check if we are already watching a specific folder
        {
            QDirIterator directoryIterator(path, QStringList(), QDir::Dirs,
                                           QDirIterator::Subdirectories);
            fileWatcher->removePath(path);
            while (directoryIterator.hasNext()) {
                fileWatcher->removePath(directoryIterator.next());
            }
        }

        std::filesystem::path directoryPath(path.toStdWString());

        // Add all sub folders to watch list, if the directory exists
        if (exists(directoryPath)) {
            QDirIterator directoryIterator(path, QStringList(), QDir::Dirs,
                                           QDirIterator::Subdirectories);
            fileWatcher->addPath(path);
            while (directoryIterator.hasNext()) {
                fileWatcher->addPath(directoryIterator.next());
            }
        }
    }
    if (explorerDirModel != nullptr) {
        explorerDirModel->refresh(explorerDirModel->index(path));
    }
}

void Editor::EditorUiController::setCurrentlyViewedDirectory(QString directory) {
    editorUi.listFileExplorer->setRootIndex(explorerDirModel->index(directory));
}

void Editor::EditorUiController::onSelectedFileChanged(const QModelIndex &current, const QModelIndex &previous) {
    auto filePathQString = explorerDirModel->filePath(current);
    std::filesystem::path filePath(filePathQString.toStdWString());
    if (is_directory(filePath)) {
        setCurrentlyViewedDirectory(filePathQString);
    }
}

void Editor::EditorUiController::onFileClicked(const QModelIndex &current) {
    auto filePathQString = explorerDirModel->filePath(current);
    std::filesystem::path filePath(filePathQString.toStdWString());
    if (is_directory(filePath)) {
        setCurrentlyViewedDirectory(filePathQString);
    }
}
