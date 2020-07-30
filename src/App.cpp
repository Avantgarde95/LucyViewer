#include <QFontDatabase>
#include <QDir>
#include <QtConcurrent>

#include <iostream>
#include <stdexcept>

#include "App.hpp"

App::App(int argc, char **argv) : QApplication(argc, argv) {
    auto fontId = QFontDatabase::addApplicationFont("res/font/Roboto-Regular.ttf");
    auto fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    auto font = QFont(fontFamily, 10);

    setFont(font);

    m_sceneModel = new SceneModel();
    m_fpsModel = new FPSModel(60.0f);

    auto objectBasePath = QString(argv[1]);
    auto objectPaths = QDir(objectBasePath).entryList(QStringList() << "*.ply", QDir::Files);

    m_statusView = new StatusView();
    m_objectsView = new ObjectsView(objectPaths);
    m_pixelsView = new PixelsView();

    //m_pixelsView->setFixedSize(600, 600);

    m_windowView = new WindowView(
            m_pixelsView,
            m_statusView,
            m_objectsView,
            nullptr
    );

    connect(m_fpsModel, &FPSModel::updated, [=](float fps) {
        if (m_allowRender) {
            m_sceneModel->setSize(m_pixelsView->getTextureSize());
            m_sceneModel->render();

            m_pixelsView->setPixels(m_sceneModel->getPixels().data());

            m_statusView->updateFrameLabel();
            m_statusView->updateFPSLabel(fps);
            m_statusView->updateRPSLabel(m_sceneModel->getRPS());
        }

        m_statusView->updateSizeLabel(m_sceneModel->getSize());
        m_pixelsView->update();
    });

    connect(m_objectsView, &ObjectsView::requested, [=](const QString &path) {
        QtConcurrent::run([=]() {
            try {
                m_objectsView->disableButtons();

                m_allowRender = false;
                auto objectPath = QString("%1/%2").arg(objectBasePath).arg(path).toStdString();
                m_sceneModel->setMainObject(objectPath);
                m_allowRender = true;

                m_objectsView->enableButtons();
            } catch (std::exception &error) {
                std::cout << error.what() << "\n";
                //std::cin.get();
            }
        });
    });
}

bool App::notify(QObject *receiver, QEvent *event) {
    bool done = true;

    try {
        done = QApplication::notify(receiver, event);
    } catch (const std::exception &error) {
        std::cout << "Error: " << error.what();
        std::cin.get();
    }

    return done;
}
