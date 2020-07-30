#pragma once

#include <QMainWindow>
#include <QDockWidget>

class WindowView : public QMainWindow {
Q_OBJECT

public:
    explicit WindowView(
            QWidget *centralWidget,
            QWidget *leftWidget,
            QWidget *rightWidget,
            QWidget *bottomWidget,
            QWidget *parent = nullptr
    );

private:
    void addDock(Qt::DockWidgetArea area, QWidget *widget);
};
