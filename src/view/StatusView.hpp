#pragma once

#include <QLabel>

#include <glm/glm.hpp>

class StatusView : public QWidget {
Q_OBJECT

public:
    explicit StatusView(QWidget *parent = nullptr);

    void updateSizeLabel(const glm::ivec2 &size);
    void updateFrameLabel();
    void updateFPSLabel(float fps);
    void updateRPSLabel(float rps);

private:
    QLabel *m_sizeLabel = new QLabel();
    QLabel *m_frameLabel = new QLabel();
    QLabel *m_fpsLabel = new QLabel();
    QLabel *m_rpsLabel = new QLabel();
};
