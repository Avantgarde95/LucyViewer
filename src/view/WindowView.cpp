#include "WindowView.hpp"

WindowView::WindowView(
        QWidget *centralWidget,
        QWidget *leftWidget,
        QWidget *rightWidget,
        QWidget *bottomWidget,
        QWidget *parent
) : QMainWindow(parent) {
    setWindowTitle("Ray tracing-based 3D model renderer");
    setCentralWidget(centralWidget);
    addDock(Qt::LeftDockWidgetArea, leftWidget);
    addDock(Qt::RightDockWidgetArea, rightWidget);
    addDock(Qt::BottomDockWidgetArea, bottomWidget);

    show();
}

void WindowView::addDock(Qt::DockWidgetArea area, QWidget *widget) {
    if (widget == nullptr) {
        return;
    }

    auto dock = new QDockWidget();

    dock->setTitleBarWidget(new QWidget());
    dock->setWidget(widget);

    addDockWidget(area, dock);
}
