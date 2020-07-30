#pragma once

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QVector>

#include <glm/glm.hpp>

class PixelsView : public QOpenGLWidget {
Q_OBJECT

public:
    explicit PixelsView(QWidget *parent = nullptr);
    ~PixelsView() override;

    const glm::ivec2 &getViewSize() const;
    const glm::ivec2 &getTextureSize() const;

    void setPixels(const void *pixels);

private:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    QSize sizeHint() const override;

    void resetTexture();

    QOpenGLShaderProgram *m_program = new QOpenGLShaderProgram();
    QOpenGLVertexArrayObject *m_vao = new QOpenGLVertexArrayObject();
    QOpenGLBuffer *m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    QOpenGLBuffer *m_ibo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    QOpenGLTexture *m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);

    glm::ivec2 m_viewSize = {1, 1};
    glm::ivec2 m_textureSize = {1, 1};
};
