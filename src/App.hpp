#pragma once

#include <QApplication>

#include "model/SceneModel.hpp"
#include "model/FPSModel.hpp"

#include "view/StatusView.hpp"
#include "view/ObjectsView.hpp"
#include "view/PixelsView.hpp"
#include "view/WindowView.hpp"

class App : public QApplication {
public:
    App(int argc, char *argv[]);

    bool notify(QObject *receiver, QEvent *event) override;

private:
    SceneModel *m_sceneModel;
    FPSModel *m_fpsModel;

    StatusView *m_statusView;
    ObjectsView *m_objectsView;
    PixelsView *m_pixelsView;
    WindowView *m_windowView;

    bool m_allowRender = true;
};
