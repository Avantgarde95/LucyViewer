#pragma once

#include <QVector>
#include <QStringList>
#include <QPushButton>
#include <QLabel>

class ObjectsView : public QWidget {
Q_OBJECT

public:
    explicit ObjectsView(const QStringList &objectPaths, QWidget *parent = nullptr);

    void enableButtons();
    void disableButtons();

signals:
    void requested(QString &path);

private:
    QVector<QPushButton *> m_modelButtons;
    QLabel *m_statusLabel = new QLabel("");
};
