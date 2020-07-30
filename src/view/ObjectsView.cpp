#include <QBoxLayout>

#include "ObjectsView.hpp"

ObjectsView::ObjectsView(const QStringList &objectPaths, QWidget *parent)
        : QWidget(parent) {
    for (auto &path: objectPaths) {
        auto button = new QPushButton(path);

        connect(button, &QPushButton::released, [this, button]() {
            QString path_ = button->text();
            emit requested(path_);
        });

        m_modelButtons.push_back(button);
    }

    auto layout = new QVBoxLayout();

    layout->setAlignment(Qt::AlignTop);

    for (auto button: m_modelButtons) {
        layout->addWidget(button);
    }

    layout->addWidget(m_statusLabel);

    setLayout(layout);
}

void ObjectsView::enableButtons() {
    for (auto button: m_modelButtons) {
        button->setDisabled(false);
    }

    m_statusLabel->setText("");
}

void ObjectsView::disableButtons() {
    for (auto button: m_modelButtons) {
        button->setDisabled(true);
    }

    m_statusLabel->setText("Loading...");
}
