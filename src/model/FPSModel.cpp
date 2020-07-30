#include <QTimer>

#include <chrono>

#include "FPSModel.hpp"

typedef std::chrono::time_point<std::chrono::steady_clock> Time;

static Time getTime() {
    return std::chrono::high_resolution_clock::now();
}

static float computeDurationInSeconds(const Time &startTime, const Time &endTime) {
    return std::chrono::duration_cast<std::chrono::duration<float>>(endTime - startTime).count();
}

static float sumFloats(const QVector<float> &values) {
    float result = 0.0f;

    for (auto &v : values) {
        result += v;
    }

    return result;
}

FPSModel::FPSModel(float fps, QObject *parent)
        : QObject(parent), m_targetFPS(fps) {
    QTimer::singleShot(0, this, &FPSModel::runFrame);
}

void FPSModel::runFrame() {
    static auto lastTime = getTime();
    auto startTime = getTime();

    if (computeDurationInSeconds(lastTime, startTime) >= 1.0f / m_targetFPS) {
        emit updated(m_averageFPS);

        // Moving average.
        static auto records = QVector<float>(10, 0.0f);
        static auto index = 0;
        auto endTime = getTime();

        if (endTime > lastTime) {
            records[index] = 1.0f / computeDurationInSeconds(lastTime, endTime);
            index = (index + 1) % records.size();
            m_averageFPS = sumFloats(records) / static_cast<float>(records.size());
        }

        lastTime = startTime;
    }

    QTimer::singleShot(0, this, &FPSModel::runFrame);
}

