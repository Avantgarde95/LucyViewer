#include <QBoxLayout>

#include "StatusView.hpp"

StatusView::StatusView(QWidget *parent) : QWidget(parent) {
    updateSizeLabel({0, 0});
    updateFrameLabel();
    updateFPSLabel(0);
    updateRPSLabel(0);

    auto layout = new QVBoxLayout();

    layout->setAlignment(Qt::AlignTop);
    //layout->addWidget(m_sizeLabel);
    //layout->addWidget(m_frameLabel);
    layout->addWidget(m_fpsLabel);
    layout->addWidget(m_rpsLabel);

    setLayout(layout);
}

void StatusView::updateSizeLabel(const glm::ivec2 &size) {
    m_sizeLabel->setText(QString("Size: %1 X %2").arg(size.x, 4).arg(size.y, 4));
}

void StatusView::updateFrameLabel() {
    static int index = 0;
    QString symbols = "--\\\\||//";

    m_frameLabel->setText(QString("Frame: %1").arg(symbols[index]));
    index = (index + 1) % symbols.size();
}

void StatusView::updateFPSLabel(float fps) {
    m_fpsLabel->setText(QString("FPS: %1").arg(fps, 7, 'f', 3, '0'));
}

void StatusView::updateRPSLabel(float rps) {
    m_rpsLabel->setText(QString("%1 Mrays/s").arg(rps / 1000000.0f, 7, 'f', 3, '0'));
}
