#pragma once

#include <QObject>

class FPSModel : public QObject {
Q_OBJECT

public:
    explicit FPSModel(float fps, QObject *parent = nullptr);

signals:
    void updated(float fps);

private:
    void runFrame();

    float m_targetFPS = 60.0f;
    float m_averageFPS = 0.0f;
};
